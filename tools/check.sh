#!/usr/bin/env bash
# The one local validation command (and what CI will run): every core/test_*.lua, every tools/test-*.sh,
# the Rust tests and a plugin build. Exits 1 if anything fails, a tool is missing or no suite is found; 2 on bad usage.
# It never talks to the running desktop: tests get a temporary HOME/XDG and no session sockets. Not a security sandbox.
#   bash tools/check.sh             everything
#   bash tools/check.sh --lua-only  only the Lua suites (packaging check())
#   LUA=lua5.4 bash tools/check.sh  another interpreter (default: lua, the 5.5 the installed Hyprland links)
set -uo pipefail

lua_only=
case "$#:${1-}" in
  0:) ;;
  1:--lua-only) lua_only=1 ;;
  *) echo "uso: bash tools/check.sh [--lua-only]" >&2; exit 2 ;;
esac

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
tmp=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-check.XXXXXXXX") || exit 1
trap 'rm -rf -- "$tmp"' EXIT

# Cargo keeps its registry (and rustup its toolchains) under the real HOME: pin them before HOME moves.
export CARGO_HOME="${CARGO_HOME:-$HOME/.cargo}" RUSTUP_HOME="${RUSTUP_HOME:-$HOME/.rustup}"
export HOME="$tmp/home" XDG_CONFIG_HOME="$tmp/config" XDG_DATA_HOME="$tmp/data" XDG_STATE_HOME="$tmp/state" \
  XDG_CACHE_HOME="$tmp/cache" XDG_RUNTIME_DIR="$tmp/runtime"
unset HYPRLAND_INSTANCE_SIGNATURE WAYLAND_DISPLAY DISPLAY DBUS_SESSION_BUS_ADDRESS
mkdir -p "$HOME" "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$XDG_STATE_HOME" "$XDG_CACHE_HOME" "$XDG_RUNTIME_DIR" || exit 1
cd "$root" || exit 1

failed=()
fail() { echo "FALHOU: $1" >&2; failed+=("$1"); }
need() { command -v "$1" >/dev/null || { fail "$2 (falta o comando $1)"; return 1; }; }

# run_each <stage> <interpreter> <glob>: every match runs even after a failure; no match at all is a failure.
run_each() {
  local t n=0
  for t in $3; do
    [ -e "$t" ] || break
    n=$((n + 1))
    "$2" "$t" || fail "$t"
  done
  [ "$n" -gt 0 ] || fail "nenhum teste $3"
  echo "== $1: $n arquivo(s)"
}

lua=${LUA:-lua}
echo "== Lua"
need "$lua" "suítes Lua" && { "$lua" -v; run_each "Lua" "$lua" "core/test_*.lua"; }

if [ -z "$lua_only" ]; then
  echo "== tools"
  run_each "tools" bash "tools/test-*.sh"

  echo "== Rust"
  need cargo "testes Rust" && { cargo test --locked --manifest-path ui/Cargo.toml || fail "cargo test"; }

  # Build into the temp dir: it never rewrites a .so the compositor may have mapped, and it always compiles.
  echo "== plugin"
  need make "plugin" && { make -C plugin TARGET="$tmp/kwinish-bars.so" || fail "make -C plugin"; }
fi

if [ "${#failed[@]}" -gt 0 ]; then
  echo "check: ${#failed[@]} falha(s):" >&2
  printf '  %s\n' "${failed[@]}" >&2
  exit 1
fi
echo "check: tudo passou${lua_only:+ (só Lua)}"
