#!/bin/bash
# dump.sh <variant-stem> : cc1 RTL dumps (jump2 + dbr) for the sandbox copy of that variant
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
V=$1
D=tmp/f43454/dump_$V
mkdir -p $D
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C tmp/f43454/wd/$V/src/text1a_c.c > $D/t.i
cd $D && ../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dJ -dd -dl -dg t.i -o t.s
ls
