#!/bin/bash
# cmp.sh <file.c> [G]: code only, cc1psx (calibration) vs our cc1 at -G$G
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
g=${2:-8}
F='^\s*#|^$|^\s*\.(frame|mask|fmask|file|ent|end|text|align|globl|set|lcomm|local|comm|type|size|ident|version|sdata|rdata|data|section|half|word|space)|gcc2_compiled|__gnu_compiled|^\.L|^\$L'
echo "=== cc1psx -G$g"; bash tools/cc1psx_wrapper.sh -O2 -G$g -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < "$1" | grep -vE "$F"
echo "=== our cc1 -G$g"; tools/gcc-2.7.2/build/cc1 -O2 -G$g -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < "$1" -o /dev/stdout | grep -vE "$F"
