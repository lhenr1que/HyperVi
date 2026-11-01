#!/usr/bin/env bash
# PLUGIN_EXIT on Hyprland 0.57 (T6), no Hyprland needed: checks the order of plugin/main.cpp's PLUGIN_EXIT body.
#  - no 0.56.2 m_renderPass: 0.57 passes live in the render session and the host clears them on reset();
#  - Q1 (docs/port-0-57/QUIESCENCE.md): context().active() is read first, before the tooltip and its timer are released,
#    and an active session is logged as a violation (unload cannot be vetoed);
#  - the tooltip timer is removed from the event loop before the global SP is dropped; the 3 rule effects are unregistered;
#  - Q1 counter: builds and runs plugin/test_passlife.cpp (plugin/passLife.hpp); both pass classes carry it and
#    PLUGIN_EXIT logs a count left over.
# KWINISH_SRC=<dir> checks another copy of plugin/.
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
src=${KWINISH_SRC:-$root/plugin}
body=$(sed -n '/APICALL EXPORT void PLUGIN_EXIT/,/^}/p' "$src/main.cpp")
fails=0
[ -n "$body" ] || { echo "FALHOU - PLUGIN_EXIT not found in main.cpp"; exit 1; }

# line of the first match of pattern in the body (0 = absent)
at() { grep -nE -m1 -- "$1" <<<"$body" | cut -d: -f1 || true; }
check() { if eval "$1"; then echo "ok - $2"; else echo "FALHOU - $2"; fails=$((fails + 1)); fi; }

active=$(at 'g_pHyprRenderer->context\(\)\.active\(\)'); active=${active:-0}
logq1=$(at 'Log::logger->log\(Log::(ERR|WARN).*Q1'); logq1=${logq1:-0}
tip=$(at 'tipHide\(true\)'); tip=${tip:-0}
rm=$(at 'removeTimer\(g_pGlobalState->tip\.timer\)'); rm=${rm:-0}
reset=$(at 'tip\.timer\.reset\(\)'); reset=${reset:-0}

check '! grep -q m_renderPass <<<"$body"' "no m_renderPass (gone in 0.57)"
check '[ "$active" -gt 0 ]' "Q1: reads context().active()"
check '[ "$logq1" -gt "$active" ]' "Q1: logs an active session as a violation"
check '[ "$active" -gt 0 ] && [ "$active" -lt "$tip" ] && [ "$active" -lt "$rm" ]' "Q1 is read before the tooltip and its timer are released"
check '[ "$tip" -gt 0 ] && [ "$rm" -gt 0 ] && [ "$rm" -lt "$reset" ]' "tooltip hidden, timer removed from the loop before the SP is dropped"
check '[ "$(grep -c unregisterEffect <<<"$body")" = 3 ]' "the 3 rule effects are unregistered"

t=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-test-lifecycle.XXXXXXXX") || exit 1
trap 'rm -rf -- "$t"' EXIT
cxx=${CXX:-g++}
if "$cxx" -std=c++2b -Wall -Wextra -Werror -I "$src" "$src/test_passlife.cpp" -o "$t/test_passlife"; then
  "$t/test_passlife" || fails=$((fails + 1))
else
  echo "FALHOU - compilar plugin/test_passlife.cpp"; fails=$((fails + 1))
fi
for f in BarPassElement.hpp PreviewPassElement.hpp; do
  check 'grep -qE "^ +KwPass::SLife +[a-zA-Z_]+;" "$src/$f"' "Q1: $f counts its elements (KwPass::SLife member)"
done
live=$(at 'KwPass::g_live != 0'); live=${live:-0}
check '[ "$live" -gt 0 ] && grep -qE "Log::logger->log\\(Log::ERR.*KwPass::g_live" <<<"$body"' "Q1: PLUGIN_EXIT logs passes still alive"

# The bar is drawn with the frame's SWindowRenderPresentation, as the native border is (docs/port-0-57/GEOMETRY.md G1): outside
# the monitor scene the host zeroes or swaps the workspace and floating offsets, and live state would misplace the bar.
# Textual (these routes need the compositor), comments ignored.
code() { sed 's|//.*||' "$src/$1"; }
fn() { awk -v f="$2" 'index($0, f) == 1 {on = 1} on {sub(/\/\/.*/, ""); print} on && /^}/ {exit}' "$src/$1"; }
check 'code BarPassElement.hpp | grep -qE "Render::SWindowRenderPresentation +presentation"' "the bar pass keeps a copy of the frame's presentation"
check 'fn barDeco.cpp "void CHyprBar::draw(" | grep -qE "SBarData\\{this, a, presentation\\}"' "draw hands the frame's presentation to the pass"
r=$(fn barDeco.cpp "void CHyprBar::renderPass(")
check '[ -n "$r" ] && ! grep -qE "m_renderOffset|presentation\\(\\)\\.floatingOffset\\(\\)|assignedBoxGlobal\\(\\)" <<<"$r"' "renderPass reads no live workspace or floating offset (nor the live assignedBoxGlobal())"
check 'grep -q "presentation\\.workspaceOffset" <<<"$r" && grep -q "presentation\\.floatingOffset" <<<"$r"' "renderPass places the bar with the presentation's offsets"
check 'fn BarPassElement.cpp "std::optional<CBox> CBarPassElement::boundingBox(" | grep -q "data\\.presentation"' "the pass bounding box uses the same presentation"

[ $fails = 0 ] && echo "lifecycle: tudo passou" || { echo "lifecycle: $fails falha(s)"; exit 1; }
