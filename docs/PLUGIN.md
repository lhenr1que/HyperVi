# kwinish-bars: the compositor plugin (primary title bars)

Decision (owner, 24/10/2026): the title bar is a Hyprland **decoration plugin**, forked from hyprbars, and the layer-shell
bars (`kwinish-ui bars`) are the fallback. The plugin gives what a layer surface cannot: real window stacking, no clipping,
frame-accurate animations, and the compositor's own borders and rounding.

Documentary review: **2026-10-29**, implementation baseline `a2506c6`. Static reading only; no build, plugin load or live test was performed in this consolidation. Recorded tests remain dated in [STATUS](STATUS.md) and the [spike history](history/initial-research/SPIKE-HYPR-WM-KWIN.md). Current architecture and limitations: [ARCHITECTURE](ARCHITECTURE.md).

## Source
`plugin/` is a fork of hyprbars (hyprland-plugins commit `7644cec`, BSD-3-Clause, notice in `plugin/NOTICE`). Builds in the recorded environment in
~13 s with `make -C plugin` (needs the Hyprland headers: `pkg-config hyprland`). **Recorded status:** the spike documents loading and live testing on 24/10, with later owner tests through 27/10. The old "never loaded" status is historical. This does not assert which plugin is loaded now. Loading a plugin into the compositor requires the owner's request; documentary consolidation does not authorize it.

## Modes (implemented in `ui/src/mode.rs`; tests recorded in the spike)
| Mode | When | What happens |
|---|---|---|
| Plugin | `kwinish-bars` is loaded | the layer-shell daemon keeps no bars |
| Other bars | `hyprbars` is loaded | the daemon keeps no bars (a second bar would double up) |
| Try load | plugin installed, its `.abi` commit equals the running Hyprland | the daemon asks Hyprland to load it |
| **Emergency** | plugin installed but built for another Hyprland commit (or no `.abi` receipt) | not loaded, notification once, fallback bars run |
| Fallback | plugin not installed | fallback bars, no notice |

The emergency notice says the plugin was built for Hyprland X while Y runs, that fallback bars may sit above other windows
or be clipped, and that the plugin must be rebuilt. The `.abi` check happens **before** any load attempt, so a stale
plugin is never offered to the compositor. Hyprland also checks the full ABI hash itself when loading.

## Staying in step with Hyprland
A plugin is locked to one Hyprland build (commit plus aquamarine, hyprutils, hyprgraphics, hyprcursor, hyprlang versions);
every Hyprland update needs a rebuild and may require source changes. Existing paths and the earlier opt-in proposal are distinguished below:
1. **Automatic**, like DKMS: a pacman hook rebuilds the plugin after the Hyprland package changes (about a minute).
2. **Manual**: `sudo kwinish-bars-rebuild` exists in the package. Running it is a separate system operation requiring the owner's request, not part of documentation maintenance.
3. **Proposed opt-in hold:** pausing the Hyprland update was an earlier alternative. The hold hook is not implemented; do not assume this package prevents an upgrade.
4. **Emergency fallback**: the implemented mode selection can use fallback bars when the installed plugin is incompatible. Its visual limitations still apply.

The pacman hook and rebuild paths have recorded scratch tests, but an actual Hyprland package upgrade was not validated. Hyprland 0.57 compatibility is a separate investigation; this document does not establish it.

## Implemented features and recorded validation
- Identity: plugin name `kwinish-bars`, config namespace `plugin:kwinish:*` (same keys as hyprbars plus our palette as
  defaults: 28 px, left-aligned bold title, bar over the border), Lua namespace `hl.plugin.kwinish.add_button`, window
  effects `kwinish:no_bar|bar_color|title_color`, decoration name `HyperViBar`. It skips windows that already carry a
  hyprbars bar.
- Buttons run **built-in actions inside the compositor**: `close`, `maximize`, `fullscreen`, `minimize`, `float`, `pin`,
  `raise`, `lower`, `menu`, `exec:<cmd>`. `on_double_click` takes the same names (default `maximize`).
