#!/usr/bin/env python3
"""cutcheck.py <tu> <first-func-of-part-2>: split feasibility under the per-file model. Lists gp symbols used
on both sides of the cut and flags those that cannot be defined in two files (initialized: addr < 0x800A3308,
or reached gp at an offset => .sdata / static). COMMON (tentative) symbols may be defined in both parts."""
import json, re, subprocess, sys
tu, cut = sys.argv[1], sys.argv[2]
A = json.load(open("/tmp/q56/model_analysis.json"))
out = subprocess.run(f"mipsel-linux-gnu-objdump -d /tmp/q56/refobj/{tu}.o", shell=True, capture_output=True, text=True).stdout
order = re.findall(r"^[0-9a-f]+ <([^>]+)>:", out, re.M)
k = order.index(cut)
side = {f: (0 if i < k else 1) for i, f in enumerate(order)}
used = {}
for S, v in A["objects"][tu]["gp"].items():
    for fn, mn, add in v:
        used.setdefault(S, set()).add(side[fn])
both = [S for S, s in used.items() if s == {0, 1}]
print(f"{tu}: cut before {cut} ({k} of {len(order)} functions: {order[k-1]} | {cut})")
print("  part-2-only gp syms:", sorted(S for S, s in used.items() if s == {1}))
print("  gp syms on both sides:", both)
for S in both:
    a = int(A["symbols"][S]["addr"], 16)
    bad = a < 0x800A3308 or A["symbols"][S]["offset_gp"]
    print(f"    {S} {hex(a)} {'BLOCKS the cut (initialized/static: one defining file)' if bad else 'ok (COMMON: tentative definition in both parts)'}")
