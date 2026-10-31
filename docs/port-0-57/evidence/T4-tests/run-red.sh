#!/usr/bin/env bash
# T4 red: tools/test-plugin-input.sh against the plugin sources of a commit (default bf2049a, the T4 base), with the
# click decisions of that commit (baseline-barInput.hpp, a hand extraction). Must exit 1 by assertion, never by a
# compile error. Usage: run-red.sh [commit]   (log: stdout)
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P); rev=${1:-bf2049a}
b=$(mktemp -d); trap 'rm -rf -- "$b"' EXIT
for f in barDeco.cpp barDeco.hpp main.cpp globals.hpp; do git -C "$root" show "$rev:plugin/$f" > "$b/$f" || exit 2; done
cp "$here/baseline-barInput.hpp" "$b/barInput.hpp"
rc=0
for c in g++ clang++; do
  echo "== $rev $c"
  out=$(CXX=$c KWINISH_SRC=$b KWINISH_CXXFLAGS=-DKW_BASELINE bash "$root/tools/test-plugin-input.sh" 2>&1); r=$?
  printf '%s\nexit=%s\n' "$out" "$r"
  # red means: exit 1, the test built and ran, and at least one behavioural assertion failed
  if [ "$r" != 1 ] || grep -q "FALHOU - compilar" <<<"$out" || ! grep -qE "^FALHOU - [a-z]+.*(press|release|drag|grab)" <<<"$out"; then rc=1; echo "NOT RED as required ($c)"; fi
done
exit $rc
