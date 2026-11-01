#!/usr/bin/env bash
# Q1 counter + T9 recheck: each mutation edits a copy of plugin/ and must make its test fail. Exit 0 = all killed.
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P)
killed=0; total=0
mut() { # name file python-old python-new test
  local name=$1 file=$2 old=$3 new=$4 test=$5 w; total=$((total + 1))
  w=$(mktemp -d); cp -r "$root/plugin" "$w/plugin"
  if ! python3 -c 'import sys;p,o,n=sys.argv[1:];s=open(p).read();assert s.count(o)==1,"no match";open(p,"w").write(s.replace(o,n))' "$w/plugin/$file" "$old" "$new"; then
    echo "ERRO - $name: mutação não aplicou"; rm -rf "$w"; return
  fi
  if KWINISH_SRC="$w/plugin" bash "$root/tools/$test" >/dev/null 2>&1; then echo "VIVA - $name"; else echo "morta - $name"; killed=$((killed + 1)); fi
  rm -rf "$w"
}
L=test-plugin-lifecycle.sh; H=test-plugin-hostdrag.sh
mut "Q1 ctor sem ++"        passLife.hpp $'        SLife() {\n            ++g_live;' $'        SLife() {\n' $L
mut "Q1 cópia sem ++"       passLife.hpp $'        SLife(const SLife&) {\n            ++g_live;' $'        SLife(const SLife&) {\n' $L
mut "Q1 dtor sem --"        passLife.hpp '            --g_live;' '' $L
mut "Q1 barra sem membro"   BarPassElement.hpp '    KwPass::SLife life;' '' $L
mut "Q1 preview sem membro" PreviewPassElement.hpp '    KwPass::SLife life;' '' $L
mut "Q1 exit sem log"       main.cpp $'    if (KwPass::g_live != 0)\n' $'    if (false)\n' $L
mut "T9 sem recheck"        hostDrag.hpp $'            if (g_layoutManager->dragController()->target())\n                return BEGIN_ALREADY_DRAGGING;\n        }' '        }' $H
mut "T9 recheck inicia"     hostDrag.hpp $'            if (g_layoutManager->dragController()->target())\n                return BEGIN_ALREADY_DRAGGING;\n        }' $'            if (g_layoutManager->dragController()->target())\n                return BEGIN_STARTED;\n        }' $H
mut "T9 recheck depois"     hostDrag.hpp $'            if (g_layoutManager->dragController()->target())\n                return BEGIN_ALREADY_DRAGGING;\n        }\n\n        g_layoutManager->beginDragTarget(PWINDOW->layoutTarget(), MODE);' $'        }\n\n        g_layoutManager->beginDragTarget(PWINDOW->layoutTarget(), MODE);\n        if (g_layoutManager->dragController()->target())\n            return BEGIN_ALREADY_DRAGGING;' $H
mut "T9 API inexistente"    hostDrag.hpp 'g_layoutManager->beginDragTarget(' 'g_layoutManager->beginDragTargetX(' $H
echo "mutações: $killed/$total mortas"; [ $killed = $total ]
