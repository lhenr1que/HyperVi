// Pure logic of the zone editor and the user-layout file (~/.config/kwinish/layouts.json, read by core/layouts.lua).
// A zone is [x, y, w, h] as fractions of the work area.
use serde_json::{json, Value};
use std::path::PathBuf;

pub type Zone = [f64; 4];
const EPS: f64 = 1e-6;

#[derive(Clone, Debug, PartialEq)]
pub struct Layout { pub name: String, pub label: String, pub gap: Option<i64>, pub zones: Vec<Zone> }

/// Split zone `i` in two halves, side by side (`vertical_cut`) or one above the other; the new half goes right after it.
pub fn split(zones: &mut Vec<Zone>, i: usize, vertical_cut: bool) {
    let Some(z) = zones.get(i).copied() else { return };
    let (a, b) = if vertical_cut {
        ([z[0], z[1], z[2] / 2.0, z[3]], [z[0] + z[2] / 2.0, z[1], z[2] / 2.0, z[3]])
    } else {
        ([z[0], z[1], z[2], z[3] / 2.0], [z[0], z[1] + z[3] / 2.0, z[2], z[3] / 2.0])
    };
    zones[i] = a;
    zones.insert(i + 1, b);
}

/// A centred block that overlaps whatever is under it.
pub fn add_floating(zones: &mut Vec<Zone>) { zones.push([0.3, 0.3, 0.4, 0.4]); }

/// Remove zone `i`; a neighbour sharing its whole edge grows to take the space. The last zone stays.
pub fn delete(zones: &mut Vec<Zone>, i: usize) {
    if zones.len() <= 1 || i >= zones.len() { return }
    let z = zones.remove(i);
    let eq = |a: f64, b: f64| (a - b).abs() < EPS;
    for n in zones.iter_mut() {
        if eq(n[1], z[1]) && eq(n[3], z[3]) && eq(n[0] + n[2], z[0]) { n[2] += z[2]; return }
        if eq(n[1], z[1]) && eq(n[3], z[3]) && eq(z[0] + z[2], n[0]) { n[0] = z[0]; n[2] += z[2]; return }
        if eq(n[0], z[0]) && eq(n[2], z[2]) && eq(n[1] + n[3], z[1]) { n[3] += z[3]; return }
        if eq(n[0], z[0]) && eq(n[2], z[2]) && eq(z[1] + z[3], n[1]) { n[1] = z[1]; n[3] += z[3]; return }
    }
}

/// An inner edge shared by zone borders: `vertical` edges sit at x = `pos` and span y `from..to`; horizontal ones the reverse.
#[derive(Clone, Debug, PartialEq)]
pub struct Edge { pub vertical: bool, pub pos: f64, pub from: f64, pub to: f64 }

const MIN: f64 = 0.05;

pub fn edges(zones: &[Zone]) -> Vec<Edge> {
    let mut out: Vec<Edge> = vec![];
    for vertical in [true, false] {
        let (p, l) = if vertical { (0, 2) } else { (1, 3) };
        let (q, m) = if vertical { (1, 3) } else { (0, 2) };
        for z in zones {
            for pos in [z[p], z[p] + z[l]] {
                if pos < EPS || pos > 1.0 - EPS { continue }
                let (a, b) = (z[q], z[q] + z[m]);
                match out.iter_mut().find(|e| e.vertical == vertical && (e.pos - pos).abs() < EPS) {
                    Some(e) => { e.from = e.from.min(a); e.to = e.to.max(b) }
                    None => out.push(Edge { vertical, pos, from: a, to: b }),
                }
            }
        }
    }
    out
}

/// Move every zone border lying on `e` to `to`; None when a zone would get thinner than 5 % of the area.
pub fn move_edge(zones: &[Zone], e: &Edge, to: f64) -> Option<Vec<Zone>> {
    let (p, l) = if e.vertical { (0, 2) } else { (1, 3) };
    let mut out = zones.to_vec();
    for z in out.iter_mut() {
        if (z[p] + z[l] - e.pos).abs() < EPS { z[l] = to - z[p] }
        else if (z[p] - e.pos).abs() < EPS { z[l] = z[p] + z[l] - to; z[p] = to }
        if z[l] < MIN || z[p] + z[l] > 1.0 + EPS { return None }
    }
    Some(out)
}

