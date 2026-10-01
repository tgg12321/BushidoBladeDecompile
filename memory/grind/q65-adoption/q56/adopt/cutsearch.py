#!/usr/bin/env python3
"""cutsearch.py <tree> <objA> <objB>: two adjacent objects (link order A then B). For every function, the
small-data addresses it reaches gp-relative (from its relocations and the ELF addresses). Each address is
owned by the first object whose file-block contains it (per-file .sdata / .sbss blocks in link order). Prints,
over the concatenated function order, every cut position at which no address is reached gp from both sides,
and the current boundary."""
import re, subprocess, sys
T, A, B = sys.argv[1:4]
addr = {}
for l in subprocess.run(f"mipsel-linux-gnu-nm '{T}/build/bb2.elf'", shell=True, capture_output=True, text=True).stdout.splitlines():
    p = l.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))
order, gp = [], {}
for obj in (A, B):
    out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn '{T}/build/src/{obj}.o'", shell=True,
                         capture_output=True, text=True).stdout.splitlines()
    fn = None
    for ln in out:
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:$", ln)
        if m:
            fn = (obj, m.group(1)); order.append(fn); continue
        r = re.match(r"^\s*[0-9a-f]+:\s+R_MIPS_GPREL16\s+(\S+)$", ln)
        if r and fn:
            s = re.sub(r"\+0x[0-9a-f]+$", "", r.group(1))
            a = addr.get(s)
            if a is None:
                mm = re.fullmatch(r"D_([0-9A-F]{8})", s)
                a = int(mm.group(1), 16) if mm else None
            if a is not None and a < 0x800A3618:   # per-file blocks only (.sdata / .sbss); COMMON is shareable
                gp.setdefault(fn, set()).add(a)
cur = next(i for i, f in enumerate(order) if f[0] == B)
ok = []
for c in range(1, len(order)):
    left = set().union(*[gp.get(f, set()) for f in order[:c]])
    right = set().union(*[gp.get(f, set()) for f in order[c:]])
    if not (left & right):
        ok.append(c)
print(f"current boundary: before {order[cur][1]} (index {cur})")
if ok:
    runs, s0 = [], ok[0]
    for x, y in zip(ok, ok[1:] + [None]):
        if y != x + 1:
            runs.append((s0, x)); s0 = y
    for s0, s1 in runs:
        print(f"valid cuts: before {order[s0][1]} .. before {order[s1][1]} (indices {s0}..{s1})")
shared_now = set().union(*[gp.get(f, set()) for f in order[:cur]]) & set().union(*[gp.get(f, set()) for f in order[cur:]])
print("shared across the current boundary:", sorted(hex(a) for a in shared_now))
