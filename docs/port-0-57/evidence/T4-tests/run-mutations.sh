#!/usr/bin/env bash
# T4 mutations: each mutant must build and then die by assertion (a build error does not count as killed).
# H*: plugin/barInput.hpp under tools/test-plugin-input.sh; S*: plugin sources under its structural check;
# R*: ui/src/menu.rs under cargo test no_move_or_resize_in_alpha0. Usage: run-mutations.sh   (log: stdout)
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P)
w=$(mktemp -d); trap 'rm -rf -- "$w"' EXIT
export CARGO_HOME="${CARGO_HOME:-$HOME/.cargo}" RUSTUP_HOME="${RUSTUP_HOME:-$HOME/.rustup}"
killed=0 total=0 bad=0

# mutate <file> <python-literal old> <python-literal new>: exactly one occurrence must change
mutate() {
  python3 - "$1" "$2" "$3" <<'EOF' || exit 3
import sys, ast
p, old, new = sys.argv[1], ast.literal_eval(sys.argv[2]), ast.literal_eval(sys.argv[3])
s = open(p).read()
if s.count(old) != 1: sys.exit(f"mutation anchor not unique ({s.count(old)}): {old!r}")
open(p, 'w').write(s.replace(old, new))
EOF
}

plug() { # id, file, old, new
  local id=$1; total=$((total + 1))
  rm -rf "$w/p"; cp -r "$root/plugin" "$w/p"; mutate "$w/p/$2" "$3" "$4"
  out=$(KWINISH_SRC="$w/p" bash "$root/tools/test-plugin-input.sh" 2>&1); r=$?
  if grep -q "FALHOU - compilar" <<<"$out"; then echo "BUILD $id (not counted as killed)"; bad=$((bad + 1))
  elif [ "$r" != 0 ] && grep -q "^FALHOU" <<<"$out"; then echo "killed $id: $(grep -m1 '^FALHOU' <<<"$out")"; killed=$((killed + 1))
  else echo "SURVIVED $id"; bad=$((bad + 1)); fi
}

menu() { # id, old, new
  local id=$1; total=$((total + 1))
  rm -rf "$w/ui" "$w/core"; mkdir -p "$w/ui" "$w/core"; cp -r "$root/ui/Cargo.toml" "$root/ui/Cargo.lock" "$root/ui/src" "$w/ui/"; cp -r "$root/core/testdata" "$w/core/"; mutate "$w/ui/src/menu.rs" "$2" "$3"
  out=$(HOME="$w/h" XDG_CONFIG_HOME="$w/h/c" CARGO_TARGET_DIR="$root/ui/target" cargo test --locked --manifest-path "$w/ui/Cargo.toml" no_move_or_resize 2>&1); r=$?
  if grep -q "^error\[" <<<"$out" || grep -q "could not compile" <<<"$out"; then echo "BUILD $id (not counted as killed)"; bad=$((bad + 1))
  elif [ "$r" != 0 ] && grep -q "panicked" <<<"$out"; then echo "killed $id: $(grep -m1 -A1 'panicked' <<<"$out" | tail -1)"; killed=$((killed + 1))
  else echo "SURVIVED $id"; bad=$((bad + 1)); fi
}

