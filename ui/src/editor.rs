// `kwinish-ui zones`: the tiling-zone editor. A full-screen layer-shell overlay draws the layout's zones; each zone has
// buttons to split it (left/right, top/bottom), add a floating block or delete it. The top bar holds the name, the
// padding between zones, "Load layout..." (presets and saved layouts as a starting point), Save and Cancel.
// Save writes ~/.config/kwinish/layouts.json and activates the layout through the core. Zones are drawn over the work area
// (screen minus bars and outer gaps).
use crate::zones::{self, Layout, Zone};
use gtk4::prelude::*;
use gtk4::{gio, glib, Application, ApplicationWindow, Box as GtkBox, Button, CssProvider, Entry, Fixed, Label, MenuButton, Orientation, Popover, SpinButton};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};
use std::cell::RefCell;
use std::process::Command;
use std::rc::Rc;

const CSS: &str = "
window.kwed { background: alpha(#000000, 0.55); }
.zone { background: alpha(#7aa2f7, 0.28); border: 2px solid #7aa2f7; border-radius: 8px; }
.zone .num { font-size: 28px; font-weight: 700; color: #e8e8ee; }
.zone button, .topbar button { background: alpha(#1c1c22, 0.95); color: #e8e8ee; border: 1px solid alpha(#ffffff, 0.15); border-radius: 6px; padding: 4px 8px; box-shadow: none; }
.zone button:hover, .topbar button:hover { background: alpha(#7aa2f7, 0.55); }
.zone button.del:hover { background: #e5484d; }
.handle { background: alpha(#ffffff, 0.35); border-radius: 6px; }
.handle:hover { background: #ffffff; }
.topbar { background: alpha(#1c1c22, 0.97); border-radius: 10px; padding: 8px 12px; color: #e8e8ee; }
";

fn t(en: &'static str) -> &'static str {
    let pt = std::env::var("LC_ALL").or_else(|_| std::env::var("LC_MESSAGES")).or_else(|_| std::env::var("LANG")).map_or(false, |l| l.starts_with("pt"));
    if !pt { return en; }
    match en {
        "Split left / right" => "Dividir em esquerda/direita",
        "Split top / bottom" => "Dividir em superior/inferior",
        "Add a floating block" => "Adicionar um bloco flutuante",
        "Delete" => "Excluir",
        "Padding" => "Preenchimento",
        "Load layout..." => "Carregar layout...",
        "Save" => "Salvar",
        "Cancel" => "Cancelar",
        "Name" => "Nome",
        "My layout" => "Meu layout",
        other => other,
    }
}

struct Ed { fixed: Fixed, handles: Fixed, zones: RefCell<Vec<Zone>>, size: (i32, i32), origin: (i32, i32) }

// The work area of the monitor under the pointer, in that monitor's pixels: minus reserved space (bars) and the outer
// gaps, as the core does. Returns (size, origin) and the monitor; the plugin's title-bar height is not subtracted.
fn work_area() -> (((i32, i32), (i32, i32)), crate::mon::Mon) {
    let (px, py) = crate::mon::cursor();
    let m = crate::mon::at(&crate::mon::all(), px, py);
    let o = Command::new("hyprctl").args(["getoption", "general:gaps_out", "-j"]).output().map(|o| String::from_utf8_lossy(&o.stdout).to_string()).unwrap_or_default();
    let o: serde_json::Value = serde_json::from_str(&o).unwrap_or_default();
    let go: Vec<i32> = o["css"].as_str().unwrap_or("0 0 0 0").split_whitespace().filter_map(|v| v.parse().ok()).collect(); // top, right, bottom, left
    let g = |i: usize| go.get(i).copied().unwrap_or(0);
    let (l, t, r, b) = (m.reserved[0] + g(3), m.reserved[1] + g(0), m.reserved[2] + g(1), m.reserved[3] + g(2));
    (((m.w - l - r, m.h - t - b), (l, t)), m)
}

fn icon_button(label: &str, tip: &str, css: Option<&str>) -> Button {
    let b = Button::with_label(label);
    b.set_tooltip_text(Some(tip));
    if let Some(c) = css { b.add_css_class(c); }
    b
}

fn redraw(ed: &Rc<Ed>) {
    while let Some(c) = ed.fixed.first_child() { ed.fixed.remove(&c); }
    let zs = ed.zones.borrow().clone();
    for (i, z) in zs.iter().enumerate() {
        let (x, y) = ((z[0] * ed.size.0 as f64).round() as i32, (z[1] * ed.size.1 as f64).round() as i32);
        let (w, h) = ((z[2] * ed.size.0 as f64).round() as i32 - 4, (z[3] * ed.size.1 as f64).round() as i32 - 4);
        let boxw = GtkBox::new(Orientation::Vertical, 6);
        boxw.add_css_class("zone");
        boxw.set_size_request(w.max(40), h.max(40));
        boxw.set_halign(gtk4::Align::Fill);
        let num = Label::new(Some(&(i + 1).to_string()));
        num.add_css_class("num");
        num.set_vexpand(true);
        num.set_valign(gtk4::Align::End);
        let row = GtkBox::new(Orientation::Horizontal, 6);
        row.set_halign(gtk4::Align::Center);
        row.set_valign(gtk4::Align::Start);
        row.set_vexpand(true);
        let mk = |glyph: &str, tip: &'static str, css: Option<&str>, f: Box<dyn Fn(&mut Vec<Zone>)>| {
            let b = icon_button(glyph, t(tip), css);
            let ed = ed.clone();
            b.connect_clicked(move |_| { f(&mut ed.zones.borrow_mut()); redraw(&ed); rebuild_handles(&ed); });
            row.append(&b);
        };
        mk("◫", "Split left / right", None, Box::new(move |v| zones::split(v, i, true)));
        mk("⬒", "Split top / bottom", None, Box::new(move |v| zones::split(v, i, false)));
        mk("＋", "Add a floating block", None, Box::new(|v| zones::add_floating(v)));
        mk("✕", "Delete", Some("del"), Box::new(move |v| zones::delete(v, i)));
        boxw.append(&num);
        boxw.append(&row);
        ed.fixed.put(&boxw, (x + ed.origin.0) as f64 + 2.0, (y + ed.origin.1) as f64 + 2.0);
    }
}

// Draggable handles over the inner edges. Dragging re-draws the zones live but keeps the handle widgets, so the
// gesture survives; the handles are rebuilt when the drag ends or a zone is split/deleted.
fn rebuild_handles(ed: &Rc<Ed>) {
    while let Some(c) = ed.handles.first_child() { ed.handles.remove(&c); }
    let base = ed.zones.borrow().clone();
    for e in zones::edges(&base) {
        let (w, h) = (ed.size.0 as f64, ed.size.1 as f64);
        let (ox, oy) = (ed.origin.0 as f64, ed.origin.1 as f64);
        let (x, y, bw, bh) = if e.vertical { (e.pos * w - 6.0 + ox, e.from * h + oy, 12, ((e.to - e.from) * h) as i32) } else { (e.from * w + ox, e.pos * h - 6.0 + oy, ((e.to - e.from) * w) as i32, 12) };
        let hd = GtkBox::new(Orientation::Horizontal, 0);
        hd.add_css_class("handle");
        hd.set_size_request(bw, bh);
        hd.set_cursor_from_name(Some(if e.vertical { "col-resize" } else { "row-resize" }));
        let g = gtk4::GestureDrag::new();
        let (ed2, base2, e2, hd2) = (ed.clone(), base.clone(), e.clone(), hd.clone());
        g.connect_drag_update(move |_, dx, dy| {
            let to = e2.pos + if e2.vertical { dx / ed2.size.0 as f64 } else { dy / ed2.size.1 as f64 };
            if let Some(z) = zones::move_edge(&base2, &e2, to) {
                *ed2.zones.borrow_mut() = z;
                redraw(&ed2);
                let off = (to - e2.pos) * if e2.vertical { ed2.size.0 as f64 } else { ed2.size.1 as f64 };
                ed2.handles.move_(&hd2, x + if e2.vertical { off } else { 0.0 }, y + if e2.vertical { 0.0 } else { off });
            }
        });
        let ed3 = ed.clone();
        g.connect_drag_end(move |_, _, _| rebuild_handles(&ed3));
        hd.add_controller(g);
        ed.handles.put(&hd, x, y);
    }
}

pub fn run(args: Vec<String>) -> glib::ExitCode {
    // `--edit NAME` starts from a saved or preset layout
    let start = args.iter().position(|a| a == "--edit").and_then(|i| args.get(i + 1)).cloned();
    let app = Application::builder().application_id("dev.kwinish.zones").flags(gio::ApplicationFlags::NON_UNIQUE).build();
    app.connect_activate(move |app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().expect("no display"), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);

        let saved = zones::load();
        let mut known: Vec<Layout> = zones::presets();
        known.extend(saved.iter().cloned());
        let first = start.as_ref().and_then(|n| known.iter().find(|l| &l.name == n)).cloned();
        let wa = work_area();
        let ed = Rc::new(Ed { fixed: Fixed::new(), handles: Fixed::new(), zones: RefCell::new(first.as_ref().map_or(vec![[0., 0., 0.5, 1.], [0.5, 0., 0.5, 1.]], |l| l.zones.clone())), size: wa.0 .0, origin: wa.0 .1 });

        let name = Entry::new();
        name.set_text(&first.as_ref().filter(|l| saved.iter().any(|s| s.name == l.name)).map_or(String::new(), |l| l.label.clone()));
        name.set_placeholder_text(Some(t("My layout")));
        let pad = SpinButton::with_range(0.0, 60.0, 1.0);
        pad.set_value(first.as_ref().and_then(|l| l.gap).unwrap_or(8) as f64);

        let load = MenuButton::new();
        load.set_label(t("Load layout..."));
        let pop = Popover::new();
        let list = GtkBox::new(Orientation::Vertical, 2);
        for l in &known {
            let b = Button::with_label(&l.label);
            let (ed2, l2, name2, pad2, pop2) = (ed.clone(), l.clone(), name.clone(), pad.clone(), pop.clone());
            b.connect_clicked(move |_| {
                *ed2.zones.borrow_mut() = l2.zones.clone();
                if let Some(g) = l2.gap { pad2.set_value(g as f64); }
                if saved_name(&l2) { name2.set_text(&l2.label); }
                redraw(&ed2);
                rebuild_handles(&ed2);
                pop2.popdown();
            });
            list.append(&b);
        }
        pop.set_child(Some(&list));
        load.set_popover(Some(&pop));

        let save = Button::with_label(t("Save"));
        let cancel = Button::with_label(t("Cancel"));
        let bar = GtkBox::new(Orientation::Horizontal, 10);
        bar.add_css_class("topbar");
        bar.set_halign(gtk4::Align::Center);
        bar.set_valign(gtk4::Align::Start);
        bar.set_margin_top(14);
        for w in [Label::new(Some(t("Name"))).upcast::<gtk4::Widget>(), name.clone().upcast(), Label::new(Some(t("Padding"))).upcast(), pad.clone().upcast(), load.upcast(), save.clone().upcast(), cancel.clone().upcast()] { bar.append(&w); }

        let overlay = gtk4::Overlay::new();
        overlay.set_child(Some(&ed.fixed));
        overlay.add_overlay(&ed.handles);
        overlay.add_overlay(&bar);
        let win = ApplicationWindow::builder().application(app).child(&overlay).build();
        win.add_css_class("kwed");
        win.init_layer_shell();
        win.set_layer(Layer::Overlay);
        win.set_namespace(Some("kwinish-zones"));
        if let Some(m) = crate::mon::gdk_monitor(&wa.1.name) { win.set_monitor(Some(&m)); }
        win.set_keyboard_mode(KeyboardMode::Exclusive);
        for e in [Edge::Left, Edge::Right, Edge::Top, Edge::Bottom] { win.set_anchor(e, true); }
        win.set_exclusive_zone(-1);

        let a = app.clone();
        cancel.connect_clicked(move |_| a.quit());
        let (a, ed2, name2, pad2) = (app.clone(), ed.clone(), name.clone(), pad.clone());
        save.connect_clicked(move |_| {
            let label = zones::clean_label(&name2.text());
            let label = if label.is_empty() { t("My layout").to_string() } else { label };
            let l = Layout { name: zones::slug(&label), label, gap: Some(pad2.value() as i64), zones: ed2.zones.borrow().clone() };
            if zones::save(l.clone()).is_ok() {
                // set_layout re-reads the file, activates the layout and republishes the zones to the plugin
                let _ = Command::new("hyprctl").args(["eval", &format!("require('kwinish').window_action('','layout:{}')", l.name)]).status();
            }
            a.quit();
        });
        let keys = gtk4::EventControllerKey::new();
        let a = app.clone();
        keys.connect_key_pressed(move |_, key, _, _| {
            if key == gtk4::gdk::Key::Escape { a.quit(); }
            glib::Propagation::Proceed
        });
        win.add_controller(keys);
        redraw(&ed);
        rebuild_handles(&ed);
        win.present();
    });
    app.run_with_args::<&str>(&[])
}

fn saved_name(l: &Layout) -> bool { l.name.starts_with("user-") }
