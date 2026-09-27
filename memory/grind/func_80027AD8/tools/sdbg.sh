#!/bin/bash
# usage: sdbg.sh <candidate.c> : run cc1 with sched debug, print SCHEDDBG lines for func_80027AD8
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
out=tmp/func_80027AD8/d_s; rm -rf $out; mkdir -p $out
python3 - "$1" "$out" <<'PY'
import sys
cand=open(sys.argv[1]).read()
src=open('src/code6cac_b.c').read()
line='INCLUDE_ASM("asm/funcs", func_80027AD8);'
src=src.replace('INCLUDE_RODATA("asm/rodata", jtbl_80010548);\n','').replace('extern s32 func_80027AD8(s32, u8 *, s32, s32, s32, Tbl8008E194 *, s32, s32 *);','')
open(sys.argv[2]+'/x.c','w').write(src.replace(line,cand))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $out/x.c 2>/dev/null > $out/x.i
cd $out && BB2_SCHED_DEBUG=1 BB2_PRIO_DEBUG=1 BB2_ALLOC_DEBUG=1 ../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float x.i -o x.s 2> err.txt
python3 - <<'PY'
lines=open('err.txt',errors='replace').read().split('\n')
# isolate func_80027AD8 section: between ALLOCDBG of prev func and this; crude: find the ALLOCDBG func=func_80027AD8 line index
idx=[i for i,l in enumerate(lines) if 'ALLOCDBG func=func_80027AD8' in l]
first=idx[0]
# sched1 lines for this function are before first ALLOCDBG of this function and after previous function's last line
prev=[i for i,l in enumerate(lines[:first]) if 'ALLOCDBG func=' in l and 'func_80027AD8' not in l]
start=prev[-1]+1 if prev else 0
open('sched1.txt','w').write('\n'.join(lines[start:first]))
print(start, first)
PY
