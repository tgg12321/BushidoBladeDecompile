#!/bin/bash
# findreg.sh <tag> <pseudo>...: the instrumented cc1's global.c find_reg trace (BB2_FINDREG_DEBUG)
# for pseudos of a body dumped by dump.sh (same t.i, same flags)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
tag=$1; shift
D=tmp/func_80057E84/r11/dumps/$tag
CCF=$(cat $D/ccflags.txt)
for ps in "$@"; do
  ( cd $D && BB2_FINDREG_DEBUG=$ps ../../../../../tools/gcc-2.7.2/cc1 $CCF t.i -o /dev/null 2>&1 | awk '/FINDREGDBG func=func_80057E84 /{p=1} /FINDREGDBG func=/&&!/func_80057E84/{p=0} p' | grep FINDREGDBG )
done
