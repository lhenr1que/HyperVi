#!/usr/bin/env bash
# Alpha-1 title-bar input (openspec enable-titlebar-drag-alpha1, end policy alt5-rc), no Hyprland needed:
#  1. builds and runs plugin/test_input.cpp against plugin/barInput.hpp (click decisions and pairs, yielding to a drag,
#     the bar's drag session, grab messages);
#  2. checks the plugin sources (the code that cannot run without the compositor): no route asks the compositor to end a
#     drag, drags start through the helper, the bar learns the end from the compositor, no touch on the bar.
# KWINISH_SRC=<dir> checks another copy of plugin/ (KWINISH_CORE=<dir> of core/), KWINISH_CXXFLAGS adds compiler flags (both used to show the checks
# fail on the baseline: docs/port-0-57/evidence/T4-tests/run-red.sh).
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
src=${KWINISH_SRC:-$root/plugin}
core=${KWINISH_CORE:-$root/core}
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
present() { # comments ignored: a call commented out is not there
  local file=$1 pat=$2 what=$3
  if sed 's|//.*||' "$src/$file" | grep -qE -- "$pat"; then echo "ok - $what"; else echo "FALHOU - $what ($file)"; fails=$((fails + 1)); fi
}

for f in barDeco.cpp main.cpp; do
  absent "$f" 'changeMouseBindMode|g_pKeybindManager|movewindow|MBIND_INVALID|endDragTarget|dragEnd\(' "alt5-rc: no route asks the compositor to end a drag"
  absent "$f" 'grabActive|grabSwallowPress' "no menu grab state (the compositor ends the grab)"
done
# the same inventory over every plugin source and the Lua core, comments ignored (they name the old calls)
end_pat='changeMouseBindMode|g_pKeybindManager|movewindow|MBIND_INVALID|endDragTarget|dragEnd\('
hits=$( { for f in "$src"/*.cpp "$src"/*.hpp; do case ${f##*/} in test_*) continue ;; esac; sed 's|//.*||' "$f" | grep -nE -- "$end_pat" | sed "s|^|${f##*/}:|"; done
          for f in "$core"/*.lua; do case ${f##*/} in test_*) continue ;; esac; sed 's|--.*||' "$f" | grep -nE -- "$end_pat" | sed "s|^|${f##*/}:|"; done; } )
