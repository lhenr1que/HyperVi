#!/bin/bash
# uso: T2-build.sh <gcc|clang> <tag> — build isolado do plugin, sem sessão gráfica; trabalho em scratch, log aqui
set -u
S=$(cd "$(dirname "$0")" && pwd); CC=$1; TAG=$2; R=/home/lhen/Projects/ryoku-work/kwinish-port-0-57
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT; mkdir -p $W/home $W/xdg/{config,data,cache,state,runtime}; chmod 700 $W/xdg/runtime
[ $CC = gcc ] && { X=g++; F=-O2; } || { X=clang++; F="-O2 -ferror-limit=0"; }
L=$S/T6-build-$CC-$TAG.log
env -i PATH=/usr/bin:/bin CXXFLAGS="$F" HOME=$W/home XDG_CONFIG_HOME=$W/xdg/config XDG_DATA_HOME=$W/xdg/data XDG_CACHE_HOME=$W/xdg/cache XDG_STATE_HOME=$W/xdg/state XDG_RUNTIME_DIR=$W/xdg/runtime \
  make -C $R/plugin CXX=$X TARGET=$W/kw.so -k > $L 2>&1
echo "exit=$?" >> $L
echo "$CC $TAG: $(tail -1 $L) errors=$(grep -c 'error:' $L) warnings=$(grep -c 'warning:' $L) so=$([ -f $W/kw.so ] && echo yes || echo no)"
