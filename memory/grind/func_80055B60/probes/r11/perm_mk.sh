#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
W=tmp/perm_b60f
mipsel-linux-gnu-cpp -Itmp/b60/inc/include -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $W/mini_src.c | grep -v '^#' > $W/base.c
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $W/base.c -o $W/mini.s
awk '/\.ent\tfunc_80055B60/,/\.end\tfunc_80055B60/' $W/mini.s | sed -E 's/\.L[0-9]+/.L/g' > $W/mini_func.s
bash tmp/b60/dump.sh pvf > /dev/null
sed -E 's/\.L[0-9]+/.L/g' tmp/b60/d_pvf/func.s | cmp - $W/mini_func.s && echo MINI_SAME_AS_FULL_TU
