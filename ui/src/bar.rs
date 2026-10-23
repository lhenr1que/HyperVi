// SPIKE: draggable title bar as a layer-shell surface above one window. Drag = we move the window ourselves
// through the Hyprland socket, so the button release reaches us (the compositor never reports it to Lua).
use gtk4::prelude::*;
use gtk4::{gio, glib, Application, ApplicationWindow, Box as GtkBox, CssProvider, EventControllerMotion, GestureClick, Label, Orientation};
use gtk4_layer_shell::{Edge, KeyboardMode, Layer, LayerShell};
use std::cell::{Cell, RefCell};
use std::io::{Read, Write};
use std::os::unix::net::UnixStream;
use std::process::Command;
use std::rc::Rc;
use std::time::Instant;

const BAR_H: i32 = 28;

fn hypr(cmd: &str) -> String {
    let dir = std::env::var("XDG_RUNTIME_DIR").unwrap_or_default();
    let sig = std::env::var("HYPRLAND_INSTANCE_SIGNATURE").unwrap_or_default();
    let mut s = match UnixStream::connect(format!("{dir}/hypr/{sig}/.socket.sock")) {
        Ok(s) => s,
        Err(_) => return String::new(),
    };
    let _ = s.write_all(cmd.as_bytes());
    let mut out = String::new();
    let _ = s.read_to_string(&mut out);
    out
}

// Global pointer position straight from the compositor: surface-local coordinates lag behind a surface we are moving.
fn cursor() -> (f64, f64) {
    let v: serde_json::Value = serde_json::from_str(&hypr("j/cursorpos")).unwrap_or_default();
    (v["x"].as_f64().unwrap_or(0.0), v["y"].as_f64().unwrap_or(0.0))
}

fn log(line: String) {
    if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open("/tmp/claude-1000/bar-spike.log") {
        let _ = writeln!(f, "{line}");
    }
}

pub fn run(addr: String) -> glib::ExitCode {
    let app = Application::builder().application_id("dev.kwinish.bar").flags(gio::ApplicationFlags::NON_UNIQUE).build();
    app.connect_activate(move |app| {
        let clients: serde_json::Value = serde_json::from_str(&hypr("j/clients")).unwrap_or_default();
        let w = clients.as_array().and_then(|a| a.iter().find(|c| c["address"] == addr.as_str()).cloned());
        let Some(w) = w else { eprintln!("window not found"); app.quit(); return };
        let mons: serde_json::Value = serde_json::from_str(&hypr("j/monitors")).unwrap_or_default();
        let (mx, my) = (mons[0]["x"].as_i64().unwrap_or(0) as i32, mons[0]["y"].as_i64().unwrap_or(0) as i32);
        let (wx, wy) = (w["at"][0].as_i64().unwrap() as i32, w["at"][1].as_i64().unwrap() as i32);
        let ww = w["size"][0].as_i64().unwrap() as i32;

        let provider = CssProvider::new();
        provider.load_from_data(".bar { background: alpha(#1c1c22, 0.95); border-radius: 10px 10px 0 0; color: #e8e8ee; padding: 0 12px; }");
        gtk4::style_context_add_provider_for_display(&gtk4::gdk::Display::default().unwrap(), &provider, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);
        let bar = GtkBox::new(Orientation::Horizontal, 0);
        bar.add_css_class("bar");
        bar.set_size_request(ww, BAR_H);
        bar.append(&Label::new(w["title"].as_str()));
        let win = ApplicationWindow::builder().application(app).child(&bar).default_width(ww).default_height(BAR_H).build();
        win.init_layer_shell();
        win.set_layer(Layer::Top);
        win.set_namespace(Some("kwinish-bar"));
        win.set_keyboard_mode(KeyboardMode::None);
        win.set_exclusive_zone(-1); // margins count from the output corner, not the bar-reserved area
        win.set_anchor(Edge::Top, true);
        win.set_anchor(Edge::Left, true);
        let place = {
            let win = win.clone();
            move |x: i32, y: i32| {
                win.set_margin(Edge::Left, x - mx);
                win.set_margin(Edge::Top, y - my - BAR_H);
            }
        };
        place(wx, wy);

        // window position at press, pointer (global) at press, current bar origin
        let origin = Rc::new(Cell::new((wx, wy)));
        let start = Rc::new(RefCell::new(None::<((f64, f64), (i32, i32), Instant, u32)>));
        let sel = format!("address:{addr}");

        let click = GestureClick::new();
        {
            let (origin, start) = (origin.clone(), start.clone());
            click.connect_pressed(move |_, _, x, y| {
                *start.borrow_mut() = Some((cursor(), origin.get(), Instant::now(), 0));
                log(format!("press at local {x:.0},{y:.0}"));
            });
        }
        {
            let (origin, start) = (origin.clone(), start.clone());
            click.connect_released(move |_, _, _, _| {
                if let Some((_, _, t0, n)) = start.borrow_mut().take() {
                    log(format!("RELEASE after {} ms, {} moves, window at {:?}", t0.elapsed().as_millis(), n, origin.get()));
                }
            });
        }
        bar.add_controller(click);

        let motion = EventControllerMotion::new();
        {
            let (origin, start) = (origin.clone(), start.clone());
            motion.connect_motion(move |_, _, _| {
                let mut st = start.borrow_mut();
                let Some((g0, (sx, sy), _, n)) = st.as_mut() else { return };
                let g = cursor();
                let (nx, ny) = (*sx + (g.0 - g0.0) as i32, *sy + (g.1 - g0.1) as i32);
                *n += 1;
                hypr(&format!("dispatch hl.dsp.window.move({{ x = {nx}, y = {ny}, window = \"{sel}\" }})"));
                origin.set((nx, ny));
                place(nx, ny);
            });
        }
        bar.add_controller(motion);
        glib::timeout_add_seconds_local_once(60, {
            let app = app.clone();
            move || app.quit()
        });
        win.present();
    });
    app.run_with_args::<&str>(&[])
}
