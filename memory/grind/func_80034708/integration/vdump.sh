#!/bin/bash
# vdump.sh <variant-dir> [dumpflags]  : cpp with <dir>/include first, cc1 -G8 with dumps, extract func_80034708
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
V=$1; DF=${2:--da}
mipsel-linux-gnu-cpp -I$V/include -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $V/src/code6cac_b3.c > $V/b3.i 2>/dev/null
cd $V && BB2_FRAME_DEBUG=1 ../../../tools/gcc-2.7.2/cc1 -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $DF b3.i -o b3.s 2> cc1.log
for p in rtl jump cse loop cse2 flow combine sched lreg greg sched2 jump2 dbr; do
  [ -f b3.i.$p ] && mv b3.i.$p f.$p
done
echo done
