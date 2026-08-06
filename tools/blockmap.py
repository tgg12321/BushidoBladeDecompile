#!/usr/bin/env python3
"""Correlate local-alloc QTYDBG block numbers to emitted asm index ranges.

The local model records GCC's basic-block numbering; GCC 2.7.2 emits blocks in
that order, so the Nth basic block of the emitted stream is block N. This splits
our sandbox object's disassembly into basic blocks (a block ends after a branch
+ its delay slot, or before a branch target) and prints each with the model's
quantities for the block of the same ordinal.

Usage: blockmap.py <stem> <func> [focus_index]
"""
import json, re, subprocess, sys
sys.path.insert(0, ".")
from engine import score

stem, func = sys.argv[1], sys.argv[2]
focus = int(sys.argv[3]) if len(sys.argv) > 3 else None

ours = score.normalized_insns(f"tmp/sandbox/{func}/{stem}.o", func, mask=False)

BR = re.compile(r"^(b|bal|beq|bne|blez|bgtz|bltz|bgez|beqz|bnez|j|jal|jr|jalr)\b")
TARGET = re.compile(r"0x([0-9a-f]+)\s*$")

# branch targets -> block starts. Addresses in the disassembly are absolute-ish;
# reuse relative ordering by collecting every referenced target address.
addrs = []
dis = subprocess.run(["mipsel-linux-gnu-objdump", "-d", f"tmp/sandbox/{func}/{stem}.o"],
                     capture_output=True, text=True).stdout
inside = False
for line in dis.splitlines():
    if line.endswith(">:") and "<" in line:
        inside = line.split("<")[1].rstrip(">:") == func
        continue
    if inside:
        m = re.match(r"^\s*([0-9a-f]+):", line)
        if m:
            addrs.append(int(m.group(1), 16))

targets = set()
for i, t in enumerate(ours):
    if BR.match(t):
        m = TARGET.search(t)
        if m:
            a = int(m.group(1), 16)
            if a in addrs:
                targets.add(addrs.index(a))

starts = {0} | targets
for i, t in enumerate(ours):
    if BR.match(t) and not t.startswith("jr"):
        starts.add(min(i + 2, len(ours)))   # branch + delay slot ends the block

starts = sorted(s for s in starts if s < len(ours))
blocks = [(s, starts[k + 1] if k + 1 < len(starts) else len(ours))
          for k, s in enumerate(starts)]

model = json.load(open("tmp/ra_solver_work/code6cac.local.json"))
fn = model.get(func) or model.get("funcs", {}).get(func) or {}
byblk = {}
for rec in (fn if isinstance(fn, list) else fn.get("qtys", [])):
    byblk.setdefault(rec.get("blk"), []).append(rec)

print(f"{func}: {len(ours)} insns, {len(blocks)} emitted basic blocks, "
      f"model blocks {sorted(byblk)}\n")
mblocks = sorted(byblk)
for ordinal, (lo, hi) in enumerate(blocks):
    mb = mblocks[ordinal] if ordinal < len(mblocks) else None
    qs = byblk.get(mb, [])
    tag = ""
    if focus is not None and lo <= focus < hi:
        tag = "   <=== FOCUS"
    regs = ", ".join(f"q{q['qty']}->${q['got']}(refs {q['refs']},"
                     f"b{q['birth']}-d{q['death']})" for q in qs)
    print(f"ord {ordinal:2d}  asm[{lo:3d}:{hi:3d}]  model blk={mb}  {regs}{tag}")
