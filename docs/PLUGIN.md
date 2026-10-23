# kwinish-bars: the compositor plugin (primary title bars)

Decision (owner, 24/10/2026): the title bar is a Hyprland **decoration plugin**, forked from hyprbars, and the layer-shell
bars (`kwinish-ui bars`) are the fallback. The plugin gives what a layer surface cannot: real window stacking, no clipping,
frame-accurate animations, and the compositor's own borders and rounding.

## Source
`plugin/` is a fork of hyprbars (hyprland-plugins commit `7644cec`, BSD-3-Clause, notice in `plugin/NOTICE`). Builds in
~13 s with `make -C plugin` (needs the Hyprland headers: `pkg-config hyprland`). **Status: builds unchanged; not renamed,
not adapted, not loaded.** Loading a plugin into the compositor needs the owner's go-ahead each time.

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

## Open work, in order
1. Rename to `kwinish-bars`, own config namespace, install as `kwinish-bars.so` plus a `.abi` receipt.
2. Buttons: built-in actions executed in the compositor (`close`, `maximize`, `minimize`, `fullscreen`, `float`, `menu`,
   `exec:<cmd>`), not shell strings. Upstream runs a shell command, which is why the Ryoku buttons silently fail on
   Hyprland 0.56 (they use the old `killactive` syntax; `hyprctl dispatch` now takes Lua).
3. Style: kwinish palette, border-aware bar, wheel, double-click and right-click like the layer-shell bars.
4. Packaging: a separate `kwinish-bars-plugin` package, the pacman rebuild hook, a rebuild command.
