// Title bars: one layer-shell surface above every floating window. A poll (80 ms) reconciles bars with
// `j/clients`; dragging moves the window through the Hyprland socket, so the button release is ours.
// Limit: layer surfaces always draw above all windows, so a bar is hidden while a floating window higher in
// the stack (client list order) overlaps its strip; a window partly covering another's bar hides the whole bar.
use gtk4::prelude::*;
use gtk4::{gio, glib, Application, ApplicationWindow, Box as GtkBox, Button, CssProvider, EventControllerScroll, EventControllerScrollFlags, GestureClick, Label, Orientation};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};
use std::cell::{Cell, RefCell};
use std::collections::HashMap;
use std::io::{Read, Write};
use std::os::unix::net::UnixStream;
use std::rc::Rc;
use std::time::Duration;

const BAR_H: i32 = 28;
const SNAP_PX: f64 = 3.0;

const CSS: &str = "
.kwbar { background: alpha(#1c1c22, 0.88); color: #b8b8c4; border-radius: 10px 10px 0 0; padding: 0 6px 0 12px; }
.kwbar.active { background: alpha(#26262e, 0.97); color: #f0f0f6; }
.kwbar label { font-size: 12px; font-weight: 600; }
.kwbar button { background: transparent; border: none; box-shadow: none; min-width: 26px; min-height: 22px; padding: 0; color: inherit; border-radius: 6px; }
.kwbar button:hover { background: alpha(#ffffff, 0.14); }
.kwbar button.close:hover { background: #e5484d; color: white; }
";

fn hypr(cmd: &str) -> String {
    let dir = std::env::var("XDG_RUNTIME_DIR").unwrap_or_default();
    let sig = std::env::var("HYPRLAND_INSTANCE_SIGNATURE").unwrap_or_default();
    let Ok(mut s) = UnixStream::connect(format!("{dir}/hypr/{sig}/.socket.sock")) else { return String::new() };
    let _ = s.write_all(cmd.as_bytes());
    let mut out = String::new();
    let _ = s.read_to_string(&mut out);
    out
}

fn json(cmd: &str) -> serde_json::Value {
    serde_json::from_str(&hypr(cmd)).unwrap_or_default()
}

fn cursor() -> (f64, f64) {
    let v = json("j/cursorpos");
    (v["x"].as_f64().unwrap_or(0.0), v["y"].as_f64().unwrap_or(0.0))
}

fn action(addr: &str, act: &str) {
    // addr comes from Hyprland and act from our own code; keep the Lua string tame regardless
    let clean = |s: &str| s.chars().filter(|c| c.is_ascii_alphanumeric() || ":+-_".contains(*c)).collect::<String>();
    hypr(&format!("eval require('kwinish').window_action('{}','{}')", clean(addr), clean(act)));
}

fn zorder(addr: &str, mode: &str) {
    hypr(&format!("dispatch hl.dsp.window.alter_zorder({{ mode = \"{mode}\", window = \"address:{addr}\" }})"));
}

#[derive(Clone, Copy)]
struct Rect { x: i32, y: i32, w: i32, h: i32 }

impl Rect {
    fn hits(&self, o: &Rect) -> bool {
        self.x < o.x + o.w && o.x < self.x + self.w && self.y < o.y + o.h && o.y < self.y + self.h
    }
}

struct Drag { g0: (f64, f64), w0: (i32, i32) }

struct Bar {
    win: ApplicationWindow,
    root: GtkBox,
    title: Label,
    pos: Cell<(i32, i32, i32)>, // window x, y, width as last placed
    mon: Cell<(i32, i32)>,      // monitor origin
    drag: RefCell<Option<Drag>>,
}

impl Bar {
    fn place(&self, x: i32, y: i32, w: i32) {
        let (mx, my) = self.mon.get();
        self.win.set_margin(Edge::Left, x - mx);
        self.win.set_margin(Edge::Top, (y - BAR_H - my).max(0));
        if self.pos.get().2 != w {
            self.root.set_size_request(w, BAR_H);
            self.win.set_default_size(w, BAR_H);
        }
        self.pos.set((x, y, w));
    }
}

struct Opts { only: Option<String>, skip: Vec<String> }

fn make_bar(app: &Application, addr: &str, mon: Rect) -> Rc<Bar> {
    let root = GtkBox::new(Orientation::Horizontal, 0);
    root.add_css_class("kwbar");
    let title_box = GtkBox::new(Orientation::Horizontal, 0);
    title_box.set_hexpand(true);
    let title = Label::new(None);
    title.set_ellipsize(gtk4::pango::EllipsizeMode::End);
    title.set_xalign(0.0);
    title_box.append(&title);
    root.append(&title_box);
    let max_b = Button::with_label("▢");
    let close_b = Button::with_label("✕");
    close_b.add_css_class("close");
    root.append(&max_b);
    root.append(&close_b);

    let win = ApplicationWindow::builder().application(app).child(&root).default_height(BAR_H).build();
    win.init_layer_shell();
    win.set_layer(Layer::Top);
    win.set_namespace(Some("kwinish-bar"));
    win.set_keyboard_mode(KeyboardMode::None);
    win.set_exclusive_zone(-1); // margins count from the output corner, not the bar-reserved area
    win.set_anchor(Edge::Top, true);
    win.set_anchor(Edge::Left, true);
    let bar = Rc::new(Bar { win, root, title, pos: Cell::new((0, 0, -1)), mon: Cell::new((mon.x, mon.y)), drag: RefCell::new(None) });

    let a = addr.to_string();
    max_b.connect_clicked({ let a = a.clone(); move |_| action(&a, "tile:max") });
    close_b.connect_clicked({ let a = a.clone(); move |_| action(&a, "close") });

    let click = GestureClick::builder().button(0).build();
    click.connect_pressed({
        let (bar, a) = (bar.clone(), a.clone());
        move |g, n, _, _| {
            match g.current_button() {
                1 if n == 2 => action(&a, "tile:max"),
                1 => {
                    let (x, y, _) = bar.pos.get();
                    *bar.drag.borrow_mut() = Some(Drag { g0: cursor(), w0: (x, y) });
                }
                3 => {
                    if let Ok(exe) = std::env::current_exe() {
                        let _ = std::process::Command::new(exe).args(["menu", &a]).spawn();
                    }
                }
                _ => {}
            }
        }
    });
    click.connect_released({
        let (bar, a) = (bar.clone(), a.clone());
        move |g, _, _, _| {
            if g.current_button() != 1 { return }
            let Some(d) = bar.drag.borrow_mut().take() else { return };
            // the compositor never reports a drag end to Lua; this release is the only signal
            let (cx, cy) = cursor();
            let m = bar.mon.get();
            let mons = json("j/monitors");
            let mw = mons.as_array().and_then(|v| v.iter().find(|o| o["x"].as_i64() == Some(m.0 as i64) && o["y"].as_i64() == Some(m.1 as i64)))
                .map(|o| (o["width"].as_f64().unwrap_or(0.0) / o["scale"].as_f64().unwrap_or(1.0), o["height"].as_f64().unwrap_or(0.0)));
            if let Some((w, _)) = mw {
                let snap = if cx <= m.0 as f64 + SNAP_PX { Some("tile:left") }
                    else if cx >= m.0 as f64 + w - SNAP_PX { Some("tile:right") }
                    else if cy <= m.1 as f64 + SNAP_PX { Some("tile:max") } else { None };
                if let Some(act) = snap {
                    // put the window back where the drag began so "restore" returns there, not to the drag's last spot
                    hypr(&format!("dispatch hl.dsp.window.move({{ x = {}, y = {}, window = \"address:{a}\" }})", d.w0.0, d.w0.1));
                    action(&a, act);
                }
            }
        }
    });
    title_box.add_controller(click);

    let motion = gtk4::EventControllerMotion::new();
    motion.connect_motion({
        let (bar, a) = (bar.clone(), a.clone());
        move |_, _, _| {
            let st = bar.drag.borrow();
            let Some(d) = st.as_ref() else { return };
            let g = cursor(); // global, from the compositor: surface-local coordinates lag behind a surface we move
            let (nx, ny) = (d.w0.0 + (g.0 - d.g0.0) as i32, d.w0.1 + (g.1 - d.g0.1) as i32);
            hypr(&format!("dispatch hl.dsp.window.move({{ x = {nx}, y = {ny}, window = \"address:{a}\" }})"));
            let w = bar.pos.get().2;
            drop(st);
            bar.place(nx, ny, w);
        }
    });
    title_box.add_controller(motion);

    // wheel on the bar: up raises the window, down lowers it
    let scroll = EventControllerScroll::new(EventControllerScrollFlags::VERTICAL);
    scroll.connect_scroll({
        let a = a.clone();
        move |_, _, dy| {
            zorder(&a, if dy < 0.0 { "top" } else { "bottom" });
            glib::Propagation::Stop
        }
    });
    root_add(&bar, scroll);
    bar
}

fn root_add(bar: &Bar, c: EventControllerScroll) {
    bar.root.add_controller(c);
}

fn reconcile(app: &Application, bars: &RefCell<HashMap<String, Rc<Bar>>>, o: &Opts) {
    let clients = json("j/clients");
    let mons = json("j/monitors");
    let active = json("j/activewindow")["address"].as_str().unwrap_or("").to_string();
    let (Some(cl), Some(ms)) = (clients.as_array(), mons.as_array()) else { return };

    let rect = |c: &serde_json::Value| Rect {
        x: c["at"][0].as_i64().unwrap_or(0) as i32, y: c["at"][1].as_i64().unwrap_or(0) as i32,
        w: c["size"][0].as_i64().unwrap_or(0) as i32, h: c["size"][1].as_i64().unwrap_or(0) as i32,
    };
    let visible = |c: &serde_json::Value| -> Option<Rect> {
        let ws = c["workspace"]["id"].as_i64()?;
        let m = ms.iter().find(|m| m["id"] == c["monitor"])?;
        (c["mapped"] == true && c["hidden"] == false && ws > 0 && m["activeWorkspace"]["id"].as_i64() == Some(ws)).then(|| Rect {
            x: m["x"].as_i64().unwrap_or(0) as i32, y: m["y"].as_i64().unwrap_or(0) as i32, w: 0, h: 0,
        })
    };
    let wants = |c: &serde_json::Value| -> bool {
        let class = c["class"].as_str().unwrap_or("");
        c["floating"] == true && c["fullscreen"].as_i64() == Some(0) && visible(c).is_some()
            && !(class.is_empty() && c["title"].as_str().unwrap_or("").is_empty())
            && o.only.as_deref().map_or(true, |p| class.starts_with(p))
            && !o.skip.iter().any(|s| s == class)
    };

    let mut seen: Vec<String> = Vec::new();
    for c in cl.iter().filter(|c| wants(c)) {
        let addr = c["address"].as_str().unwrap_or("").to_string();
        let mon = visible(c).unwrap();
        let r = rect(c);
        let bar = bars.borrow_mut().entry(addr.clone()).or_insert_with(|| make_bar(app, &addr, mon)).clone();
        seen.push(addr.clone());
        bar.mon.set((mon.x, mon.y));
        bar.title.set_text(c["title"].as_str().unwrap_or(""));
        if addr == active { bar.root.add_css_class("active") } else { bar.root.remove_css_class("active") }
        if bar.drag.borrow().is_none() { bar.place(r.x, r.y, r.w) }
        // `j/clients` lists windows bottom to top, so a bar is hidden while a later floating window overlaps its strip
        let strip = Rect { x: r.x, y: r.y - BAR_H, w: r.w, h: BAR_H };
        let idx = cl.iter().position(|d| d["address"] == c["address"]).unwrap_or(0);
        let covered = cl.iter().skip(idx + 1).any(|d| d["floating"] == true && visible(d).is_some()
            && d["workspace"]["id"] == c["workspace"]["id"] && rect(d).hits(&strip));
        bar.win.set_visible(!covered || bar.drag.borrow().is_some());
    }
    let gone: Vec<String> = bars.borrow().keys().filter(|k| !seen.contains(k)).cloned().collect();
    for k in gone {
        if let Some(b) = bars.borrow_mut().remove(&k) { b.win.close(); }
    }
}

pub fn run(args: Vec<String>) -> glib::ExitCode {
    let mut o = Opts { only: None, skip: vec![] };
    let mut it = args.into_iter();
    while let Some(a) = it.next() {
        match a.as_str() {
            "--only" => o.only = it.next(),
            "--skip" => o.skip = it.next().unwrap_or_default().split(',').map(String::from).collect(),
            _ => {}
        }
    }
    let app = Application::builder().application_id("dev.kwinish.bars").flags(gio::ApplicationFlags::NON_UNIQUE).build();
    let o = Rc::new(o);
    app.connect_activate(move |app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().expect("no display"), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);
        let bars: Rc<RefCell<HashMap<String, Rc<Bar>>>> = Rc::new(RefCell::new(HashMap::new()));
        let (a, b, o) = (app.clone(), bars.clone(), o.clone());
        glib::timeout_add_local(Duration::from_millis(80), move || {
            reconcile(&a, &b, &o);
            glib::ControlFlow::Continue
        });
        std::mem::forget(app.hold());
    });
    app.run_with_args::<&str>(&[])
}

#[cfg(test)]
mod tests {
    use super::Rect;

    #[test]
    fn rect_hits() {
        let a = Rect { x: 0, y: 0, w: 10, h: 10 };
        assert!(a.hits(&Rect { x: 5, y: 5, w: 10, h: 10 }));
        assert!(!a.hits(&Rect { x: 10, y: 0, w: 5, h: 5 })); // touching edges do not overlap
        assert!(!a.hits(&Rect { x: 20, y: 20, w: 5, h: 5 }));
    }
}
