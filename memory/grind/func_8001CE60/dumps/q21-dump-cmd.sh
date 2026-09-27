#!/bin/bash
# RTL dumps (.rtl .cse .cse2 .combine .lreg) for func_800340A0 under the array declaration (A340_1) and scalars (A340_0).
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
O=tmp/func_8001CE60/out_b
for n in A340_1 A340_0; do
  mkdir -p $O/dump_$n
  cp $O/$n.i $O/dump_$n/t.i
  (cd $O/dump_$n && ../../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -ds -dt -dc -dl t.i -o t.s)
  ls $O/dump_$n
done
