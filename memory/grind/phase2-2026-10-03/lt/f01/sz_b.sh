#!/bin/bash
# sz_b.sh [include-dir] : cc1 negative-array size asserts for Unk1F800000Unk00 / Unk1F800000Rec
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
inc=${1:-include}
mipsel-linux-gnu-cpp -I$inc -Iinclude -undef -D__GNUC__=2 -Dmips -D__mips__ -D_LANGUAGE_C -DLANGUAGE_C memory/grind/phase2-2026-10-03/lt/f01/sz_b.c > tmp/sz_b.i 2>&1
tools/gcc-2.7.2/build/cc1 -O2 -quiet -mel -msoft-float tmp/sz_b.i -o /dev/null 2>&1 | grep -v warning | head
echo "cc1 rc=${PIPESTATUS[0]}"
