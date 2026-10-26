// Snap Assist: after a window fills a layout zone the core writes $XDG_RUNTIME_DIR/kwinish/assist.json (the next empty zone and the loose
// windows) and starts this one-shot overlay. It covers that zone with a card per window; a choice goes back to the core as
// window_action(addr, "zone:N"), which offers the next empty zone. Esc, a click outside or 20 s end it.
use gtk4::prelude::*;
use gtk4::{glib, Application, ApplicationWindow, Box as GtkBox, Button, CssProvider, Fixed, GestureClick, Image, Label, Orientation};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};

const CSS: &str = "
window.kwa { background: alpha(#000000, 0.25); }
.zone { background: alpha(#1c1c22, 0.94); border-radius: 12px; padding: 10px; border: 2px solid #7aa2f7; }
.zone button.card { background: transparent; color: #e8e8ee; border: none; border-radius: 8px; padding: 8px 12px; box-shadow: none; }
.zone button.card:hover, .zone button.card:focus { background: alpha(#7aa2f7, 0.35); }
.zone .ttl { font-size: 14px; font-weight: 600; }
.zone .cls { font-size: 11px; opacity: 0.7; }
.zone .hint { font-size: 12px; opacity: 0.7; margin-bottom: 6px; }
";

#[derive(Debug, PartialEq)]
pub struct Assist { pub monitor: String, pub zone: u32, pub rect: [i32; 4], pub items: Vec<(String, String, String)> } // items: address, class, title

pub fn parse(json: &str) -> Option<Assist> {
    let v: serde_json::Value = serde_json::from_str(json).ok()?;
    let r = v["rect"].as_array()?;
    let n = |i: usize| r.get(i).and_then(|x| x.as_i64()).map(|x| x as i32);
    let s = |it: &serde_json::Value, k: &str| it[k].as_str().unwrap_or("").to_string();
    let items: Vec<_> = v["items"].as_array()?.iter().map(|it| (s(it, "address"), s(it, "class"), s(it, "title"))).filter(|i| !i.0.is_empty()).collect();
    if items.is_empty() { return None; }
    Some(Assist { monitor: v["monitor"].as_str()?.into(), zone: v["zone"].as_u64()? as u32, rect: [n(0)?, n(1)?, n(2)?.max(1), n(3)?.max(1)], items })
}

fn state_path() -> std::path::PathBuf {
    std::path::PathBuf::from(std::env::var("XDG_RUNTIME_DIR").unwrap_or_else(|_| "/tmp".into())).join("kwinish").join("assist.json")
}

pub fn run() -> glib::ExitCode {
    let Some(a) = std::fs::read_to_string(state_path()).ok().and_then(|t| parse(&t)) else {
        eprintln!("kwinish-ui assist: no state to show");
        return glib::ExitCode::FAILURE;
    };
    let app = Application::builder().application_id("dev.kwinish.assist").flags(gtk4::gio::ApplicationFlags::NON_UNIQUE).build();
    app.connect_activate(move |app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().expect("no display"), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);
        let mons = crate::mon::all();
        let mon = mons.iter().find(|m| m.name == a.monitor).cloned().unwrap_or_else(|| crate::mon::at(&mons, a.rect[0], a.rect[1]));
        let panel = GtkBox::new(Orientation::Vertical, 2);
        panel.add_css_class("zone");
        panel.set_size_request(a.rect[2], a.rect[3]);
        let hint = Label::new(Some("Pick a window for this space (Esc to skip)"));
        hint.add_css_class("hint");
        hint.set_xalign(0.0);
        panel.append(&hint);
        let mut first: Option<Button> = None;
        for (addr, class, title) in &a.items {
            let row = GtkBox::new(Orientation::Horizontal, 10);
            let icon = Image::from_icon_name(&class.to_lowercase());
            icon.set_pixel_size(32);
            let text = GtkBox::new(Orientation::Vertical, 0);
            let t = Label::new(Some(title));
            t.add_css_class("ttl");
            t.set_xalign(0.0);
            t.set_ellipsize(gtk4::pango::EllipsizeMode::End);
            let c = Label::new(Some(class));
            c.add_css_class("cls");
            c.set_xalign(0.0);
            text.append(&t);
            text.append(&c);
            row.append(&icon);
            row.append(&text);
            let b = Button::builder().child(&row).build();
            b.add_css_class("card");
            let (app2, addr, zone) = (app.clone(), addr.clone(), a.zone);
            b.connect_clicked(move |_| {
                crate::menu::run_action(&addr, &format!("zone:{zone}"));
                app2.quit();
            });
            panel.append(&b);
            first.get_or_insert(b);
        }
        let fixed = Fixed::new();
        fixed.put(&panel, (a.rect[0] - mon.x) as f64, (a.rect[1] - mon.y) as f64);
        let win = ApplicationWindow::builder().application(app).child(&fixed).build();
        win.add_css_class("kwa");
        win.init_layer_shell();
        win.set_layer(Layer::Overlay);
        win.set_namespace(Some("kwinish-assist"));
        if let Some(m) = crate::mon::gdk_monitor(&mon.name) { win.set_monitor(Some(&m)); }
        win.set_keyboard_mode(KeyboardMode::Exclusive);
        for e in [Edge::Left, Edge::Right, Edge::Top, Edge::Bottom] { win.set_anchor(e, true); }
        win.set_exclusive_zone(-1);
        // a click outside the zone panel skips
        let click = GestureClick::new();
        let app2 = app.clone();
        click.connect_pressed(move |g, _, x, y| {
            let mut w = g.widget().and_then(|w| w.pick(x, y, gtk4::PickFlags::DEFAULT));
            while let Some(cur) = w {
                if cur.has_css_class("zone") { return; }
                w = cur.parent();
            }
            app2.quit();
        });
        fixed.add_controller(click);
        let keys = gtk4::EventControllerKey::new();
        let app2 = app.clone();
        keys.connect_key_pressed(move |_, key, _, _| {
            if key == gtk4::gdk::Key::Escape { app2.quit(); }
            glib::Propagation::Proceed
        });
        win.add_controller(keys);
        let app2 = app.clone();
        glib::timeout_add_seconds_local_once(20, move || app2.quit());
        win.present();
        if let Some(b) = first { b.grab_focus(); }
    });
    app.run_with_args::<&str>(&[])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_core_state() {
        let a = parse(r#"{"monitor":"DP-1","zone":2,"rect":[500,0,500,600],"items":[{"address":"0x1","class":"kitty","title":"a\"b"},{"address":"","class":"x","title":"y"}]}"#).unwrap();
        assert_eq!((a.monitor.as_str(), a.zone, a.rect), ("DP-1", 2, [500, 0, 500, 600]));
        assert_eq!(a.items, vec![("0x1".to_string(), "kitty".to_string(), "a\"b".to_string())]);
        assert!(parse(r#"{"monitor":"A","zone":1,"rect":[0,0,1,1],"items":[]}"#).is_none());
        assert!(parse("garbage").is_none());
    }
}
