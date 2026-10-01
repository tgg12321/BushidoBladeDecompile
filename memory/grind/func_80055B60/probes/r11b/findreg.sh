#!/bin/bash
# findreg.sh <dumpdir-name> <pseudo>... : instrumented cc1 (BB2_FINDREG_DEBUG / BB2_ALLOC_DEBUG) per pseudo, func_80055B60 only
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80055B60/r11/d_$1"; shift
for p in "$@"; do
  BB2_FINDREG_DEBUG=$p BB2_ALLOC_DEBUG=1 ../../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float tu.i -o /dev/null 2> fr.tmp
  awk '/FINDREGDBG func=func_80055B60 pseudo='"$p"' /{p=1} p&&/FINDREGDBG/{print} /FINDREGDBG  class/{if(p) exit}' fr.tmp
  grep "ALLOCDBG func=func_80055B60 ord=.* pseudo=$p " fr.tmp
done
rm -f fr.tmp
