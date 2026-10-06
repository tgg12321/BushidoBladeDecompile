#!/usr/bin/env python3
# F20 batch c, on top of f20b: func_80077D00 returns Unk8009BD24Block * (&D_8009BD24) and its callers
# read the record by member: 3AB48 func_8005C2A8, 51268 func_80069A30 / func_80069A8C, 5ED34
# func_8006E8CC, 25788 func_80034F88 / func_8003504C / func_80035280, 2B344 func_8003C714.
# usage: f20c.py [opt=<name>,...]   writes tmp/p2/f20c/ (scratch only)
#   opts name measured alternatives (cvar / cif, idxvar / sw, q8, fl_shift, dsth, walker variants)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f20b as B
sys.argv = _argv
OUT = "tmp/p2/f20c/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = B.sub1
fn = B.fn
T = "Unk8009BD24Block"

def bb2(h):
    h = sub1(h, "extern u8 D_8008EC30;\n", "extern u8 D_8008EC30[4];\n")
    return sub1(h, "extern s32 *func_80077D00(void);", "extern %s *func_80077D00(void);" % T)

def s64FD8(t):
    return sub1(t, "s32* func_80077D00(void) {\n    return (s32 *)&D_8009BD24;\n",
                "%s *func_80077D00(void) {\n    return &D_8009BD24;\n" % T)

def s3AB48(a):
    return sub1(a, "(func_80077D00()[5] & 0xF) == 3", "func_80077D00()->unk14_0 == 3")

def s51268(s):
    for f in ("func_80069A30", "func_80069A8C"):
        def g(b):
            b = sub1(b, "    s32 *p = func_80077D00();\n", "    %s *p = func_80077D00();\n" % T)
            return sub1(b, "if (p[8] & 1) {", "if (p->unk20_0) {")
        s = fn(s, f, g)
    return s

def s5ED34(e):
    def g(b):
        b = sub1(b, "    s32 *p;\n", "    %s *p;\n" % T)
        return sub1(b, "if (p[8] & 1) {", "if (p->unk20_0) {")
    return fn(e, "func_8006E8CC", g)

def b34F88(b):
    b = sub1(b, "    s32 *p;\n", "    %s *p;\n" % T)
    if not OPT & {"cvar", "cif", "cif12", "cvar1", "cvar2"}:   # the bit read into c at its word position (FAKE)
        b = sub1(b, "c = p[8] & 1;", "c = p->unk20_0;")
        b = sub1(b, "            c = p[8] & 2;\n",
                 "            /* FAKE: bits 1 and 2 are read back at their word position (<< 1 / << 2;\n"
                 "               c is only tested): read plain, each extract adds an srl (srl + andi 1\n"
                 "               where the target tests andi 2 / andi 4), score 4 (2 per site);\n"
                 "               tested directly (`if (p->unk20_1) {` / `if (p->unk20_2) {`), 35. */\n"
                 "            c = p->unk20_1 << 1;\n")
        b = sub1(b, "c = p[8] & 4;", "c = p->unk20_2 << 2;")
    elif OPT & {"cif12", "cvar1", "cvar2"}:
        b = sub1(b, "c = p[8] & 1;", "c = p->unk20_0;")
        if "cif12" in OPT:
            for k, m in ((1, 2), (2, 4)):
                b = re.sub(r"( +)c = p\[8\] & %d;\n +if \(c\) \{" % m, r"\1if (p->unk20_%d) {" % k, b)
        else:
            b = sub1(b, "c = p[8] & 2;", "c = p->unk20_1;" if "cvar1" in OPT else "c = p->unk20_1 << 1;")
            b = sub1(b, "c = p[8] & 4;", "c = p->unk20_2;" if "cvar2" in OPT else "c = p->unk20_2 << 2;")
    elif "cvar" in OPT:   # the bit read into c, then tested
        b = sub1(b, "c = p[8] & 1;", "c = p->unk20_0;")
        b = sub1(b, "c = p[8] & 2;", "c = p->unk20_1;")
        b = sub1(b, "c = p[8] & 4;", "c = p->unk20_2;")
    elif "cif" in OPT:   # the bit tested directly
        for k, m in ((0, 1), (1, 2), (2, 4)):
            b = re.sub(r"( +)c = p\[8\] & %d;\n +if \(c\) \{" % m, r"\1if (p->unk20_%d) {" % k, b)
        assert "p[8]" not in b
    b = sub1(b, "c = *((u8 *)p + (s32)q + 0x17);", "c = p->unk17[(s32)q];")
    return b

