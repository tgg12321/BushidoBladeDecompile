#!/bin/bash
# usage: cc.sh file.c  -> prints cc1 asm (pre-maspsx) of file
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
f=$1
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $f 2>/dev/null | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float 2>&1 | grep -v "^\s*\.\(loc\|stabn\|stabs\|file\|align\|ent\|end\|frame\|mask\|fmask\|set\|text\|globl\|type\|size\)"
