# Kwinish

KWin-style window management for **Hyprland 0.57** (Lua config). **Alpha**: used and tested live by its author on one machine; not yet packaged in any repository. The plugin is built against the exact Hyprland it runs on, so another Hyprland release needs a rebuild and may need code changes (the 0.56.2 line lives in the git history).

Start with the [documentation index](docs/README.md), [implemented architecture](docs/ARCHITECTURE.md) and [current status](docs/STATUS.md). The [product documentation](docs/product/README.md) describes the future suite; approval of a product decision does not imply implementation.

Three cooperating layers; the Lua core owns window-management policy, and the UI and plugin provide its surfaces:

| Layer | What | Needs |
|---|---|---|
| `core/` (Lua) | Tiling/Scroll/Floating workspace policies, rules, quick tile/zones, one-axis maximize, pack to edge, show desktop, keep above, placement, auto-raise, hot corners, Alt+Tab logic and binds | Hyprland with a Lua config |
| `ui/` (Rust, `kwinish-ui`) | Alt+Tab overlay, window menu, layout picker/editor, Snap Assist, settings, shortcut dialog and fallback title bars | GTK4, gtk4-layer-shell |
| `plugin/` (C++, `kwinish-bars`) | the title bar: a compositor plugin forked from hyprbars | Hyprland headers, librsvg; rebuilt for each Hyprland update |

## Title bars

The plugin is the primary bar: app icon, pin button and title on the left; minimize, maximize and close on the right; drawn
by the compositor, so stacking, animations, borders and rounding are the real ones. Right click opens the window menu,
double click maximizes, the wheel over the bar raises or lowers the window, and dropping a dragged window on the left or
right screen edge snaps it to a half, and on the top edge maximizes it (`snap_top_edge`, on by default). Dragging across the seam between two monitors takes an extra push of `seam_resistance` pixels (default 200, 0 turns it off). Bars show on every window; `floating_only`
limits them to floating ones.

**Snap preview and layouts.** While a window is dragged by its bar, a translucent rectangle shows where releasing it on a screen edge will put it. Holding **Shift** shows every zone of the active tiling layout (two columns, three columns, 2x2 grid, main and side, wide centre) and releasing snaps the window into the zone under the pointer. Pick the layout with `kwinish-ui layouts` (`SUPER + T`) or in the window menu. Make your own with the zone editor (`kwinish-ui zones`, `SUPER + CTRL + E`, or "Edit zones..." in the picker): split, add floating blocks, set the padding, load a preset to start from; layouts are saved in `~/.config/kwinish/layouts.json` and show up in the picker. Dragging a snapped window by its bar gives back its old size.

**Settings.** `kwinish-ui settings` (or "Kwinish settings..." under More actions in the window menu) opens a window with the title-bar, colour and snapping options (and the Snap Assist switch). Changes apply at once and are kept in `~/.config/kwinish/settings.json`, which the core applies on every config load; options you never touch there keep coming from your Hyprland config.

**Window menu.** KWin-style, born from the bar: click the app icon or right-click the bar. Workspaces, maximize, minimize, keep above or below, fullscreen, no frame, hide from captures, snap, tiling layout, close. State shows as check marks.

A plugin is locked to one Hyprland build, so after a Hyprland update the plugin can be stale. Then Kwinish runs in
**emergency mode**: it does not load the plugin, shows a notification, and `kwinish-ui bars` draws fallback bars
(layer-shell surfaces that sit above all windows, so they hide or clip when another window covers them). The pacman hook
in the plugin package rebuilds the plugin after `hyprland` changes; or run `sudo kwinish-bars-rebuild`. Details in
[plugin operations](docs/PLUGIN.md).

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

Packages (Arch, `makepkg` from a clone, nothing published to the AUR yet): `packaging/PKGBUILD` (core and UI) and
`packaging/plugin/PKGBUILD` (plugin, its sources, a rebuild script and a pacman hook that rebuilds it after `hyprland` updates).
A real install, removal and upgrade have not been tested yet.

Default binds with the default `mod = "SUPER"` ([table](core/binds.lua), installed by `recipes/hyprland-snippet.lua`):
`SUPER + ALT + arrows` halves/top/bottom; `SUPER + ALT + Q/W/A/S` corners; `SUPER + ALT + Return` maximize;
`SUPER + ALT + H/V` one axis; `SUPER + CTRL + ALT + arrows` pack; `SUPER + D` show desktop;
`SUPER + ALT + T` keep above; `SUPER + T` layout picker; `SUPER + CTRL + E` zone editor.
The recipe also enables `ALT + Tab` and `ALT + F3` (window menu). Check local conflicts and use `mod`/`override`.

## Tests
```
bash tools/check.sh
```
One command for every Lua suite, the tool tests, the Rust tests and a plugin build, in a temporary HOME; it exits non-zero
on any failure. `--lua-only` runs just the Lua suites. The same command runs in CI on Arch Linux (`.github/workflows/check.yml`). Details: [testing guide](docs/TESTING.md). Tests that need a live session are scripted for a person to run, with their results, in [STATUS](docs/STATUS.md).

## Development records
Changes go through [OpenSpec](openspec/) (proposals, designs, tasks and the accepted specs). `docs/STATUS.md`, `docs/DEVELOPMENT_STATE.md`, `docs/port-0-57/` and `docs/history/` are the author's working records: dated, kept as they were, and written for the author's machine (absolute paths in the live test scripts are deliberate). Most project documentation is in Portuguese.

## Known limits
- Restore geometry and show-desktop origins live in Lua tables and are lost on a core/config reload. Keep-above and keep-below lists are rebuilt from window tags when the core is loaded; this does not make every state reload-safe.
- The fallback bars are layer-shell surfaces: not at window level, and they trail animated windows by up to 80 ms.
- Recorded tests include two monitors, but cross-monitor drops, workspace modes on the second monitor and non-default scale/rotation still have pending scenarios. XWayland-only setups and a real Hyprland package upgrade are not established as validated.
- The name Kwinish is provisional.

## License
MIT, see `LICENSE`. `plugin/` is a fork of hyprbars (BSD 3-Clause, `plugin/LICENSE.hyprland-plugins`, `plugin/NOTICE`).
