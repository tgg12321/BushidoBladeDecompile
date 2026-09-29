#!/bin/bash
# usage: cc.sh file.c -> prints asm (cc1 output, pre-maspsx)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx "$1" | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $2 -o "${1%.c}.s"
grep -v "^\s*\.\(loc\|stab\|def\|scl\|type\|endef\|size\|file\|ent\|end\|frame\|mask\|fmask\)" "${1%.c}.s" | grep -v "^\$L\|^#"
