#!/bin/bash
# RTL dumps (.rtl expand, .jump, .cse) of code6cac_b_tu2.c for dump trees A (landed) and B (alternatives).
# usage (WSL, repo root, after python3 tmp/laneH/mkdtree.py): bash tmp/laneH/dump2.sh
set -e
CC1=tools/gcc-2.7.2/cc1
DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
OUT=tmp/laneH/dumps2
mkdir -p $OUT
for v in A B; do
  T=tmp/laneH/dtree_$v
  mipsel-linux-gnu-cpp -I$T/include -Isrc -undef -Wall -lang-c -fno-builtin $DEFS $T/src/code6cac_b_tu2.c > $OUT/$v.i 2>/dev/null
  $CC1 $FLAGS -dr -dj -ds $OUT/$v.i -o $OUT/$v.s
  echo "built $v"
done
