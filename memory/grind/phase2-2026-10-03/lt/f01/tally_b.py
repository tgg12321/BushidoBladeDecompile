#!/usr/bin/env python3
# tally_b.py : cast tally of the staged 51268.c vs HEAD (removed / added per cast spelling), code lines only
import re, collections, subprocess
d = subprocess.run(["git", "diff", "HEAD", "--", "src/main/51268.c"], capture_output=True, text=True, encoding="utf-8").stdout
c = collections.Counter()
for l in d.splitlines():
    if l[:1] not in "+-" or l[:3] in ("+++", "---"):
        continue
    t = l[1:].strip()
    if t.startswith("/*") or re.match(r"\*(\s|/|$)", t):
        continue
    for m in re.findall(r"\((?:s8|u8|s16|u16|s32|u32|void|Unk\w+|SVECTOR|MATRIX|POLY_FT4|struct Ob)\s*\**\s*\)", t):
        c[(l[0], re.sub(r"\s+", " ", m))] += 1
for k in sorted(set(x[1] for x in c)):
    print(k, "-%d" % c[("-", k)], "+%d" % c[("+", k)])
