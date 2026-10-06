#!/usr/bin/env python3
# F07 (on f04b.py): 3AB48's stage-collision work area. D_800A33F4 becomes Work_80053E9C * (game.h)
# and the W cast macro goes (its users read D_800A33F4-> directly). The two 16-byte points the
# entry functions copy in (+0x08 start, +0x18 end; their +0x14 / +0x24 pads unread) are VECTORs;
# the four per-function 16-byte copy structs go. D_800EF9F8 (the work area func_80053304 /
# func_80053584 use) is declared as the Work_80053E9C it is, which retires its +8 alias D_800EFA00.
# usage: f07.py [opt=<name>,...]   writes tmp/p2/f07/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f04b as B
sys.argv = _argv
OUT = "tmp/p2/f07/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = B.sub1
fn = B.fn
R = "D_800A33F4"
VEC = {"unk8": "unk8.vx", "unkC": "unk8.vy", "unk10": "unk8.vz",
       "unk18": "unk18.vx", "unk1C": "unk18.vy", "unk20": "unk18.vz"}

def game(g):
    g = sub1(g, """    s32 unk0;
    s16 unk4;
    s16 unk6;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
    s32 unk24;
    s32 unk28;
""", """    s32 unk0;
    u16 unk4;
    s16 unk6;
    VECTOR unk8;
    VECTOR unk18;
    s32 unk28;
""")
    return g