def b3504C(b):
    b = sub1(b, "    s32 *p;\n", "    %s *p;\n" % T)
    for a, c in (("((u32)p[5] >> 4) & 0x3F", "p->unk14_4"),
                 ("((u32)p[5] >> 17) & 1", "p->unk14_17"),
                 ("((u32)p[5] >> 18) & 7", "p->unk14_18"),
                 ("(((u32)p[5] >> 10) & 3)", "p->unk14_10"),
                 ("((u32)p[5] >> 12) & 3", "p->unk14_12"),
                 ("(u32)p[5] & 0x4000", "p->unk14_14"),
                 ("((u32)p[5] >> 15) & 3", "p->unk14_15")):
        b = sub1(b, a, c)
    if "sw" in OPT:
        b = sub1(b, "        D_800A389B = p->unk14_10 + 3;\n        idx = p->unk14_12;\n",
                 "        idx = p->unk14_12;\n        D_800A389B = p->unk14_10 + 3;\n")
    if "idxvar" not in OPT and "sw" not in OPT:
        b = sub1(b, "        D_800A389B = p->unk14_10 + 3;\n        idx = p->unk14_12;\n        D_800A36CC = (&D_8008EC30)[idx];\n",
                 "        D_800A389B = p->unk14_10 + 3;\n        D_800A36CC = D_8008EC30[p->unk14_12];\n")
        b = sub1(b, "        u32 idx;\n", "")
    if "w1" in OPT:   # a typed round-record walker for the first loop
        b = sub1(b, "    u8 *s;\n", "    u8 *s;\n    Unk8009BD24Record *r;\n")
        b = sub1(b, "    s = (u8 *)p;\n", "    r = p->unk00[0];\n")
        b = sub1(b, "(&D_8008D55C)[s[0]]", "(&D_8008D55C)[r->chr]")
        b = sub1(b, "        tmp = s[1];\n        s += 10;\n", "        tmp = r->unk1;\n        r += 5;\n")
    if "w3" in OPT:   # s itself becomes the typed walker; its second use (D_801027D8) goes direct
        b = sub1(b, "    u8 *s;\n", "    Unk8009BD24Record *s;\n")
        b = sub1(b, "    s = (u8 *)p;\n", "    s = p->unk00[0];\n")
        b = sub1(b, "(&D_8008D55C)[s[0]]", "(&D_8008D55C)[s->chr]")
        b = sub1(b, "        tmp = s[1];\n        s += 10;\n", "        tmp = s->unk1;\n        s += 5;\n")
        b = sub1(b, "        s = &D_801027D8;\n", "")
        b = sub1(b, "            u8 *dst_d = s;\n", "            u8 *dst_d = &D_801027D8;\n")
    if "w2" in OPT:   # indexed, no walker
        b = sub1(b, "    s = (u8 *)p;\n", "")
        b = sub1(b, "(&D_8008D55C)[s[0]]", "(&D_8008D55C)[p->unk00[i][0].chr]")
        b = sub1(b, "        tmp = s[1];\n        s += 10;\n", "        tmp = p->unk00[i][0].unk1;\n")
    if "pp1" in OPT:   # the round record by index
        b = sub1(b, "                    u8 *pp = (u8 *)p + off;\n                    *db = (&D_8008D55C)[pp[0]];\n                    off += 10;\n",
                 "                    Unk8009BD24Record *pp = &p->unk00[k][j];\n                    *db = (&D_8008D55C)[pp->chr];\n")
        b = sub1(b, "                    *da = pp[1];\n", "                    *da = pp->unk1;\n")
        b = sub1(b, "                s32 off = j << 1;\n", "")
    if "q8" not in OPT:
        b = sub1(b, "    q = &p[8];\n    D_80102778.unk_E = ((u32)*q >> 3) & 1;\n", "    D_80102778.unk_E = p->unk20_3;\n")
        b = sub1(b, "    s32 *q;\n", "")
    return b