/// Strings in the file only hold what core/layouts.lua can read back: letters, digits, space and '-'.
pub fn clean_label(s: &str) -> String {
    let c: String = s.chars().filter(|c| c.is_ascii_alphanumeric() || *c == ' ' || *c == '-').collect();
    c.trim().to_string()
}

pub fn slug(label: &str) -> String {
    let s: String = clean_label(label).to_lowercase().split_whitespace().collect::<Vec<_>>().join("-");
    format!("user-{}", if s.is_empty() { "layout" } else { &s })
}

pub fn file_path() -> PathBuf {
    let base = std::env::var("XDG_CONFIG_HOME").unwrap_or_else(|_| format!("{}/.config", std::env::var("HOME").unwrap_or_default()));
    PathBuf::from(base).join("kwinish").join("layouts.json")
}

/// Limits shared with the core's reader (core/kwjson.lua): bytes of JSON text in one file, levels of arrays and objects.
pub const JSON_LIMIT: usize = 1 << 20;
pub const JSON_DEPTH: usize = 32;

/// `text` as JSON data, or None when it is not JSON or is over the limits (the core refuses the same: core/testdata/json-cases.txt).
/// Levels are counted in the text, before parsing, as the core counts them: a repeated member must not hide the deeper one.
pub fn json(text: &str) -> Option<Value> {
    let (mut depth, mut deepest, mut in_string, mut escaped) = (0usize, 0usize, false, false);
    for b in text.bytes() {
        if in_string {
            if escaped { escaped = false } else if b == b'\\' { escaped = true } else if b == b'"' { in_string = false }
        } else {
            match b {
                b'"' => in_string = true,
                b'[' | b'{' => { depth += 1; deepest = deepest.max(depth) }
                b']' | b'}' => depth = depth.saturating_sub(1),
                _ => {}
            }
        }
    }
    if text.len() > JSON_LIMIT || deepest > JSON_DEPTH { return None }
    serde_json::from_str(text).ok()
}

/// The text of `path`, reading at most JSON_LIMIT + 1 bytes (enough for `json` to refuse a larger file); None when absent or not UTF-8.
pub fn read_text(path: &std::path::Path) -> Option<String> {
    use std::io::Read;
    let mut buf = vec![];
    std::fs::File::open(path).ok()?.take(JSON_LIMIT as u64 + 1).read_to_end(&mut buf).ok()?;
    String::from_utf8(buf).ok()
}

/// The JSON data in `path`: every JSON file kwinish-ui reads goes through here.
pub fn read_json(path: &std::path::Path) -> Option<Value> { read_text(path).as_deref().and_then(json) }

/// Entries the core (core/layouts.lua) would reject are dropped here too, so the picker never offers what cannot be used.
pub fn parse(text: &str) -> Vec<Layout> {
    let v = json(text).unwrap_or_default();
    v.as_array().map(|a| a.iter().filter_map(|l| {
        let name = l["name"].as_str()?.to_string();
        if name.is_empty() || name.len() > 64 || !name.chars().all(|c| c.is_ascii_alphanumeric() || c == '-') { return None }
        let raw = l["zones"].as_array()?;
        let mut zones: Vec<Zone> = vec![];
        for z in raw {
            let n: Vec<f64> = z.as_array()?.iter().map(Value::as_f64).collect::<Option<Vec<f64>>>()?;
            if n.len() != 4 || n[0] < 0.0 || n[1] < 0.0 || n[2] <= 0.0 || n[3] <= 0.0 || n[0] + n[2] > 1.0 + 1e-6 || n[1] + n[3] > 1.0 + 1e-6 { return None }
            zones.push([n[0], n[1], n[2], n[3]]);
        }
        if zones.is_empty() { return None }
        Some(Layout { label: l["label"].as_str().unwrap_or(&name).to_string(), name, gap: l["gap"].as_i64().map(|g| g.clamp(0, 60)), zones })
    }).collect()).unwrap_or_default()
}

