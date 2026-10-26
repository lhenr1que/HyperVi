# Kwinish

KWin-style window management for **Hyprland 0.55+** (Lua config). Early work, tested on Hyprland 0.56.2 only.

Three layers, each usable without the next:

| Layer | What | Needs |
|---|---|---|
| `core/` (Lua) | quick tile, one-axis maximize, pack to edge, show desktop, keep above, smart placement, auto-raise, hot corners, Alt+Tab logic, default binds | Hyprland with a Lua config |
| `ui/` (Rust, `kwinish-ui`) | Alt+Tab overlay, window menu, fallback title bars | GTK4, gtk4-layer-shell |
| `plugin/` (C++, `kwinish-bars`) | the title bar: a compositor plugin forked from hyprbars | Hyprland headers, librsvg; rebuilt for each Hyprland update |

## Title bars

The plugin is the primary bar: app icon, pin button and title on the left; minimize, maximize and close on the right; drawn
by the compositor, so stacking, animations, borders and rounding are the real ones. Right click opens the window menu,
double click maximizes, the wheel over the bar raises or lowers the window, and dropping a dragged window on the left or
right screen edge snaps it to a half, and on the top edge maximizes it (`snap_top_edge`, on by default). Dragging across the seam between two monitors takes an extra push of `seam_resistance` pixels (default 200, 0 turns it off). Bars show on every window; `floating_only`
limits them to floating ones.

**Snap preview and layouts.** While a window is dragged by its bar, a translucent rectangle shows where releasing it on a screen edge will put it. Holding **Shift** shows every zone of the active tiling layout (two columns, three columns, 2x2 grid, main and side, wide centre) and releasing snaps the window into the zone under the pointer. Pick the layout with `kwinish-ui layouts` (`SUPER + CTRL + T`) or in the window menu. Make your own with the zone editor (`kwinish-ui zones`, `SUPER + CTRL + E`, or "Edit zones..." in the picker): split, add floating blocks, set the padding, load a preset to start from; layouts are saved in `~/.config/kwinish/layouts.json` and show up in the picker. Dragging a snapped window by its bar gives back its old size.

**Window menu.** KWin-style, born from the bar: click the app icon or right-click the bar. Workspaces, maximize, minimize, keep above or below, fullscreen, no frame, hide from captures, snap, tiling layout, close. State shows as check marks.

A plugin is locked to one Hyprland build, so after a Hyprland update the plugin can be stale. Then Kwinish runs in
**emergency mode**: it does not load the plugin, shows a notification, and `kwinish-ui bars` draws fallback bars
(layer-shell surfaces that sit above all windows, so they hide or clip when another window covers them). The pacman hook
in the plugin package rebuilds the plugin after `hyprland` changes; or run `sudo kwinish-bars-rebuild`. Details in
`docs/PLUGIN.md`.

## Try it
> **The plugin alone is not enough.** Snap, the drag preview, Shift-drag zones, the window menu actions and every bind live in
> the Lua core. If `require("kwinish")` is not in your Hyprland config (step 1), dragging a bar only moves the window; the plugin
> shows a one-time warning when it sees a drag with no core. Loading the core with `hyprctl eval` works for tests but is lost on
> every config reload or restart.

1. Core (required): copy or link `core/` to `~/.local/share/kwinish/core`, add `recipes/hyprland-snippet.lua` to your `hyprland.lua`
   (check with `Hyprland --verify-config -c ~/.config/hypr/hyprland.lua`), optionally paste blocks from `recipes/native.lua`.
2. UI: `cargo build --release` in `ui/`, put `kwinish-ui` on your `PATH`, start it from `exec-once` (Alt+Tab overlay) and
   `kwinish-ui bars` for the fallback bars.
3. Plugin: `make -C plugin abi`, then `hyprctl plugin load $PWD/plugin/kwinish-bars.so`. Turn off any other title-bar
   plugin first (hyprbars). Loading a plugin can crash Hyprland if the plugin is wrong; keep a way to restart the session.

Packages (drafts, `makepkg --nodeps` works, nothing published): `packaging/PKGBUILD` (core and UI) and
`packaging/plugin/PKGBUILD` (plugin, rebuild script, pacman hook).

Default binds (`recipes/hyprland-snippet.lua`): `SUPER + ALT + arrows` halves, top and bottom; `Q W A S` corners;
`Return` maximize; `H`/`V` one axis; `CTRL + ALT + arrows` pack; `SUPER + D` show desktop; `ALT + T` keep above; `SUPER + CTRL + T` layout picker; `ALT + Tab`
switcher (`ALT + F3` window menu). They clash with some Ryoku binds: use `mod` and `override`.

## Tests
```
lua5.4 core/test_geometry.lua
lua5.4 core/test_binds.lua
lua5.4 core/test_switcher.lua
lua5.4 core/test_layouts.lua
cargo test --manifest-path ui/Cargo.toml
```
The plugin and the UI are checked by running them on a live session (see `docs/SPIKE-HYPR-WM-KWIN.md` for what was
exercised and what was not).

## Known limits
- Restore state, show-desktop origins and the keep-above list live in Lua tables and are lost on a config reload.
- The fallback bars are layer-shell surfaces: not at window level, and they trail animated windows by up to 80 ms.
- Not tested with several monitors, XWayland-only setups or an upgrade of Hyprland through pacman.
- The name Kwinish is provisional.

## License
MIT, see `LICENSE`. `plugin/` is a fork of hyprbars (BSD 3-Clause, `plugin/LICENSE.hyprland-plugins`, `plugin/NOTICE`).
