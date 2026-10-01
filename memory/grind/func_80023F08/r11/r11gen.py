"""Generate Ruling 11 split spellings of `temp` from a body file.
usage: python r11gen.py <body.c> <outdir>
Value line ranges are located by anchors so the script survives small edits."""
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


def gen(vals, name):
    L = list(L0)
    for v in vals:
        a, n = V[v]
        for i in range(a, a + n):
            if v == "face" and "ang[0] = ang[1] = temp;" in L[i]:
                L[i] = L[i].replace("= temp;", "= face;")
                continue
            if v == "face90" and "temp += 0x400;" in L[i]:
                L[i] = L[i].replace("temp += 0x400;", "face90 = %s + 0x400;" % ("face" if "face" in vals else "temp"))
                continue
            L[i] = re.sub(r"\btemp\b", v, L[i])
    t = "\n".join(L)
    decls = "".join(f"    s32 {v};\n" for v in vals)
    t = t.replace("    s32 temp;\n", "    s32 temp;\n" + decls, 1)
    if not re.search(r"\btemp\b", t.split("s32 temp;", 1)[1]):
        t = t.replace("    s32 temp;\n", "", 1)
    open(f"{outdir}/{name}.c", "w", newline="\n").write(t)


for v in V:
    gen([v], "split_" + v)
gen(list(V), "split_all")
