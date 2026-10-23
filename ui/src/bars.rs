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

use crate::mode::{decide, emergency_text, Facts, Mode, PLUGIN_NAME};

const BAR_H: i32 = 28;
const SNAP_PX: f64 = 3.0;

type Rgba = [f64; 4];

#[derive(Clone, PartialEq)]
struct Style { border: i32, radius: i32, power: f64, active: Rgba, inactive: Rgba }

const BG: Rgba = [0x1c as f64 / 255.0, 0x1c as f64 / 255.0, 0x22 as f64 / 255.0, 0.88];
const BG_ACTIVE: Rgba = [0x26 as f64 / 255.0, 0x26 as f64 / 255.0, 0x2e as f64 / 255.0, 0.97];

// Hyprland prints gradients as "aarrggbb 0deg"; the first stop is enough for a flat border.
fn parse_color(g: &str, fallback: Rgba) -> Rgba {
    let h = g.split_whitespace().next().unwrap_or("");
    if h.len() == 8 && h.chars().all(|c| c.is_ascii_hexdigit()) {
        let v = |i: usize| u8::from_str_radix(&h[i..i + 2], 16).unwrap_or(0) as f64 / 255.0;
        [v(2), v(4), v(6), v(0)]
    } else {
        fallback
    }
}

fn rgba_css(c: &Rgba) -> String {
    format!("rgba({}, {}, {}, {:.2})", (c[0] * 255.0).round(), (c[1] * 255.0).round(), (c[2] * 255.0).round(), c[3])
}

fn read_style() -> Style {
    let int = |o: &str, d: i64| json(&format!("j/getoption {o}"))["int"].as_i64().unwrap_or(d) as i32;
    let col = |o: &str, d: Rgba| parse_color(json(&format!("j/getoption {o}"))["gradient"].as_str().unwrap_or(""), d);
    Style {
        border: int("general:border_size", 1).max(0),
        radius: int("decoration:rounding", 0).max(0),
        power: json("j/getoption decoration:rounding_power")["float"].as_f64().unwrap_or(2.0).clamp(1.0, 10.0),
        active: col("general:col.active_border", [0.78, 0.76, 0.86, 1.0]),
        inactive: col("general:col.inactive_border", [0.08, 0.07, 0.09, 1.0]),
    }
}

