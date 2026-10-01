"""All set partitions of temp's values -> one body per partition (group k uses its own local).
usage: python r11part.py <body.c> <outdir>"""
import re
import sys

src, outdir = sys.argv[1:3]
s = open(src).read()
L0 = s.split("\n")


def find(anchor):
    for i, l in enumerate(L0):
        if anchor in l:
            return i
    raise KeyError(anchor)


V = {
    "lim": (find("temp = (rec->unk_58[2] >> 4) * 0x88;"), 5),
    "gap": (find("temp = (rec->unk_1D8 - rec->unk_1C8.vy) & 0xFFF;"), 7),
    "turn": (find("temp = (rec->unk_14C * rec->unk_44) / 24576;"), 3),
    "side": (find("temp = (rec->unk_24.held & 0x1000) ? 1"), 6),
}
names = list(V)


def partitions(seq):
    if not seq:
        yield []
        return
    first, rest = seq[0], seq[1:]
    for p in partitions(rest):
        for i in range(len(p)):
            yield p[:i] + [[first] + p[i]] + p[i + 1:]
        yield [[first]] + p


GV = ["temp", "tmp_b", "tmp_c", "tmp_d"]
for p in partitions(names):
    if len(p) == 1:
        continue
    L = list(L0)
    for gi, grp in enumerate(p):
        for v in grp:
            a, n = V[v]
            for i in range(a, a + n):
                L[i] = re.sub(r"\btemp\b", GV[gi], L[i])
    t = "\n".join(L)
    decl = "".join(f"    s32 {GV[gi]};\n" for gi in range(1, len(p)))
    t = t.replace("    s32 temp;\n", "    s32 temp;\n" + decl, 1)
    name = "part_" + "__".join("-".join(g) for g in p)
    open(f"{outdir}/{name}.c", "w", newline="\n").write(t)
