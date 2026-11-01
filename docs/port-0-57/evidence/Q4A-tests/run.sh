#!/usr/bin/env bash
# Run from any directory. Copies fixtures to scratch; never writes build artifacts into the repository.
set -euo pipefail
P=$(cd "$(dirname "$0")" && pwd)
W=${1:-$(git -C "$P" rev-parse --show-toplevel)}
O=$(mktemp -d "${TMPDIR:-/home/lhen/.hermes/cache/scratch}/q4a-run.XXXXXX")
printf 'scratch: %s\n' "$O"
cp "$P/q4model.hpp" "$P/q4test.cpp" "$P/mut.sh" "$P/probe.cpp" "$O/"
export HOME="$O/home" XDG_CONFIG_HOME="$O/home/.config" XDG_CACHE_HOME="$O/home/.cache" XDG_DATA_HOME="$O/home/.local/share"
mkdir -p "$HOME"
unset DISPLAY WAYLAND_DISPLAY HYPRLAND_INSTANCE_SIGNATURE
cd "$O"
F="-std=c++2b -Wno-c++11-narrowing -I$W/plugin $(pkg-config --cflags pixman-1 libdrm hyprland libinput libudev wayland-server xkbcommon cairo librsvg-2.0)"
for C in g++ clang++; do
    $C -std=c++2b -Wall -Wextra -g -fsanitize=address,undefined -fno-sanitize-recover=all q4test.cpp -o "q4-$C" $(pkg-config --cflags --libs hyprutils) > "build-$C.log" 2>&1
    "./q4-$C" > "run-$C.txt" 2>&1
    printf '%s model: ' "$C"; python3 -c 'import sys; print(open(sys.argv[1]).read().splitlines()[-1])' "run-$C.txt"
    $C $F -fsyntax-only probe.cpp > "probe-$C.log" 2>&1
    if $C $F -DNEG -fsyntax-only probe.cpp > "probe-neg-$C.log" 2>&1; then printf 'FAIL: NEG compiled\n'; exit 1; fi
    python3 - "$C" <<'PY'
import re,sys,pathlib
s=pathlib.Path('probe-neg-'+sys.argv[1]+'.log').read_text()
e=[l for l in s.splitlines() if re.search(r': (fatal )?error:',l)]
assert len(e)==1 and 'sessionId' in e[0] and ('no member named' in e[0] or 'no member' in e[0]),s
print(sys.argv[1]+' probe: positive OK, negative only expected sessionId diagnostic')
PY
done
cmp run-g++.txt run-clang++.txt
bash mut.sh > mutations.log 2>&1
python3 -c 'import pathlib; print(pathlib.Path("mutations.log").read_text())'
printf 'PASS: model GCC/Clang, real-header probes, mutations; no live host\n'
