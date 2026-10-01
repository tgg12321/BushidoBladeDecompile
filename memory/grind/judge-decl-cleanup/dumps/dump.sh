#!/bin/bash
# RTL dumps (.rtl = expand output, .cse, .lreg) of code6cac_tu2 for two D_8008EB40 spellings.
# usage (WSL, repo root): bash tmp/laneH/dump.sh
set -e
CC1=tools/gcc-2.7.2/cc1
DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
OUT=tmp/laneH/dumps
mkdir -p $OUT
for v in sqrt_tables sqrt_tables_o2; do
  T=tmp/laneH/w_$v
  mipsel-linux-gnu-cpp -I$T/include -Isrc -undef -Wall -lang-c -fno-builtin $DEFS $T/src/code6cac_tu2.c > $OUT/$v.i
  $CC1 $FLAGS -dr -ds -dl $OUT/$v.i -o $OUT/$v.s
  echo "built $v: $(ls $OUT/$v.i.* | tr '\n' ' ')"
done
