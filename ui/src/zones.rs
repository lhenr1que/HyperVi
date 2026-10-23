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

pub fn parse(text: &str) -> Vec<Layout> {
    let v: Value = serde_json::from_str(text).unwrap_or_default();
    v.as_array().map(|a| a.iter().filter_map(|l| {
        let zones: Vec<Zone> = l["zones"].as_array()?.iter().filter_map(|z| {
            let n: Vec<f64> = z.as_array()?.iter().filter_map(Value::as_f64).collect();
            (n.len() == 4).then(|| [n[0], n[1], n[2], n[3]])
        }).collect();
        let name = l["name"].as_str()?.to_string();
        if zones.is_empty() { return None }
        Some(Layout { label: l["label"].as_str().unwrap_or(&name).to_string(), name, gap: l["gap"].as_i64(), zones })
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

pub fn load() -> Vec<Layout> { std::fs::read_to_string(file_path()).map(|t| parse(&t)).unwrap_or_default() }

/// Insert or replace (by name) and write the file.
pub fn save(l: Layout) -> std::io::Result<()> {
    let mut all = load();
    match all.iter_mut().find(|x| x.name == l.name) { Some(x) => *x = l, None => all.push(l) }
    let p = file_path();
    if let Some(d) = p.parent() { std::fs::create_dir_all(d)?; }
    std::fs::write(p, render(&all))
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
    fn floating_block_delete_just_removes() {
        let mut z = vec![[0., 0., 1., 1.]];
        add_floating(&mut z);
        delete(&mut z, 1);
        assert_eq!(z, vec![[0., 0., 1., 1.]]);
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
