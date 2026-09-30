#!/bin/bash
# sdbg.sh <tag>: instrumented-cc1 sched1 trace of func_8006CFBC for dump_<tag>/f.i (made by dump.sh).
# Prints, for each sched1 block holding a `+ 12` add, the ADJPRI/PICK lines.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=$1
D=tmp/func_8006CFBC/dump_$T
CC_FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
BB2_ALLOC_DEBUG=1 BB2_SCHED_DEBUG=1 tools/gcc-2.7.2/cc1 $CC_FLAGS $D/f.i -o $D/fi.s 2> $D/sdbg.log || true
cmp -s $D/f.s $D/fi.s && echo "instrumented run == dump run (same .s)" || echo "WARNING: instrumented .s differs"
python3 - "$D" <<'PY'
import sys, re
d = sys.argv[1]
fn = open(d + '/f.i.sched.fn').read()
# add insns: (insn UID ... (set (reg...) (plus:SI (reg:SI N) (const_int 12)))
adds = re.findall(r'\(insn (\d+) \d+ \d+ \(set \((reg[^)]*)\)\s*\(plus:SI \(reg:SI (\d+)\)\s*\(const_int 12\)\)\)', fn)
print('adds (uid, dest, src):', adds)
uids = {a[0] for a in adds}
L = open(d + '/sdbg.log', errors='replace').read().splitlines()
idx = [i for i, l in enumerate(L) if l.startswith('ALLOCDBG') and 'func_8006CFBC' in l]
first = idx[0]
b0 = max(i for i, l in enumerate(L[:first]) if l.startswith('SCHEDDBG block=0 '))
seg = L[b0:first]
blocks = [i for i, l in enumerate(seg) if l.startswith('SCHEDDBG block=')]
for bi, b in enumerate(blocks):
    end = blocks[bi + 1] if bi + 1 < len(blocks) else len(seg)
    chunk = seg[b:end]
    if any(re.search(r'insn=(%s) ' % '|'.join(uids), l) or re.search(r'picked=(%s) ' % '|'.join(uids), l) for l in chunk):
        print('\n'.join(l for l in chunk if not l.startswith('PRIODBG')))
        print('-----')
PY
