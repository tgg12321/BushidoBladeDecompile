#!/usr/bin/env python3
"""regionmap.py LO HI: every address in [LO,HI) that any shipped function (asm/funcs) references, with the
referencing functions' C objects (TU) and access kind (gp / lo / la), plus the original image bytes.
Uses the adopt scratch clone's reference build."""
import os, re, subprocess, sys
A = "/tmp/q56/adopt tree"
lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
EXE = open(A + "/disc/SLUS_006.63", "rb").read()
def sh(c): return subprocess.run(c, shell=True, capture_output=True, text=True).stdout
addr, by = {}, {}
for l in sh(f"mipsel-linux-gnu-nm '{A}/build/bb2.elf'").splitlines():
    p = l.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16)); by.setdefault(int(p[0], 16), []).append(p[2])
tu = {}
for o in os.listdir(A + "/build/src"):
    if o.endswith(".o"):
        for l in sh(f"mipsel-linux-gnu-nm --defined-only '{A}/build/src/{o}'").splitlines():
            p = l.split()
            if len(p) == 3 and p[1] in "Tt" and p[2] in addr:
                tu[addr[p[2]]] = o[:-2]
def res(s):
    if s in addr: return addr[s]
    m = re.fullmatch(r"D_([0-9A-Fa-f]{8})", s)
    return int(m.group(1), 16) if m else None
L = re.compile(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]{8}\s*\*/\s+(\S+)\s+(.*)")
refs = {}
for fn in os.listdir(A + "/asm/funcs"):
    first = None
    for ln in open(f"{A}/asm/funcs/{fn}", errors="replace"):
        m = L.search(ln)
        if not m: continue
        ia, mn, ops = int(m.group(1), 16), m.group(2), m.group(3)
        first = first or ia
        for r in re.finditer(r"%(gp_rel|lo)\((\w+)(?:\s*\+\s*(0x[0-9A-Fa-f]+|\d+))?\)", ops):
            b = res(r.group(2))
            if b is None: continue
            a = b + (int(r.group(3), 0) if r.group(3) else 0)
            if lo <= a < hi:
                k = "gp" if r.group(1) == "gp_rel" else ("la" if mn == "addiu" else "lo")
                refs.setdefault(a, set()).add((tu.get(first, "?"), k))
for a in sorted(refs):
    off = a - 0x80010000 + 0x800
    b = EXE[off:off + 4].hex() if off + 4 <= len(EXE) else "(beyond image)"
    names = ",".join(by.get(a, [])[:2])
    kinds = sorted(refs[a])
    print(f"{a:08x} {b:10} {names:30} " + " ".join(f"{t}:{k}" for t, k in kinds))
