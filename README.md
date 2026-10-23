# kwinish

KWin-style window management for Hyprland 0.55+ (Lua config). Early work; see `docs/PLANO-HYPR-WM-KWIN.md`.

## Status
Core in Lua (`core/`): quick tile, hot corners (`kw.hotcorners`), maximize on one axis, pack to edge, show desktop, keep above, window placement (smart, cascade, center, cursor), auto-raise. Planned: Alt+Tab, window menu, hot corners and a title bar in a Rust UI.
Alt+Tab: `core/switcher.lua` (keys and focus) plus `ui/` (`kwinish-ui`, a Rust GTK4 layer-shell overlay that only draws; build with `cargo build --release` and run it from `exec-once`).

## Try it
1. Copy or link `core/` to `~/.local/share/kwinish/core`.
2. Add `recipes/hyprland-snippet.lua` to your `hyprland.lua` (check with `Hyprland --verify-config -c ~/.config/hypr/hyprland.lua`).
3. Optional: paste blocks from `recipes/native.lua` for features Hyprland already has.

Default binds are `SUPER + ALT + arrows` (halves, top, bottom), `Q W A S` (corners), `Return` (maximize), `H`/`V` (one axis), `CTRL + ALT + arrows` (pack), `SUPER + D` (show desktop), `ALT + T` (keep above). Change them with `override`.

## Tests
```
lua5.4 core/test_geometry.lua
lua5.4 core/test_binds.lua
lua5.4 core/test_switcher.lua
```
Restore state, show-desktop origins and the keep-above list live in Lua tables and are lost on a config reload.
