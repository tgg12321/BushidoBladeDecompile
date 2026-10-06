#!/bin/bash
# sz_c.sh [include-dir] [file] : cc1 negative-array size asserts for TexRec / Unk1F800000Rec (F01c)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
inc=${1:-include}
f=${2:-memory/grind/phase2-2026-10-03/lt/f01/sz_c.c}
mipsel-linux-gnu-cpp -I$inc -Iinclude -undef -D__GNUC__=2 -Dmips -D__mips__ -D_LANGUAGE_C -DLANGUAGE_C $f > tmp/sz_c.i 2>&1
tools/gcc-2.7.2/build/cc1 -O2 -quiet -mel -msoft-float tmp/sz_c.i -o /dev/null 2>&1 | grep -v warning | grep -v "In file\|from "
echo "cc1 rc=${PIPESTATUS[0]}"
