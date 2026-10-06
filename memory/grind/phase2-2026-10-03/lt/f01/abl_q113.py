#!/usr/bin/env python3
# Q113 ablations: func_8006C21C's FAKEs (mode; col; the dtd / xpos / ypos / tw cluster, and tw alone) on
# q113.py's output (tmp/p2/q113/51268.c). Writes tmp/p2/q113/abl/<name>.c + list.txt (sandbox --candidate
# on the tree once Q113 is applied, or score_b2.py on the scratch copy).
import os, re
NL = chr(10)
S = open("tmp/p2/q113/51268.c", encoding="utf-8").read()
m = re.search(r"\nvoid func_8006C21C\(s32 \*arg0\) \{", S); i = m.start() + 1; j = S.index("\n}\n", i) + 3
B0 = S[i:j]
OUT = "tmp/p2/q113/abl"
os.makedirs(OUT, exist_ok=True)

def rep(b, a, c, n=1):
    assert b.count(a) == n, (a[:70], b.count(a), n)
    return b.replace(a, c)

def drop_comment(b, start):
    a = b.index(start)
    return b[:a] + b[b.index("*/\n", a) + 3:]

V = {}
b = drop_comment(B0, "    /* FAKE: constant-holder (named-local-fake-exception)")
b = rep(b, "    s32 mode;\n", ""); b = rep(b, "    mode = 0;\n", "")
b = rep(b, ", mode)", ", 0)", 3)
V["mode"] = b
# col: each arm's constant written where it is read
b = drop_comment(B0, "    /* FAKE: per-branch constant holder")
b = rep(b, "    s32 col;\n", "")
out, cur = [], None
for l in b.split(NL):
    mm = re.match(r"\s+col = (0x80|0);$", l)
    if mm:
        cur = mm.group(1); continue
    if re.search(r"= col;$", l):
        l = l.replace("= col;", "= %s;" % cur)
    out.append(l)
b = NL.join(out)
assert not re.search(r"\bcol\b", b)
V["col"] = b
# the always-zero narrow locals, as a cluster, and tw alone
b = drop_comment(B0, "    /* FAKE: always-zero narrow locals")
b = rep(b, "    s16 dtd;\n    s16 xpos;\n    s16 ypos;\n    s16 tw;\n", "")
b = rep(b, "    dtd = 0;\n    xpos = 0;\n    ypos = 0;\n    tw = 0;\n", "")
b = rep(b, ", 1, dtd, ", ", 1, 0, ", 2)
b = rep(b, ", (RECT *)tw);", ", 0);", 2)
b = re.sub(r"= (xpos|ypos);", "= 0;", b)
assert not re.search(r"\b(dtd|xpos|ypos|tw)\b", b)
V["zeros"] = b
b = rep(B0, "    s16 tw;\n", "")
b = rep(b, "    tw = 0;\n", "")
b = rep(b, ", (RECT *)tw);", ", 0);", 2)
V["tw"] = b
with open(OUT + "/list.txt", "w", encoding="utf-8", newline=NL) as fl:
    for n, b in V.items():
        open("%s/%s.c" % (OUT, n), "w", encoding="utf-8", newline=NL).write(b)
        fl.write("%s func_8006C21C\n" % n)
print(len(V), "variants")
