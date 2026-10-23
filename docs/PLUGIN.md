# kwinish-bars: the compositor plugin (primary title bars)

Decision (owner, 24/10/2026): the title bar is a Hyprland **decoration plugin**, forked from hyprbars, and the layer-shell
bars (`kwinish-ui bars`) are the fallback. The plugin gives what a layer surface cannot: real window stacking, no clipping,
frame-accurate animations, and the compositor's own borders and rounding.

## Source
`plugin/` is a fork of hyprbars (hyprland-plugins commit `7644cec`, BSD-3-Clause, notice in `plugin/NOTICE`). Builds in
~13 s with `make -C plugin` (needs the Hyprland headers: `pkg-config hyprland`). **Status (24/10/2026): renamed and adapted, builds, never loaded.** Loading a plugin into the compositor needs the
owner's go-ahead each time.

## Modes (implemented in `ui/src/mode.rs`, tested)
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
every Hyprland update needs a rebuild. Options for the user, all compatible with the fallback:
1. **Automatic**, like DKMS: a pacman hook rebuilds the plugin after the Hyprland package changes (about a minute).
2. **Manual**: one command to rebuild; `docs/` is written so an AI agent can do it unattended.
3. **Pause the Hyprland update** until the plugin is rebuilt: the plugin package depends on the exact `hyprland` version
   so pacman holds the upgrade. Opt-in only, because it blocks `pacman -Syu`.
4. **Do nothing**: emergency mode, the system keeps working with the fallback bars.

## What the fork does now (compiled, not yet run)
- Identity: plugin name `kwinish-bars`, config namespace `plugin:kwinish:*` (same keys as hyprbars plus our palette as
  defaults: 28 px, left-aligned bold title, bar over the border), Lua namespace `hl.plugin.kwinish.add_button`, window
  effects `kwinish:no_bar|bar_color|title_color`, decoration name `KwinishBar`. It skips windows that already carry a
  hyprbars bar.
- Buttons run **built-in actions inside the compositor**: `close`, `maximize`, `fullscreen`, `minimize`, `float`, `pin`,
  `raise`, `lower`, `menu`, `exec:<cmd>`. `on_double_click` takes the same names (default `maximize`).
- `add_button` ignores a button with the same icon and action (the duplicated-buttons bug seen with the Hub's live apply).
- Wheel over the bar raises (up) or lowers (down) the window.
- `make -C plugin` builds `kwinish-bars.so`; `make -C plugin abi` writes `kwinish-bars.abi` with the same formula the
  Ryoku package uses (it matched the installed hyprbars receipt byte for byte).
- Unverified without loading: that `fullscreen`, `movetoworkspacesilent` and `exec` exist in the dispatcher table under
  those names (each call is guarded with `contains`, so a missing one does nothing instead of crashing), and the look.

## Open work, in order
1. Load it once, with the owner's ok, and check the look, the buttons and the wheel.
2. Edge snap on drag release (the plugin sees the release in `handleUpEvent`), right-click menu, active/inactive colours,
   rounding-aware corners.
3. Packaging: a separate `kwinish-bars-plugin` package, the pacman rebuild hook, a rebuild command.
