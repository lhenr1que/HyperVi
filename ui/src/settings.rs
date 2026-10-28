// `kwinish-ui settings`: a normal window with the plugin's options (and the Snap Assist switch). Each change is written to
// ~/.config/kwinish/settings.json and applied at once through the core (`apply_settings`), which also applies the file on every
// config load, so the values survive `hyprctl reload`. Only the keys changed here are written: anything else keeps coming from the
// user's own config. The key list and limits mirror core/settings.lua (keep them in step; the core validates again).
use gtk4::prelude::*;
use gtk4::{gdk, glib, Application, ApplicationWindow, Box as GtkBox, ColorDialog, ColorDialogButton, DropDown, Entry, Label, Orientation, ScrolledWindow, SpinButton, Switch};
use serde_json::{Map, Value};
use std::process::Command;

enum Kind { Section, Bool(bool), Int(i32, i32, i32), Color(u32), Font, Choice(&'static [(&'static str, &'static str, &'static str)]) }

// (key, English, Portuguese, kind); Int is (min, max, default), Color the default as 0xAARRGGBB, Choice (value, English, Portuguese).
const ROWS: &[(&str, &str, &str, Kind)] = &[
    ("", "Title bars", "Barras de título", Kind::Section),
    ("enabled", "Show title bars", "Mostrar barras de título", Kind::Bool(true)),
    ("floating_only", "Only on floating windows", "Só em janelas flutuantes", Kind::Bool(false)),
    ("bar_height", "Height", "Altura", Kind::Int(10, 80, 28)),
    ("bar_padding", "Padding", "Margem interna", Kind::Int(0, 40, 7)),
    ("show_icon", "App icon", "Ícone do app", Kind::Bool(true)),
    ("bar_title_enabled", "Window title", "Título da janela", Kind::Bool(true)),
    ("bar_text_font", "Title font", "Fonte do título", Kind::Font),
    ("bar_text_size", "Title size", "Tamanho do título", Kind::Int(6, 40, 12)),
    ("bar_text_weight", "Title weight", "Peso do título", Kind::Int(100, 1000, 400)),
    ("bar_text_align", "Title alignment", "Alinhamento do título", Kind::Choice(&[("left", "Left", "Esquerda"), ("center", "Centre", "Centro")])),
    ("bar_buttons_alignment", "Buttons side", "Lado dos botões", Kind::Choice(&[("right", "Right", "Direita"), ("left", "Left", "Esquerda")])),
    ("bar_button_padding", "Button spacing", "Espaço entre botões", Kind::Int(0, 40, 5)),
    ("icon_on_hover", "Button symbols only on hover", "Símbolos dos botões só ao passar o mouse", Kind::Bool(false)),
    ("on_double_click", "Double click on the bar", "Duplo clique na barra", Kind::Choice(&[
        ("maximize", "Maximize", "Maximizar"), ("fullscreen", "Full screen", "Tela cheia"), ("minimize", "Minimize", "Minimizar"),
        ("float", "Float / tile", "Flutuar / ladrilhar"), ("close", "Close", "Fechar"), ("menu", "Window menu", "Menu da janela"), ("none", "Nothing", "Nada")])),
    ("bar_blur", "Blur behind the bar", "Desfoque atrás da barra", Kind::Bool(false)),
    ("bar_part_of_window", "Bar takes space from the window", "A barra ocupa espaço da janela", Kind::Bool(true)),
    ("bar_precedence_over_border", "Bar inside the border", "Barra por dentro da borda", Kind::Bool(true)),
    ("", "Colours", "Cores", Kind::Section),
    ("bar_color", "Bar, focused window", "Barra, janela focada", Kind::Color(0xf726262e)),
    ("inactive_bar_color", "Bar, other windows", "Barra, outras janelas", Kind::Color(0xe01c1c22)),
    ("col.text", "Title, focused window", "Título, janela focada", Kind::Color(0xfff0f0f6)),
    ("inactive_col.text", "Title, other windows", "Título, outras janelas", Kind::Color(0xffb8b8c4)),
    ("button_hover_color", "Button under the pointer", "Botão sob o ponteiro", Kind::Color(0x33ffffff)),
    ("pinned_color", "Pin button while pinned", "Botão fixar, quando fixada", Kind::Color(0x667aa2f7)),
    ("preview_color", "Snap preview", "Prévia do encaixe", Kind::Color(0x407aa2f7)),
    ("", "Snapping", "Encaixe", Kind::Section),
    ("snap_top_edge", "Maximize when dropped on the top edge", "Maximizar ao soltar na borda de cima", Kind::Bool(true)),
    ("snap_preview", "Show where the window will land", "Mostrar onde a janela vai encaixar", Kind::Bool(true)),
    ("snap_assist", "Snap Assist (offer the other windows for the next zone)", "Snap Assist (oferecer as outras janelas para a próxima zona)", Kind::Bool(true)),
    ("seam_resistance", "Push needed to cross to the next monitor (px, 0 = off)", "Empurrão para passar ao outro monitor (px, 0 = desliga)", Kind::Int(0, 2000, 200)),
];

