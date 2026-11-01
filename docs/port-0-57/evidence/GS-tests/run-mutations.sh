#!/usr/bin/env bash
# guard-titlebar-drag-start: each mutation edits a copy of plugin/ and must make tools/test-plugin-input.sh fail BY AN
# ASSERTION (a "FALHOU - " line that is not the compile step). An unmutated control runs first and must pass. Each run's
# output is kept under $out (default: a temp dir, printed). Exit 0 = control green and all mutants killed by assertion.
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P)
out=${1:-$(mktemp -d "${TMPDIR:-/tmp}/kwinish-gs-mut.XXXXXXXX")}; mkdir -p "$out"; echo "saídas em $out"
apply() { python3 -I -c 'import sys;p,o,n=sys.argv[1:];s=open(p).read();assert s.count(o)==1,"no match";open(p,"w").write(s.replace(o,n))' "$@"; }
killed=0; total=0; n=0
run() { # name [file old new]
  local name=$1 w="$out/w$n" log="$out/$n.log"; n=$((n + 1))
  rm -rf "$w"; cp -r "$root/plugin" "$w"
  if [ $# -gt 1 ]; then apply "$w/$2" "$3" "$4" || { echo "ERRO - $name: não aplicou"; rm -rf "$w"; return 2; }; fi
  KWINISH_SRC="$w" bash "$root/tools/test-plugin-input.sh" >"$log" 2>&1; local rc=$?
  rm -rf "$w"
  if [ $# -eq 1 ]; then return $rc; fi
  total=$((total + 1))
  local why; why=$(grep -E '^FALHOU - ' "$log" | grep -v '^FALHOU - compilar' | head -1)
  if [ "$rc" != 0 ] && [ -n "$why" ]; then echo "morta - $name :: $why"; killed=$((killed + 1))
  elif grep -q '^FALHOU - compilar' "$log"; then echo "SÓ COMPILAÇÃO - $name ($log)"
  else echo "VIVA - $name"; fi
}
run "controle" || { echo "controle sem mutação falhou: $out/0.log"; exit 1; }
echo "controle: ok"
B=barInput.hpp D=barDeco.cpp M=main.cpp
run "porta ignora drag ativo"            $B '        if (dragActive)' '        if (dragActive && false)'
run "porta ignora a janela"              $B 'return pointerOnOwnWindow ? eStartGate::GO' 'return (pointerOnOwnWindow || true) ? eStartGate::GO'
run "abort sem efeito"                   $B $'        void abort() {\n            active = false;\n        }' $'        void abort() {\n        }'
run "YIELD não libera a soltura"         $B '            pairs.dragStarted(BTN_LEFT);' '            (void)pairs;'
run "porta não limpa o início pendente"  $B '        pending = false;' '        (void)pending;'
run "grab ignora foco recusado"          $B '        if (!focused)' '        if (!focused && false)'
run "grab volta a sair em silêncio"      $M $'        break;\n    }\n    // every grab' $'        break;\n    }\n    if (!found)\n        return 0;\n    // every grab'
run "porta removida do movimento"        $D $'    if ((m_unsnap.active || m_bDragPending) &&\n        !KwIn::passGate(KwIn::startGate(g_layoutManager->dragController()->target() != nullptr, true), g_pGlobalState->pairs, m_unsnap, m_bDragPending))\n        return;\n' ''
run "porta removida do início"           $D $'    if (!KwIn::passGate(KwIn::startGate(g_layoutManager->dragController()->target() != nullptr, pointerOverWindow()), g_pGlobalState->pairs, m_unsnap,\n                        m_bDragPending)) {' $'    if (false) {'
run "sem porta na soltura"               $D '            if (KwIn::passGate(KwIn::startGate(g_layoutManager->dragController()->target() != nullptr, true), g_pGlobalState->pairs, m_unsnap, m_bDragPending)) {' '            if (true) {'
echo "mutações: $killed/$total mortas por assertiva"; res=$([ "$killed" = "$total" ] && echo 0 || echo 1)
# Known limit, not counted: the barDeco.cpp routes cannot run without the compositor, so their checks are textual (order of
# the gate call). A guard neutralised in place, with its text kept, survives them. Covered only by the live script.
k0=$killed t0=$total
run "limite: porta neutralizada no movimento" $D '    if ((m_unsnap.active || m_bDragPending) &&' '    if (false &&'
run "limite: porta neutralizada no início"    $D '    if (!KwIn::passGate(KwIn::startGate(g_layoutManager->dragController()->target() != nullptr, pointerOverWindow())' '    if (false && !KwIn::passGate(KwIn::startGate(g_layoutManager->dragController()->target() != nullptr, pointerOverWindow())'
killed=$k0 total=$t0
exit "$res"
