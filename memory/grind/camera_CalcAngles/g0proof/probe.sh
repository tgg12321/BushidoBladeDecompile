#!/bin/bash
# probe.sh <file.c> [G] [dumpflags] [dumpext]: compile a probe with our cc1 at -G$G; print asm (and a dump)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R="$(pwd)"
f=$1; g=${2:-0}; dflags=$3; dext=$4
D=/tmp/camCA; mkdir -p $D; cp "$f" $D/p.c; cd $D; rm -f p.c.*
"$R/tools/gcc-2.7.2/build/cc1" -O2 -G$g -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $dflags p.c -o p.s
grep -vE '^\s*#|^$|^\s*\.(frame|mask|fmask|file|ent|end|text|align|globl|set|local|comm|type|size|ident|version|lcomm)' p.s
if [ -n "$dext" ]; then echo "===== dump $dext"; grep -v '^$' p.c.$dext; fi
