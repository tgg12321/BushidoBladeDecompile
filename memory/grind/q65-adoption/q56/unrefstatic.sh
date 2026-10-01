#!/bin/bash
# unrefstatic.sh: does our cc1 (and cc1psx) emit an unreferenced file-scope static?
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
printf 'static int used;\nstatic int unused;\nstatic short after;\nint f(void) { return used + after; }\n' > /tmp/q56/unref.c
echo "== our cc1 -O2 -G0"; tools/gcc-2.7.2/build/cc1 -O2 -G0 -quiet -mcpu=3000 -mips1 -mno-abicalls -w -mel -msoft-float < /tmp/q56/unref.c | grep -E "local|comm"
echo "== cc1psx -O2 -G8"; bash tools/cc1psx_wrapper.sh -O2 -G8 -mcpu=3000 -mips1 -msoft-float -w < /tmp/q56/unref.c | grep -E "local|comm"
