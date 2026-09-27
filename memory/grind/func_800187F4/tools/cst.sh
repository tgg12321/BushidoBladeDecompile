#!/bin/bash
# cst.sh <tags...>: fast diff + livelen/pri of the hoisted 3 / -2 constants
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for T in "$@"; do
  bash tmp/func_800187F4/fast.sh $T
  bash tmp/func_800187F4/dump.sh $T -dl -dg >/dev/null 2>&1
  D=tmp/func_800187F4/dump_$T
  BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $D/f.i -o $D/fi.s 2>$D/alloc.log
  grep "func=func_800187F4" $D/alloc.log | grep "ord=" | awk '$0 ~ /nrefs=7 / && $0 ~ /livelen=8[0-9][0-9]/' | sed 's/ALLOCDBG func=func_800187F4 /   /'
done
