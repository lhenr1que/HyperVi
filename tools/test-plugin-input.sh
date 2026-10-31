#!/usr/bin/env bash
# Alpha-0 title-bar input (openspec disable-titlebar-drag-alpha0), no Hyprland needed:
#  1. builds and runs plugin/test_input.cpp against plugin/barInput.hpp (click decisions and pairs, grab refusal);
#  2. checks the plugin sources for any route left that starts, ends or arms a window drag, touch on the bar, or the
#     menu grab (the code that cannot be run without the compositor).
# KWINISH_SRC=<dir> checks another copy of plugin/, KWINISH_CXXFLAGS adds compiler flags (both used to show the checks
# fail on the baseline: docs/port-0-57/evidence/T4-tests/run-red.sh).
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
src=${KWINISH_SRC:-$root/plugin}
t=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-test-input.XXXXXXXX") || exit 1
trap 'rm -rf -- "$t"' EXIT
cxx=${CXX:-g++}
command -v "$cxx" >/dev/null || { echo "FALHOU - falta o compilador $cxx"; exit 1; }
fails=0

if "$cxx" -std=c++2b -Wall -Wextra -Werror ${KWINISH_CXXFLAGS:-} -I "$src" "$root/plugin/test_input.cpp" -o "$t/test_input"; then
  "$t/test_input" || fails=$((fails + 1))
else
  echo "FALHOU - compilar plugin/test_input.cpp"; fails=$((fails + 1))
fi

# absent: pattern must not appear in the file
absent() {
  local file=$1 pat=$2 what=$3
  [ -f "$src/$file" ] || { echo "FALHOU - $what: $file missing"; fails=$((fails + 1)); return; }
  if grep -nE -- "$pat" "$src/$file" >"$t/hits"; then
    echo "FALHOU - $what ($file):"; sed 's/^/    /' "$t/hits"; fails=$((fails + 1))
  else
    echo "ok - $what"
  fi
}
present() {
  local file=$1 pat=$2 what=$3
  if grep -qE -- "$pat" "$src/$file"; then echo "ok - $what"; else echo "FALHOU - $what ($file)"; fails=$((fails + 1)); fi
}

for f in barDeco.cpp main.cpp; do
  absent "$f" 'KwDrag::|hostDrag\.hpp' "no drag begin/end helper"
  absent "$f" 'changeMouseBindMode|g_pKeybindManager|movewindow|MBIND_' "no mouse-bind start/end/rearm"
  absent "$f" 'grabActive|grabSwallowPress' "no menu grab state"
  absent "$f" 'holdAtSeam|createFunctionHook|onMouseMoved|m_rearmPending|KEY_LEFTSHIFT' "no seam-resistance hook or Shift rearm"
done
absent barDeco.cpp 'm_bDraggingThis|m_bDragPending|m_bCancelledDown|m_bTouchEv|m_touchId|m_dragStart|m_pressCursor|m_unsnap|m_resist' "no drag/touch/unsnap state"
absent barDeco.cpp 'snapOnRelease|maybeUnsnap|stepUnsnap|handleMovement|handleDownEvent|handleUpEvent' "no drag, unsnap or snap-on-release routes"
absent barDeco.cpp 'ITouch|input\.touch|onTouch' "no touch on the bar"
absent barDeco.cpp 'hyprctl eval' "the bar sends no snap to the core"
absent barDeco.hpp 'ITouch|onTouch|m_bDraggingThis|m_bDragPending|m_bCancelledDown|m_rearmPending|holdAtSeam|rearmDrag|snapOnRelease|maybeUnsnap|stepUnsnap' "header: no drag/touch members"
absent globals.hpp 'grabActive|grabSwallowPress' "globals: no grab flags"
present barDeco.cpp 'KwIn::apply\(m_clicks\.onPress\(p, g_pGlobalState->pairs' "onMouseButton decides and acts through barInput.hpp"
present main.cpp 'g_pGlobalState->pairs\.onRelease\(e\.button\)\.cancel' "releases are taken by the listener that outlives the bars"
present main.cpp 'KwIn::grabError\(' "luaGrab answers through barInput.hpp"
# the preview stays compiled (its render port is group R), but nothing draws it in alpha-0
n=$(grep -c 'drawSnapPreview(' "$src/barDeco.cpp")
if [ "$n" = 1 ]; then echo "ok - drawSnapPreview defined, never called"; else echo "FALHOU - drawSnapPreview( appears $n times in barDeco.cpp (want 1: the definition)"; fails=$((fails + 1)); fi

[ "$fails" = 0 ] && echo "input: tudo passou" || { echo "input: $fails grupo(s) falharam"; exit 1; }
