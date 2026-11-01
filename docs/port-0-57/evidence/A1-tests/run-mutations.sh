#!/usr/bin/env bash
# alpha-1 (enable-titlebar-drag-alpha1, alt5-rc), same method as GS-tests/run-mutations.sh (achado 4 da revisão):
# each mutation edits a copy and must make its test fail BY AN ASSERTION: a "FALHOU - " line that is not the compile
# step (plugin), or a failed Rust test, not a build error (menu). An unmutated control runs first and must pass. Each
# run's output is kept under $out (default: a temp dir, printed). Exit 0 = controls green and all mutants killed.
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P)
out=${1:-$(mktemp -d "${TMPDIR:-/tmp}/kwinish-a1-mut.XXXXXXXX")}; mkdir -p "$out"; echo "saídas em $out"
apply() { python3 -I -c 'import sys;p,o,n=sys.argv[1:];s=open(p).read();assert s.count(o)==1,"no match";open(p,"w").write(s.replace(o,n))' "$@"; }
killed=0; total=0; n=0
verdict() { # name rc why compile-marker log
  total=$((total + 1))
  if [ "$2" != 0 ] && [ -n "$3" ]; then echo "morta - $1 :: $3"; killed=$((killed + 1))
  elif grep -qE -- "$4" "$5"; then echo "SÓ COMPILAÇÃO - $1 ($5)"
  else echo "VIVA - $1"; fi
}
plug() { # name [file old new]; the file is relative to the repo root
  local name=$1 w="$out/w$n" log="$out/$n.log" rc; n=$((n + 1))
  rm -rf "$w"; mkdir -p "$w"; cp -r "$root/plugin" "$root/core" "$w/"
  if [ $# -gt 1 ]; then apply "$w/$2" "$3" "$4" || { echo "ERRO - $name: não aplicou"; rm -rf "$w"; return 2; }; fi
  KWINISH_SRC="$w/plugin" KWINISH_CORE="$w/core" bash "$root/tools/test-plugin-input.sh" >"$log" 2>&1; rc=$?
  rm -rf "$w"
  [ $# -eq 1 ] && return $rc
  verdict "$name" $rc "$(grep -E '^FALHOU - ' "$log" | grep -v '^FALHOU - compilar' | head -1)" '^FALHOU - compilar' "$log"
}
menu() { # name [old new]
  local name=$1 w="$out/w$n" log="$out/$n.log" rc; n=$((n + 1))
  # ui/src include_str!s ../../core/testdata: copy both, side by side (the old script did not, so the menu never built)
  rm -rf "$w"; mkdir -p "$w/ui"; cp -r "$root/ui/Cargo.toml" "$root/ui/Cargo.lock" "$root/ui/src" "$w/ui/"; cp -r "$root/core" "$w/"
  if [ $# -gt 1 ]; then apply "$w/ui/src/menu.rs" "$2" "$3" || { echo "ERRO - $name: não aplicou"; rm -rf "$w"; return 2; }; fi
  CARGO_TARGET_DIR="$root/ui/target" cargo test --locked --quiet --manifest-path "$w/ui/Cargo.toml" move_and_resize >"$log" 2>&1; rc=$?
  rm -rf "$w"
  [ $# -eq 1 ] && return $rc
  verdict "$name" $rc "$(grep -m1 -E '^test result: FAILED|panicked at' "$log")" '^error(\[E[0-9]+\])?:' "$log"
}
plug "controle" || { echo "controle sem mutação falhou: $out/0.log"; exit 1; }
menu "controle do menu" || { echo "controle do menu falhou: $out/1.log"; exit 1; }
echo "controles: ok"
B=plugin/barInput.hpp D=plugin/barDeco.cpp
plug "barra toma press com drag ativo"  $B '|| e.hit == eHit::OUTSIDE || e.dragActive)' '|| e.hit == eHit::OUTSIDE)'
plug "soltura do drag tomada"           $B $'            if (const int S = slot(button); S >= 0)\n                held[S] = false;' '            (void)button;'
plug "soltura limpa sem esperar o fim"  $B $'            if (dragGoing) {\n                released = true;\n                return eDragStep::WAIT;\n            }' '            (void)dragGoing;'
plug "encaixa sem soltura"              $B $'            if (released) {' $'            if (true) {'
plug "rearm com drag ativo"             $B '!active || !rearm || dragGoing || !overWindow' '!active || !rearm || (dragGoing && false) || !overWindow'
plug "grab válido recusado"             $B $'            return "grab: mode must be move or resize";\n        return "";' $'            return "grab: mode must be move or resize";\n        return "grab: unavailable";'
plug "pedido de fim reintroduzido"      $D $'    const bool STARTED = KwDrag::begin(MBIND_MOVE)' $'    (void)g_layoutManager->endDragTarget();\n    const bool STARTED = KwDrag::begin(MBIND_MOVE)'
plug "float sem guarda"                 $D 'if (!g_layoutManager->dragController()->target()) // floating ends' 'if (true) // floating ends'
plug "par não liberado no início"       $D '        g_pGlobalState->pairs.dragStarted(BTN_LEFT); //' '        (void)0; //'
plug "prévia não desenhada"             $D $'    drawSnapPreview(ctx, pMonitor);\n' ''
plug "fim não observado"                $D '    m_pDragEndedCallback = g_layoutManager->dragController()->m_events.ended.listen([&] { onDragEnded(); });' ''
plug "easing linear"                    $B '1.0 - (1.0 - P) * (1.0 - P) * (1.0 - P); // ease-out' 'P; // ease-out'
plug "arrasto antes do fim da animação" $D $'        if (ST.done)\n            handleMovement(); // the ease is over' $'        handleMovement(); // the ease is over'
plug "soltura sem tamanho final"        $B $'            return at(cx, cy, 1.0);' $'            (void)cx, (void)cy;\n            active = false;\n            return {};'
# achado 5: what the old checks let through (a call commented out, an end request outside barDeco.cpp/main.cpp, the
# floating press start of accept-floating-press-drag)
plug "par só em comentário"             $D '        g_pGlobalState->pairs.dragStarted(BTN_LEFT); //' '        // g_pGlobalState->pairs.dragStarted(BTN_LEFT); //'
plug "pedido de fim no hostDrag"        plugin/hostDrag.hpp $'    inline eBeginResult begin(eMouseBindMode MODE) {' $'    inline eBeginResult begin(eMouseBindMode MODE) {\n        (void)g_layoutManager->endDragTarget();'
plug "pedido de fim no núcleo"          core/kwinish.lua $'local M = {}\n' $'local M = {}\nlocal function stop() hl.dispatch("movewindow") end\n'
plug "flutuante espera o limiar"        $D $'        handleMovement();\n    else\n        m_bDragPending = true;' $'        m_bDragPending = true;\n    else\n        m_bDragPending = true;'
menu "menu sem Mover"                   $'        act(t("Move"), "grab:move"),\n' ''
echo "mutações: $killed/$total mortas por assertiva"; [ "$killed" = "$total" ]
