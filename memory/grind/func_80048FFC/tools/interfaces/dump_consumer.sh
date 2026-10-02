#!/bin/bash
set -e
cd '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
R=$(pwd)
for name in plain pair4; do
 D="$R/tmp/codex_cam/plain_consumer/dump_$name"
 mkdir -p "$D"
 mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "tmp/codex_cam/plain_consumer/$name.build.c" > "$D/t.i" 2> "$D/cpp.err"
 (
  cd "$D"
  BB2_ALLOC_DEBUG=1 BB2_QTY_DEBUG=1 "$R/tools/gcc-2.7.2/cc1" -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dc -dl -dg -ds -dS t.i -o t.s 2> err.txt
 )
done
