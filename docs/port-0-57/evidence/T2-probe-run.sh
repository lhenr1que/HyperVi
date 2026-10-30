#!/bin/bash
# T2 runner (group W): type probe (GCC and Clang, Makefile flags), host-free test of the real onNewWindow
# extracted from plugin/main.cpp, mutations of that body, and exact-line checks on plugin/main.cpp.
# HOME/XDG in a temp dir, no session vars. Exit 0 only if every case gives the expected result.
set -u
E=$(cd "$(dirname "$0")" && pwd); PL=${T2_PLUGIN:-$(cd "$E/../../../plugin" && pwd)} # T2_PLUGIN: run the line checks/host-free on another copy (red run on HEAD)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
INC=$(pkg-config --cflags pixman-1 libdrm hyprland libinput libudev wayland-server xkbcommon cairo librsvg-2.0)
ENV="env -i PATH=/usr/bin:/bin HOME=$T XDG_CONFIG_HOME=$T XDG_CACHE_HOME=$T XDG_DATA_HOME=$T XDG_STATE_HOME=$T"
fail=0
ok() { echo "OK   $*"; }
bad() { echo "FAIL $*"; fail=1; }

# --- 1. type probe ----------------------------------------------------------------------
probe() { # compiler define expect(ok|fail) pattern label
    local out rc
    out=$($ENV $1 -O2 -fPIC -std=c++2b -Wno-c++11-narrowing -fsyntax-only $INC $2 "$E/T2-probe.cpp" 2>&1); rc=$?
    if [ "$3" = ok ] && [ $rc -eq 0 ]; then ok "$5 [$1]"
    elif [ "$3" = fail ] && [ $rc -ne 0 ] && grep -qE "$4" <<<"$out"; then ok "$5 [$1] rejected: $(grep -m1 -E "$4" <<<"$out" | sed 's/.*error: //')"
    else bad "$5 [$1] rc=$rc"; echo "$out" | grep -m5 error; fi
}
for C in g++ clang++; do
    probe $C "" ok "" "probe positive (0.57 forms)"
    probe $C -DPROBE_NEG=1 fail "m_X11DoesntWantBorders" "NEG1 m_X11DoesntWantBorders"
    probe $C -DPROBE_NEG=2 fail "m_windowDecorations" "NEG2 m_windowDecorations"
    probe $C -DPROBE_NEG=3 fail "CUniquePointer" "NEG3 UP handed to addWindowDecoration"
    probe $C -DPROBE_NEG=4 fail "updateWindowDecos" "NEG4 updateWindowDecos"
    probe $C -DPROBE_NEG=5 fail "m_isMapped" "NEG5 m_isMapped (private)"
done

