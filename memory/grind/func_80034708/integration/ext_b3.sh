#!/bin/bash
# Small (.extern, size <= 8) symbols cc1 -G8 records for the new TU and for the two approved -G8 files,
# each marked in/out of sdata_syms.txt.
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
emit() { # $1 = source path
  mipsel-linux-gnu-cpp -Itmp/func_80034708/integ/include -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$1" 2>/dev/null \
   | tools/gcc-2.7.2/build/cc1 -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float 2>/dev/null \
   | grep -E '^\s*\.extern' | awk '{gsub(",","",$2); print $2, $3}'
}
for f in tmp/func_80034708/integ/src/code6cac_b3.c src/text1a_pre.c src/text1a_post.c; do
  echo "== $f"
  emit $f | while read s n; do
    if grep -qx "$s" sdata_syms.txt; then echo "  $s $n in-sdata_syms"; else echo "  $s $n NOT-in-sdata_syms"; fi
  done
done
