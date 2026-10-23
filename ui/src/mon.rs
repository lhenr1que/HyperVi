// Monitor lookup for the menu and the zone editor: Hyprland reports global layout coordinates, a layer-shell surface
// is placed per output, so the pointer is converted to the monitor it is on.
use gtk4::prelude::*;
use serde_json::Value;
use std::process::Command;

#[derive(Clone, Debug, PartialEq)]
pub struct Mon { pub name: String, pub x: i32, pub y: i32, pub w: i32, pub h: i32, pub reserved: [i32; 4] } // reserved: left, top, right, bottom

fn hyprctl(args: &[&str]) -> String {
    Command::new("hyprctl").args(args).output().map(|o| String::from_utf8_lossy(&o.stdout).to_string()).unwrap_or_default()
}

/// Logical size (scale and rotation applied) of every monitor from `hyprctl monitors -j`.
pub fn parse(json: &str) -> Vec<Mon> {
    let v: Value = serde_json::from_str(json).unwrap_or_default();
    v.as_array().map(|a| a.iter().map(|m| {
        let sc = m["scale"].as_f64().unwrap_or(1.0).max(0.1);
        let (mut w, mut h) = (m["width"].as_f64().unwrap_or(1920.0) / sc, m["height"].as_f64().unwrap_or(1080.0) / sc);
        if m["transform"].as_i64().unwrap_or(0) % 2 == 1 { std::mem::swap(&mut w, &mut h); }
        let r = |i: usize| m["reserved"][i].as_i64().unwrap_or(0) as i32;
        Mon { name: m["name"].as_str().unwrap_or("").into(), x: m["x"].as_i64().unwrap_or(0) as i32, y: m["y"].as_i64().unwrap_or(0) as i32, w: w as i32, h: h as i32, reserved: [r(0), r(1), r(2), r(3)] }
    }).collect()).unwrap_or_default()
}

/// The monitor containing (x, y); the first one when the point is in none (or the list is empty: a 1920x1080 stand-in).
pub fn at(mons: &[Mon], x: i32, y: i32) -> Mon {
    mons.iter().find(|m| x >= m.x && x < m.x + m.w && y >= m.y && y < m.y + m.h).or(mons.first()).cloned()
        .unwrap_or(Mon { name: String::new(), x: 0, y: 0, w: 1920, h: 1080, reserved: [0; 4] })
}

pub fn all() -> Vec<Mon> { parse(&hyprctl(&["monitors", "-j"])) }

pub fn cursor() -> (i32, i32) {
    let text = hyprctl(&["cursorpos"]);
    let mut it = text.trim().split(',').map(|v| v.trim().parse::<i32>().unwrap_or(0));
    (it.next().unwrap_or(0), it.next().unwrap_or(0))
}

/// The GDK monitor with this connector name, for `LayerShell::set_monitor`.
pub fn gdk_monitor(name: &str) -> Option<gtk4::gdk::Monitor> {
    let list = gtk4::gdk::Display::default()?.monitors();
    (0..list.n_items()).filter_map(|i| list.item(i).and_downcast::<gtk4::gdk::Monitor>()).find(|m| m.connector().as_deref() == Some(name))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn picks_monitor_by_point() {
        let m = parse(r#"[{"name":"A","x":0,"y":0,"width":1920,"height":1080,"scale":1,"reserved":[60,10,10,10]},
                          {"name":"B","x":1920,"y":0,"width":3840,"height":2160,"scale":2,"transform":0,"reserved":[0,0,0,0]}]"#);
        assert_eq!(at(&m, 100, 100).name, "A");
        assert_eq!((at(&m, 2000, 10).name.as_str(), at(&m, 2000, 10).w), ("B", 1920));
        assert_eq!(at(&m, 9999, 9999).name, "A", "outside every monitor: the first");
        assert_eq!(at(&[], 5, 5).w, 1920);
        assert_eq!(m[0].reserved, [60, 10, 10, 10]);
    }
}
