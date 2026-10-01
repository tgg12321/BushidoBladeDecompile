#!/bin/bash
# findreg.sh <tag> <pseudo>...: BB2_FINDREG_DEBUG trace (global.c find_reg) for pseudos of a dumped body
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
tag=$1; shift
D=tmp/func_80058580/r11b/dumps/$tag
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for ps in "$@"; do
  ( cd $D && BB2_FINDREG_DEBUG=$ps ../../../../../tools/gcc-2.7.2/cc1 $FLAGS t.i -o /dev/null 2>&1 | awk '/FINDREGDBG func=func_80058580 pseudo=/{p=1} /FINDREGDBG func=/&&!/func_80058580/{p=0} p' | grep FINDREGDBG )
done
