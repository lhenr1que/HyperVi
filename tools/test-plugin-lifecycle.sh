#!/usr/bin/env bash
# PLUGIN_EXIT on Hyprland 0.57 (T6), no Hyprland needed: checks the order of plugin/main.cpp's PLUGIN_EXIT body.
#  - no 0.56.2 m_renderPass: 0.57 passes live in the render session and the host clears them on reset();
#  - Q1 (docs/port-0-57/QUIESCENCE.md): context().active() is read first, before the tooltip and its timer are released,
#    and an active session is logged as a violation (unload cannot be vetoed);
#  - the tooltip timer is removed from the event loop before the global SP is dropped; the 3 rule effects are unregistered.
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

[ $fails = 0 ] && echo "lifecycle: tudo passou" || { echo "lifecycle: $fails falha(s)"; exit 1; }
