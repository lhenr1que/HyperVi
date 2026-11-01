#!/usr/bin/env bash
# Packaging and plugin build checks (etapa E of the 27/10 audit), no root, nothing installed on the system, no compile:
#  1. plugin/Makefile rebuilds the plugin when any plugin header changes (make -q on a temp copy);
#  2. packaging/plugin/PKGBUILD ships every source the rebuild hook needs: package() runs into a temp pkgdir, and every
#     #include "..." of the shipped sources must be shipped too;
#  3. packaging/plugin/kwinish-bars-rebuild reports a failure, not success, when installing the rebuilt plugin fails.
# KWINISH_ROOT=<dir> checks another copy of the repository.
set -u
root=${KWINISH_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)}
t=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-test-packaging.XXXXXXXX") || exit 1
trap 'rm -rf -- "$t"' EXIT
fails=0
ok() { echo "ok - $1"; }
bad() { echo "FALHOU - $1"; fails=$((fails + 1)); }

# 1. Makefile: a header newer than the plugin makes it out of date
cp -r "$root/plugin" "$t/mk"
touch -d '2000-01-01' "$t"/mk/*
touch "$t/mk/kwinish-bars.so"
if make -q -C "$t/mk" kwinish-bars.so >/dev/null 2>&1; then ok "Makefile: plugin up to date when nothing changed"; else bad "Makefile: plugin out of date with nothing changed"; fi
stale=()
for h in "$t"/mk/*.hpp; do
  case ${h##*/} in test_*) continue ;; esac
  touch -d '2000-01-01' "$t/mk/kwinish-bars.so"; touch -d '2001-01-01' "$h"
  make -q -C "$t/mk" kwinish-bars.so >/dev/null 2>&1 && stale+=("${h##*/}")
  touch -d '2000-01-01' "$h"; touch "$t/mk/kwinish-bars.so"
done
[ ${#stale[@]} = 0 ] && ok "Makefile: a changed header rebuilds the plugin" || bad "Makefile: changing these headers does not rebuild: ${stale[*]}"

# 2. PKGBUILD package(): the shipped plugin sources are complete
mkdir -p "$t/src" "$t/pkg"
git -C "$root" ls-files -z | (cd "$root" && xargs -0 cp --parents -t "$t/src" 2>/dev/null) || true
mkdir -p "$t/src/kwinish"; (cd "$t/src" && find . -mindepth 1 -maxdepth 1 ! -name kwinish -exec mv -t kwinish {} +)
: > "$t/src/kwinish/plugin/kwinish-bars.so"; : > "$t/src/kwinish/plugin/kwinish-bars.abi"
if (cd "$t/src" && srcdir=$PWD pkgdir=$t/pkg bash -euc 'source "$1"; package' _ "$root/packaging/plugin/PKGBUILD") >"$t/pkg.log" 2>&1; then
  ok "PKGBUILD: package() runs"
  ps=$t/pkg/usr/share/kwinish/plugin-src
  missing=()
  for f in "$ps"/*.cpp "$ps"/*.hpp; do
    [ -f "$f" ] || continue
    for inc in $(sed -n 's/^#include "\([^"]*\)".*/\1/p' "$f"); do [ -f "$ps/$inc" ] || missing+=("${f##*/}->$inc"); done
  done
  for s in $(sed -n 's/^SRC *= *//p' "$root/plugin/Makefile"); do [ -f "$ps/$s" ] || missing+=("Makefile->$s"); done
  [ ${#missing[@]} = 0 ] && ok "PKGBUILD: the rebuild sources include everything they need" || bad "PKGBUILD: shipped sources miss: ${missing[*]}"
  ls "$ps" | grep -q '^test_' && bad "PKGBUILD: ships test sources" || ok "PKGBUILD: no test sources shipped"
else
  bad "PKGBUILD: package() failed:"; sed 's/^/    /' "$t/pkg.log"
fi

# 2b. packaging/PKGBUILD package(): every core module (not the tests) is shipped
mkdir -p "$t/pkgcore" "$t/src/kwinish/ui/target/release"; : > "$t/src/kwinish/ui/target/release/kwinish-ui"
if (cd "$t/src" && srcdir=$PWD pkgdir=$t/pkgcore bash -euc 'source "$1"; package' _ "$root/packaging/PKGBUILD") >"$t/pkgcore.log" 2>&1; then
  missing=()
  for m in "$root"/core/*.lua; do
    case ${m##*/} in test_*) continue ;; esac
    [ -f "$t/pkgcore/usr/share/kwinish/core/${m##*/}" ] || missing+=("${m##*/}")
  done
  [ ${#missing[@]} = 0 ] && ok "PKGBUILD (core): every core module is shipped" || bad "PKGBUILD (core): not shipped: ${missing[*]}"
else
  bad "PKGBUILD (core): package() failed:"; sed 's/^/    /' "$t/pkgcore.log"
fi

# 3. kwinish-bars-rebuild: an install that fails is reported as a failure
mkdir -p "$t/fake" "$t/bin"
printf 'abi:\n\t@: > kwinish-bars.so; : > kwinish-bars.abi\n' > "$t/fake/Makefile"
printf '#!/bin/sh\n[ "$1" = --modversion ] && echo 0.57.0; exit 0\n' > "$t/bin/pkg-config"; chmod +x "$t/bin/pkg-config"
: > "$t/notadir" # dest under a regular file: install cannot create it
out=$(PATH="$t/bin:$PATH" KWINISH_PLUGIN_SRC="$t/fake" KWINISH_PLUGIN_DEST="$t/notadir/plugins" sh "$root/packaging/plugin/kwinish-bars-rebuild" 2>&1); rc=$?
if grep -q 'rebuilt for' <<<"$out"; then bad "rebuild: claims success when install failed: $out"; else ok "rebuild: no success message when install failed"; fi
[ "$rc" = 0 ] && ok "rebuild: never fails the pacman transaction" || bad "rebuild: exit $rc (must be 0)"
out=$(PATH="$t/bin:$PATH" KWINISH_PLUGIN_SRC="$t/fake" KWINISH_PLUGIN_DEST="$t/dest" sh "$root/packaging/plugin/kwinish-bars-rebuild" 2>&1)
[ -f "$t/dest/kwinish-bars.so" ] && grep -q 'rebuilt for' <<<"$out" && ok "rebuild: success when install works" || bad "rebuild: did not install or report: $out"

[ "$fails" = 0 ] && echo "packaging: tudo passou" || { echo "packaging: $fails falha(s)"; exit 1; }
