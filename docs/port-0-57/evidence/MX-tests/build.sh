#!/bin/bash
# uso: build.sh <gcc|clang> <outdir> — build isolado do plugin (env -i, HOME/XDG temporários, sem sessão gráfica);
# o .so fica em <outdir>/kwinish-bars-<cc>.so para a inspeção ELF; log aqui.
set -u
S=$(cd "$(dirname "$0")" && pwd); CC=$1; O=$2; R=$(cd "$S/../../../.." && pwd -P)
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT; mkdir -p $W/home $W/xdg/{config,data,cache,state,runtime}; chmod 700 $W/xdg/runtime
[ $CC = gcc ] && { X=g++; F=-O2; } || { X=clang++; F="-O2 -ferror-limit=0"; }
L=$S/build-$CC.log; SO=$O/kwinish-bars-$CC.so; rm -f "$SO"
env -i PATH=/usr/bin:/bin CXXFLAGS="$F" HOME=$W/home XDG_CONFIG_HOME=$W/xdg/config XDG_DATA_HOME=$W/xdg/data XDG_CACHE_HOME=$W/xdg/cache XDG_STATE_HOME=$W/xdg/state XDG_RUNTIME_DIR=$W/xdg/runtime \
  make -C $R/plugin CXX=$X TARGET=$SO > $L 2>&1
echo "exit=$?" >> $L
echo "$CC: $(tail -n 1 $L) errors=$(grep -c 'error:' $L) warnings=$(grep -c 'warning:' $L) so=$([ -f $SO ] && echo yes || echo no)"
