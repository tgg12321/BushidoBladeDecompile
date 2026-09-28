#!/bin/bash
# usage: comb.sh <name>   (needs tmp/c21c/out/<name>.i from orph.py)
# -> tmp/c21c/out/<name>.comb : COMBDBG/ORPHAN log for func_8006C21C ; checks output == build cc1
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
n=$1
BB2_COMB_DEBUG=1 /tmp/gccdbg/cc1 $F tmp/c21c/out/$n.i -o /tmp/c21c_dbg_$n.s 2> /tmp/c21c_dbg_$n.err
tools/gcc-2.7.2/build/cc1 $F tmp/c21c/out/$n.i -o /tmp/c21c_ref_$n.s 2>/dev/null
cmp -s /tmp/c21c_dbg_$n.s /tmp/c21c_ref_$n.s && echo "[$n] dbg output identical to build cc1" || echo "[$n] OUTPUT DIFFERS"
awk '/^COMBDBG /{p=($2=="func_8006C21C")} /^ORPHAN/{if(p)print; next} p' /tmp/c21c_dbg_$n.err > tmp/c21c/out/$n.comb
echo "combos: $(grep -c '^COMBDBG' tmp/c21c/out/$n.comb)  3->2: $(grep -c 'newi2=1' tmp/c21c/out/$n.comb)  orphans: $(grep -c '^ORPHAN' tmp/c21c/out/$n.comb)"
