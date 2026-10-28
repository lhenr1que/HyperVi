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
| `settings.json` | `{"<option>":value}`, the options of `docs/CONFIGURAVEL.md` |
| `shortcuts.json` | `{"<class>":"SUPER + ALT + K"}`: capital letters, digits, `+` and spaces, at most 40 characters |
| `workspace-modes.json` | `{"3":"floating"}`: workspace to `tiling`, `scrolling` or `floating` |
| `workspace-columns.json` | `{"2":3}`: workspace 2 keeps 3 columns |

Runtime (`$XDG_RUNTIME_DIR/kwinish/`, written by the core, read by `kwinish-ui`): `switcher.json` (`open`, `selected`, `items[]` with `class`, `title`), `assist.json` (`{"generation":N,"monitor":name,"zone":N,"rect":[x,y,w,h],"items":[{"address","class","title"}]}`; generation increases on each atomic publication, including core reloads; core launches are serialized with `flock` on `assist.lock`, obsolete queued launches are skipped, overlays close when their generation is replaced and recheck it and window existence before acting). Runtime files are disposable: the core publishes each one whole through `kwjson.write` (temporary file + rename, so `kwinish-ui` never reads half a file; a failed write leaves the previous state) and never backs them up.
Writers must be atomic (temp file plus rename); readers must tolerate a bad file (the core drops bad entries, never crashes).

Writing (the core through `kwjson.write`: `rules.json`, `workspace-modes.json`, `workspace-columns.json`, `layout`): the text is serialized first, written to `<file>.tmp` in the same directory and renamed over the file, so a reader sees the old file or the new one, never a part. A failure at any step (open, write, close, rename) leaves the old file byte for byte, removes the temporary file and is reported: the function returns false and the core writes one line `kwinish: cannot write <file>: <reason>` to stderr. The new text must pass the file's own reader (a file the reader would refuse is never published). Before replacing a JSON file the reader refuses (judged on its first 1 MiB + 1 byte, as the reader reads it: not a JSON object, over a limit, only whitespace), the core copies it byte for byte, in blocks, to `<file>.bak`; an empty file has nothing to keep. An old file that cannot be opened (other than absent), read or copied stops the write. This is per file, not a transaction: a workspace mode, the active layout and the remembered columns take effect in the running session even when their file could not be written (they are back to the file's value after a reload), and two writers of one file still race (the last rename wins).

`kwinish-ui` (`zones::write_atomic`: `layouts.json`, `settings.json`, `shortcuts.json`) follows the same rule: the new text must pass the file's reader, it goes to `<file>.tmp<pid>` (checked with `sync_all`, which reports the write errors closing would drop) + rename, the temporary file removed on any error, and a file its reader refuses (by `read_json` and the file's root, on its first 1 MiB + 1 byte) copied byte for byte to `<file>.bak` first; an existing file it cannot read, or a copy it cannot make, stops the save. A failed save writes `kwinish-ui: cannot write <path>: <reason>` to stderr and applies nothing: the settings window does not send the change to the core, the zone editor stays open with the drawing and its button reads "Could not save", and the shortcut dialog stays open with the reason and does not reload Hyprland.

Reading (the core through `core/kwjson.lua`, `kwinish-ui` through `zones::read_json`, with the same verdicts on `core/testdata/json-cases.txt`): every file above, runtime ones included, is JSON read as data, never run as Lua. RFC 8259: UTF-8, no comments, no trailing commas, finite numbers. At most 1 MiB (the core reads 1 MiB + 1 byte and refuses a larger file) and 32 levels of arrays and objects. A file with a syntax error, over a limit, or whose root is not the shape in the table is ignored whole: no entries, the defaults apply, and the core writes one line `kwinish: <file> ignored: <reason>` to Hyprland's stderr per file and reason. In a valid file only the top-level members of the shape count; an entry of the wrong type or out of range is dropped alone, unknown members are ignored, and a repeated member takes its last value. `kwinish-ui` treats a refused file as absent, copies it to `<name>.bak` before saving over it, and its window menu marks only the rules and modes the core applies. Until 27/10 the core ran `rules.json` and `layouts.json` as Lua and picked the other files' entries out of the text: Lua expressions, entries of truncated files, of a wrong root or nested in another object no longer count.

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
