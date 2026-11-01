#!/usr/bin/env bash
# The drag-start helper (plugin/hostDrag.hpp, P8-B), kept for the alpha-1, no compositor run:
#  1. compiles plugin/test_hostdrag.cpp (syntax only) with g++ and clang++ against the installed Hyprland headers, since
#     no plugin source includes the helper while the title-bar drag is off;
#  2. T9 / H1 (docs/port-0-57/evidence/Q4A1-alt5-contract.md): a decoration's DRAG_START may start a drag itself and still
#     return false; begin() must look at the drag target again after asking the decorations, and leave that drag alone.
#     Checked in the source order (the helper cannot run without the compositor).
# KWINISH_SRC=<dir> checks another copy of plugin/.
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
src=${KWINISH_SRC:-$root/plugin}
fails=0
check() { if eval "$1"; then echo "ok - $2"; else echo "FALHOU - $2"; fails=$((fails + 1)); fi; }

if inc=$(pkg-config --cflags pixman-1 libdrm hyprland libinput libudev wayland-server xkbcommon 2>/dev/null); then
  for c in g++ clang++; do
    command -v "$c" >/dev/null || { echo "FALHOU - falta o compilador $c"; fails=$((fails + 1)); continue; }
    # shellcheck disable=SC2086
    check '"$c" -std=c++2b -fsyntax-only -Wno-c++11-narrowing $inc -I "$src" "$src/test_hostdrag.cpp" 2>&1 | head -n 5 >&2; [ "${PIPESTATUS[0]}" = 0 ]' "hostDrag.hpp compiles against the 0.57 headers ($c)"
  done
else
  echo "FALHOU - pkg-config: headers do Hyprland não encontrados"; fails=$((fails + 1))
fi

body=$(sed -n '/inline eBeginResult begin(/,/^    }$/p' "$src/hostDrag.hpp")
at() { grep -nE -- "$1" <<<"$body" | sed -n "${2:-1}p" | cut -d: -f1; }
deco=$(at 'checkInputOnDecorations\(INPUT_TYPE_DRAG_START'); deco=${deco:-0}
start=$(at 'beginDragTarget\('); start=${start:-0}
recheck=$(grep -nE 'dragController\(\)->target\(\)' <<<"$body" | cut -d: -f1 | awk -v d="$deco" '$1 > d' | head -n 1); recheck=${recheck:-0}
check '[ "$deco" -gt 0 ] && [ "$start" -gt "$deco" ]' "begin() asks the decorations before starting"
check '[ "$recheck" -gt "$deco" ] && [ "$recheck" -lt "$start" ]' "H1: begin() looks at the drag target again after the decorations"
check '[ "$recheck" -gt 0 ] && sed -n "${recheck},$((recheck + 1))p" <<<"$body" | grep -q "return BEGIN_ALREADY_DRAGGING"' "H1: a drag started meanwhile is left alone (BEGIN_ALREADY_DRAGGING)"

[ $fails = 0 ] && echo "hostdrag: tudo passou" || { echo "hostdrag: $fails falha(s)"; exit 1; }
