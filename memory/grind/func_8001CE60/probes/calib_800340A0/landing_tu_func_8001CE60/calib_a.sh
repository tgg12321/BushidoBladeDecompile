#!/bin/bash
# usage: calib_a.sh <outname> [source.c, default src/code6cac.c]: compile the CURRENT src/code6cac.c (landing tree) with the build cc1 and cc1psx (same flags as
# calib_b.sh), cut func_8001CE60 from each -S output (before maspsx).
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
n=$1; o=tmp/func_8001CE60/out_a; mkdir -p $o
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C ${2:-src/code6cac.c} 2>/dev/null > $o/$n.i || true
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $o/$n.i -o $o/$n.s
bash tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w -msoft-float < $o/$n.i | tr -d '\r' > $o/$n.psx.s
for f in $o/$n.s $o/$n.psx.s; do awk '/^func_8001CE60:/{p=1} p{print} p&&/\.end\tfunc_8001CE60/{exit}' $f | grep -v "^\s*#" > $f.fn; done
wc -l $o/$n.s.fn $o/$n.psx.s.fn
