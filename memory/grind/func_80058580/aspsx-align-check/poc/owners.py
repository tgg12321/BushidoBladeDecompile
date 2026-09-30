#!/usr/bin/env python3
"""owners.py: data-ownership map for TU-boundary evidence.

For every function in asm/funcs (target code, independent of our C), collect the data symbols it
references (%hi/%lo/%gp_rel). Resolve each to its absolute address via build/bb2.elf. Output a
JSON with: funcs (addr-ordered, with our object), items (addr-ordered data symbols with section,
size-to-next and referencing function indices), and section bounds."""
import re, os, subprocess, json, bisect
R = "/home/user/BushidoBladeDecompile/"
nm = subprocess.run(["mipsel-linux-gnu-nm", "-n", R + "build/bb2.elf"], capture_output=True, text=True).stdout
addr = {}
for line in nm.splitlines():
    p = line.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))
bounds = {k: addr[k] for k in ("main_TEXT_START", "main_DATA_START", "main_BSS_START", "main_BSS_END")}
# our object per text address, from the map
mp = open(R + "build/bb2.map").read()
objs = {}
for sec in ("text", "rodata", "data", "bss"):
    objs[sec] = []
    for m in re.finditer(r"^ \.%s\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) (\S+)" % sec, mp, re.M):
        a, n = int(m.group(1), 16), int(m.group(2), 16)
        if n:
            objs[sec].append((a, a + n, m.group(3).split("/")[-1]))
def obj_of(sec, x):
    for a, e, o in objs[sec]:
        if a <= x < e:
            return o
    return "?"
funcs = []
refs = {}
REF = re.compile(r"%(?:hi|lo|gp_rel)\(([A-Za-z_][A-Za-z0-9_]*)")
for fn in sorted(os.listdir(R + "asm/funcs")):
    if not fn.endswith(".s"):
        continue
    txt = open(R + "asm/funcs/" + fn).read()
    m = re.search(r"/\* [0-9A-F]+ ([0-9A-F]{8}) ", txt)
    g = re.search(r"glabel (\S+)", txt)
    if not m or not g:
        continue
    fa = int(m.group(1), 16)
    funcs.append((fa, g.group(1), set(REF.findall(txt))))
funcs.sort()
# dedupe duplicate-address entries
seen, fl = set(), []
for f in funcs:
    if f[0] in seen:
        continue
    seen.add(f[0]); fl.append(f)
funcs = fl
items = {}
for i, (fa, name, syms) in enumerate(funcs):
    for s in syms:
        if s in addr and not (bounds["main_TEXT_START"] <= addr[s] < bounds["main_DATA_START"]):
            items.setdefault(addr[s], {"names": set(), "refs": set()})
            items[addr[s]]["names"].add(s)
            items[addr[s]]["refs"].add(i)
def section(a):
    if a < bounds["main_TEXT_START"]:
        return "rodata"
    if a < bounds["main_BSS_START"]:
        return "data"
    return "bss"
out = {"bounds": bounds,
       "funcs": [{"addr": fa, "name": n, "obj": obj_of("text", fa)} for fa, n, _ in funcs],
       "items": [{"addr": a, "names": sorted(v["names"]), "sec": section(a),
                  "obj": obj_of(section(a), a), "refs": sorted(v["refs"])}
                 for a, v in sorted(items.items())]}
json.dump(out, open(R + "tmp/owners.json", "w"), indent=0)
print(len(out["funcs"]), "funcs,", len(out["items"]), "items;", {k: hex(v) for k, v in bounds.items()})
