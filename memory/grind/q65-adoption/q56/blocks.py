#!/usr/bin/env python3
"""blocks.py: collapse regionmap_full.txt into per-owner blocks: owner = the set of TUs that reach the
address gp-relative (a gp access means that file DEFINES the object); '-' = no gp access (unowned)."""
import re
rows = []
for l in open("/tmp/q56/regionmap_full.txt"):
    p = l.split()
    a = int(p[0], 16); b = p[1]
    gp = sorted({x.split(":")[0] for x in p[3:] if x.endswith(":gp")})
    other = sorted({x.split(":")[0] for x in p[3:] if not x.endswith(":gp")})
    rows.append((a, b, ",".join(gp) or "-", ",".join(other)))
cur = None
for a, b, g, o in rows:
    key = g
    if cur and cur[0] == key:
        cur[2] = a; cur[3].add(o) if o else None; cur[4] += 1
    else:
        if cur: print(f"{cur[1]:08x}..{cur[2]:08x} n={cur[4]:3} gp-owner={cur[0]:45} other-refs={sorted(x for x in cur[3] if x)}")
        cur = [key, a, a, {o} if o else set(), 1]
print(f"{cur[1]:08x}..{cur[2]:08x} n={cur[4]:3} gp-owner={cur[0]:45} other-refs={sorted(x for x in cur[3] if x)}")
