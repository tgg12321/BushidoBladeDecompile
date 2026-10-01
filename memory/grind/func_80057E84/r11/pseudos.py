#!/usr/bin/env python3
"""pseudos.py <tag>...: for each dump (dump.sh output), the pseudo written by each value's write
insn (insn UIDs from the reuse body's .lreg; identical statement lists give identical UIDs, checked
by the insn's SET_SRC shape), its .lreg `Register` line, its global.c ALLOCDBG line (or the
local-alloc QTYDBG line that seated it) and its .greg disposition."""
import re
import sys
from pathlib import Path

D = Path("tmp/func_80057E84/r11/dumps")
# value -> insn uid of its write in the reuse body (route_pick: both arms)
WRITES = {"vtx_a": [190], "vtx_b": [223], "vtx_dn": [497], "route_dn": [600], "node_dn": [632],
          "vtx_up": [760], "route_up": [869], "node_up": [901], "route_pick": [1045, 1053]}
REGN = {0: "zero", 2: "v0", 3: "v1", 4: "a0", 5: "a1", 6: "a2", 7: "a3", 8: "t0", 9: "t1",
        10: "t2", 11: "t3", 12: "t4", 13: "t5", 14: "t6", 15: "t7", 16: "s0", 17: "s1",
        18: "s2", 19: "s3", 20: "s4", 21: "s5", 22: "s6", 23: "s7", 24: "t8", 25: "t9",
        30: "fp", 31: "ra"}

for tag in sys.argv[1:]:
    lreg = (D / tag / f"{tag}.lreg").read_text()
    greg = (D / tag / f"{tag}.greg").read_text()
    alloc = (D / tag / f"{tag}.alloc").read_text()
    disp = dict(re.findall(r"(\d+) in (\d+)", greg.split(";; Register dispositions:")[1].split("\n\n")[0]))
    print(f"== {tag}")
    for val, uids in WRITES.items():
        regs = set()
        for u in uids:
            m = re.search(r"\(insn %d \d+ \d+ \(set \(reg/v:SI (\d+)\)" % u, lreg)
            regs.add(m.group(1) if m else "?")
        for r in sorted(regs):
            rl = re.search(r"^Register %s used .*$" % r, lreg, re.M)
            al = re.search(r"ALLOCDBG func=func_80057E84 ord=\d+ pseudo=%s .*$" % r, alloc, re.M)
            hr = disp.get(r)
            print(f"  {val:11s} pseudo {r:>4s} -> {REGN.get(int(hr), hr) if hr else '-':4s} | "
                  f"{rl.group(0) if rl else ''} | {al.group(0).split(' ', 2)[2] if al else 'local-alloc (no ALLOCDBG)'}")
