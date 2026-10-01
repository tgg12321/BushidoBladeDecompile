#!/usr/bin/env python3
"""static_blocks.py [tree]: the per-file .lcomm (static) region 0x800A3308..0x800A3618 of the linked image,
object by object. For every C object (our file) it lists the addresses its code references (C and
INCLUDE_ASM alike, from the object's relocations), the symbol names used, the splat object size (the
data blob's dlabel extent) and whether another object references the same address (shared -> the files
must be one original file). Output: /tmp/q56/static_blocks.json + a readable summary."""
import json, os, re, subprocess, sys
T = sys.argv[1] if len(sys.argv) > 1 else "/tmp/q56/adopt tree"
LO, HI = 0x800A3308, 0x800A3618

def sh(c):
    return subprocess.run(c, shell=True, capture_output=True, text=True).stdout

addr = {}
for l in sh(f"mipsel-linux-gnu-nm '{T}/build/bb2.elf'").splitlines():
    p = l.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))
# splat object extents from the data blob
ext = {}
cur = None
for l in open(f"{T}/asm/data/91C98.data.s"):
    m = re.match(r"dlabel (\w+)", l)
    if m:
        cur = m.group(1); continue
    m = re.match(r"enddlabel", l)
    if m:
        cur = None; continue
    m = re.search(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s", l)
    if m and cur:
        a = int(m.group(1), 16)
        s, e = ext.get(cur, (a, a))
        ext[cur] = (min(s, a), max(e, a + 4))
starts = {s: (n, e - s) for n, (s, e) in ext.items()}

refs = {}   # addr -> {obj: set(names)}
for o in sorted(os.listdir(f"{T}/build/src")):
    if not o.endswith(".o"):
        continue
    for l in sh(f"mipsel-linux-gnu-objdump -r '{T}/build/src/{o}'").splitlines():
        m = re.match(r"^[0-9a-f]+\s+R_MIPS_(HI16|LO16|GPREL16|32)\s+(\S+)$", l)
        if not m:
            continue
        nm = re.sub(r"\+0x[0-9a-f]+$", "", m.group(2))
        a = addr.get(nm)
        if a is None:
            mm = re.fullmatch(r"D_([0-9A-F]{8})", nm)
            a = int(mm.group(1), 16) if mm else None
        if a is None or not (LO <= a < HI):
            continue
        refs.setdefault(a, {}).setdefault(o[:-2], set()).add(nm)

res = {}
for a in sorted(refs):
    objs = refs[a]
    for o, names in objs.items():
        res.setdefault(o, []).append({"addr": hex(a), "names": sorted(names), "shared_with": sorted(set(objs) - {o}),
                                      "splat": starts.get(a, (None, None))[0], "splat_size": starts.get(a, (None, None))[1]})
json.dump(res, open("/tmp/q56/static_blocks.json", "w"), indent=1)
for o, lst in res.items():
    sh_ = [x for x in lst if x["shared_with"]]
    print(f"{o}: {len(lst)} addresses {lst[0]['addr']}..{lst[-1]['addr']}; shared: " +
          (", ".join(f"{x['addr']}({','.join(x['shared_with'])})" for x in sh_) or "none"))
