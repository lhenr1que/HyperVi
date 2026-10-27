#!/usr/bin/env bash
# Tests for tools/live-backup.sh, only inside a temp dir: each case changes a path the way a live test would
# and checks that restore puts back the file, the link (even broken) or the absence that was there before.
set -u
lb=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)/live-backup.sh
t=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-test-live-backup.XXXXXXXX") || exit 1
trap 'chmod -R u+rwx "$t" 2>/dev/null; rm -rf -- "$t"' EXIT
fails=0
is() { if eval "$2"; then echo "ok - $1"; else echo "FALHOU - $1"; fails=$((fails + 1)); fi; }
run() { bash "$lb" "$@" > "$t/out" 2>&1; }

h=$t/home st=$t/state/teste
mkdir -p "$h/.config/kwinish" "$h/.local/bin" "$h/real"
printf '{"bar_height":30}\n' > "$h/.config/kwinish/settings.json"; chmod 640 "$h/.config/kwinish/settings.json"
echo old > "$h/real/kwinish-ui"
ln -s "$h/real/kwinish-ui" "$h/.local/bin/kwinish-ui"
ln -s "$h/nowhere" "$h/.local/bin/broken"
cfg=$h/.config/kwinish/settings.json ui=$h/.local/bin/kwinish-ui br=$h/.local/bin/broken
new1=$h/.config/kwinish/new.json new2=$h/.local/bin/created
before=$(cat "$cfg")

is "save guarda arquivo, link, link quebrado e ausentes" 'run save "$st" "$cfg" "$ui" "$br" "$new1" "$new2"'
# What a live test does: rewrite the file, repoint the link, replace the broken link with a file, create a link and a file.
printf '{"bar_height":34}\n' > "$cfg.tmp" && mv "$cfg.tmp" "$cfg"
ln -sfn "$t/repo-build/kwinish-ui" "$ui"
rm "$br"; echo file > "$br"
ln -s "$t/x" "$new1"; echo made > "$new2"
is "restore termina sem erro" 'run restore "$st"'
is "arquivo volta com o mesmo conteúdo e modo" '[ "$(cat "$cfg")" = "$before" ] && [ "$(stat -c %a "$cfg")" = 640 ] && [ ! -L "$cfg" ]'
is "link volta para o mesmo alvo" '[ -L "$ui" ] && [ "$(readlink "$ui")" = "$h/real/kwinish-ui" ] && [ "$(cat "$h/real/kwinish-ui")" = old ]'
is "link quebrado volta como link quebrado" '[ -L "$br" ] && [ "$(readlink "$br")" = "$h/nowhere" ] && [ ! -e "$br" ]'
is "caminhos ausentes voltam a não existir" '[ ! -e "$new1" ] && [ ! -L "$new1" ] && [ ! -e "$new2" ]'
is "diretório de estado apagado depois" '[ ! -e "$st" ] && [ -d "$t/state" ]'
is "nenhum temporário de restore sobrou" '[ -z "$(find "$h" -name "*.live-backup.*")" ]'

is "restore sem nada para fazer: ausente continua ausente" 'run save "$st" "$new1" && run restore "$st" && [ ! -e "$new1" ]'

run save "$st" "$new1"; mkdir "$new1"; echo keep > "$new1/inside"
is "não remove diretório criado no lugar de um ausente" '! run restore "$st" && [ "$(cat "$new1/inside")" = keep ] && [ -e "$st" ]'
rm -rf "$new1" "$st"

mkdir -p "$st"; echo mine > "$st/kept"
is "save recusa diretório de estado já existente e não o altera" '! run save "$st" "$cfg" && [ "$(ls "$st")" = kept ]'
is "restore recusa diretório alheio e não o apaga" '! run restore "$st" && [ "$(cat "$st/kept")" = mine ]'
rm -rf "$st"

run save "$st" "$cfg"; rm "$st/kwinish-live-backup"; echo changed > "$cfg"
is "restore recusa backup sem a marca e não altera nada" '! run restore "$st" && [ "$(cat "$cfg")" = changed ] && [ -e "$st/1.data" ]'
rm -rf "$st"; printf '%s\n' "$before" > "$cfg"

is "save recusa caminho relativo sem criar estado" '! run save "$st" relative/path && [ ! -e "$st" ]'
is "save recusa diretório sem criar estado" '! run save "$st" "$h/real" && [ ! -e "$st" ]'
is "uso errado falha" '! run && ! run frobnicate && ! run save "$st" && ! run restore'

if [ "$(id -u)" != 0 ]; then # root reads anything, so the copy cannot be made to fail
  echo secret > "$t/unreadable"; chmod 000 "$t/unreadable"
  is "falha ao copiar: save sai com erro e o backup incompleto não restaura" \
    '! run save "$st" "$cfg" "$t/unreadable" && [ ! -e "$st/kwinish-live-backup" ] && ! run restore "$st" && [ -e "$st" ]'
fi

[ "$fails" -eq 0 ] || { echo "test-live-backup: $fails falha(s)"; exit 1; }
echo "test-live-backup: tudo passou"