n=$(ls "$src"/*.cpp "$src"/*.hpp "$core"/*.lua 2>/dev/null | grep -vc '/test_')
if [ "$n" -lt 10 ]; then echo "FALHOU - alt5-rc inventory: only $n source files found"; fails=$((fails + 1))
elif [ -n "$hits" ]; then echo "FALHOU - alt5-rc: no end request anywhere in plugin/ or core/:"; sed 's/^/    /' <<<"$hits"; fails=$((fails + 1))
else echo "ok - alt5-rc: no end request anywhere in plugin/ or core/ ($n files)"; fi
absent barDeco.cpp 'm_bCancelledDown|m_bTouchEv|m_touchId|m_unsnapAnim|stepUnsnap' "no touch state, no unsnap that pauses the compositor's move"
absent barDeco.cpp 'ITouch|input\.touch|onTouch' "no touch on the bar"
absent barDeco.hpp 'ITouch|onTouch|m_bCancelledDown|stepUnsnap' "header: no touch members"
absent globals.hpp 'grabActive|grabSwallowPress' "globals: no grab flags"
present barDeco.cpp 'KwIn::apply\(m_clicks\.onPress\(p, g_pGlobalState->pairs' "onMouseButton decides and acts through barInput.hpp"
present barDeco.cpp '\.dragActive += ' "presses tell barInput.hpp about an active drag (the bar yields)"
present barDeco.cpp 'KwDrag::begin\(MBIND_MOVE\)' "K1: the bar starts its drag through the helper"
present barDeco.cpp 'm_drag\.start\(' "the bar's drag is armed from the helper's result"
present barDeco.cpp 'pairs\.dragStarted\(BTN_LEFT\)' "a started drag lets its release reach the compositor"
present barDeco.cpp 'dragController\(\)->m_events\.ended\.listen' "the bar learns from the compositor that its drag ended"
present barDeco.cpp 'hyprctl eval' "snap on release goes to the core"
present barDeco.cpp 'm_unsnap\.start\(' "unsnap eases (barInput.hpp SUnsnap)"
# the drag starts only once the ease is over (alt5-rc: no pause, so nothing may start it earlier)
if grep -A3 -E 'm_unsnap\.step\(' "$src/barDeco.cpp" | grep -q 'handleMovement()' && grep -B1 -A3 -E 'm_unsnap\.step\(' "$src/barDeco.cpp" | grep -q '\.done'; then echo "ok - the drag starts when the unsnap ease is done"; else echo "FALHOU - the drag must start from m_unsnap.step(...).done"; fails=$((fails + 1)); fi
present main.cpp 'g_pGlobalState->pairs\.onRelease\(e\.button\)\.cancel' "releases are taken by the listener that outlives the bars"
present main.cpp 'KwIn::grabError\(' "luaGrab checks its arguments through barInput.hpp"
present main.cpp 'KwDrag::begin\(mode == "move"' "K2: the menu grab starts through the helper"
present main.cpp 'KwIn::grabStartError\(' "a grab that did not start is reported (grabRefused through grabStartError)"
present main.cpp 'createFunctionHook' "seam resistance: motion hook"
present main.cpp 'KEY_LEFTSHIFT' "Shift rearm listener"
# the C++ float action yields to an active drag (floating ends any drag in progress)
if grep -A2 -E 'action == "float"' "$src/barDeco.cpp" | grep -q 'dragController()->target()'; then echo "ok - float action guarded by an active drag"; else echo "FALHOU - float action not guarded by dragController()->target()"; fails=$((fails + 1)); fi
# the preview is drawn again while the bar drags
n=$(grep -c 'drawSnapPreview(' "$src/barDeco.cpp")
if [ "$n" -ge 2 ]; then echo "ok - drawSnapPreview is called"; else echo "FALHOU - drawSnapPreview( appears $n times in barDeco.cpp (want the definition and a call)"; fails=$((fails + 1)); fi

# guard-titlebar-drag-start: in each route, the gate comes before any geometry or focus change (comments ignored)
body() { awk -v f="$2" 'index($0, f) == 1 {on = 1} on {sub(/\/\/.*/, ""); print} on && /^}/ {exit}' "$src/$1"; }
# fix-input-ipc: a press or a wheel turn only counts for the bar whose window is under the pointer (not a covered focused one),
# and the shared button symbols keep one texture per monitor scale
s=$(body barDeco.cpp 'void CHyprBar::onMouseButton(' | grep -c 'inputIsValid(true)'); [ "$s" -ge 1 ] && ! body barDeco.cpp 'void CHyprBar::onMouseButton(' | grep -q 'inputIsValid()' && echo "ok - press: only the bar under the pointer takes it" || { echo "FALHOU - press: onMouseButton must call inputIsValid(true), never inputIsValid()"; fails=$((fails + 1)); }
s=$(body barDeco.cpp 'void CHyprBar::onMouseAxis(' | grep -c 'inputIsValid(true)'); [ "$s" -ge 1 ] && echo "ok - wheel: only the bar under the pointer takes it" || { echo "FALHOU - wheel: onMouseAxis must call inputIsValid(true)"; fails=$((fails + 1)); }
present globals.hpp 'unordered_map<float, SP<Render::ITexture>> iconTex' "button symbols: one texture per monitor scale"
present barDeco.cpp 'button\.iconTex\[scale\]' "button symbols: drawn with the texture of this monitor's scale"
before() { # file function first then what
  local b a z
  b=$(body "$1" "$2")
  a=$(grep -n -m1 -F -- "$3" <<<"$b" | cut -d: -f1); z=$(grep -n -m1 -F -- "$4" <<<"$b" | cut -d: -f1)
  if [ -n "$a" ] && [ -n "$z" ] && [ "$a" -lt "$z" ]; then echo "ok - $5"; else echo "FALHOU - $5 ($2: '$3' at ${a:-none}, '$4' at ${z:-none})"; fails=$((fails + 1)); fi
}
before barDeco.cpp 'void CHyprBar::onMouseMove()' 'startGate(' 'applyUnsnap(' "gate before an unsnap step"
before barDeco.cpp 'void CHyprBar::onMouseMove()' 'startGate(' 'fullscreenWindow(' "gate before leaving maximized"
before barDeco.cpp 'void CHyprBar::onMouseMove()' 'startGate(' 'startUnsnap(' "gate before starting an unsnap"
before barDeco.cpp 'void CHyprBar::onMouseButton(' 'startGate(' 'applyUnsnap(' "gate before landing an unsnap on release"
before barDeco.cpp 'void CHyprBar::handleMovement()' 'startGate(' 'fullWindowFocus(' "gate before focusing for the drag"
before barDeco.cpp 'void CHyprBar::handleMovement()' 'startGate(' 'KwDrag::begin(' "gate before starting the drag"
before main.cpp 'int luaGrab(' 'grabStartError(' 'return 0' "grab: every way out goes through grabStartError"
# leaving maximized: the size after fullscreenWindow( is the animation's goal, not its current (still maximized) value,
# or the pointer ends off the bar (live test GS, 01/11/2026)
s=$(body barDeco.cpp 'void CHyprBar::onMouseMove()' | awk '/fullscreenWindow\(/ {f = 1; next} f && /->size\(/ {print; exit}')
if grep -q 'GEOMETRIC_GOAL' <<<"$s"; then echo "ok - leaving maximized reads the restored size (goal)"; else echo "FALHOU - leaving maximized reads the size mid-animation: ${s:-none}"; fails=$((fails + 1)); fi
# accept-floating-press-drag: a floating, unmaximized, unsnapped window starts the drag on the press; the others wait
s=$(body barDeco.cpp 'void CHyprBar::onTitlePress()' | tr -s ' \n' ' ')
if grep -qE 'if \(W->isFloating\(\) && !Fullscreen::controller\(\)->isFullscreen\(W, Fullscreen::FSMODE_MAXIMIZED\) && !snappedHere\(W\)\) handleMovement\(\); else m_bDragPending = true;' <<<"$s"; then
  echo "ok - floating title press starts the drag; tiled, maximized and snapped wait for the threshold"
else echo "FALHOU - onTitlePress: floating must start on the press, the rest set m_bDragPending: ${s:-none}"; fails=$((fails + 1)); fi
n=$(body main.cpp 'int luaGrab(' | grep -c 'return 0')
if [ "$n" = 1 ]; then echo "ok - grab: one success return"; else echo "FALHOU - grab: 'return 0' appears $n times in luaGrab (want 1, after grabStartError)"; fails=$((fails + 1)); fi

[ "$fails" = 0 ] && echo "input: tudo passou" || { echo "input: $fails grupo(s) falharam"; exit 1; }
