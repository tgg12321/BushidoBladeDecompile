#!/usr/bin/env python3
"""libfuncs.py <tree>: Q69 per-function provenance test (read-only on <tree>'s build/). A function has library
provenance when its address lies in a verbatim module placement of memory/closer/psyq-library-census.md
section (a), or has a docs/naming/libscan/rename_manifest.csv row (libscan evidence, not REJECTED). A file
is library code when every function it defines has provenance. Prints every file with any provenance."""
import csv, os, re, subprocess, sys
T = sys.argv[1]
os.chdir(T)
cen = open("memory/closer/psyq-library-census.md").read()
mods = [(int(a, 16), int(b, 16), m.strip()) for a, b, m in
        re.findall(r"^\| (0x[0-9A-F]{8}) \| (0x[0-9A-F]{8}) \| ([^|]+)\|", cen.split("## (b)")[0], re.M)]
man = {}
for r in csv.DictReader(open("docs/naming/libscan/rename_manifest.csv", newline="")):
    if r["evidence"].startswith("libscan") and not r["classification"].startswith("REJECTED"):
        man[int(r["addr"], 16)] = f'{r["lib"]}/{r["module"]}'
mp = open("build/bb2.map").read()
base = {f: int(a, 16) for a, f in re.findall(r"^ \.text\s+0x([0-9a-f]+)\s+0x[0-9a-f]+ build/src/(\w+)\.o", mp, re.M)}
for f in sorted(base, key=base.get):
    out = subprocess.run(f"mipsel-linux-gnu-nm -n --defined-only build/src/{f}.o", shell=True,
                         capture_output=True, text=True).stdout
    fns = [(base[f] + int(l.split()[0], 16), l.split()[2]) for l in out.splitlines()
           if len(l.split()) == 3 and l.split()[1] in "tT"]
    if not fns:
        continue
    prov = [(a, n, next((m for x, y, m in mods if x <= a < y), None) or man.get(a)) for a, n in fns]
    have = [p for p in prov if p[2]]
    if not have:
        continue
    miss = [f"{n}@{a:#x}" for a, n, p in prov if not p]
    tag = "LIBRARY (all)" if not miss else f"PARTLY ({len(have)}/{len(prov)})"
    print(f"{f:22s} {tag}" + (f"  no provenance: {', '.join(miss[:12])}{' ...' if len(miss) > 12 else ''}" if miss else ""))