fn pt() -> bool {
    std::env::var("LC_ALL").or_else(|_| std::env::var("LC_MESSAGES")).or_else(|_| std::env::var("LANG")).map_or(false, |l| l.starts_with("pt"))
}

fn file_path() -> std::path::PathBuf {
    let base = std::env::var("XDG_CONFIG_HOME").unwrap_or_else(|_| format!("{}/.config", std::env::var("HOME").unwrap_or_default()));
    std::path::PathBuf::from(base).join("kwinish").join("settings.json")
}

/// 0xAARRGGBB (what the plugin stores) -> "rgba(RRGGBBAA)" (what settings.json and hl.config take).
pub fn argb_to_rgba(v: u32) -> String { format!("rgba({:06x}{:02x})", v & 0xffffff, v >> 24) }
pub fn rgba_to_argb(s: &str) -> Option<u32> {
    let h = s.strip_prefix("rgba(")?.strip_suffix(')')?;
    if h.len() != 8 { return None; }
    let v = u32::from_str_radix(h, 16).ok()?;
    Some((v >> 8) | (v << 24))
}

/// The value the compositor holds now (from the user's config, an earlier save, or the default), or None without the plugin.
fn live(key: &str) -> Option<Value> {
    if key == "snap_assist" { return None; } // core-only, not a compositor option
    let out = Command::new("hyprctl").args(["getoption", &format!("plugin:kwinish:{key}"), "-j"]).output().ok()?;
    let v: Value = serde_json::from_slice(&out.stdout).ok()?;
    v.get("int").or_else(|| v.get("str")).cloned()
}

fn load() -> Map<String, Value> {
    match crate::zones::read_json(&file_path()) { Some(Value::Object(m)) => m, _ => Map::new() }
}

/// Set one key, re-reading the file first so an edit made while the window is open is kept.
/// Lua that applies one change now: through the core's apply_settings when it has it, else straight to the compositor (an older
/// core, e.g. the installed one before the package is updated, has no apply_settings and would make the change wait for a reload).
pub fn live_code(key: &str, v: &Value) -> Option<String> {
    let lit = match v {
        Value::Bool(b) => b.to_string(),
        Value::Number(n) if n.is_i64() => n.to_string(),
        Value::String(t) if t.chars().all(|c| c.is_ascii_alphanumeric() || " -_()".contains(c)) => format!("'{t}'"),
        _ => return None, // the core would drop it too
    };
    let direct = if key == "snap_assist" { format!("if k then k.snap_assist={lit} end") }
                 else { format!("if hl.plugin and hl.plugin.kwinish then hl.config({{plugin={{kwinish={{['{key}']={lit}}}}}}}) end") };
    Some(format!("local k=package.loaded.kwinish; if k and k.apply_settings then k.apply_settings() else {direct} end"))
}

fn save(key: &str, v: Value) {
    let code = live_code(key, &v);
    let mut map = load();
    map.insert(key.to_string(), v);
    let text = serde_json::to_string_pretty(&map).unwrap_or_else(|_| "{}".into());
    if let Err(e) = crate::zones::write_atomic(&file_path(), &text, |t| crate::zones::json(t).is_some_and(|v| v.is_object())) {
        eprintln!("kwinish-ui: cannot write {}: {e}", file_path().display());
        return;
    }
    if let Some(code) = code { let _ = Command::new("hyprctl").args(["eval", &code]).spawn(); }
}

