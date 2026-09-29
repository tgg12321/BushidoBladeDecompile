#!/bin/bash
# alloc.sh <tag>: instrumented cc1 ALLOCDBG for func_800187F4 (uses dump_<tag>/f.i from dump.sh)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=$1
bash tmp/rv3/dump2.sh $T -dl -dg >/dev/null 2>&1
D=tmp/func_800187F4/dump_$T
BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $D/f.i -o $D/fi.s 2>$D/alloc.log
diff <(grep -v '^\s*#' $D/f.s) <(grep -v '^\s*#' $D/fi.s) >/dev/null && echo "instrumented==build" || echo "WARNING instrumented differs"
grep "func=func_800187F4" $D/alloc.log | grep "ord=" > $D/alloc.txt
wc -l < $D/alloc.txt
