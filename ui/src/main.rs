// kwinish-ui: display-only Alt+Tab overlay. The Lua core owns keys and focus and writes
// $XDG_RUNTIME_DIR/kwinish/switcher.json; this process only draws it (no keyboard focus).
use gtk4::prelude::*;
use gtk4::{gio, glib, Application, ApplicationWindow, Box as GtkBox, CssProvider, Label, Orientation};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};
use std::path::PathBuf;

mod bars;
mod editor;
mod zones;
mod mode;
mod assist;
mod menu;
mod mon;
mod shortcut;
mod settings;

const CSS: &str = "
window.kw { background: transparent; }
.panel { background: alpha(#1c1c22, 0.92); border-radius: 18px; padding: 14px; }
.card { padding: 14px 18px; border-radius: 12px; min-width: 150px; color: #e8e8ee; }
.card.sel { background: alpha(#7aa2f7, 0.35); outline: 2px solid #7aa2f7; }
.cls { font-size: 11px; opacity: 0.7; }
.ttl { font-size: 14px; font-weight: 600; }
";

fn state_path() -> PathBuf {
    let dir = std::env::var("XDG_RUNTIME_DIR").unwrap_or_else(|_| "/tmp".into());
    PathBuf::from(dir).join("kwinish").join("switcher.json")
}

fn render(win: &ApplicationWindow, row: &GtkBox, path: &PathBuf) {
    let state = zones::read_json(path).unwrap_or(serde_json::Value::Null);
    let items = state["items"].as_array().cloned().unwrap_or_default();
    if state["open"] != true || items.is_empty() {
        win.set_visible(false);
        return;
    }
    while let Some(child) = row.first_child() {
        row.remove(&child);
    }
    let selected = state["selected"].as_u64().unwrap_or(0) as usize;
    for (i, it) in items.iter().enumerate() {
        let card = GtkBox::new(Orientation::Vertical, 4);
        card.add_css_class("card");
        if i == selected {
            card.add_css_class("sel");
        }
        let cls = Label::new(it["class"].as_str());
        cls.add_css_class("cls");
        cls.set_xalign(0.0);
        let ttl = Label::new(it["title"].as_str());
        ttl.add_css_class("ttl");
        ttl.set_xalign(0.0);
        ttl.set_max_width_chars(22);
        ttl.set_ellipsize(gtk4::pango::EllipsizeMode::End);
        card.append(&cls);
        card.append(&ttl);
        row.append(&card);
    }
    win.set_visible(true);
}

fn run_switcher() -> glib::ExitCode {
    let app = Application::builder().application_id("dev.kwinish.ui").build();
    app.connect_activate(|app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(
            &gtk4::gdk::Display::default().expect("no display"),
            &provider,
            gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
        );
        let row = GtkBox::new(Orientation::Horizontal, 8);
        row.add_css_class("panel");
        let win = ApplicationWindow::builder().application(app).child(&row).build();
        win.add_css_class("kw");
        win.init_layer_shell();
        win.set_layer(Layer::Overlay);
        win.set_keyboard_mode(KeyboardMode::None);
        win.set_namespace(Some("kwinish-switcher"));
        for e in [Edge::Left, Edge::Right, Edge::Top, Edge::Bottom] {
            win.set_anchor(e, false); // no anchors: centred on the output
        }
        let path = state_path();
        if let Some(dir) = path.parent() {
            let _ = std::fs::create_dir_all(dir);
        }
        // watch the directory: the Lua side replaces the file, which a file monitor can miss
        match gio::File::for_path(path.parent().unwrap()).monitor_directory(gio::FileMonitorFlags::NONE, gio::Cancellable::NONE) {
            Ok(monitor) => {
                let (w, r, p) = (win.clone(), row.clone(), path.clone());
                monitor.connect_changed(move |_, _, _, _| render(&w, &r, &p));
                std::mem::forget(monitor);
            }
            Err(e) => eprintln!("kwinish-ui: cannot watch {}: {e}", path.display()), // the overlay stays hidden instead of panicking
        }
        render(&win, &row, &path);
        std::mem::forget(app.hold());
    });
    app.run_with_args::<&str>(&[])
}

fn main() -> glib::ExitCode {
    let args: Vec<String> = std::env::args().collect();
    match args.get(1).map(String::as_str) {
        Some("bars") => bars::run(args[2..].to_vec()),
        Some("shortcut") => shortcut::run(args.get(2).cloned().unwrap_or_default()),
        Some("zones") => editor::run(args.get(2..).unwrap_or(&[]).to_vec()),
        Some("layouts") => menu::run_layouts(args.get(2..).unwrap_or(&[]).to_vec()),
        Some("assist") => assist::run(),
        Some("settings") => settings::run(),
        Some("menu") => menu::run(args.get(2).cloned().unwrap_or_default(), args.get(3..).unwrap_or(&[]).to_vec()),
        None => run_switcher(),
        Some(other) => {
            eprintln!("usage: kwinish-ui [assist | layouts | settings | shortcut <window address> | zones [--edit NAME] | menu <window address> [--at X,Y] | bars [--only <class prefix>] [--skip a,b]]  (got {other})");
            glib::ExitCode::FAILURE
        }
    }
}