fn css(st: &Style) -> String {
    format!(
        "window.kwwin {{ background: transparent; }}
.kwbar {{ background: {bg}; color: #b8b8c4; padding: 0 6px 0 12px; border: {b}px solid {i}; border-bottom: none; border-radius: {r}px {r}px 0 0; }}
.kwbar.active {{ background: {bga}; color: #f0f0f6; border-color: {a}; }}
.kwbar.cut-left {{ border-left: none; }}
.kwbar.cut-right {{ border-right: none; }}
.kwbar label {{ font-size: 12px; font-weight: 600; }}
.kwbar button {{ background: transparent; border: none; box-shadow: none; min-width: 26px; min-height: 22px; padding: 0; color: inherit; border-radius: {br}px; }}
.kwbar button:hover {{ background: alpha(#ffffff, 0.14); }}
.kwbar button.close:hover {{ background: #e5484d; color: white; }}",
        b = st.border, r = st.radius, i = rgba_css(&st.inactive), a = rgba_css(&st.active),
        bg = rgba_css(&BG), bga = rgba_css(&BG_ACTIVE), br = st.radius.min(6)
    )
}

// Corner points of the window content's rounded corner (top-left, window origin at 0,0, radius r, curve power p):
// from (0, r) to (r, 0). Hyprland rounds corners as a superellipse |dx/r|^p + |dy/r|^p = 1.
fn corner_curve(r: f64, p: f64, n: usize) -> Vec<(f64, f64)> {
    (0..=n).map(|k| {
        let t = std::f64::consts::FRAC_PI_2 * k as f64 / n as f64;
        (r - r * t.cos().powf(2.0 / p), r - r * t.sin().powf(2.0 / p))
    }).collect()
}

#[derive(Clone)]
struct Fill { style: Style, active: bool, cut_left: bool, cut_right: bool, width: f64 }

// Draws, under the bar, the part that makes bar and window one shape when the window has rounded corners:
// a straight border strip on each side plus bar-coloured fill between the strip and the curved content corner.
fn draw_fill(cr: &gtk4::cairo::Context, f: &Fill) {
    let (r, b) = (f.style.radius as f64, f.style.border as f64);
    if r <= 0.0 { return }
    let bg = if f.active { BG_ACTIVE } else { BG };
    let bc = if f.active { f.style.active } else { f.style.inactive };
    let curve = corner_curve(r, f.style.power, 24);
    for left in [true, false] {
        if (left && f.cut_left) || (!left && f.cut_right) { continue }
        // local x of the window's own edge: b from the left; width - b from the right
        let edge = if left { b } else { f.width - b };
        let dir = if left { 1.0 } else { -1.0 };
        let px = |x: f64| edge + dir * x;
        cr.set_source_rgba(bc[0], bc[1], bc[2], bc[3]);
        if b > 0.0 {
            cr.rectangle(if left { 0.0 } else { f.width - b }, 0.0, b, r);
            let _ = cr.fill();
        }
        cr.set_source_rgba(bg[0], bg[1], bg[2], bg[3]);
        cr.move_to(px(0.0), 0.0);
        cr.line_to(px(r), 0.0);
        for &(cx, cy) in curve.iter().rev() { cr.line_to(px(cx), cy); }
        cr.close_path();
        let _ = cr.fill();
    }
}

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

fn plugin_dir() -> String {
    std::env::var("KWINISH_PLUGIN_DIR").unwrap_or_else(|_| "/usr/lib/hyprland/plugins".into())
}

fn current_mode() -> Mode {
    let loaded: Vec<String> = json("j/plugin list").as_array()
        .map(|a| a.iter().filter_map(|p| p["name"].as_str().map(String::from)).collect()).unwrap_or_default();
    let dir = plugin_dir();
    let installed = std::path::Path::new(&format!("{dir}/{PLUGIN_NAME}.so")).exists();
    let abi = std::fs::read_to_string(format!("{dir}/{PLUGIN_NAME}.abi")).ok();
    let abi_commit = abi.as_deref().and_then(|a| a.trim().split('_').next()).filter(|c| !c.is_empty());
    let running = json("j/version")["commit"].as_str().unwrap_or("").to_string();
    decide(&Facts { loaded: &loaded, installed, abi_commit, running_commit: &running })
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
    fill: Rc<RefCell<Fill>>,
    da: gtk4::DrawingArea,
    title: Label,
    pos: Cell<(i32, i32, i32)>, // window x, y, width as last placed
    mon: Cell<(i32, i32, i32)>, // monitor origin and logical width
    drag: RefCell<Option<Drag>>,
}

impl Bar {
    // x, y, w: the window. `clip` limits the bar to an absolute x range (when another window covers part of it).
    fn place(&self, x: i32, y: i32, w: i32, border: i32, clip: Option<(i32, i32)>) {
        let (mx, my, mw) = self.mon.get();
        let (bx0, bx1) = clip.unwrap_or((x - border, x + w + border));
        self.win.set_margin(Edge::Left, (bx0 - mx).max(0));
        self.win.set_margin(Edge::Right, (mx + mw - bx1).max(0));
        // bottom edge sits on the window's top edge, so the bar's background covers the compositor's top border
        self.win.set_margin(Edge::Top, (y - BAR_H - my).max(0));
        self.pos.set((x, y, w));
    }
}

struct Opts { only: Option<String>, skip: Vec<String> }

fn make_bar(app: &Application, addr: &str, mon: (i32, i32, i32), border: Rc<Cell<i32>>) -> Rc<Bar> {
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

    root.set_size_request(-1, BAR_H);
    let fill = Rc::new(RefCell::new(Fill { style: read_style(), active: false, cut_left: false, cut_right: false, width: 0.0 }));
    let da = gtk4::DrawingArea::new();
    da.set_can_target(false);
    da.set_content_height(fill.borrow().style.radius);
    da.set_draw_func({
        let fill = fill.clone();
        move |w, cr, width, _| {
            let mut f = fill.borrow().clone();
            f.width = width as f64;
            let _ = w;
            draw_fill(cr, &f);
        }
    });
    let outer = GtkBox::new(Orientation::Vertical, 0);
    outer.append(&root);
    outer.append(&da);
    let win = ApplicationWindow::builder().application(app).child(&outer).default_height(BAR_H).build();
    win.add_css_class("kwwin");
    // only the bar row takes input; the filler below it must let clicks through to the window
    win.connect_map(|w| {
        if let Some(surface) = w.surface() {
            surface.set_input_region(Some(&gtk4::cairo::Region::create_rectangle(&gtk4::cairo::RectangleInt::new(0, 0, 32767, BAR_H))));
        }
    });
    win.init_layer_shell();
    win.set_layer(Layer::Top);
    win.set_namespace(Some("kwinish-bar"));
    win.set_keyboard_mode(KeyboardMode::None);
    win.set_exclusive_zone(-1); // margins count from the output corner, not the bar-reserved area
    win.set_anchor(Edge::Top, true);
    win.set_anchor(Edge::Left, true);
    win.set_anchor(Edge::Right, true);
    let bar = Rc::new(Bar { win, root, fill, da, title, pos: Cell::new((0, 0, -1)), mon: Cell::new(mon), drag: RefCell::new(None) });

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
            let mw = Some((m.2 as f64, 0.0));
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
        let (bar, a, border) = (bar.clone(), a.clone(), border.clone());
        move |_, _, _| {
            let st = bar.drag.borrow();
            let Some(d) = st.as_ref() else { return };
            let g = cursor(); // global, from the compositor: surface-local coordinates lag behind a surface we move
            let (nx, ny) = (d.w0.0 + (g.0 - d.g0.0) as i32, d.w0.1 + (g.1 - d.g0.1) as i32);
            hypr(&format!("dispatch hl.dsp.window.move({{ x = {nx}, y = {ny}, window = \"address:{a}\" }})"));
            let w = bar.pos.get().2;
            drop(st);
            bar.place(nx, ny, w, border.get(), None);
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

// Largest stretch of [x0, x1) not covered by any of `covers` (each [start, end)); None if nothing wide enough remains.
fn widest_gap(x0: i32, x1: i32, covers: &[(i32, i32)], min_w: i32) -> Option<(i32, i32)> {
    let mut cs: Vec<(i32, i32)> = covers.iter().map(|&(a, b)| (a.max(x0), b.min(x1))).filter(|&(a, b)| a < b).collect();
    cs.sort();
    let (mut best, mut cur) = (None::<(i32, i32)>, x0);
    for (a, b) in cs.into_iter().chain(std::iter::once((x1, x1))) {
        if a > cur && best.map_or(true, |(s, e)| a - cur > e - s) { best = Some((cur, a)) }
        cur = cur.max(b);
    }
    best.filter(|&(a, b)| b - a >= min_w)
}

struct State { bars: RefCell<HashMap<String, Rc<Bar>>>, style: RefCell<Style>, provider: CssProvider, border: Rc<Cell<i32>>, tick: Cell<u32> }

fn reconcile(app: &Application, st: &State, o: &Opts) {
    // border colours and rounding follow the compositor config; re-read about once a second
    let n = st.tick.get();
    st.tick.set(n.wrapping_add(1));
    if n % 12 == 0 {
        let now = read_style();
        if now != *st.style.borrow() {
            st.provider.load_from_data(&css(&now));
            st.border.set(now.border);
            for b in st.bars.borrow().values() {
                b.fill.borrow_mut().style = now.clone();
                b.da.set_content_height(now.radius);
                b.win.set_default_size(-1, BAR_H);
                b.da.queue_draw();
            }
            *st.style.borrow_mut() = now;
        }
    }
    let border = st.border.get();
    let bars = &st.bars;
    let clients = json("j/clients");
    let mons = json("j/monitors");
    let active = json("j/activewindow")["address"].as_str().unwrap_or("").to_string();
    let (Some(cl), Some(ms)) = (clients.as_array(), mons.as_array()) else { return };

    let rect = |c: &serde_json::Value| Rect {
        x: c["at"][0].as_i64().unwrap_or(0) as i32, y: c["at"][1].as_i64().unwrap_or(0) as i32,
        w: c["size"][0].as_i64().unwrap_or(0) as i32, h: c["size"][1].as_i64().unwrap_or(0) as i32,
    };
    // monitor origin and logical width for a window on a visible, normal workspace
    let visible = |c: &serde_json::Value| -> Option<(i32, i32, i32)> {
        let ws = c["workspace"]["id"].as_i64()?;
        let m = ms.iter().find(|m| m["id"] == c["monitor"])?;
        (c["mapped"] == true && c["hidden"] == false && ws > 0 && m["activeWorkspace"]["id"].as_i64() == Some(ws)).then(|| (
            m["x"].as_i64().unwrap_or(0) as i32, m["y"].as_i64().unwrap_or(0) as i32,
            (m["width"].as_f64().unwrap_or(0.0) / m["scale"].as_f64().unwrap_or(1.0)) as i32,
        ))
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
        let bar = bars.borrow_mut().entry(addr.clone()).or_insert_with(|| make_bar(app, &addr, mon, st.border.clone())).clone();
        seen.push(addr.clone());
        bar.mon.set(mon);
        bar.title.set_text(c["title"].as_str().unwrap_or(""));
        if addr == active { bar.root.add_css_class("active") } else { bar.root.remove_css_class("active") }
        { // a bar created after the last style refresh starts from the current style
            let mut f = bar.fill.borrow_mut();
            if f.style != *st.style.borrow() { f.style = st.style.borrow().clone(); bar.da.set_content_height(f.style.radius); }
            if f.active != (addr == active) { f.active = addr == active; bar.da.queue_draw(); }
        }
        let dragging = bar.drag.borrow().is_some();
        // `j/clients` lists windows bottom to top: windows later in the list can cover this bar's strip,
        // so the bar is clipped to the widest stretch they leave free (hidden if too narrow)
        let strip = Rect { x: r.x - border, y: r.y - BAR_H, w: r.w + 2 * border, h: BAR_H };
        let idx = cl.iter().position(|d| d["address"] == c["address"]).unwrap_or(0);
        let covers: Vec<(i32, i32)> = cl.iter().skip(idx + 1)
            .filter(|d| d["floating"] == true && visible(d).is_some() && d["workspace"]["id"] == c["workspace"]["id"])
            .map(|d| { let dr = rect(d); (dr, Rect { x: dr.x - border, y: dr.y - border, w: dr.w + 2 * border, h: dr.h + 2 * border }) })
            .filter(|(_, e)| e.hits(&strip)).map(|(_, e)| (e.x, e.x + e.w)).collect();
        let gap = if dragging { Some((strip.x, strip.x + strip.w)) } else { widest_gap(strip.x, strip.x + strip.w, &covers, 48) };
        match gap {
            Some(g) => {
                if !dragging { bar.place(r.x, r.y, r.w, border, Some(g)) }
                // a side cut by another window gets no border: the outline belongs only to the top and the window's own sides
                for (class, cut) in [("cut-left", g.0 > strip.x), ("cut-right", g.1 < strip.x + strip.w)] {
                    if cut { bar.root.add_css_class(class) } else { bar.root.remove_css_class(class) }
                }
                {
                    let mut f = bar.fill.borrow_mut();
                    let (cl, cr) = (g.0 > strip.x, g.1 < strip.x + strip.w);
                    if f.cut_left != cl || f.cut_right != cr { f.cut_left = cl; f.cut_right = cr; bar.da.queue_draw(); }
                }
                bar.win.set_visible(true);
            }
            None => bar.win.set_visible(false),
        }
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
        let style = read_style();
        provider.load_from_data(&css(&style));
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().expect("no display"), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);
        let st = Rc::new(State { bars: RefCell::new(HashMap::new()), border: Rc::new(Cell::new(style.border)), style: RefCell::new(style), provider, tick: Cell::new(0) });
        let (a, o) = (app.clone(), o.clone());
        let notified = Cell::new(false);
        let mut ticks = 0u32;
        let mut idle = false;
        glib::timeout_add_local(Duration::from_millis(80), move || {
            // re-check who owns the title bars about every 2 s; while the plugin (or hyprbars) draws them, keep no bars
            if ticks % 25 == 0 {
                match current_mode() {
                    Mode::Plugin | Mode::OtherBars(_) => idle = true,
                    Mode::Emergency { built, running } => {
                        idle = false;
                        if !notified.replace(true) {
                            let text = emergency_text(&built, &running);
                            eprintln!("{text}");
                            hypr(&format!("eval hl.notification.create({{ text = [[{text}]], timeout = 20000, icon = \"warning\" }})"));
                        }
                    }
                    Mode::TryLoad => {
                        idle = false;
                        let dir = plugin_dir();
                        hypr(&format!("plugin load {dir}/{PLUGIN_NAME}.so"));
                    }
                    Mode::Fallback => idle = false,
                }
            }
            ticks = ticks.wrapping_add(1);
            if idle {
                let gone: Vec<String> = st.bars.borrow().keys().cloned().collect();
                for k in gone { if let Some(b) = st.bars.borrow_mut().remove(&k) { b.win.close(); } }
            } else {
                reconcile(&a, &st, &o);
            }
            glib::ControlFlow::Continue
        });
        std::mem::forget(app.hold());
    });
    app.run_with_args::<&str>(&[])
}

#[cfg(test)]
mod tests {
    use super::{corner_curve, parse_color, widest_gap, Rect};

    #[test]
    fn rect_hits() {
        let a = Rect { x: 0, y: 0, w: 10, h: 10 };
        assert!(a.hits(&Rect { x: 5, y: 5, w: 10, h: 10 }));
        assert!(!a.hits(&Rect { x: 10, y: 0, w: 5, h: 5 })); // touching edges do not overlap
        assert!(!a.hits(&Rect { x: 20, y: 20, w: 5, h: 5 }));
    }

    #[test]
    fn gaps() {
        assert_eq!(widest_gap(0, 100, &[], 10), Some((0, 100)));
        assert_eq!(widest_gap(0, 100, &[(0, 60)], 10), Some((60, 100))); // covered from the left
        assert_eq!(widest_gap(0, 100, &[(70, 200)], 10), Some((0, 70))); // covered from the right
        assert_eq!(widest_gap(0, 100, &[(40, 50)], 10), Some((50, 100))); // middle cut: the wider side
        assert_eq!(widest_gap(0, 100, &[(0, 95)], 10), None); // too narrow to show
        assert_eq!(widest_gap(0, 100, &[(-5, 120)], 10), None); // fully covered
        assert_eq!(widest_gap(0, 100, &[(10, 40), (30, 80)], 10), Some((80, 100))); // overlapping covers merge
    }

    #[test]
    fn curve_runs_from_left_edge_to_top_edge() {
        let c = corner_curve(12.0, 2.0, 24);
        let (first, last) = (c[0], c[24]);
        assert!((first.0 - 0.0).abs() < 1e-9 && (first.1 - 12.0).abs() < 1e-9);
        assert!((last.0 - 12.0).abs() < 1e-9 && (last.1 - 0.0).abs() < 1e-9);
        // a circle passes through the 45 degree point at r * (1 - cos 45 deg)
        let mid = c[12];
        assert!((mid.0 - 12.0 * (1.0 - 0.5f64.sqrt())).abs() < 1e-6 && (mid.1 - mid.0).abs() < 1e-9);
    }

    #[test]
    fn colour_parse() {
        let c = parse_color("ffc8c3dc 0deg", [0.0; 4]);
        assert!((c[0] - 200.0 / 255.0).abs() < 1e-9 && (c[3] - 1.0).abs() < 1e-9);
        assert_eq!(parse_color("nope", [1.0, 0.0, 0.0, 1.0]), [1.0, 0.0, 0.0, 1.0]);
    }
}
