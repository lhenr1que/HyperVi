# Kwinish contract (v0, draft)

What other software (a shell, a settings app, a store) may rely on. Anything not listed here is internal and may change.
Facts below were read from the code on 2026-10-26; items marked **TODO** are not documented yet, not absent.

## 1. Lua core: `require("kwinish")`
Loaded from the Hyprland Lua config (see `recipes/hyprland-snippet.lua`). Public functions on the returned table:

| Function | Effect |
|---|---|
| `setup(opts)` | `placement`, `autoraise_ms` |
| `window_action(addr, action)` | the single entry point for UIs, see section 2 |
| `tile(where)`, `maximize_axis(axis)`, `pack(dir)` | keyboard actions |
| `show_desktop()`, `keep_above()`, `keep_below()` | |
| `layout_names()`, `set_layout(name)` | tiling layouts (zones) |
| `set_workspace_mode(ws, mode)`, `cycle_workspace_mode(back)` | tiling / scrolling / floating per workspace |
| `set_scrolling_columns(ws, n)` | 1 to 3 columns per screen |
| `menu()`, `layouts()`, `zone_editor()`, `hotcorners(opts)` | launch the matching `kwinish-ui` view |
| `snap_assist` (boolean field) | set `false` to disable Snap Assist |

## 2. `window_action(addr, action)`
`addr` is a Hyprland window address. `action` is `kind` or `kind:arg`:
`layout:<name>`, `tile:<where>`, `axis:<h|v>`, `pack:<dir>`, `zone:<n>`, `above`, `below`, `float`, `fullscreen`, `maximize`,
`minimize`, `pin`, `close`, `workspace:<n|new>`, `grab:<move|resize>`, `rule:<name>`, `mode:<name>`, `columns:<n>`,
`noframe`, `nocapture`. `tile:*` and `zone:*` accept a monitor suffix, `zone:2@HDMI-A-1`: the rectangle is computed on that monitor (the plugin sends the monitor of the drop); an unknown name falls back to the window's own monitor. Unknown kinds are ignored. `grab:*` needs the plugin.

## 3. Compositor plugin `kwinish-bars` (Lua namespace `hl.plugin.kwinish`)
`add_button`, `set_zone(monitor, name, x, y, w, h)`, `clear_zones`, `set_bar_hidden(address, bool)`, `grab("move"|"resize", address)`,
`remember`, `forget`. Config namespace `plugin:kwinish:*` (`enabled`, `bar_height`, `floating_only`, ...; full list **TODO**, see `docs/PLUGIN.md`).
The plugin is locked to one Hyprland build; `<plugin>.abi` records which.

## 4. Files
Config (user data, `$XDG_CONFIG_HOME/kwinish/`):

| File | Shape |
|---|---|
| `layouts.json` | `[{"name","label","gap"?,"zones":[[x,y,w,h],..]}]`, fractions of the work area |
| `rules.json` | `{"<class>":{"above","below","noframe","nocapture":true,"workspace":N,"rect":[x,y,w,h]}}` |
| `shortcuts.json` | per-class window shortcuts (shape **TODO**) |
| `workspace-modes.json` | workspace to mode |
| `workspace-columns.json` | `{"2":3}`: workspace 2 keeps 3 columns |

Runtime (`$XDG_RUNTIME_DIR/kwinish/`, written by the core, read by `kwinish-ui`): `switcher.json` (`open`, `selected`, `items[]` with `class`, `title`), `assist.json` (`{"generation":N,"monitor":name,"zone":N,"rect":[x,y,w,h],"items":[{"address","class","title"}]}`; generation increases on each atomic publication, including core reloads; core launches are serialized with `flock` on `assist.lock`, obsolete queued launches are skipped, overlays close when their generation is replaced and recheck it and window existence before acting).
Writers must be atomic (temp file plus rename); readers must tolerate a bad file (the core drops bad entries, never crashes).

## 5. Window tags (set by the core, readable by a dock)
`kwinish-above`, `kwinish-below`, `kwinish-noframe`, `kwinish-nocapture`, `kwinish-mode-floated`, `kwinish-rect-X-Y-W-H` (geometry before a mode change).

## 6. Minimized windows (what a dock needs)
`minimize` moves the window to workspace `special:minimized` without following it. A dock lists windows there as minimized
and restores through `focus` (the core restores for real: moves to the active workspace, raises, focuses). **TODO:** a tag or
event that marks "minimized" without relying on the workspace name.

## 7. `kwinish-ui` subcommands
`menu <addr> [--at X,Y]`, `layouts`, `zones [--edit NAME]`, `shortcut <addr>`, `assist`, `bars [--only <prefix>] [--skip a,b]`, none = Alt+Tab overlay (daemon).

## Stability
v0 until beta: names above may still change. From beta, changes to sections 1 to 6 follow semver and are listed in a changelog.