# click decisions, pairs and what runs: executed by plugin/test_input.cpp
plug H1-release-not-taken barInput.hpp '"held[S] = false;\n            return {eAction::NONE, true};"' '"held[S] = false;\n            return {};"'
plug H2-right-release-leaks barInput.hpp '"pairs.held[SLOT] = true;"' '"pairs.held[SLOT] = SLOT == 0;"'
plug H3-steals-other-bar barInput.hpp '"|| e.taken || !e.valid"' '"|| !e.valid"'
plug H4-takes-invalid barInput.hpp '"e.taken || !e.valid || "' '"e.taken || "'
plug H5-takes-outside barInput.hpp '" || e.hit == eHit::OUTSIDE)"' '")"'
plug H6-middle-is-left barInput.hpp '"button == BTN_LEFT ? 0"' '"button != BTN_RIGHT ? 0"'
plug H7-button-action-lost barInput.hpp '"return {eAction::BUTTON, true};"' '"return {eAction::FOCUS, true};"'
plug H8-icon-menu-lost barInput.hpp '"return {eAction::ICON_MENU, true};"' '"return {eAction::FOCUS, true};"'
plug H9-always-double barInput.hpp '"now - *lastTitle < DOUBLE_CLICK_WINDOW"' '"true"'
plug H10-double-ignores-option barInput.hpp '"if (doubleClickOn && lastTitle"' '"if ((doubleClickOn || true) && lastTitle"'
plug H11-grab-silent barInput.hpp '"return \"grab: unavailable"' '"return \"\"; (void)\"grab: unavailable"'
plug H12-grab-mode-check-lost barInput.hpp '"if (mode != \"move\" && mode != \"resize\")"' '"if (mode.empty())"'
plug H13-release-twice barInput.hpp '"            held[S] = false;\n"' '""'
plug A1-no-cancel barInput.hpp '"        if (r.cancel)\n            host.cancel();"' '"        (void)r.cancel;"'
plug A2-no-focus barInput.hpp '"        host.focus();\n"' '""'
plug A3-raise-on-menu barInput.hpp '"        if (r.action != eAction::BAR_MENU) // a right"' '"        if (true) // a right"'
plug A4-action-twice barInput.hpp '"        host.run(r.action);\n"' '"        host.run(r.action);\n        host.run(r.action);\n"'
plug A5-acts-on-release barInput.hpp '"        if (r.action == eAction::NONE)\n            return;"' '"        if (false)\n            return;"'
# routes in the plugin sources: killed by the structural inventory (textual, not executed)
plug S1-drag-begin barDeco.cpp '"    KwIn::apply(m_clicks.onPress("' '"    KwDrag::begin(MBIND_MOVE);\n    KwIn::apply(m_clicks.onPress("'
plug S2-bind-end barDeco.cpp '"    KwIn::apply(m_clicks.onPress("' '"    g_pKeybindManager->changeMouseBindMode(MBIND_INVALID);\n    KwIn::apply(m_clicks.onPress("'
plug S3-touch-listener barDeco.cpp '"    m_pMouseAxisCallback"' '"    m_pTouchDownCallback = Event::bus()->m_events.input.touch.down.listen([&](ITouch::SDownEvent, Event::SCallbackInfo&) {});\n    m_pMouseAxisCallback"'
plug S4-preview-drawn barDeco.cpp '"    const auto PWINDOW = m_pWindow.lock();\n\n    if (!PWINDOW->m_ruleApplicator"' '"    drawSnapPreview(pMonitor);\n    const auto PWINDOW = m_pWindow.lock();\n\n    if (!PWINDOW->m_ruleApplicator"'
plug S5-grab-flag globals.hpp '"    std::unordered_set<uintptr_t> hiddenBars;"' '"    bool grabActive = false;\n    std::unordered_set<uintptr_t> hiddenBars;"'
plug S6-grab-bypass main.cpp '"KwIn::grabError("' '"std::string("'
plug S7-click-bypass barDeco.cpp '"KwIn::apply(m_clicks.onPress("' '"(void)(m_clicks.onPress("'
plug S8-seam-hook main.cpp '"    // window keys are pointers and get reused"' '"    HyprlandAPI::createFunctionHook(PHANDLE, nullptr, nullptr);\n    // window keys are pointers and get reused"'
plug S10-release-unhandled main.cpp '"} else if (g_pGlobalState->pairs.onRelease(e.button).cancel)\n            info.cancelled = true;"' '"}"'
plug S9-snap-to-core barDeco.cpp '"    else if (action.starts_with(\"exec:\"))"' '"    else if (action == \"snap\")\n        KwAct::spawn(KwAct::SITE_MENU, \"hyprctl eval x\");\n    else if (action.starts_with(\"exec:\"))"'
# window menu
menu R1-move-back '"        act(t(\"Minimize\"), \"minimize\"),\n"' '"        act(t(\"Minimize\"), \"minimize\"),\n        act(t(\"Move\"), \"grab:move\"),\n"'
menu R2-resize-in-submenu '"        act(t(\"Float / tile this window\"), \"float\"),"' '"        act(t(\"Float / tile this window\"), \"float\"),\n        act(t(\"Resize\"), \"grab:resize\"),"'
menu R3-shortcut-index '"root.insert(3, shortcut_item(st, addr));"' '"root.insert(4, shortcut_item(st, addr));"'
menu R4-action-dropped '"        tog(t(\"Keep below others\"), st.below, \"below\"),\n"' '""'

echo "mutations: $killed/$total killed, $bad not (build or survived); H*/A* by executed assertion, S* by the textual inventory, R* by cargo test"
[ "$bad" = 0 ]
