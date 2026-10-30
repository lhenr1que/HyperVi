#!/bin/bash
# uso: build.sh <gcc|clang> <tag>  — build isolado do plugin, sem sessão gráfica
set -u
S=$(dirname "$0"); CC=$1; TAG=$2; R=/home/lhen/Projects/ryoku-work/kwinish-port-0-57
W=$S/w-$CC-$TAG; mkdir -p $W/home $W/xdg/{config,data,cache,state,runtime}; chmod 700 $W/xdg/runtime
[ $CC = gcc ] && X=g++ || X=clang++
env -i PATH=/usr/bin:/bin ${XFLAGS:+CXXFLAGS="$XFLAGS"} HOME=$W/home XDG_CONFIG_HOME=$W/xdg/config XDG_DATA_HOME=$W/xdg/data XDG_CACHE_HOME=$W/xdg/cache XDG_STATE_HOME=$W/xdg/state XDG_RUNTIME_DIR=$W/xdg/runtime \
  make -C $R/plugin CXX=$X TARGET=$W/kw.so -k > $S/build-$CC-$TAG.log 2>&1
echo "exit=$?" >> $S/build-$CC-$TAG.log
echo "$CC $TAG: $(tail -1 $S/build-$CC-$TAG.log) errors=$(grep -c 'error:' $S/build-$CC-$TAG.log) so=$([ -f $W/kw.so ] && echo yes || echo no)"
