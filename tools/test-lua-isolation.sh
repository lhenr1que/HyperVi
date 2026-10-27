#!/usr/bin/env bash
# Every core/test_*.lua must keep its files in its own temp dir, even when run by hand from a desktop session:
# run each suite with HOME and XDG pointing at empty sentinel dirs and fail if a suite leaves anything there.
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
lua=${LUA:-lua}
command -v "$lua" >/dev/null || { echo "FALHOU: falta o comando $lua" >&2; exit 1; }
s=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-isolation.XXXXXXXX") || exit 1
trap 'rm -rf -- "$s"' EXIT

fails=0 n=0
for t in "$root"/core/test_*.lua; do
  [ -e "$t" ] || break
  n=$((n + 1))
  rm -rf -- "$s/env" && mkdir -p "$s/env"/{home,config,data,state,cache,runtime} || exit 1
  HOME=$s/env/home XDG_CONFIG_HOME=$s/env/config XDG_DATA_HOME=$s/env/data XDG_STATE_HOME=$s/env/state \
    XDG_CACHE_HOME=$s/env/cache XDG_RUNTIME_DIR=$s/env/runtime "$lua" "$t" >/dev/null 2>&1
  left=$(find "$s/env" -mindepth 2 -print -quit)
  if [ -n "$left" ]; then
    echo "FALHOU: ${t#"$root"/} gravou no HOME/XDG herdado: ${left#"$s"/env/}"
    fails=$((fails + 1))
  fi
done
[ "$n" -gt 0 ] || { echo "FALHOU: nenhuma suíte core/test_*.lua"; exit 1; }
[ "$fails" -eq 0 ] || exit 1
echo "isolamento ok: $n suítes não gravaram no HOME/XDG herdado"
