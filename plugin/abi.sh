#!/bin/sh
# Prints the ABI receipt for the installed Hyprland: its commit plus major.minor of the libraries it was built with.
# Same formula as Hyprland's client hash; kwinish-ui compares the first field with the running compositor's commit.
h="$(pkg-config --variable=prefix hyprland)/include/hyprland/src/version.h"
[ -f "$h" ] || h="$(pkg-config --variable=prefix hyprland)/hyprland/src/version.h"
v() { sed -n "s/^#define $1 *\"\([^\"]*\)\".*/\1/p" "$h" | sed 's/\.[^.]*$//'; }
printf '%s_aq_%s_hu_%s_hg_%s_hc_%s_hlg_%s\n' \
  "$(sed -n 's/^#define GIT_COMMIT_HASH *"\([^"]*\)".*/\1/p' "$h")" \
  "$(v AQUAMARINE_VERSION)" "$(v HYPRUTILS_VERSION)" "$(v HYPRGRAPHICS_VERSION)" "$(v HYPRCURSOR_VERSION)" "$(v HYPRLANG_VERSION)"
