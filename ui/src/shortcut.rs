// `kwinish-ui shortcut <window address>`: capture a key combination and bind it to the window's app (by class); the
// core (core/kwinish.lua) reads ~/.config/kwinish/shortcuts.json when the config loads and focuses the app on that combo.
// Binds are only added while the config loads (adding one from a running eval can crash Hyprland 0.56.2), so saving
// ends with `hyprctl reload`.
use gtk4::prelude::*;
use gtk4::{gio, glib, Application, ApplicationWindow, Box as GtkBox, Label, Orientation, CssProvider};
use gtk4_layer_shell::{KeyboardMode, Layer, LayerShell};
use serde_json::Value;
use std::cell::RefCell;
use std::collections::BTreeMap;
use std::process::Command;
use std::rc::Rc;

// Hyprland's modifier mask bits
const SHIFT: u32 = 1;
const CTRL: u32 = 4;
const ALT: u32 = 8;
const SUPER: u32 = 64;

/// "SUPER + ALT + K" for (SUPER|ALT, "k"); None without a modifier (a bare key would steal typing) or for an odd key name.
pub fn combo(mask: u32, key: &str) -> Option<String> {
    let ok = !key.is_empty() && key.len() <= 12 && key.chars().all(|c| c.is_ascii_alphanumeric());
    if mask & (CTRL | ALT | SUPER) == 0 || !ok { return None }
    let mut parts: Vec<&str> = vec![];
    for (bit, name) in [(SUPER, "SUPER"), (CTRL, "CTRL"), (ALT, "ALT"), (SHIFT, "SHIFT")] { if mask & bit != 0 { parts.push(name) } }
    let key = key.to_uppercase();
    parts.push(&key);
    Some(parts.join(" + "))
}

/// Whether a bind of the main submap already uses this modifier mask and key, from `hyprctl binds -j`.
pub fn conflicts(binds_json: &str, mask: u32, key: &str) -> bool {
    let v: Value = serde_json::from_str(binds_json).unwrap_or_default();
    v.as_array().map_or(false, |a| a.iter().any(|b| {
        b["submap"].as_str().unwrap_or("").is_empty() && b["modmask"].as_u64() == Some(mask as u64)
            && b["key"].as_str().map_or(false, |k| k.eq_ignore_ascii_case(key))
    }))
}

pub fn file_path() -> std::path::PathBuf { crate::zones::file_path().with_file_name("shortcuts.json") }

pub fn load() -> BTreeMap<String, String> {
    let v: Value = std::fs::read_to_string(file_path()).ok().and_then(|s| serde_json::from_str(&s).ok()).unwrap_or_default();
    v.as_object().map(|o| o.iter().filter_map(|(k, v)| Some((k.clone(), v.as_str()?.to_string()))).collect()).unwrap_or_default()
}

pub fn save(map: &BTreeMap<String, String>) -> std::io::Result<()> {
    crate::zones::write_atomic(&file_path(), &serde_json::to_string_pretty(map).unwrap_or_else(|_| "{}".into()), |t| serde_json::from_str::<Value>(t).is_ok())
}

