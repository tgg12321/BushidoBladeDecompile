#!/usr/bin/env python3
# tally_c.py [OLD NEW] : cast tally (removed / added per cast spelling, code lines only) between two
# copies of 51268.c; default tmp/p2/f01b2/51268.c (F01b2) -> tmp/p2/f01c/51268.c (F01c).
import re, sys, difflib, collections
old = sys.argv[1] if len(sys.argv) > 2 else "tmp/p2/f01b2/51268.c"
new = sys.argv[2] if len(sys.argv) > 2 else "tmp/p2/f01c/51268.c"
a = open(old, encoding="utf-8").read().splitlines()
b = open(new, encoding="utf-8").read().splitlines()
c = collections.Counter()
for l in difflib.unified_diff(a, b, lineterm="", n=0):
    if l[:1] not in "+-" or l[:3] in ("+++", "---"):
        continue
    t = l[1:].strip()
    if t.startswith("/*") or re.match(r"\*(\s|/|$)", t):
        continue
    t = re.sub(r"/\*.*?\*/", "", t)
    for m in re.findall(r"\((?:s8|u8|s16|u16|s32|u32|void|Unk\w+|TexRec|POLY_FT4|SVECTOR|MATRIX)\s*\**\s*\)", t):
        c[(l[0], re.sub(r"\s+", " ", m))] += 1
for k in sorted(set(x[1] for x in c)):
    print(k, "-%d" % c[("-", k)], "+%d" % c[("+", k)])
print("total -%d +%d" % (sum(v for (s, _), v in c.items() if s == "-"), sum(v for (s, _), v in c.items() if s == "+")))
