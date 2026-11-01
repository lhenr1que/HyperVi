#!/usr/bin/env bash
# alpha-1 (enable-titlebar-drag-alpha1, alt5-rc): each mutation edits a copy and must make its test fail. Exit 0 = all killed.
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P)
killed=0; total=0
apply() { python3 -c 'import sys;p,o,n=sys.argv[1:];s=open(p).read();assert s.count(o)==1,"no match";open(p,"w").write(s.replace(o,n))' "$@"; }
plug() { # name file old new
  local w; total=$((total + 1)); w=$(mktemp -d); cp -r "$root/plugin" "$w/plugin"
  apply "$w/plugin/$2" "$3" "$4" || { echo "ERRO - $1: não aplicou"; rm -rf "$w"; return; }
  if KWINISH_SRC="$w/plugin" bash "$root/tools/test-plugin-input.sh" >/dev/null 2>&1; then echo "VIVA - $1"; else echo "morta - $1"; killed=$((killed + 1)); fi
  rm -rf "$w"
}
menu() { # name old new
  local w; total=$((total + 1)); w=$(mktemp -d); cp -r "$root/ui/Cargo.toml" "$root/ui/Cargo.lock" "$root/ui/src" "$w/"; mkdir -p "$w/../core"; 
  apply "$w/src/menu.rs" "$2" "$3" || { echo "ERRO - $1: não aplicou"; rm -rf "$w"; return; }
  if CARGO_TARGET_DIR="$root/ui/target" cargo test --locked --quiet --manifest-path "$w/Cargo.toml" move_and_resize >/dev/null 2>&1; then echo "VIVA - $1"; else echo "morta - $1"; killed=$((killed + 1)); fi
  rm -rf "$w"
}
B=barInput.hpp
plug "barra toma press com drag ativo"  $B '|| e.hit == eHit::OUTSIDE || e.dragActive)' '|| e.hit == eHit::OUTSIDE)'
plug "soltura do drag tomada"           $B $'            if (const int S = slot(button); S >= 0)\n                held[S] = false;' '            (void)button;'
plug "soltura limpa sem esperar o fim"  $B $'            if (dragGoing) {\n                released = true;\n                return eDragStep::WAIT;\n            }' '            (void)dragGoing;'
plug "encaixa sem soltura"              $B $'            if (released) {' $'            if (true) {'
plug "rearm com drag ativo"             $B '!active || !rearm || dragGoing || !overWindow' '!active || !rearm || !overWindow'
plug "grab válido recusado"             $B $'            return "grab: mode must be move or resize";\n        return "";' $'            return "grab: mode must be move or resize";\n        return "grab: unavailable";'
plug "pedido de fim reintroduzido"      barDeco.cpp $'    const bool STARTED = KwDrag::begin(MBIND_MOVE)' $'    (void)g_layoutManager->endDragTarget();\n    const bool STARTED = KwDrag::begin(MBIND_MOVE)'
plug "float sem guarda"                 barDeco.cpp 'if (!g_layoutManager->dragController()->target()) // floating ends' 'if (true) // floating ends'
plug "par não liberado no início"       barDeco.cpp '        g_pGlobalState->pairs.dragStarted(BTN_LEFT); //' '        (void)0; //'
plug "prévia não desenhada"             barDeco.cpp $'    drawSnapPreview(ctx, pMonitor);\n' ''
plug "fim não observado"                barDeco.cpp '    m_pDragEndedCallback = g_layoutManager->dragController()->m_events.ended.listen([&] { onDragEnded(); });' ''
plug "easing linear"                   $B '1.0 - (1.0 - P) * (1.0 - P) * (1.0 - P); // ease-out' 'P; // ease-out'
plug "arrasto antes do fim da animação" barDeco.cpp $'        if (ST.done)\n            handleMovement(); // the ease is over' $'        handleMovement(); // the ease is over'
plug "soltura sem tamanho final"        $B $'            return at(cx, cy, 1.0);' $'            active = false;\n            return {};'
menu "menu sem Mover"                   $'        act(t("Move"), "grab:move"),\n' ''
echo "mutações: $killed/$total mortas"; [ $killed = $total ]