def s3AB48(a):
    a = sub1(a, "static s32 D_800A33F4;\n", "static Work_80053E9C *D_800A33F4;\n")
    a = sub1(a, "#define W ((Work_80053E9C *)D_800A33F4)\n", "")
    a = sub1(a, "#undef W\n", "")
    def w(m):
        f = m.group(1)
        return "D_800A33F4->" + VEC.get(f, f)
    a = re.sub(r"\bW->(unk[0-9A-F]+)\b", w, a)
    assert not re.search(r"\bW->", a)
    a = sub1(a, "extern u8 D_800EFA00;\nextern u8 D_800EF9F8;\n", "extern Work_80053E9C D_800EF9F8;\n")
    for t in ("_S16_53304", "_S16_5344C", "_S16_53584", "_S16_53614"):
        a = sub1(a, "typedef struct { s32 a, b, c, d; } %s;\n" % t, "")
        a = a.replace("*(%s *)&D_800EFA00 = *(%s *)arg0;" % (t, t), "D_800EF9F8.unk8 = *(VECTOR *)arg0;")
        a = a.replace("*(%s *)((u8 *)D_800A33F4 + 8) = *(%s *)arg0;" % (t, t), "D_800A33F4->unk8 = *(VECTOR *)arg0;")
        a = a.replace("*(%s *)((u8 *)D_800A33F4 + 0x18) = *(%s *)arg1;" % (t, t), "D_800A33F4->unk18 = *(VECTOR *)arg1;")
    a = a.replace("D_800A33F4 = (s32)&D_800EF9F8;", "D_800A33F4 = &D_800EF9F8;")
    a = sub1(a, "    D_800A33F4 = (u8 *)arg4;\n", "    D_800A33F4 = (Work_80053E9C *)arg4;\n")
    a = sub1(a, "    D_800A33F4 = arg4;\n", "    D_800A33F4 = (Work_80053E9C *)arg4;\n")
    sq = """            *(s32 *)((u8 *)D_800A33F4 + 0x18) - *(s32 *)((u8 *)D_800A33F4 + 0x8),
            *(s32 *)((u8 *)D_800A33F4 + 0x1C) - *(s32 *)((u8 *)D_800A33F4 + 0xC),
            *(s32 *)((u8 *)D_800A33F4 + 0x20) - *(s32 *)((u8 *)D_800A33F4 + 0x10)) <= 0x9C3F) {
"""
    a = sub1(a, sq, """            D_800A33F4->unk18.vx - D_800A33F4->unk8.vx,
            D_800A33F4->unk18.vy - D_800A33F4->unk8.vy,
            D_800A33F4->unk18.vz - D_800A33F4->unk8.vz) <= 0x9C3F) {
""", 2)
    blk = """        p = (u8 *)D_800A33F4;
        hi0 = *(s32 *)(p + 0x18);
        a = *(s32 *)(p + 0x8);
        hi1 = *(s32 *)(p + 0x1C);
        b = *(s32 *)(p + 0xC);
        *(s32 *)(p + 0x5C) = (s32)func_80053754;
        hi2 = *(s32 *)(p + 0x20);
        *(s32 *)(p + 0x8)  = a - ((hi0 - a) << 1);
        *(s32 *)(p + 0xC)  = b - ((hi1 - b) << 1);
        c = *(s32 *)(p + 0x10);
        *(s32 *)(p + 0x10) = c - ((hi2 - c) << 1);
"""
    a = sub1(a, blk, """        p = D_800A33F4;
        hi0 = p->unk18.vx;
        a = p->unk8.vx;
        hi1 = p->unk18.vy;
        b = p->unk8.vy;
        p->unk5C = func_80053754;
        hi2 = p->unk18.vz;
        p->unk8.vx = a - ((hi0 - a) << 1);
        p->unk8.vy = b - ((hi1 - b) << 1);
        c = p->unk8.vz;
        p->unk8.vz = c - ((hi2 - c) << 1);
""", 2)
    a = a.replace("*(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;", "D_800A33F4->unk5C = func_80053E9C;")
    for f in ("func_80053304", "func_8005344C"):
        a = fn(a, f, lambda b: sub1(b, "    u8 *p;\n", "    Work_80053E9C *p;\n"))
    def g(b):
        b = sub1(b, "    u8 *p = D_800A33F4;\n", "    Work_80053E9C *p = D_800A33F4;\n")
        for o, m in (("*(s32 *)(p + 0)", "p->unk0"), ("*(s16 *)(p + 0x48)", "p->unk48"), ("*(s16 *)(p + 0x4A)", "p->unk4A"),
                     ("*(s32 *)(p + 0x38)", "p->unk38"), ("*(s32 *)(p + 0x3C)", "p->unk3C"), ("*(s32 *)(p + 0x40)", "p->unk40"),
                     ("*(s32 *)(p + 0x28)", "p->unk28"), ("*(s32 *)(p + 0x2C)", "p->unk2C"), ("*(s32 *)(p + 0x30)", "p->unk30"),
                     ("*(u16 *)(p + 4)", "p->unk4")):
            b = sub1(b, o, m)
        return b
    a = fn(a, "func_80053694", g)
    if "no17" not in OPT:   # F17: the per-cell plane stream is walked as halfwords
        def h(b):
            b = sub1(b, "    s32 data;\n", "    s16 *data;\n")
            b = sub1(b, "    data = D_800A33F0 + D_800A33F4->unkE0;\n", "    data = (s16 *)(D_800A33F0 + D_800A33F4->unkE0);\n")
            b = b.replace("*(s16 *)data", "*data").replace("*(u16 *)data", "(u16)*data")
            b = b.replace("data += 2;", "data++;").replace("data += 18;", "data += 9;")
            b = b.replace("data += n * 4;", "data += n * 2;").replace("data += (n + 1) * 4;", "data += (n + 1) * 2;")
            b = b.replace("func_80052C4C(data, ", "func_80052C4C((s32)data, ")
            assert not re.search(r"\(s16 \*\)data|\(u16 \*\)data|data \+= (2|18|n \* 4)", b)
            return b
        a = fn(a, "func_80053754", h)
        a = fn(a, "func_80053E9C", h)
    left = [l.strip() for l in a.split(NL) if "D_800A33F4" in l and ("(u8 *)" in l or "(s32)" in l)]
    for l in left:
        print("LEFT", l)
    return a

def syms(u):
    return sub1(u, "D_800EFA00 = 0x800EFA00;\n", "")

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(B.write())
    out["3AB48.c"] = s3AB48(out["3AB48.c"])
    out["game.h"] = game(out["game.h"])
    out["undefined_syms_auto.txt"] = syms(out["undefined_syms_auto.txt"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f07", sorted(OPT))
