#!/bin/bash
# dump.sh <full_text1b.c> [extra cc1 flags]: cc1 -G0 with RTL dumps + BB2_ALLOC_DEBUG; extracts func_80048FFC's
# part of each dump into /tmp/f48d/<pass>.txt and prints the ALLOCDBG lines.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R="$(pwd)"
D=/tmp/f48d; rm -rf $D; mkdir -p $D
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$1" > $D/t.i 2>/dev/null
cd $D
BB2_ALLOC_DEBUG=1 BB2_QTY_DEBUG=1 "$R/tools/gcc-2.7.2/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dl -dg -dc -ds -dS -dR $2 t.i -o t.s 2> err.txt
grep "func=func_80048FFC" err.txt
for p in rtl cse combine lreg greg sched sched2; do
  awk '/^;; Function func_80048FFC/{f=1} /^;; Function / && !/func_80048FFC/{f=0} f' t.i.$p > $p.txt 2>/dev/null
done
awk '/^func_80048FFC:/{f=1} f{print} /^\.Lfe/{if(f)exit}' t.s > f.s
awk '/ALLOCDBG func=/{ if ($0 ~ /func_80048FFC/) {found=1} else if (!found) {buf=""} ; next} /^QTYDBG/{ if(!found) buf=buf $0 "\n"} END{printf "%s", buf}' err.txt > qty.txt
