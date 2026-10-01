#!/usr/bin/env python3
"""interleave.py SYM...: every function (shipped asm/funcs) touching SYM's address, in address order,
with gp / direct-lo / la / indexed classification and the C object (TU) it is built into."""
import os, re, subprocess, sys
T = "/tmp/q56/tree"
nm = subprocess.run(f"mipsel-linux-gnu-nm {T}/build/bb2.elf", shell=True, capture_output=True, text=True).stdout
addr = {}; by = {}
for l in nm.splitlines():
    p = l.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16)); by.setdefault(int(p[0], 16), []).append(p[2])
tu = {}
for o in os.listdir("/tmp/q56/refobj"):
    for l in subprocess.run(f"mipsel-linux-gnu-nm --defined-only /tmp/q56/refobj/{o}", shell=True, capture_output=True, text=True).stdout.splitlines():
        p = l.split()
        if len(p) == 3 and p[1] in "Tt":
            tu[int(addr.get(p[2], 0))] = o[:-2]
def res(s):
    if s in addr: return addr[s]
    m = re.fullmatch(r"D_([0-9A-F]{8})", s)
    return int(m.group(1), 16) if m else None
L = re.compile(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]{8}\s*\*/\s+(\S+)\s+(.*)")
for S in sys.argv[1:]:
    A = addr[S]; rows = []
    for fn in os.listdir(T + "/asm/funcs"):
        first = None; kinds = []; lui = {}
        for ln in open(f"{T}/asm/funcs/{fn}", errors="replace"):
            m = L.search(ln)
            if not m: continue
            ia, mn, ops = int(m.group(1), 16), m.group(2), m.group(3)
            first = first or ia
            for r in re.finditer(r"%(gp_rel|hi|lo)\((\w+)\)(?:\((\$\w+)\))?", ops):
                if res(r.group(2)) != A: continue
                if r.group(1) == "gp_rel": kinds.append((hex(ia), mn, "GP"))
                elif r.group(1) == "hi": lui[ops.split(",")[0]] = ia
                else:
                    b = r.group(3)
                    kinds.append((hex(ia), mn, "la" if mn == "addiu" else ("direct" if b in lui and ia - lui[b] <= 8 else "lo(indexed?)")))
        if kinds: rows.append((first, fn[:-2], kinds))
    print("==", S, hex(A))
    for first, f, k in sorted(rows):
        print(f"  {first:08x} {f:28} tu={tu.get(first, '?'):22} " + " ".join(f"{x[1]}:{x[2]}" for x in k))
