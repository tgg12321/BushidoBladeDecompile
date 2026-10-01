#!/usr/bin/env python3
"""stmtcheck.py <reuse.c> <twin.c>: Ruling 11 (C)(2) receipt. Strips comments, deletes the
`s32 work1..work5;` / `s32 <value>_;` declarations, renames every `<value>_` back to the work
variable that holds it (roles.VALUES), and compares the token streams."""
import re, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import roles

back = {f"{v}_": var for var, vals in roles.VALUES.items() for v in vals}


def norm(path):
    s = open(path).read()
    s = re.sub(r"/\*.*?\*/", " ", s, flags=re.S)
    s = re.sub(r"\bs32 (work[1-5]|[a-z0-9]+_);", " ", s)
    toks = re.findall(r"[A-Za-z_]\w*|0x[0-9A-Fa-f]+|\d+|\S", s)
    return [back.get(t, t) for t in toks]


a, b = norm(sys.argv[1]), norm(sys.argv[2])
print("IDENTICAL statement lists" if a == b else f"DIFFER ({len(a)} vs {len(b)} tokens)")
