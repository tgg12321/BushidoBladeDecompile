#!/bin/bash
# usage: alloc.sh <name>... : BB2_ALLOC_DEBUG lines for func_8006C21C from the private diagnostic cc1
# (/tmp/gccdbg, output verified identical to build cc1 by comb.sh). Needs tmp/c21c/out/<name>.i
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for n in "$@"; do
  BB2_ALLOC_DEBUG=1 /tmp/gccdbg/cc1 $F tmp/c21c/out/$n.i -o /tmp/c21c_alloc_$n.s 2> tmp/c21c/out/$n.allocdbg
  tools/gcc-2.7.2/build/cc1 $F tmp/c21c/out/$n.i -o /tmp/c21c_allocref_$n.s 2>/dev/null
  cmp -s /tmp/c21c_alloc_$n.s /tmp/c21c_allocref_$n.s && echo "[$n] dbg output identical to build cc1" || echo "[$n] OUTPUT DIFFERS"
  grep -c "func=func_8006C21C" tmp/c21c/out/$n.allocdbg
  grep "func=func_8006C21C" tmp/c21c/out/$n.allocdbg | head -3
done
