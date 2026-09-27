#!/bin/bash
# usage: sched0.sh <tag> : sched1 debug for func_80027AD8 block 0 (first SCHEDDBG block after the function's start)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80027AD8/r11/rtl"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
BB2_SCHED_DEBUG=1 BB2_PRIO_DEBUG=1 BB2_ALLOC_DEBUG=1 ../../../../tools/gcc-2.7.2/cc1 $FLAGS $1.i -o /dev/null 2> /tmp/s0_$1.log
python3 - "$1" <<'PY'
import sys
t=sys.argv[1]
L=open('/tmp/s0_%s.log'%t,errors='replace').read().split('\n')
first=[i for i,l in enumerate(L) if 'ALLOCDBG func=func_80027AD8' in l][0]
prev=[i for i,l in enumerate(L[:first]) if 'ALLOCDBG func=' in l and 'func_80027AD8' not in l]
start=prev[-1]+1
seg=L[start:first]
# block 0 of this function = first 'SCHEDDBG block=0' in seg
b=[i for i,l in enumerate(seg) if l.startswith('SCHEDDBG block=0 ')][0]
e=[i for i,l in enumerate(seg) if l.startswith('SCHEDDBG block=') and i>b][0]
open('%s.sched1_block0.txt'%t,'w').write('\n'.join(seg[b:e])+'\n')
print(len(seg[b:e]),'lines')
PY
