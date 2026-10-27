#!/usr/bin/env bash
# Save, then restore, the paths a live test is about to change, so "undo" puts back exactly what was there:
# a file's bytes and mode, a symlink itself (even a broken one), or nothing (then only what the test created goes).
#   bash tools/live-backup.sh save <state dir> <absolute path>...   refuses if <state dir> exists; fails before the test touches anything
#   bash tools/live-backup.sh restore <state dir>                   deletes <state dir> once everything is back
set -euo pipefail
die() { echo "live-backup: $*" >&2; exit 1; }
mark=kwinish-live-backup

save() {
  [ $# -ge 2 ] || die "uso: save <diretório de estado> <caminho absoluto>..."
  local dir=$1 p i=0; shift
  for p; do
    [[ $p == /* ]] || die "caminho não absoluto: $p"
    [ -L "$p" ] || [ ! -d "$p" ] || die "diretório não suportado: $p"
  done
  mkdir -p -- "$(dirname -- "$dir")"
  mkdir -- "$dir" || die "$dir já existe: restaure ou confira esse backup antes"
  for p; do
    i=$((i + 1))
    printf '%s' "$p" > "$dir/$i.path"
    if [ -L "$p" ] || [ -e "$p" ]; then cp -a -- "$p" "$dir/$i.data"; else : > "$dir/$i.absent"; fi
  done
  : > "$dir/$mark" # written last: a backup interrupted halfway is never restored
  echo "live-backup: $i caminho(s) guardado(s) em $dir"
}

restore() {
  [ $# -eq 1 ] || die "uso: restore <diretório de estado>"
  local dir=$1 f i p
  [ -f "$dir/$mark" ] || die "$dir não é um backup completo do live-backup; nada foi alterado"
  for f in "$dir"/*.path; do
    i=${f##*/}; i=${i%.path}
    p=$(<"$f")
    if [ -e "$dir/$i.absent" ]; then
      if [ -L "$p" ] || [ -f "$p" ]; then rm -f -- "$p"; elif [ -e "$p" ]; then die "$p não existia e agora não é arquivo nem link; não removo"; fi
    else
      cp -a -- "$dir/$i.data" "$p.live-backup.$$"
      mv -fT -- "$p.live-backup.$$" "$p"
    fi
  done
  rm -rf -- "$dir"
  echo "live-backup: restaurado"
}

case "${1-}" in
  save|restore) "$@" ;;
  *) die "uso: save <diretório de estado> <caminho>... | restore <diretório de estado>" ;;
esac
