#!/bin/bash
# T1 probe runner: GCC and Clang, Makefile flags, HOME/XDG in a temp dir, no session vars.
# Exit 0 only if every case gives the expected result (and the expected diagnostic for NEG).
set -u
P=$(cd "$(dirname "$0")" && pwd)/T1-probe.cpp
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
INC=$(pkg-config --cflags pixman-1 libdrm hyprland libinput libudev wayland-server xkbcommon cairo librsvg-2.0)
fail=0
run() { # compiler define expect(ok|fail) pattern label
    local out rc
    out=$(env -i PATH=/usr/bin:/bin HOME=$T XDG_CONFIG_HOME=$T XDG_CACHE_HOME=$T XDG_DATA_HOME=$T XDG_STATE_HOME=$T \
        $1 -O2 -fPIC -std=c++2b -Wno-c++11-narrowing -fsyntax-only $INC $2 "$P" 2>&1); rc=$?
    if [ "$3" = ok ] && [ $rc -eq 0 ]; then echo "OK   $5 [$1]";
    elif [ "$3" = fail ] && [ $rc -ne 0 ] && grep -qE "$4" <<<"$out"; then echo "OK   $5 [$1] rejected: $(grep -m1 -E "$4" <<<"$out" | sed 's/.*error: //')";
    else echo "FAIL $5 [$1] rc=$rc"; echo "$out" | grep -m5 error; fail=1; fi
}
for C in g++ clang++; do
    run $C "" ok "" "positive (0.57 forms, 256-mask oracle)"
    run $C -DPROBE_NEG=1 fail "HL_MODIFIER_SHIFT" "NEG1 old Shift test"
    run $C -DPROBE_NEG=2 fail "CONFIG_LEGACY" "NEG2 CONFIG_LEGACY"
    run $C -DPROBE_NEG=3 fail "cannot convert|no viable conversion" "NEG3 mask as uint32_t"
    for m in 1 2 3 4 5; do run $C -DPROBE_MUT=$m fail "differs from 0.56.2 on some modifier mask" "MUT$m"; done
done
# Production link: the probed forms are the ones in plugin/, as exact lines (review 1 minor 1, review 2 minors 1-3).
PL=$(cd "$(dirname "$0")/../../../plugin" && pwd)
chk() { if eval "$2"; then echo "OK   $1"; else echo "FAIL $1"; fail=1; fi; }
SH='getModsFromAllKBs() & Input::eKeyboardModifiers::HL_MODIFIER_SHIFT) {'
chk "barDeco.cpp:486 exact Shift test" '[ "$(sed -n 486p $PL/barDeco.cpp)" = "            if (g_pInputManager->$SH" ]'
chk "barDeco.cpp:605 exact Shift test" '[ "$(sed -n 605p $PL/barDeco.cpp)" = "    if (g_pInputManager->$SH" ]'
chk "barDeco.cpp: no other getModsFromAllKBs use" '[ "$(grep -c getModsFromAllKBs $PL/barDeco.cpp)" = 2 ]'
noMatch() { grep -rnE --include="*.cpp" --include="*.hpp" "$1" "$PL"; [ $? -eq 1 ]; } # 1 = no match; 2 (I/O error) fails
chk "plugin/: no CONFIG_LEGACY / bare HL_MODIFIER_ / hyprlang button keyword" '[ -d "$PL" ] && noMatch "CONFIG_LEGACY|[^:]HL_MODIFIER_|\"plugin:kwinish:button\""'
LUA=$(printf '%s\n' "" add_button:newLuaButton remember:luaRemember forget:luaForget set_zone:luaSetZone clear_zones:luaClearZones set_bar_hidden:luaSetBarHidden grab:luaGrab |
    sed -E 's/^([a-z_]+):(.*)$/    HyprlandAPI::addLuaFunction(PHANDLE, "kwinish", "\1", ::\2);/')
chk "main.cpp: the 7 Lua functions, exact consecutive block after a blank line, no others" '[ "$(grep -B1 -A6 -F "\"add_button\"" $PL/main.cpp)" = "$LUA" ] && [ "$(grep -c addLuaFunction $PL/main.cpp)" = 7 ]'
[ $fail -eq 0 ] && echo "ALL OK" || echo "FAILURES"
exit $fail
