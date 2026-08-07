#!/usr/bin/env python3
"""Independent census of the game (non-library) functions in MOVOVL.EXE.

Verifies the movovl_scan headline ("game code is only 3,832 bytes") from
first principles: function starts = GCC prologues (`addiu $sp,$sp,-N`) union
JAL targets inside the non-library code span 0x801D919C..0x801DA094.
Plain `j` targets are NOT function starts (GCC 2.7.2 emits `j` for
intra-function gotos). Prints per-function extent, callers, callees.

Usage (repo root, binary present at movovl/MOVOVL.EXE):
    python3 movovl/tools/census_game_funcs.py
"""
import struct, collections, os

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TADDR, TSIZE = 0x801D8800, 0x1E000
LO, HI = 0x801D919C, 0x801DA094      # non-library code span (see analyze_movovl.py)

b = open(os.path.join(REPO, "movovl", "MOVOVL.EXE"), "rb").read()
w = list(struct.unpack_from("<%dI" % (TSIZE // 4), b, 2048))

syms = {}
for line in open(os.path.join(REPO, "docs", "naming", "movovl_scan", "symbols.txt")):
    if not line.startswith("#") and line.strip():
        p = line.split()
        syms[int(p[0], 16)] = p[2]

jal = collections.defaultdict(list)
jmp = collections.defaultdict(list)
for i, x in enumerate(w):
    op = x >> 26
    if op in (2, 3):
        tgt = ((x & 0x03FFFFFF) << 2) | 0x80000000
        (jal if op == 3 else jmp)[tgt].append(TADDR + i * 4)

def is_prologue(x): return (x & 0xFFFF0000) == 0x27BD0000 and bool(x & 0x8000)

pro = [TADDR + i * 4 for i, x in enumerate(w) if is_prologue(x) and LO <= TADDR + i * 4 < HI]
jt = sorted(t for t in jal if LO <= t < HI)
starts = sorted(set(pro) | set(jt))
print("prologues=%d  jal-targets=%d  function starts=%d  span bytes=%d"
      % (len(pro), len(jt), len(starts), HI - LO))

ends = starts[1:] + [HI]
for s, e in zip(starts, ends):
    i0, i1 = (s - TADDR) // 4, (e - TADDR) // 4
    callees = []
    for i in range(i0, i1):
        if (w[i] >> 26) == 3:
            tgt = ((w[i] & 0x03FFFFFF) << 2) | 0x80000000
            callees.append(syms.get(tgt, "func_%08X" % tgt))
    kind = "prologue" if s in pro else ("stub" if e - s <= 24 else "frameless-leaf")
    print("func_%08X  size=%4d  %-14s callers(jal)=%d  callees: %s"
          % (s, e - s, kind, len(jal.get(s, [])), ", ".join(callees) or "-"))
