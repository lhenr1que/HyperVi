// Window menu: one-shot process. A transparent full-screen layer catches outside clicks and Esc;
// each entry runs a kwinish core action through `hyprctl eval`. It exits after one choice or 20 s.
use gtk4::prelude::*;
use gtk4::{gio, glib, Application, ApplicationWindow, Box as GtkBox, Button, CssProvider, Fixed, GestureClick, Orientation};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};
use std::process::Command;

const CSS: &str = "
window.kwm { background: transparent; }
.menu { background: alpha(#1c1c22, 0.96); border-radius: 12px; padding: 6px; }
.menu button { background: transparent; color: #e8e8ee; border: none; border-radius: 8px; padding: 6px 14px; min-width: 210px; }
.menu button label { font-size: 13px; }
.menu button:hover { background: alpha(#7aa2f7, 0.35); }
.sep { min-height: 1px; background: alpha(#ffffff, 0.12); margin: 4px 8px; }
";

// (label, action understood by kwinish.window_action); an empty action is a separator
const ENTRIES: &[(&str, &str)] = &[
    ("Maximize", "tile:max"),
    ("Tile left", "tile:left"),
    ("Tile right", "tile:right"),
    ("Maximize horizontally", "axis:h"),
    ("Maximize vertically", "axis:v"),
    ("", ""),
    ("Keep above", "above"),
    ("Float / tile", "float"),
    ("Fullscreen", "fullscreen"),
    ("", ""),
    ("Move to next workspace", "workspace:r+1"),
    ("Move to previous workspace", "workspace:r-1"),
    ("", ""),
    ("Close", "close"),
];

fn cursor() -> (i32, i32) {
    let out = Command::new("hyprctl").arg("cursorpos").output().ok();
    let text = out.map(|o| String::from_utf8_lossy(&o.stdout).to_string()).unwrap_or_default();
    let mut it = text.trim().split(',').map(|v| v.trim().parse::<i32>().unwrap_or(0));
    (it.next().unwrap_or(0), it.next().unwrap_or(0))
}

fn run_action(addr: &str, action: &str) {
    // addr and action come from our own tables and Hyprland, but keep the Lua string safe anyway
    let clean = |s: &str| s.chars().filter(|c| c.is_ascii_alphanumeric() || ":+-_".contains(*c)).collect::<String>();
    let code = format!("require('kwinish').window_action('{}','{}')", clean(addr), clean(action));
    let _ = Command::new("hyprctl").args(["eval", &code]).status();
}

pub fn run(addr: String) -> glib::ExitCode {
    let app = Application::builder()
        .application_id("dev.kwinish.menu")
        .flags(gio::ApplicationFlags::NON_UNIQUE)
        .build();
    app.connect_activate(move |app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(
            &gtk4::gdk::Display::default().expect("no display"),
            &provider,
            gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
        );
        let menu = GtkBox::new(Orientation::Vertical, 0);
        menu.add_css_class("menu");
        for (label, action) in ENTRIES {
            if action.is_empty() {
                let sep = GtkBox::new(Orientation::Horizontal, 0);
                sep.add_css_class("sep");
                menu.append(&sep);
                continue;
            }
            let b = Button::with_label(label);
            if let Some(l) = b.child().and_downcast::<gtk4::Label>() {
                l.set_halign(gtk4::Align::Start);
            }
            let (app, addr, action) = (app.clone(), addr.clone(), action.to_string());
            b.connect_clicked(move |_| {
                run_action(&addr, &action);
                app.quit();
            });
            menu.append(&b);
        }
        let (cx, cy) = cursor();
        let fixed = Fixed::new();
        fixed.put(&menu, cx as f64, cy as f64);
        let win = ApplicationWindow::builder().application(app).child(&fixed).build();
        win.add_css_class("kwm");
        win.init_layer_shell();
        win.set_layer(Layer::Overlay);
        win.set_namespace(Some("kwinish-menu"));
        win.set_keyboard_mode(KeyboardMode::Exclusive);
        for e in [Edge::Left, Edge::Right, Edge::Top, Edge::Bottom] {
            win.set_anchor(e, true);
        }
        win.set_exclusive_zone(-1);
        let click = GestureClick::new();
        let a = app.clone();
        click.connect_pressed(move |g, _, x, y| {
            // a click that lands outside the menu closes it
            let inside = g.widget().and_then(|w| w.pick(x, y, gtk4::PickFlags::DEFAULT)).map(|p| p.css_classes().iter().any(|c| c == "menu") || p.ancestor(gtk4::Button::static_type()).is_some()).unwrap_or(false);
            if !inside {
                a.quit();
            }
        });
        fixed.add_controller(click);
        let keys = gtk4::EventControllerKey::new();
        let a = app.clone();
        keys.connect_key_pressed(move |_, key, _, _| {
            if key == gtk4::gdk::Key::Escape {
                a.quit();
            }
            glib::Propagation::Proceed
        });
        win.add_controller(keys);
        let a = app.clone();
        glib::timeout_add_seconds_local_once(20, move || a.quit());
        win.present();
    });
    app.run_with_args::<&str>(&[])
}
