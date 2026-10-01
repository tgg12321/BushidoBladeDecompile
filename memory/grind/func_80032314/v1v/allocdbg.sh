#!/bin/bash
# ALLOCDBG trace (global.c allocno priorities) of func_80032314 as staged in src/code6cac_b_tu2.c.
# usage (WSL, repo root): bash tmp/laneH/allocdbg.sh <out>
set -e
OUT=$1
CC1=tools/gcc-2.7.2/cc1
DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin $DEFS src/code6cac_b_tu2.c > /tmp/laneH_b_tu2.i 2>/dev/null
BB2_ALLOC_DEBUG=1 $CC1 $FLAGS /tmp/laneH_b_tu2.i -o /tmp/laneH_b_tu2.s 2> /tmp/laneH_alloc.txt
{
  echo "# BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 $FLAGS <src/code6cac_b_tu2.c preprocessed>  (staged combined landing, 2026-09-30)"
  echo "# section for func_80032314 only"
  awk '/func_80032314/{p=1} p&&/func_800324D0/{exit} p' /tmp/laneH_alloc.txt
} > "$OUT"
wc -l "$OUT"
