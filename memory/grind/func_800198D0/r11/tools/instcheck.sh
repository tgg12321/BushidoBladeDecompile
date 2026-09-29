#!/bin/bash
# instcheck.sh DUMPDIR... : compile each dump dir's code6cac.i with the BUILD cc1
# and compare its assembly with the instrumented cc1's code6cac.s (same flags).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for d in "$@"; do
  tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$d/code6cac.i" -o "$d/build_cc1.s" 2>/dev/null
  if diff -q <(grep -v "^ #" "$d/build_cc1.s") <(grep -v "^ #" "$d/code6cac.s") >/dev/null; then echo "$d: instrumented == build (option-echo comment lines excluded)"; else echo "$d: DIFFERENT"; fi
done