def b35280(b):
    b = sub1(b, "    s32 *p;\n", "    %s *p;\n" % T)
    b = sub1(b, "        ((u8 *)p + i)[0x17] = *src;\n        ((u8 *)p + i)[0x1D] = *src;\n",
             "        p->unk17[i] = *src;\n        p->unk1D[i] = *src;\n")
    old = """    flags = p[8];
    flags0 = (flags & ~1) | (D_80106A50.flags & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (D_80106A50.flags & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (D_80106A50.flags & 4);
    p[8] = flags2;
"""
    if "fl_shift" in OPT:
        new = ("    p->unk20_0 = D_80106A50.flags;\n"
               "    p->unk20_1 = D_80106A50.flags >> 1;\n"
               "    p->unk20_2 = D_80106A50.flags >> 2;\n")
    else:
        new = ("    p->unk20_0 = D_80106A50.flags & 1;\n"
               "    p->unk20_1 = (D_80106A50.flags >> 1) & 1;\n"
               "    p->unk20_2 = (D_80106A50.flags >> 2) & 1;\n")
    b = sub1(b, old, new)
    i = b.index("    /* FAKE: the flag merge is staged")
    j = b.index("    s32 flags2;\n") + len("    s32 flags2;\n")
    b = b[:i] + b[j:]
    b = sub1(b, "    s32 flags;\n", "")
    for o, f, v in (("0x21", "unk0", "mn"), ("0x22", "unk1", "sc"), ("0x23", "unk2", "hs"), ("0x24", "unk3", "t")):
        b = sub1(b, "((u8 *)p)[i * 4 + %s] = %s;" % (o, v), "p->unk21[i].%s = %s;" % (f, v))
    return b

def s25788(u):
    u = sub1(u, """ * (include/game.h): flags bits 0-2 are cleared and re-set from p[8] bits 0-2,
 * then the three colour bytes at p+0x17 are copied to D_80106A50.color.""", """ * (include/game.h): flags bits 0-2 are cleared and re-set from p->unk20_0..2,
 * then the three colour bytes at p->unk17 are copied to D_80106A50.color.""")
    for f, g in (("func_80034F88", b34F88), ("func_8003504C", b3504C), ("func_80035280", b35280)):
        u = fn(u, f, g)
    return u

def s2B344(w):
    def g(b):
        b = sub1(b, "    s32 *s0;\n", "    %s *s0;\n" % T)
        if "dsth" not in OPT:   # no holder
            b = sub1(b, "    u8 *dst;\n", "")
            b = sub1(b, "        dst = (u8 *)s0 + i * 4;\n", "")
            for o, m in (("0x21", "unk0"), ("0x22", "unk1"), ("0x23", "unk2"), ("0x24", "unk3")):
                b = sub1(b, "dst[%s] = " % o, "s0->unk21[i].%s = " % m)
        else:   # Unk8009BD45Rec * holder
            b = sub1(b, "    u8 *dst;\n", "    Unk8009BD45Rec *dst;\n")
            b = sub1(b, "        dst = (u8 *)s0 + i * 4;\n", "        dst = &s0->unk21[i];\n")
            for o, m in (("0x21", "unk0"), ("0x22", "unk1"), ("0x23", "unk2"), ("0x24", "unk3")):
                b = sub1(b, "dst[%s] = " % o, "dst->%s = " % m)
        for o, m in (("0x2D", "unk2D[0]"), ("0x2E", "unk2D[1]"), ("0x2F", "unk2D[2]"), ("0x30", "unk30")):
            b = sub1(b, "*((u8 *)s0 + %s) = " % o, "s0->%s = " % m)
        return b
    return fn(w, "func_8003C714", g)

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(B.write())
    out["bb2.h"] = bb2(out["bb2.h"])
    out["64FD8.c"] = s64FD8(out["64FD8.c"])
    out["3AB48.c"] = s3AB48(out["3AB48.c"])
    out["51268.c"] = s51268(out["51268.c"])
    out["5ED34.c"] = s5ED34(out["5ED34.c"])
    out["25788.c"] = s25788(B.A.show("src/main/25788.c"))
    out["2B344.c"] = s2B344(B.A.show("src/main/2B344.c"))
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f20c", sorted(OPT))
