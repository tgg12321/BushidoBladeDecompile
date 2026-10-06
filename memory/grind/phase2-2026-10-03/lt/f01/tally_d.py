#!/usr/bin/env python3
# tally_d.py OLD NEW : cast tally (removed / added per cast spelling) between two copies of a C file,
# code only: /* */ and // comments are blanked first (line structure kept), so cast-like text inside
# comments is not counted. Prints each spelling and the totals.
import re, sys, difflib, collections
def code(path):
    t = open(path, encoding="utf-8").read()
    t = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), t, flags=re.S)
    t = re.sub(r"//[^\n]*", "", t)
    return [l.rstrip() for l in t.splitlines()]
a, b = code(sys.argv[1]), code(sys.argv[2])
c = collections.Counter()
for l in difflib.unified_diff(a, b, lineterm="", n=0):
    if l[:1] not in "+-" or l[:3] in ("+++", "---"):
        continue
    for m in re.findall(r"\((?:const\s+)?(?:s8|u8|s16|u16|s32|u32|void|char|int|Unk\w+|TexRec|POLY_FT4|SVECTOR|VECTOR|MATRIX)\s*\**\s*\)", l[1:]):
        c[(l[0], re.sub(r"\s+", " ", m))] += 1
for k in sorted(set(x[1] for x in c)):
    print(k, "-%d" % c[("-", k)], "+%d" % c[("+", k)])
print("total -%d +%d" % (sum(v for (s, _), v in c.items() if s == "-"), sum(v for (s, _), v in c.items() if s == "+")))
