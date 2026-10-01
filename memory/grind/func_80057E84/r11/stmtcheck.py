#!/usr/bin/env python3
"""stmtcheck.py <reuse.c> <twin.c>: Ruling 11 (C)(2) receipt. Strips comments, deletes the
declarations of vtx/node/route and of every split value, renames each value back to the
variable that holds it in the reuse body (gen.VALUES), and compares the token streams."""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen

back = {v: var for var, vals in gen.VALUES.items() for v in vals}
names = "|".join(list(back) + list(gen.VALUES))


def norm(path):
    s = open(path).read()
    s = re.sub(r"/\*.*?\*/", " ", s, flags=re.S)
    s = re.sub(r"\b(s16 \*|CpuWaypoint \*|CpuRoute \*)(%s);" % names, " ", s)
    toks = re.findall(r"[A-Za-z_]\w*|0x[0-9A-Fa-f]+|\d+|->|\S", s)
    return [back.get(t, t) for t in toks]


a, b = norm(sys.argv[1]), norm(sys.argv[2])
print("IDENTICAL statement lists" if a == b else f"DIFFER ({len(a)} vs {len(b)} tokens)")