pub fn render(layouts: &[Layout]) -> String {
    let r = |x: f64| (x * 1e6).round() / 1e6;
    let list: Vec<Value> = layouts.iter().map(|l| {
        let mut o = json!({ "name": l.name, "label": l.label, "zones": l.zones.iter().map(|z| [r(z[0]), r(z[1]), r(z[2]), r(z[3])]).collect::<Vec<_>>() });
        if let Some(g) = l.gap { o["gap"] = json!(g) }
        o
    }).collect();
    serde_json::to_string_pretty(&list).unwrap_or_else(|_| "[]".into())
}

pub fn load() -> Vec<Layout> { read_text(&file_path()).map(|t| parse(&t)).unwrap_or_default() }

/// Write `text` to `path` through a temporary file and a rename, so a crash or a second writer never leaves a half-written file. A file
/// that exists but does not parse (`parses`, the reader's own test) is first copied byte for byte to `<name>.bak`, since saving over it
/// would silently drop whatever it held. Nothing is written when that file cannot be read or copied; on any error the old file is left
/// as it was and the temporary file is removed.
pub fn write_atomic(path: &std::path::Path, text: &str, parses: impl Fn(&str) -> bool) -> std::io::Result<()> {
    if let Some(d) = path.parent() { std::fs::create_dir_all(d)?; }
    match std::fs::read(path) {
        Ok(old) => {
            let blank = String::from_utf8_lossy(&old).trim().is_empty();
            if !blank && !std::str::from_utf8(&old).is_ok_and(&parses) { publish(&path.with_extension("json.bak"), &old, |a, b| std::fs::rename(a, b))?; }
        }
        Err(e) if e.kind() == std::io::ErrorKind::NotFound => {}
        Err(e) => return Err(e),
    }
    publish(path, text.as_bytes(), |a, b| std::fs::rename(a, b))
}

/// `bytes` to `path` through `<path>.tmp<pid>` and `rename` (a parameter so tests can make it fail); the temporary file goes on error.
fn publish(path: &std::path::Path, bytes: &[u8], rename: impl Fn(&std::path::Path, &std::path::Path) -> std::io::Result<()>) -> std::io::Result<()> {
    let mut tmp = path.as_os_str().to_owned();
    tmp.push(format!(".tmp{}", std::process::id()));
    let tmp = PathBuf::from(tmp);
    let done = std::fs::write(&tmp, bytes).and_then(|_| rename(&tmp, path));
    if done.is_err() { let _ = std::fs::remove_file(&tmp); }
    done
}

/// Insert or replace (by name) and write the file.
pub fn save(l: Layout) -> std::io::Result<()> {
    let mut all = load();
    match all.iter_mut().find(|x| x.name == l.name) { Some(x) => *x = l, None => all.push(l) }
    write_atomic(&file_path(), &render(&all), |t| json(t).is_some_and(|v| v.is_array())) // what load refuses goes to .bak first
}