fn t(en: &'static str) -> &'static str {
    let pt = std::env::var("LC_ALL").or_else(|_| std::env::var("LC_MESSAGES")).or_else(|_| std::env::var("LANG")).map_or(false, |l| l.starts_with("pt"));
    if !pt { return en; }
    match en {
        "Press the shortcut for" => "Pressione o atalho para",
        "Esc cancels, Delete removes the current shortcut" => "Esc cancela, Delete remove o atalho atual",
        "needs SUPER, CTRL or ALT" => "precisa de SUPER, CTRL ou ALT",
        "is already used" => "já está em uso",
        "could not read the current binds" => "não consegui ler os atalhos atuais",
        "The Kwinish core is not up to date: reload Hyprland and try again" => "O núcleo do Kwinish não está atualizado: recarregue o Hyprland e tente de novo",
        other => other,
    }
}

// Hyprland handles its binds before any client sees a key, so pressing an existing combination would run it (SUPER+Q closed
// a window) instead of being captured. While the dialog is open the compositor sits in an empty submap, which turns the
// binds off; the dialog leaves it on every way out, and a detached watchdog leaves it anyway if the process dies.
fn submap(name: &str) {
    let _ = Command::new("hyprctl").args(["eval", &format!("hl.dispatch(hl.dsp.submap('{name}'))")]).output();
}

/// True when the compositor is now in the empty submap; false (core too old or not loaded) means keys cannot be captured safely.
fn enter_capture() -> bool {
    submap("kwcapture");
    let on = Command::new("hyprctl").arg("submap").output().map_or(false, |o| String::from_utf8_lossy(&o.stdout).trim() == "kwcapture");
    if !on { return false }
    // only when the compositor is still in kwcapture and no dialog is alive (another one may have started since): an old watchdog must not
    // turn the binds back on under a newer capture
    let _ = Command::new("sh").args(["-c", "sleep 35; if [ \"$(hyprctl submap 2>/dev/null)\" = kwcapture ] && ! pgrep -f 'kwinish-ui shortcut' >/dev/null; then hyprctl eval \"hl.dispatch(hl.dsp.submap('reset'))\" >/dev/null 2>&1; fi"]).spawn();
    true
}

const CSS: &str = "window.kwsc { background: transparent; } .box { background: alpha(#1c1c22, 0.97); border-radius: 12px; padding: 22px 30px; border: 1px solid alpha(#ffffff, 0.12); } .box label { color: #e8e8ee; } .big { font-size: 18px; font-weight: 700; } .dim { opacity: 0.7; font-size: 12px; } .err { color: #ff8585; }";

pub fn run(addr: String) -> glib::ExitCode {
    let clients: Value = serde_json::from_str(&Command::new("hyprctl").args(["clients", "-j"]).output().map(|o| String::from_utf8_lossy(&o.stdout).to_string()).unwrap_or_default()).unwrap_or_default();
    let class = clients.as_array().and_then(|a| a.iter().find(|c| c["address"] == addr.as_str())).and_then(|c| c["class"].as_str()).unwrap_or("").to_string();
    if class.is_empty() || !class.chars().all(|c| c.is_ascii_alphanumeric() || "-._".contains(c)) { return glib::ExitCode::FAILURE; }
    let app = Application::builder().application_id("dev.kwinish.shortcut").flags(gio::ApplicationFlags::NON_UNIQUE).build();
    app.connect_activate(move |app| {
        let provider = CssProvider::new();
        provider.load_from_data(CSS);
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().expect("no display"), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);
        let current = load().get(&class).cloned();
        let boxw = GtkBox::new(Orientation::Vertical, 8);
        boxw.add_css_class("box");
        let title = Label::new(Some(&format!("{} {}", t("Press the shortcut for"), class)));
        title.add_css_class("big");
        let now = Label::new(current.as_deref());
        let hint = Label::new(Some(t("Esc cancels, Delete removes the current shortcut")));
        hint.add_css_class("dim");
        let err = Label::new(None);
        err.add_css_class("err");
        for w in [&title, &now, &err, &hint] { boxw.append(w); }
        let win = ApplicationWindow::builder().application(app).child(&boxw).build();
        win.add_css_class("kwsc");
        win.init_layer_shell();
        win.set_layer(Layer::Overlay);
        win.set_namespace(Some("kwinish-shortcut"));
        win.set_keyboard_mode(KeyboardMode::Exclusive);
        if let Some(m) = crate::mon::gdk_monitor(&crate::mon::at(&crate::mon::all(), crate::mon::cursor().0, crate::mon::cursor().1).name) { win.set_monitor(Some(&m)); }

        if !enter_capture() {
            // the binds are still live, so a key would run its bind: refuse to listen
            title.set_text(t("The Kwinish core is not up to date: reload Hyprland and try again"));
            let a = app.clone();
            glib::timeout_add_seconds_local_once(4, move || a.quit());
            win.present();
            return;
        }

        let class = Rc::new(class.clone());
        let app2 = app.clone();
        let keys = gtk4::EventControllerKey::new();
        let done = Rc::new(RefCell::new(false));
        keys.connect_key_pressed(move |_, key, _, state| {
            use gtk4::gdk::{Key, ModifierType as M};
            if *done.borrow() { return glib::Propagation::Stop }
            if key == Key::Escape { app2.quit(); return glib::Propagation::Stop }
            let name = key.name().map(|n| n.to_string()).unwrap_or_default();
            let is_mod = ["Shift", "Control", "Alt", "Super", "Meta", "Hyper", "ISO_Level3"].iter().any(|m| name.starts_with(m));
            if is_mod { return glib::Propagation::Stop }
            let mut map = load();
            if key == Key::Delete || key == Key::BackSpace {
                map.remove(class.as_str());
            } else {
                let mut mask = 0;
                for (m, bit) in [(M::SHIFT_MASK, SHIFT), (M::CONTROL_MASK, CTRL), (M::ALT_MASK, ALT), (M::SUPER_MASK, SUPER)] { if state.contains(m) { mask |= bit } }
                let Some(c) = combo(mask, &name) else { err.set_text(&format!("{} ({})", t("needs SUPER, CTRL or ALT"), name)); return glib::Propagation::Stop };
                let binds = Command::new("hyprctl").args(["binds", "-j"]).output().map(|o| String::from_utf8_lossy(&o.stdout).to_string()).unwrap_or_default();
                // not being able to read the existing binds is not "no conflict": refuse rather than risk shadowing one
                if !serde_json::from_str::<Value>(&binds).map_or(false, |v| v.as_array().map_or(false, |a| !a.is_empty())) {
                    err.set_text(t("could not read the current binds"));
                    return glib::Propagation::Stop
                }
                // a bind this app already owns is not a conflict
                if map.get(class.as_str()) != Some(&c) && conflicts(&binds, mask, &name) { err.set_text(&format!("{c} {}", t("is already used"))); return glib::Propagation::Stop }
                map.insert(class.to_string(), c);
            }
            if save(&map).is_ok() { submap("reset"); let _ = Command::new("hyprctl").arg("reload").spawn(); }
            *done.borrow_mut() = true;
            app2.quit();
            glib::Propagation::Stop
        });
        win.add_controller(keys);
        let a = app.clone();
        glib::timeout_add_seconds_local_once(30, move || a.quit());
        win.present();
    });
    app.connect_shutdown(|_| submap("reset"));
    app.run_with_args::<&str>(&[])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn combos() {
        assert_eq!(combo(SUPER | ALT, "k").as_deref(), Some("SUPER + ALT + K"));
        assert_eq!(combo(SUPER | CTRL | ALT | SHIFT, "F5").as_deref(), Some("SUPER + CTRL + ALT + SHIFT + F5"));
        assert_eq!(combo(SHIFT, "a"), None, "shift alone would break typing");
        assert_eq!(combo(0, "a"), None);
        assert_eq!(combo(ALT, "bad key!"), None);
    }

    #[test]
    fn conflict_check() {
        let b = r#"[{"submap":"","modmask":64,"key":"K"},{"submap":"resize","modmask":0,"key":"k"}]"#;
        assert!(conflicts(b, SUPER, "k"));
        assert!(!conflicts(b, SUPER | ALT, "k"));
        assert!(!conflicts(b, 0, "k"), "other submaps do not count");
        assert!(!conflicts("garbage", SUPER, "k"));
    }
}
