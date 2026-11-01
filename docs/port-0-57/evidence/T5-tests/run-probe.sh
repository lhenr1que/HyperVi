#!/usr/bin/env bash
# T5 probe: run-probe.sh [plugin dir] (default: the working tree). Compiles probe.cpp with g++ and clang++ (syntax only)
# and checks the plugin sources for 0.56.2 render forms outside PLUGIN_EXIT (which is T6). Exit 0 = all pass.
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P); src=${1:-$root/plugin}
inc=$(pkg-config --cflags pixman-1 libdrm hyprland libinput libudev wayland-server xkbcommon cairo librsvg-2.0)
fails=0
for c in g++ clang++; do
  if $c -std=c++2b -fsyntax-only -Wno-c++11-narrowing $inc -I "$src" "$here/probe.cpp" > "$here/probe-$c.out" 2>&1; then echo "ok - probe compiles ($c)"
  else echo "FALHOU - probe ($c): $(grep -m3 -E 'static assertion|error:' "$here/probe-$c.out" | sed 's/^.*error: //' | tr '\n' ' ')"; fails=$((fails + 1)); fi
done
# old forms; main.cpp's PLUGIN_EXIT body is excluded (T6)
body() { if [ "$1" = main.cpp ]; then sed '/APICALL EXPORT void PLUGIN_EXIT/,$d' "$src/$1"; else cat "$src/$1"; fi; }
for f in barDeco.cpp BarPassElement.cpp PreviewPassElement.cpp main.cpp; do
  if hits=$(body $f | grep -nE 'm_renderPass|m_renderData|->roundingPower\(\)|scissor\((nullptr|titleBarBox)\)|renderRect\([^c]|renderTexture\([^c]|eRenderStage stage\)'); then
    echo "FALHOU - 0.56.2 render forms in $f:"; sed 's/^/    /' <<<"$hits"; fails=$((fails + 1))
  else echo "ok - no 0.56.2 render forms in $f"; fi
done
[ $fails = 0 ] && echo "probe: tudo passou" || { echo "probe: $fails falha(s)"; exit 1; }