/// The presets of core/layouts.lua, to load as a starting point (keep in step with it).
pub fn presets() -> Vec<Layout> {
    let l = |name: &str, label: &str, zones: &[Zone]| Layout { name: name.into(), label: label.into(), gap: None, zones: zones.to_vec() };
    let t = 1.0 / 3.0;
    vec![
        l("halves", "Two columns", &[[0., 0., 0.5, 1.], [0.5, 0., 0.5, 1.]]),
        l("thirds", "Three columns", &[[0., 0., t, 1.], [t, 0., t, 1.], [2. * t, 0., t, 1.]]),
        l("grid", "Grid 2x2", &[[0., 0., 0.5, 0.5], [0.5, 0., 0.5, 0.5], [0., 0.5, 0.5, 0.5], [0.5, 0.5, 0.5, 0.5]]),
        l("main-side", "Main and side", &[[0., 0., 0.6, 1.], [0.6, 0., 0.4, 0.5], [0.6, 0.5, 0.4, 0.5]]),
        l("wide-center", "Wide centre", &[[0., 0., 0.25, 1.], [0.25, 0., 0.5, 1.], [0.75, 0., 0.25, 1.]]),
    ]
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn split_and_delete_roundtrip() {
        let mut z = vec![[0., 0., 1., 1.]];
        split(&mut z, 0, true);
        split(&mut z, 1, false);
        assert_eq!(z, vec![[0., 0., 0.5, 1.], [0.5, 0., 0.5, 0.5], [0.5, 0.5, 0.5, 0.5]]);
        delete(&mut z, 2);
        assert_eq!(z[1], [0.5, 0., 0.5, 1.]);
        delete(&mut z, 0);
        assert_eq!(z, vec![[0., 0., 1., 1.]]);
        delete(&mut z, 0);
        assert_eq!(z.len(), 1, "last zone stays");
    }

    #[test]
    fn edges_and_moving() {
        let z = vec![[0., 0., 0.5, 1.], [0.5, 0., 0.5, 0.5], [0.5, 0.5, 0.5, 0.5]];
        let e = edges(&z);
        assert_eq!(e.len(), 2);
        assert_eq!(e[0], Edge { vertical: true, pos: 0.5, from: 0., to: 1. });
        assert_eq!((e[1].vertical, e[1].pos, e[1].from, e[1].to), (false, 0.5, 0.5, 1.));
        let m = move_edge(&z, &e[0], 0.7).unwrap();
        let want = [[0., 0., 0.7, 1.], [0.7, 0., 0.3, 0.5], [0.7, 0.5, 0.3, 0.5]];
        assert!(m.iter().zip(want).all(|(a, b)| a.iter().zip(b).all(|(x, y)| (x - y).abs() < 1e-9)));
        assert!(move_edge(&z, &e[0], 0.99).is_none() && move_edge(&z, &e[0], 0.01).is_none());
    }

    #[test]
    fn floating_block_delete_just_removes() {
        let mut z = vec![[0., 0., 1., 1.]];
        add_floating(&mut z);
        delete(&mut z, 1);
        assert_eq!(z, vec![[0., 0., 1., 1.]]);
    }

    #[test]
    fn parse_drops_what_the_core_would_reject() {
        let t = r#"[{"name":"ok","zones":[[0,0,0.5,1]],"gap":999},{"name":"bad name","zones":[[0,0,1,1]]},{"name":"neg","zones":[[-0.1,0,1,1]]},
                    {"name":"big","zones":[[0,0,2,1]]},{"name":"nan","zones":[["x",0,1,1]]},{"name":"short","zones":[[0,0,1]]},{"name":"none","zones":[]}]"#;
        let l = parse(t);
        assert_eq!(l.len(), 1);
        assert_eq!((l[0].name.as_str(), l[0].gap), ("ok", Some(60)));
    }

    #[test]
    fn parse_refuses_what_the_core_refuses() {
        // over 1 MiB or 32 levels the whole file is refused, as core/kwjson.lua does (list, entry and member are levels 1 to 3)
        let entry = |x: &str| format!(r#"[{{"name":"a","zones":[[0,0,1,1]],"x":{x}}}]"#);
        let nest = |n: usize| format!("{}{}", "[".repeat(n), "]".repeat(n));
        assert_eq!(parse(&entry(&nest(30))).len(), 1, "32 levels");
        assert!(parse(&entry(&nest(31))).is_empty(), "33 levels");
        assert!(parse(&entry(&format!("\"{}\"", "a".repeat(1 << 20)))).is_empty(), "over 1 MiB");
    }

    #[test]
    fn json_agrees_with_the_core() {
        // core/test_kwjson.lua checks the core's reader against the same file
        let mut n = 0;
        for line in include_str!("../../core/testdata/json-cases.txt").lines() {
            let (verdict, text) = line.split_once(' ').unwrap_or((line, ""));
            match verdict {
                "ok" => assert!(json(text).is_some(), "should decode: {text}"),
                "bad" => assert!(json(text).is_none(), "should be refused: {text}"),
                _ => continue,
            }
            n += 1;
        }
        assert!(n > 50, "cases file read");
        let nest = |n: usize| format!("{}{}", "[".repeat(n), "]".repeat(n));
        assert!(json(&nest(32)).is_some() && json(&nest(33)).is_none(), "32 levels pass, 33 do not");
        let big = format!("\"{}\"", "a".repeat(JSON_LIMIT - 2));
        assert!(json(&big).is_some() && json(&format!("{big} ")).is_none(), "1 MiB passes, one byte more does not");
    }

    #[test]
    fn read_stops_after_the_limit() {
        let d = std::env::temp_dir().join(format!("kwinish-read-{}", std::process::id()));
        std::fs::create_dir_all(&d).unwrap();
        let p = d.join("x.json");
        std::fs::write(&p, format!("\"{}\"", "a".repeat(2 * JSON_LIMIT))).unwrap();
        assert_eq!(read_text(&p).map(|t| t.len()), Some(JSON_LIMIT + 1), "reads LIMIT + 1 bytes, not the whole file");
        assert!(read_json(&p).is_none(), "a larger file is refused");
        std::fs::write(&p, b"[\"\xff\"]").unwrap();
        assert!(read_json(&p).is_none(), "not UTF-8");
        std::fs::write(&p, "[1]").unwrap();
        assert_eq!(read_json(&p), Some(json!([1])));
        let _ = std::fs::remove_dir_all(&d);
        assert!(read_json(&p).is_none(), "missing");
    }

    #[test]
    fn a_file_the_reader_refuses_is_backed_up_before_saving() {
        // valid JSON over 1 MiB: load() gives nothing, so saving over it without a copy would lose it (same check as save())
        let d = std::env::temp_dir().join(format!("kwinish-big-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&d);
        std::fs::create_dir_all(&d).unwrap();
        let p = d.join("layouts.json");
        let big = format!("[\"{}\"]", "a".repeat(JSON_LIMIT));
        std::fs::write(&p, &big).unwrap();
        write_atomic(&p, "[]", |t| json(t).is_some_and(|v| v.is_array())).unwrap();
        assert_eq!(std::fs::read_to_string(p.with_extension("json.bak")).unwrap(), big);
        std::fs::write(&p, r#"{"a":1}"#).unwrap();
        write_atomic(&p, "[]", |t| json(t).is_some_and(|v| v.is_array())).unwrap();
        assert_eq!(std::fs::read_to_string(p.with_extension("json.bak")).unwrap(), r#"{"a":1}"#, "wrong root too");
        let _ = std::fs::remove_dir_all(&d);
    }

    #[test]
    fn valid_files_as_written_are_read_back() {
        // core/testdata/formats: the files this crate writes, byte for byte, which core/test_userdata.lua reads too
        let layouts = vec![
            Layout { name: "user-my-layout".into(), label: "My layout".into(), gap: Some(8), zones: vec![[0., 0., 0.5, 1.], [0.5, 0., 0.5, 1.]] },
            Layout { name: "user-thirds".into(), label: "Thirds".into(), gap: None, zones: vec![[0., 0., 1. / 3., 1.]] },
        ];
        let file = include_str!("../../core/testdata/formats/layouts.json");
        assert_eq!(render(&layouts), file, "zones::render writes exactly the fixture");
        assert_eq!(parse(file).len(), 2);
        for name in ["settings.json", "shortcuts.json"] {
            let text = match name { "settings.json" => include_str!("../../core/testdata/formats/settings.json"), _ => include_str!("../../core/testdata/formats/shortcuts.json") };
            let v = json(text).unwrap_or_else(|| panic!("{name} refused"));
            assert!(v.is_object());
            assert_eq!(serde_json::to_string_pretty(&v).unwrap() + if text.ends_with('\n') { "\n" } else { "" }, text, "{name} is what to_string_pretty writes");
        }
        let rules = json(include_str!("../../core/testdata/formats/rules.json")).unwrap();
        assert!(rules["kitty"]["above"] == true && rules["com.example.App"]["workspace"] == 3);
    }

    #[test]
    fn layout_cases_agree_with_the_core() {
        // core/test_layouts.lua reads the same file
        let mut wrong = vec![];
        for line in include_str!("../../core/testdata/layout-cases.txt").lines().filter(|l| !l.starts_with('#')) {
            let Some((expect, text)) = line.split_once(' ') else { continue };
            let l = parse(text);
            let got = match l.as_slice() {
                [] => "drop".to_string(),
                [one] => format!("gap={}", one.gap.map_or("default".to_string(), |g| g.to_string())),
                _ => "many".to_string(),
            };
            if got != expect { wrong.push(format!("{expect} but got {got}: {text}")); }
        }
        assert!(wrong.is_empty(), "layout cases differ from the core:\n  {}", wrong.join("\n  "));
    }

    // A scratch directory for the write tests (never ~/.config); failures are made with real files, not permissions.
    fn scratch(tag: &str) -> PathBuf {
        let d = std::env::temp_dir().join(format!("kwinish-{tag}-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&d);
        std::fs::create_dir_all(&d).unwrap();
        d
    }
    fn leftovers(d: &std::path::Path) -> Vec<String> {
        std::fs::read_dir(d).unwrap().map(|e| e.unwrap().file_name().to_string_lossy().to_string()).filter(|n| n.contains(".tmp")).collect()
    }

    #[test]
    fn a_failed_backup_stops_the_save() {
        let d = scratch("nobak");
        let p = d.join("layouts.json");
        std::fs::write(&p, "{ not json").unwrap();
        std::fs::create_dir(p.with_extension("json.bak")).unwrap(); // the copy cannot be made
        assert!(write_atomic(&p, "[]", |t| json(t).is_some_and(|v| v.is_array())).is_err(), "the failure is reported");
        assert_eq!(std::fs::read_to_string(&p).unwrap(), "{ not json", "and the file is not replaced without its copy");
        assert!(leftovers(&d).is_empty(), "no temporary file left: {:?}", leftovers(&d));
        let _ = std::fs::remove_dir_all(&d);
    }

    #[test]
    fn a_file_that_is_not_text_is_kept_too() {
        let d = scratch("bytes");
        let p = d.join("settings.json");
        std::fs::write(&p, b"{\"a\":\"\xff\"}").unwrap();
        write_atomic(&p, "{}", |t| json(t).is_some_and(|v| v.is_object())).unwrap();
        assert_eq!(std::fs::read(p.with_extension("json.bak")).unwrap(), b"{\"a\":\"\xff\"}", "exact bytes kept");
        assert_eq!(std::fs::read_to_string(&p).unwrap(), "{}");
        let _ = std::fs::remove_dir_all(&d);
    }

    #[test]
    fn a_failed_rename_leaves_the_old_file() {
        let d = scratch("rename");
        let p = d.join("layouts.json");
        std::fs::write(&p, "[1]").unwrap();
        let e = publish(&p, b"[]", |_, _| Err(std::io::Error::other("simulated"))).unwrap_err();
        assert_eq!(e.to_string(), "simulated");
        assert_eq!(std::fs::read_to_string(&p).unwrap(), "[1]");
        assert!(leftovers(&d).is_empty(), "temporary file removed: {:?}", leftovers(&d));
        let _ = std::fs::remove_dir_all(&d);
    }

    #[test]
    fn an_unreadable_file_stops_the_save() {
        let d = scratch("unread");
        let p = d.join("shortcuts.json");
        std::fs::create_dir(&p).unwrap(); // there, but cannot be read as a file
        assert!(write_atomic(&p, "{}", |t| json(t).is_some_and(|v| v.is_object())).is_err());
        assert!(p.is_dir() && leftovers(&d).is_empty(), "nothing replaced, no temporary file left: {:?}", leftovers(&d));
        let _ = std::fs::remove_dir_all(&d);
    }

    #[test]
    fn atomic_write_keeps_a_backup_of_an_unreadable_file() {
        let d = std::env::temp_dir().join(format!("kwinish-test-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&d);
        let p = d.join("layouts.json");
        std::fs::create_dir_all(&d).unwrap();
        std::fs::write(&p, "{ not json").unwrap();
        write_atomic(&p, "[]", |t| serde_json::from_str::<Value>(t).is_ok()).unwrap();
        assert_eq!(std::fs::read_to_string(&p).unwrap(), "[]");
        assert_eq!(std::fs::read_to_string(p.with_extension("json.bak")).unwrap(), "{ not json");
        let _ = std::fs::remove_dir_all(&d);
    }

    #[test]
    fn file_roundtrip_and_slug() {
        assert_eq!(slug("  My  Lay-out! "), "user-my-lay-out");
        assert_eq!(slug("!!"), "user-layout");
        let l = Layout { name: "user-a".into(), label: "A".into(), gap: Some(6), zones: vec![[0., 0., 0.5, 1.], [0.5, 0., 0.5, 1.]] };
        assert_eq!(parse(&render(&[l.clone()])), vec![l]);
        assert!(parse("garbage").is_empty());
        assert_eq!(presets().len(), 5);
    }
}
