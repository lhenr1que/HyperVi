#!/usr/bin/env bash
# Builds and runs plugin/test_tooltip.cpp: the title bar tooltip text and placement (plugin/tooltip.hpp), no Hyprland needed.
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
t=$(mktemp -d "${TMPDIR:-/tmp}/kwinish-test-tooltip.XXXXXXXX") || exit 1
trap 'rm -rf -- "$t"' EXIT
cxx=${CXX:-g++}
command -v "$cxx" >/dev/null || { echo "FALHOU - falta o compilador $cxx"; exit 1; }
"$cxx" -std=c++2b -Wall -Wextra -Werror -I "$root/plugin" "$root/plugin/test_tooltip.cpp" -o "$t/test_tooltip" || { echo "FALHOU - compilar plugin/test_tooltip.cpp"; exit 1; }
"$t/test_tooltip"