- `add_button` ignores a button with the same icon and action (the duplicated-buttons bug seen with the Hub's live apply).
- Wheel over the bar raises (up) or lowers (down) the window.
- Hover tips (27/10/2026): minimize/maximize/close were verified by the owner under `add-titlebar-button-tooltips`. Pin and icon belong to the still-active `add-pin-and-icon-tooltips`; their text tests/build are recorded, but the targeted live test 10 remains pending. Implemented behavior:
  resting the pointer 700 ms on a `minimize`, `maximize`, `close`, `pin` or `menu` button, or on the app icon, shows its name
  under it, KDE style (Minimizar/Minimize, Maximizar/Maximize, Restaurar/Restore on a maximized window, Fechar/Close,
  Em todas as áreas de trabalho/On all workspaces, Só nesta área de trabalho/Only on this workspace on a pinned window,
  Menu da janela/Window menu for the icon and `menu`; Portuguese when the first non-empty of `LC_ALL`, `LC_MESSAGES`,
  `LANG` starts with `pt`). Drawn after every window (`RENDER_POST_WINDOWS`), hidden by any click until the pointer leaves the button; no
  option. Text and placement live in `tooltip.hpp`, tested by `tools/test-plugin-tooltip.sh`.
- `make -C plugin` builds `kwinish-bars.so`; `make -C plugin abi` writes `kwinish-bars.abi` with the same formula the
  Ryoku package uses (it matched the installed hyprbars receipt byte for byte).
- Dispatcher lookups are guarded with `contains`. The old blanket "unverified without loading" note predates the recorded live tests; it is not a current statement that the plugin has never run. Targeted remaining cases are listed in [STATUS](STATUS.md).

## KWin-style bar (implemented; source decision 24/10/2026)
Decisions from the owner: bars on **all** windows like KWin, with `plugin:kwinish:floating_only` to restrict them; copy
KWin's bar: app icon and a pin button as well as the title.
- Layout (one function, `layoutButtons`, shared by drawing, hover and hit tests): `[app icon][left buttons] title ...
  [right buttons]`. Buttons take an optional `align = "left"|"right"` and `hover_color` in `add_button`.
- App icon: found from the window class through the desktop entries (`StartupWMClass`, file name), then the icon themes
  and pixmaps; PNG through cairo, SVG through librsvg (new dependency `librsvg`). `show_icon = false` turns it off. The
  first icon lookup scans the desktop entries once, inside the compositor.
- Default buttons when the config adds none (`default_buttons`, on): pin at the left, close, maximize, minimize at the
  right, flat Breeze-style glyphs with a hover background (red for close). The pin button shows a highlight while the
  window is pinned. Glyphs are drawn with strokes (`builtin:pin|minimize|maximize|close` as the button icon), so every button has the same size and weight whatever fonts are installed; custom buttons can still use any text as `icon`.
- **Edge snap on drag release**: releasing a bar drag with the pointer within 3 px of a screen edge snaps the window
  (left/right half, top maximized). The window goes back to where the drag began first, so restore returns there. The
  placement itself is done by the Lua core (`kwinish.lua`, `window_action`), spawned through `hyprctl eval`; without the
  core it does nothing.
- **Right click on the bar** opens the window menu (`kwinish-ui menu`).
- Recorded live coverage: title-bar layout, buttons, wheel, active/inactive appearance and snapping have results in the spike/STATUS. Cross-monitor drop, overlapping-bar corrections and other targeted scenarios remain pending; do not generalize those results to every configuration.

## Packaging (implemented package recipes; results are historical)
`packaging/plugin/` builds `kwinish-bars-plugin-git`: the `.so` and its `.abi` receipt in `/usr/lib/hyprland/plugins/`, the
plugin sources in `/usr/share/kwinish/plugin-src`, `/usr/bin/kwinish-bars-rebuild` and a pacman hook
(`kwinish-bars.hook`, PostTransaction on `hyprland` install/upgrade). The hook rebuilds the plugin for the new Hyprland
(about 15 s; needs `gcc`, `make` and the Hyprland headers, listed as optional dependencies) and **never fails the pacman
transaction**: if it cannot build, it prints why, keeps the old plugin, and HyperVi runs in emergency mode. The script is
also the manual route: `sudo kwinish-bars-rebuild`. Tested with the sources and a scratch destination: success wrote a
`.abi` identical to the installed formula; a missing source exited 0 with the message. Not done: the opt-in "hold the
Hyprland update" hook.

## Active and inactive colours (implemented; decision 24/10/2026)
`bar_color` and `col.text` are now the colours of the **focused** window's bar (defaults `#26262e` at 97 % and `#f0f0f6`);
`inactive_bar_color` (`#1c1c22` at 88 %) and `inactive_col.text` (`#b8b8c4`) colour every other bar. Focus changes repaint
all bars and re-render their titles; the bar colour fades through the existing animated colour variable. Window rules
(`kwinish:bar_color`, `kwinish:title_color`) still override both.

## Remaining validation and limitations

The earlier checklist to load the plugin for the first time, implement snapping and add packaging has been superseded by code and recorded tests. The original text is preserved in the source archive recorded by the [history manifest](history/README.md).

Current pending scenarios and complete owner-only test/undo procedures are in [STATUS](STATUS.md): overlapping bars/drag cancellation, cross-monitor drop and pin/icon tooltips. The emergency path was exercised on 24/10 with an empty `.so` and an incompatible receipt; that specific test does not validate an actual compositor package upgrade.

The plugin still depends on Hyprland internals and a symbol hook (`plugin/main.cpp`, plugin initialization); matching a receipt does not prove that a new upstream API or ABI is supported. The existing scene/render and input risks from the 28/10 audit remain technical work, independent of this documentation update.

The audited package inventory of 28/10 records `r172.5333087` installed, superseding the former "not installed" snapshot; no package database was queried in this consolidation. See [STATUS](STATUS.md) for the distinction between repository, installed and loaded code.