# --- 2. host-free: the real onNewWindow body against a fake 0.57 host ----------------------
extract() { awk '/^static void onNewWindow\(PHLWINDOW window\) \{$/{f=1} f{print} f&&/^\}$/{exit}' "$PL/main.cpp"; }
BODY=$(extract)
[ -n "$BODY" ] || bad "onNewWindow not found in main.cpp"
# hostfree compiler body-file label expect(ok|fail)
# Each fake-destructor variant is compiled first (a compile error is never a kill), then run.
# A variant "kills" only by exit 1 with the test's own "FAIL S<n>" assertion line and no sanitizer report.
# ok   = both variants compile and print ALL OK.
# fail = both compile, and at least one variant kills; the label says which (real-dtor = behavioural; no-erase only = structural).
hostfree() {
    local d=$T/$RANDOM$RANDOM V tag out rc kills="" summary=""; mkdir -p $d; cp "$2" $d/onNewWindow.inc
    for V in "" -DT2_DTOR_NO_ERASE; do
        tag=${V:+no-erase}; tag=${tag:-real-dtor}
        if ! out=$($ENV $1 $V -std=c++2b -Wall -Werror -fsanitize=address,undefined -fno-sanitize-recover=all -I$d $(pkg-config --cflags hyprutils) "$E/T2-hostfree.cpp" -o $d/t-$tag 2>&1); then
            bad "host-free $3 [$1] $tag: does not compile"; echo "$out" | grep -m3 error; return
        fi
        out=$(ASAN_OPTIONS=detect_leaks=1 $d/t-$tag 2>&1); rc=$?
        if grep -qE 'AddressSanitizer|LeakSanitizer|runtime error' <<<"$out"; then bad "host-free $3 [$1] $tag: sanitizer report"; echo "$out" | head -5; return; fi
        if [ $rc -eq 0 ] && grep -qx "ALL OK" <<<"$out"; then summary+="$tag:ALL-OK "
        elif [ $rc -eq 1 ] && grep -q "^FAIL S" <<<"$out"; then kills+="$tag "; summary+="$tag:$(grep -m1 '^FAIL S' <<<"$out") | "
        else bad "host-free $3 [$1] $tag: unexpected rc=$rc"; echo "$out" | head -5; return; fi
    done
    if [ "$4" = ok ] && [ -z "$kills" ]; then ok "host-free $3 [$1]: $summary"
    elif [ "$4" = fail ] && [ -n "$kills" ]; then
        case $kills in real-dtor*) k=behavioural; MB=$((MB+1));; *) k=structural; MS=$((MS+1));; esac
        ok "host-free $3 [$1] killed ($k): $summary"
    else bad "host-free $3 [$1]: $summary"; fi
}
MB=0; MS=0
printf '%s\n' "$BODY" > $T/body.inc
mut() { # id sed-expression description
    printf '%s\n' "$BODY" | sed -E "$2" > $T/m$1.inc
    if cmp -s $T/body.inc $T/m$1.inc; then bad "MUT$1 did not apply ($3)"; return; fi
    for C in g++ clang++; do hostfree $C $T/m$1.inc "MUT$1 $3" fail; done
}
for C in g++ clang++; do hostfree $C $T/body.inc "main.cpp onNewWindow" ok; done
mut 1 's/if \(!HyprlandAPI::addWindowDecoration\((.*)\)\)$/HyprlandAPI::addWindowDecoration(\1); if (false)/' "ignore the false return"
mut 2 '/bars.emplace_back/d; s/^( *)if \(!HyprlandAPI/\1g_pGlobalState->bars.emplace_back(bar);\n\1if (!HyprlandAPI/' "publish before the host answers (0.56.2 order)"
mut 3 's/d->getDisplayName\(\) == "Hyprbar" \|\| //' "no Hyprbar dedupe"
mut 4 's/ \|\| d->getDisplayName\(\) == "KwinishBar"//' "no KwinishBar dedupe"
mut 5 's/if \(!window->backend/if (window->backend/' "inverted no-border test"
mut 6 '/bar->m_self = bar;/d' "m_self not set"
mut 7 's/SP<IHyprWindowDecoration>\(bar\)/std::move(bar)/' "move the bar into the host, publish a moved-from pointer"
mut 8 '/^ *g_pGlobalState->bars.emplace_back\(bar\);$/d' "never published"

# --- 3. exact lines in plugin/main.cpp -------------------------------------------------------
EXPECT='static void onNewWindow(PHLWINDOW window) {
    if (!window->backend().traits().suggestsNoBorder) {
        if (std::ranges::any_of(window->presentation().decorations(), [](const auto& d) { return d->getDisplayName() == "Hyprbar" || d->getDisplayName() == "KwinishBar"; }))
            return;

        auto bar    = makeShared<CHyprBar>(window);
        bar->m_self = bar;
        // publish only what the host took; on failure the bar dies here, before anyone saw it
        if (!HyprlandAPI::addWindowDecoration(PHANDLE, window, SP<IHyprWindowDecoration>(bar)))
            return;
        g_pGlobalState->bars.emplace_back(bar);
    }
}'
chk() { if eval "$2"; then ok "$1"; else bad "$1"; fi; }
chk "main.cpp: onNewWindow exact body" '[ "$BODY" = "$EXPECT" ]'
chk "main.cpp: the only bars.emplace_back is in onNewWindow" '[ "$(grep -c "bars.emplace_back" $PL/main.cpp)" = 1 ]'
chk "main.cpp: onUpdateWindowRules uses presentation().updateDecorations()" '[ "$(grep -A8 "^static void onUpdateWindowRules" $PL/main.cpp | grep -cx "    window->presentation().updateDecorations();")" = 1 ]'
chk "main.cpp: pluginInit skips hidden or unmapped windows via mapped()" '[ "$(grep -cx "        if (w->isHidden() || !w->mapped())" $PL/main.cpp)" = 1 ]'
noMatch() { grep -nE "$1" "$PL/main.cpp"; [ $? -eq 1 ]; }
chk "main.cpp: no 0.56.2 window forms left" 'noMatch "m_X11DoesntWantBorders|m_windowDecorations|updateWindowDecos|->m_isMapped|makeUnique<CHyprBar>"'

echo "mutation kills (compiler runs): behavioural=$MB structural-only=$MS"
[ $fail -eq 0 ] && echo "ALL OK" || echo "FAILURES"
exit $fail
