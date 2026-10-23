// Window menu in the style of KWin's: a one-shot process. A transparent full-screen layer catches outside clicks and
// Esc; the menu opens at the pointer with submenus that open on hover, check marks for toggles, and each entry runs a
// kwinish core action through `hyprctl eval`. It exits after one choice or 20 s.
use gtk4::prelude::*;
use gtk4::{gio, glib, graphene, Application, ApplicationWindow, Box as GtkBox, Button, CssProvider, EventControllerMotion, Fixed, GestureClick, Label, Orientation};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};
use std::cell::RefCell;
use std::process::Command;
use std::rc::Rc;

const CSS: &str = "
window.kwm { background: transparent; }
.menu { background: alpha(#1c1c22, 0.97); border-radius: 10px; padding: 5px; border: 1px solid alpha(#ffffff, 0.10); }
.menu button.row { background: transparent; color: #e8e8ee; border: none; border-radius: 6px; padding: 5px 10px; min-width: 230px; box-shadow: none; }
.menu button.row label { font-size: 13px; }
.menu button.row:hover, .menu button.row.open { background: alpha(#7aa2f7, 0.35); }
.menu .tick { min-width: 18px; color: #7aa2f7; font-weight: 700; }
.menu .arrow { opacity: 0.7; }
.sep { min-height: 1px; background: alpha(#ffffff, 0.12); margin: 4px 6px; }
";

#[derive(Clone)]
enum Item {
    Act { label: String, check: Option<bool>, action: String },
    Sub { label: String, items: Vec<Item> },
    Sep,
}

fn act(label: &str, action: &str) -> Item {
    Item::Act { label: label.into(), check: None, action: action.into() }
}

fn tog(label: &str, on: bool, action: &str) -> Item {
    Item::Act { label: label.into(), check: Some(on), action: action.into() }
}

// What the menu needs to know about the window; read from `hyprctl clients`, where the core also leaves tags.
#[derive(Default)]
struct State { pinned: bool, maximized: bool, fullscreen: bool, ws: i64, above: bool, below: bool, noframe: bool, nocapture: bool, workspaces: Vec<i64> }

fn hyprctl(args: &[&str]) -> String {
    Command::new("hyprctl").args(args).output().map(|o| String::from_utf8_lossy(&o.stdout).to_string()).unwrap_or_default()
}

fn read_state(addr: &str) -> State {
    let mut st = State::default();
    let clients: serde_json::Value = serde_json::from_str(&hyprctl(&["clients", "-j"])).unwrap_or_default();
    if let Some(c) = clients.as_array().and_then(|a| a.iter().find(|c| c["address"] == addr)) {
        let tag = |t: &str| c["tags"].as_array().map_or(false, |v| v.iter().any(|x| x == t));
        st.pinned = c["pinned"] == true;
        st.maximized = c["fullscreen"].as_i64() == Some(1);
        st.fullscreen = c["fullscreen"].as_i64() == Some(2);
        st.ws = c["workspace"]["id"].as_i64().unwrap_or(0);
        st.above = tag("kwinish-above");
        st.below = tag("kwinish-below");
        st.noframe = tag("kwinish-noframe");
        st.nocapture = tag("kwinish-nocapture");
    }
    let ws: serde_json::Value = serde_json::from_str(&hyprctl(&["workspaces", "-j"])).unwrap_or_default();
    st.workspaces = ws.as_array().map(|a| a.iter().filter_map(|w| w["id"].as_i64()).filter(|id| *id > 0).collect()).unwrap_or_default();
    st.workspaces.sort();
    if st.ws > 0 && !st.workspaces.contains(&st.ws) { st.workspaces.push(st.ws); st.workspaces.sort(); }
    st
}

// Portuguese when the locale says so, English otherwise.
fn t(en: &'static str) -> &'static str {
    let pt = std::env::var("LC_ALL").or_else(|_| std::env::var("LC_MESSAGES")).or_else(|_| std::env::var("LANG")).map_or(false, |l| l.starts_with("pt"));
    if !pt { return en; }
    match en {
        "Workspaces" => "Áreas de trabalho",
        "All workspaces" => "Todas as áreas de trabalho",
        "Move to workspace" => "Mover para a área",
        "Move to a new workspace" => "Mover para uma nova área",
        "Workspace" => "Área",
        "Maximize" => "Maximizar",
        "Minimize" => "Minimizar",
        "Keep above others" => "Manter acima das outras",
        "More actions" => "Mais ações",
        "Keep below others" => "Manter abaixo das outras",
        "Fullscreen" => "Tela cheia",
        "No title bar and frame" => "Sem barra de título e moldura",
        "Hide from captures and recordings" => "Ocultar de capturas e gravações",
        "Float / tile" => "Flutuar / ladrilhar",
        "Snap" => "Encaixar",
        "Left half" => "Metade esquerda",
        "Right half" => "Metade direita",
        "Top half" => "Metade de cima",
        "Bottom half" => "Metade de baixo",
        "Top left" => "Canto superior esquerdo",
        "Top right" => "Canto superior direito",
        "Bottom left" => "Canto inferior esquerdo",
        "Bottom right" => "Canto inferior direito",
        "Maximize horizontally" => "Maximizar na horizontal",
        "Maximize vertically" => "Maximizar na vertical",
        "Close" => "Fechar",
        other => other,
    }
}

fn build(st: &State) -> Vec<Item> {
    let mut ws = vec![tog(t("All workspaces"), st.pinned, "pin"), Item::Sep];
    for id in &st.workspaces {
        ws.push(tog(&format!("{} {}", t("Workspace"), id), *id == st.ws && !st.pinned, &format!("workspace:{id}")));
    }
    ws.push(Item::Sep);
    ws.push(act(t("Move to a new workspace"), "workspace:new"));
    let snap = vec![
        act(t("Left half"), "tile:left"), act(t("Right half"), "tile:right"), act(t("Top half"), "tile:top"), act(t("Bottom half"), "tile:bottom"),
        Item::Sep,
        act(t("Top left"), "tile:topleft"), act(t("Top right"), "tile:topright"), act(t("Bottom left"), "tile:bottomleft"), act(t("Bottom right"), "tile:bottomright"),
        Item::Sep,
        act(t("Maximize horizontally"), "axis:h"), act(t("Maximize vertically"), "axis:v"),
    ];
    let more = vec![
        Item::Sub { label: t("Snap").into(), items: snap },
        tog(t("Keep below others"), st.below, "below"),
        tog(t("Fullscreen"), st.fullscreen, "fullscreen"),
        act(t("Float / tile"), "float"),
        tog(t("No title bar and frame"), st.noframe, "noframe"),
        tog(t("Hide from captures and recordings"), st.nocapture, "nocapture"),
    ];
    vec![
        Item::Sub { label: t("Workspaces").into(), items: ws },
        tog(t("Maximize"), st.maximized, "maximize"),
        act(t("Minimize"), "minimize"),
        tog(t("Keep above others"), st.above, "above"),
        Item::Sub { label: t("More actions").into(), items: more },
        Item::Sep,
        act(t("Close"), "close"),
    ]
}

fn cursor() -> (i32, i32) {
    let text = hyprctl(&["cursorpos"]);
    let mut it = text.trim().split(',').map(|v| v.trim().parse::<i32>().unwrap_or(0));
    (it.next().unwrap_or(0), it.next().unwrap_or(0))
}

fn screen() -> (i32, i32) {
    let m: serde_json::Value = serde_json::from_str(&hyprctl(&["monitors", "-j"])).unwrap_or_default();
    (m[0]["width"].as_f64().unwrap_or(1920.0) as i32, m[0]["height"].as_f64().unwrap_or(1080.0) as i32)
}

fn run_action(addr: &str, action: &str) {
    // addr and action come from our own tables and Hyprland, but keep the Lua string safe anyway
    let clean = |s: &str| s.chars().filter(|c| c.is_ascii_alphanumeric() || ":+-_".contains(*c)).collect::<String>();
    let code = format!("require('kwinish').window_action('{}','{}')", clean(addr), clean(action));
    let _ = Command::new("hyprctl").args(["eval", &code]).status();
}

struct Ctx { app: Application, fixed: Fixed, addr: String, open: RefCell<Vec<(GtkBox, Button)>>, screen: (i32, i32) }

// One menu level; `level` 0 is the root. Hovering a row removes deeper levels, and a submenu row opens its own.
fn make_box(items: &[Item], ctx: &Rc<Ctx>, level: usize) -> GtkBox {
    let menu = GtkBox::new(Orientation::Vertical, 0);
    menu.add_css_class("menu");
    for item in items {
        match item {
            Item::Sep => {
                let sep = GtkBox::new(Orientation::Horizontal, 0);
                sep.add_css_class("sep");
                menu.append(&sep);
            }
            Item::Act { label, check, action } => {
                let row = Button::new();
                row.add_css_class("row");
                let line = GtkBox::new(Orientation::Horizontal, 6);
                let tick = Label::new(Some(if *check == Some(true) { "✓" } else { "" }));
                tick.add_css_class("tick");
                let text = Label::new(Some(label));
                text.set_xalign(0.0);
                text.set_hexpand(true);
                line.append(&tick);
                line.append(&text);
                row.set_child(Some(&line));
                let (ctx2, action) = (ctx.clone(), action.clone());
                row.connect_clicked(move |_| {
                    run_action(&ctx2.addr, &action);
                    ctx2.app.quit();
                });
                let motion = EventControllerMotion::new();
                let ctx3 = ctx.clone();
                motion.connect_enter(move |_, _, _| close_below(&ctx3, level));
                row.add_controller(motion);
                menu.append(&row);
            }
            Item::Sub { label, items: sub } => {
                let row = Button::new();
                row.add_css_class("row");
                let line = GtkBox::new(Orientation::Horizontal, 6);
                let tick = Label::new(Some(""));
                tick.add_css_class("tick");
                let text = Label::new(Some(label));
                text.set_xalign(0.0);
                text.set_hexpand(true);
                let arrow = Label::new(Some("›"));
                arrow.add_css_class("arrow");
                line.append(&tick);
                line.append(&text);
                line.append(&arrow);
                row.set_child(Some(&line));
                let sub_items: Vec<Item> = sub.clone();
                let (ctx2, parent) = (ctx.clone(), menu.clone());
                let open_sub = {
                    let row = row.clone();
                    move || {
                        close_below(&ctx2, level);
                        let child = make_box(&sub_items, &ctx2, level + 1);
                        // beside the row, flipped to the left or shifted up when the screen edge is in the way
                        let origin = row.compute_point(&ctx2.fixed, &graphene::Point::new(0.0, 0.0)).unwrap_or_else(|| graphene::Point::new(0.0, 0.0));
                        let (_, nat) = child.preferred_size();
                        let (pw, _) = (parent.width(), 0);
                        let mut x = origin.x() + pw as f32 - 4.0;
                        if x as i32 + nat.width() > ctx2.screen.0 - 6 { x = origin.x() - nat.width() as f32 + 4.0; }
                        let y = (origin.y() - 5.0).min((ctx2.screen.1 - nat.height() - 6) as f32).max(6.0);
                        ctx2.fixed.put(&child, x as f64, y as f64);
                        row.add_css_class("open");
                        ctx2.open.borrow_mut().push((child, row.clone()));
                    }
                };
                let open_for_click = open_sub.clone();
                row.connect_clicked(move |_| open_for_click());
                let motion = EventControllerMotion::new();
                motion.connect_enter(move |_, _, _| open_sub());
                row.add_controller(motion);
                menu.append(&row);
            }
        }
    }
    menu
}

// remove every submenu deeper than `level` (level 0 is the root menu, which is never in `open`)
fn close_below(ctx: &Rc<Ctx>, level: usize) {
    let mut open = ctx.open.borrow_mut();
    while open.len() > level {
        if let Some((b, row)) = open.pop() {
            ctx.fixed.remove(&b);
            row.remove_css_class("open");
        }
    }
}

pub fn run(addr: String, args: Vec<String>) -> glib::ExitCode {
    // `--at X,Y`: open with the top-left corner there (global layout coordinates); without it, at the pointer
    let at: Option<(i32, i32)> = args.iter().position(|a| a == "--at").and_then(|i| args.get(i + 1)).and_then(|v| {
        let mut it = v.split(',').map(|n| n.trim().parse::<i32>().ok());
        Some((it.next()??, it.next()??))
    });
    let app = Application::builder().application_id("dev.kwinish.menu").flags(gio::ApplicationFlags::NON_UNIQUE).build();
    app.connect_activate(move |app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().expect("no display"), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);

        let fixed = Fixed::new();
        let ctx = Rc::new(Ctx { app: app.clone(), fixed: fixed.clone(), addr: addr.clone(), open: RefCell::new(vec![]), screen: screen() });
        let items = build(&read_state(&addr));
        let root = make_box(&items, &ctx, 0);
        let (cx, cy) = at.unwrap_or_else(cursor);
        let (_, nat) = root.preferred_size();
        fixed.put(&root, cx.min(ctx.screen.0 - nat.width() - 6).max(6) as f64, cy.min(ctx.screen.1 - nat.height() - 6).max(6) as f64);

        let win = ApplicationWindow::builder().application(app).child(&fixed).build();
        win.add_css_class("kwm");
        win.init_layer_shell();
        win.set_layer(Layer::Overlay);
        win.set_namespace(Some("kwinish-menu"));
        win.set_keyboard_mode(KeyboardMode::Exclusive);
        for e in [Edge::Left, Edge::Right, Edge::Top, Edge::Bottom] { win.set_anchor(e, true); }
        win.set_exclusive_zone(-1);

        // a click outside every menu level closes it
        let click = GestureClick::new();
        let a = app.clone();
        click.connect_pressed(move |g, _, x, y| {
            // inside when the widget under the pointer, or any of its parents, is a menu level
            let mut w = g.widget().and_then(|w| w.pick(x, y, gtk4::PickFlags::DEFAULT));
            let mut inside = false;
            while let Some(cur) = w {
                if cur.has_css_class("menu") { inside = true; break; }
                w = cur.parent();
            }
            if !inside { a.quit(); }
        });
        fixed.add_controller(click);
        let keys = gtk4::EventControllerKey::new();
        let a = app.clone();
        keys.connect_key_pressed(move |_, key, _, _| {
            if key == gtk4::gdk::Key::Escape { a.quit(); }
            glib::Propagation::Proceed
        });
        win.add_controller(keys);
        let a = app.clone();
        glib::timeout_add_seconds_local_once(20, move || a.quit());
        win.present();
    });
    app.run_with_args::<&str>(&[])
}