pub fn run() -> glib::ExitCode {
    let app = Application::builder().application_id("dev.kwinish.settings").build();
    app.connect_activate(|app| {
        if let Some(w) = app.active_window() { w.present(); return; }
        let pt = pt();
        let saved = load();
        let list = GtkBox::new(Orientation::Vertical, 6);
        for m in ["top", "bottom", "start", "end"] { list.set_property(&format!("margin-{m}"), 18); }
        for (key, en, pt_label, kind) in ROWS {
            let text = if pt { *pt_label } else { *en };
            if let Kind::Section = kind {
                let h = Label::new(None);
                h.set_markup(&format!("<b>{}</b>", glib::markup_escape_text(text)));
                h.set_xalign(0.0);
                h.set_margin_top(12);
                list.append(&h);
                continue;
            }
            let key = key.to_string();
            let cur = saved.get(&key).cloned().or_else(|| live(&key));
            let row = GtkBox::new(Orientation::Horizontal, 12);
            let label = Label::new(Some(text));
            label.set_xalign(0.0);
            label.set_hexpand(true);
            label.set_wrap(true);
            row.append(&label);
            let set = { let key = key.clone(); move |v: Value| save(&key, v) };
            let widget: gtk4::Widget = match kind {
                Kind::Bool(def) => {
                    let s = Switch::new();
                    s.set_valign(gtk4::Align::Center);
                    s.set_active(cur.and_then(|v| v.as_bool().or(v.as_i64().map(|i| i != 0))).unwrap_or(*def));
                    s.connect_active_notify(move |s| set(Value::Bool(s.is_active())));
                    s.upcast()
                }
                Kind::Int(lo, hi, def) => {
                    let s = SpinButton::with_range(*lo as f64, *hi as f64, 1.0);
                    s.set_value(cur.and_then(|v| v.as_i64()).map_or(*def, |i| i.clamp(*lo as i64, *hi as i64) as i32) as f64);
                    s.connect_value_changed(move |s| set(Value::from(s.value_as_int())));
                    s.upcast()
                }
                Kind::Color(def) => {
                    let argb = cur.and_then(|v| v.as_u64().map(|i| i as u32).or_else(|| v.as_str().and_then(rgba_to_argb))).unwrap_or(*def);
                    let b = ColorDialogButton::new(Some(ColorDialog::new()));
                    let f = |s: u32| ((argb >> s) & 0xff) as f32 / 255.0;
                    b.set_rgba(&gdk::RGBA::new(f(16), f(8), f(0), f(24)));
                    b.connect_rgba_notify(move |b| {
                        let c = b.rgba();
                        let u = |x: f32| (x.clamp(0.0, 1.0) * 255.0).round() as u32;
                        set(Value::from(argb_to_rgba(u(c.alpha()) << 24 | u(c.red()) << 16 | u(c.green()) << 8 | u(c.blue()))));
                    });
                    b.upcast()
                }
                Kind::Font => {
                    let e = Entry::new();
                    e.set_text(cur.as_ref().and_then(|v| v.as_str()).unwrap_or("Sans"));
                    // applied on Enter or when leaving the field, not on every keystroke (a half-typed name is a different font)
                    let set2 = set.clone();
                    e.connect_activate(move |e| set2(Value::from(e.text().to_string())));
                    let focus = gtk4::EventControllerFocus::new();
                    let e2 = e.clone();
                    focus.connect_leave(move |_| set(Value::from(e2.text().to_string())));
                    e.add_controller(focus);
                    e.upcast()
                }
                Kind::Choice(opts) => {
                    let names: Vec<&str> = opts.iter().map(|o| if pt { o.2 } else { o.1 }).collect();
                    let d = DropDown::from_strings(&names);
                    let now = cur.as_ref().and_then(|v| v.as_str()).unwrap_or(opts[0].0);
                    d.set_selected(opts.iter().position(|o| o.0 == now).unwrap_or(0) as u32);
                    d.connect_selected_notify(move |d| set(Value::from(opts[d.selected() as usize].0)));
                    d.upcast()
                }
                Kind::Section => unreachable!(),
            };
            widget.set_valign(gtk4::Align::Center);
            row.append(&widget);
            list.append(&row);
        }
        let note = Label::new(Some(if pt { "As mudanças valem na hora e ficam em ~/.config/kwinish/settings.json." }
                                   else { "Changes apply at once and are kept in ~/.config/kwinish/settings.json." }));
        note.add_css_class("dim-label");
        note.set_wrap(true);
        note.set_margin_top(12);
        list.append(&note);
        let scroll = ScrolledWindow::builder().child(&list).hscrollbar_policy(gtk4::PolicyType::Never).build();
        let win = ApplicationWindow::builder().application(app).title(if pt { "Kwinish: configurações" } else { "Kwinish settings" })
            .default_width(560).default_height(720).child(&scroll).build();
        win.present();
    });
    app.run_with_args::<&str>(&[])
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn colour_roundtrip() {
        assert_eq!(argb_to_rgba(0xf726262e), "rgba(26262ef7)");
        assert_eq!(rgba_to_argb("rgba(26262ef7)"), Some(0xf726262e));
        assert_eq!(rgba_to_argb("rgba(26262e)"), None);
        assert_eq!(rgba_to_argb("red"), None);
    }
    #[test]
    fn live_code_is_plain_lua() {
        assert_eq!(live_code("bar_padding", &Value::from(4)).unwrap(),
            "local k=package.loaded.kwinish; if k and k.apply_settings then k.apply_settings() else if hl.plugin and hl.plugin.kwinish then hl.config({plugin={kwinish={['bar_padding']=4}}}) end end");
        assert!(live_code("bar_color", &Value::from("rgba(26262ef7)")).unwrap().contains("['bar_color']='rgba(26262ef7)'"));
        assert!(live_code("snap_assist", &Value::Bool(false)).unwrap().contains("k.snap_assist=false"));
        assert!(live_code("bar_text_font", &Value::from("x'); os.exit(")).is_none());
    }
}
