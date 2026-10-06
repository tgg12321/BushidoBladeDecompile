#!/usr/bin/env python3
# F20 batch b, on top of f20a: the holders of the settings record become Unk8009BD24Block *.
# 51268: D_800A3524 (func_80068F70's arg1, typed so; 64FD8 passes &D_8009BD24).
# usage: f20b.py [measure] [opt=<name>,...]   writes tmp/p2/f20b/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f20a as A
sys.argv = _argv
OUT = "tmp/p2/f20b/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = A.sub1
R = "D_800A3524"

def fn(s, f, g):
    m = re.search(r"\n[A-Za-z0-9_]+[ *]+%s\([^;{]*\)\s*\{" % f, s)
    i = m.start() + 1
    j = s.index("\n}\n", i) + 3
    return s[:i] + g(s[i:j]) + s[j:]

W8 = "((s32 *)D_800A3524)[8]"

def b68F70(b):
    b = sub1(b, "s32 func_80068F70(s32 arg0, s32 *arg1) {", "s32 func_80068F70(s32 arg0, Unk8009BD24Block *arg1) {")
    b = sub1(b, "            D_800A3524 = (s32)arg1;\n", "            D_800A3524 = arg1;\n")
    b = sub1(b, "D_800A34FC->unk_30 = %s & 1;" % W8, "D_800A34FC->unk_30 = D_800A3524->unk20_0;")
    return b

def b69120(b):
    b = sub1(b, "    s32 *v0 = (s32 *)D_800A3524;\n", "    Unk8009BD24Block *v0 = D_800A3524;\n")
    b = sub1(b, "    v0 = (s32 *)D_800A3524;\n", "    v0 = D_800A3524;\n")
    b = sub1(b, "(v0[8] & 1)", "v0->unk20_0")
    b = sub1(b, "v0[8] & 1", "v0->unk20_0")
    return b

def b693CC(b):
    b = sub1(b, "    %s &= ~8;\nafter_move:" % W8, "    D_800A3524->unk20_3 = 0;\nafter_move:")
    b = sub1(b, """        %s =
            (%s & ~8) |
            (((((u32)%s >> 3) & 1) ^ 1) << 3);
""" % (W8, W8, W8), "        D_800A3524->unk20_3 ^= 1;\n" if "not3" not in OPT else
         "        D_800A3524->unk20_3 = !D_800A3524->unk20_3;\n")
    b = sub1(b, "        %s |= 8;\n" % W8, "        D_800A3524->unk20_3 = 1;\n")
    b = sub1(b, "        %s &= ~8;\n" % W8, "        D_800A3524->unk20_3 = 0;\n")
    return b

def bit3(b):
    return b.replace("%s & 8" % W8, "D_800A3524->unk20_3")

def bit0(b):
    return b.replace("%s & 1" % W8, "D_800A3524->unk20_0")

def b6B120(b):
    b = sub1(b, "(((u32 *)D_800A3524)[8] & 1) == i", "D_800A3524->unk20_0 == i")
    b = sub1(b, "((((u32 *)D_800A3524)[8] >> 1) & 1) == i", "D_800A3524->unk20_1 == i")
    b = sub1(b, "((((u32 *)D_800A3524)[8] >> 2) & 1) == i", "D_800A3524->unk20_2 == i")
    return b

def b6B578(b):
    if "w8" in OPT:
        return b
    for k, body in ((0, """            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~1u;
            u32 bit = f & 1;
            bit ^= 1;
            a3 |= bit;
            p[8] = (s32)a3;
"""), (1, """            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~2u;
            u32 bit = (f >> 1) & 1;
            bit ^= 1;
            bit <<= 1;
            a3 |= bit;
            p[8] = (s32)a3;
"""), (2, """            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~4u;
            u32 bit = (f >> 2) & 1;
            bit ^= 1;
            bit <<= 2;
            a3 |= bit;
            p[8] = (s32)a3;
""")):
        b = sub1(b, body, "            D_800A3524->unk20_%d ^= 1;\n" % k)
    return b

def b6C21C(b):
    return sub1(b, "*(u8 *)(D_800A3524 + D_800A34FC->unk_28.half[pl] + 0x17)",
                "D_800A3524->unk17[D_800A34FC->unk_28.half[pl]]")

def b6CBD4(b):
    b = sub1(b, "*((u8 *)D_800A3524 + i + 0x17) |= mask;", "D_800A3524->unk17[i] |= mask;")
    b = sub1(b, "*((u8 *)D_800A3524 + i + 0x17) &= ~mask;", "D_800A3524->unk17[i] &= ~mask;")
    return b

def b6CCC8(b):
    if not OPT & {"typed", "cc1", "cc2", "cc3", "cc3b", "cc4", "cc5", "cc6"}:
        return b   # no typed form matches: the rec lines stay as at HEAD (debt row)
    for v in ("cc3b", "cc6"):
        if v in OPT:
            for src in ("1A", "1D"):
                old = ("                    rec = (u8 *)D_800A3524 + j;\n"
                       "                    masked = *(rec + 0x%s) & (nib << fade);\n"
                       "                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);\n" % src)
                if v == "cc3b":   # per-member holders, source first
                    new = ("                    src = &D_800A3524->unk%s[j];\n"
                           "                    rec = &D_800A3524->unk17[j];\n"
                           "                    masked = *src & (nib << fade);\n"
                           "                    *rec = (*rec & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                else:   # the source byte read into a typed local first
                    new = ("                    c = D_800A3524->unk%s[j];\n"
                           "                    masked = c & (nib << fade);\n"
                           "                    D_800A3524->unk17[j] = (D_800A3524->unk17[j] & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                b = sub1(b, old, new)
            b = sub1(b, "    u8 *rec;\n", "    u8 *src;\n    u8 *rec;\n" if v == "cc3b" else "    u8 c;\n")
            return b
    for v in ("cc4", "cc5"):
        if v in OPT:
            for src in ("1A", "1D"):
                old = ("                    rec = (u8 *)D_800A3524 + j;\n"
                       "                    masked = *(rec + 0x%s) & (nib << fade);\n"
                       "                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);\n" % src)
                if v == "cc4":   # record holder, reloaded per byte
                    new = ("                    blk = D_800A3524;\n"
                           "                    masked = blk->unk%s[j] & (nib << fade);\n"
                           "                    blk->unk17[j] = (blk->unk17[j] & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                else:   # array-base holders
                    new = ("                    rec = D_800A3524->unk17;\n"
                           "                    src = D_800A3524->unk%s;\n"
                           "                    masked = src[j] & (nib << fade);\n"
                           "                    rec[j] = (rec[j] & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                b = sub1(b, old, new)
            b = sub1(b, "    u8 *rec;\n", "    Unk8009BD24Block *blk;\n" if v == "cc4" else "    u8 *rec;\n    u8 *src;\n")
            return b
    for v in ("cc1", "cc2", "cc3"):
        if v in OPT:
            for src in ("1A", "1D"):
                old = ("                    rec = (u8 *)D_800A3524 + j;\n"
                       "                    masked = *(rec + 0x%s) & (nib << fade);\n"
                       "                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);\n" % src)
                if v == "cc1":   # holder of the unk17 byte
                    new = ("                    rec = &D_800A3524->unk17[j];\n"
                           "                    masked = D_800A3524->unk%s[j] & (nib << fade);\n"
                           "                    *rec = (*rec & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                elif v == "cc2":   # holder of the source byte
                    new = ("                    rec = &D_800A3524->unk%s[j];\n"
                           "                    masked = *rec & (nib << fade);\n"
                           "                    D_800A3524->unk17[j] = (D_800A3524->unk17[j] & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                else:   # both
                    new = ("                    rec = &D_800A3524->unk17[j];\n"
                           "                    src = &D_800A3524->unk%s[j];\n"
                           "                    masked = *src & (nib << fade);\n"
                           "                    *rec = (*rec & ((i == 0) ? 0xF0 : 0xF)) + masked;\n" % src)
                b = sub1(b, old, new)
            if v == "cc3":
                b = sub1(b, "    u8 *rec;\n", "    u8 *rec;\n    u8 *src;\n")
            return b
    b = sub1(b, "                    rec = (u8 *)D_800A3524 + j;\n                    masked = *(rec + 0x1A) & (nib << fade);\n",
             "                    masked = D_800A3524->unk1A[j] & (nib << fade);\n")
    b = sub1(b, "                    rec = (u8 *)D_800A3524 + j;\n                    masked = *(rec + 0x1D) & (nib << fade);\n",
             "                    masked = D_800A3524->unk1D[j] & (nib << fade);\n")
    b = sub1(b, "*(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);",
             "D_800A3524->unk17[j] = (D_800A3524->unk17[j] & ((i == 0) ? 0xF0 : 0xF)) + masked;", 2)
    b = sub1(b, "    u8 *rec;\n", "")
    return b

def b6CFBC(b):
    return sub1(b, "*(u8 *)(D_800A3524 + outer + 0x17)", "D_800A3524->unk17[outer]")

def b6D5D4(b):
    b = sub1(b, """            p = (s32 *)((s32)D_800A3524 + 0x14);
            v = *p;
            *p = (v & 0xFFFDFFFF) | ((sval & 1) << 17);
""", "            D_800A3524->unk14_17 = sval;\n")
    b = sub1(b, "    s32 *p;\n    s32 v;\n", "")
    return b

def b6D808(b):
    b = sub1(b, "*(u8 *)(D_800A3524 + (i << 2) + 0x24)", "D_800A3524->unk21[i].unk3")
    for o, m in ((0x21, 0), (0x22, 1), (0x23, 2)):
        b = sub1(b, "*(u8 *)(D_800A3524 + (k << 2) + 0x%X)" % o, "D_800A3524->unk21[k].unk%d" % m)
    return b

BODIES = [("func_80068F70", b68F70), ("func_80069120", b69120), ("func_800693CC", b693CC),
          ("func_80069F80", bit3), ("func_8006A1A0", bit3), ("func_8006A880", bit0),
          ("func_8006B120", b6B120), ("func_8006B578", b6B578), ("func_8006C21C", b6C21C),
          ("func_8006CBD4", b6CBD4), ("func_8006CCC8", b6CCC8), ("func_8006CFBC", b6CFBC),
          ("func_8006D5D4", b6D5D4), ("func_8006D808", b6D808), ("func_8006E10C", bit0)]
FUNCS = [f for f, _ in BODIES]

def s51268(s):
    s = sub1(s, "static s32 D_800A3524;\n", "static Unk8009BD24Block *D_800A3524;\n")
    for f, g in BODIES:
        s = fn(s, f, g)
    return s

def bb2(h):
    h = sub1(h, "extern s32 func_8006E534(s32, s32, u8 *, u32);", "extern s32 func_8006E534(s32, s32, Unk8009BD24Block *, u32);")
    return sub1(h, "extern s32 func_80068F70(s32, s32 *);", "extern s32 func_80068F70(s32, Unk8009BD24Block *);")

def game(g):
    return sub1(g, "typedef struct {\n    void *f00;\n    s32 *f04;\n", "typedef struct {\n    Unk8009BD24Block *f00;\n    s32 *f04;\n")

S76D74 = """typedef struct {
    u8 cells[2][5][2];  /* 0x00: [row][col][{glyph, attr}] */
    u32 pad10 : 10;     /* 0x14 */
    u32 f10 : 2;
    u32 f12 : 2;
    u32 f14 : 1;
    u32 f15 : 2;
} S_80076D74;

"""

def s64FD8(t):
    t = sub1(t, "func_80068F70(a0, (s32 *)&D_8009BD24);", "func_80068F70(a0, &D_8009BD24);")
    t = sub1(t, "&D_8009BD24.unk00[0][0].chr", "&D_8009BD24")
    t = sub1(t, "s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {", "s32 func_800770B8(s32 arg0, Unk8009BD24Block *arg1, s32 arg2) {")
    t = sub1(t, "s32 func_800770B8(s32, s32, s32);", "s32 func_800770B8(s32, Unk8009BD24Block *, s32);")
    t = sub1(t, "func_800770B8(a0, (s32)&D_8009BD24, D_800A35E8);", "func_800770B8(a0, &D_8009BD24, D_800A35E8);")
    # func_80076D74: the result record is the settings block (SelWork.f00)
    t = sub1(t, " * SELWORK->f00 (f65, f66, f67, f68 packed into bitfields; per player and pick,",
             " * SELWORK->f00 (f65, f66, f67, f68 into its unk14 bit fields; per player and pick,")
    t = sub1(t, S76D74, "")
    t = sub1(t, "    S_80076D74 *hdr;\n", "    Unk8009BD24Block *hdr;\n")
    for a, b in (("hdr->f10 =", "hdr->unk14_10 ="), ("hdr->f12 =", "hdr->unk14_12 ="),
                 ("hdr->f14 =", "hdr->unk14_14 ="), ("hdr->f15 =", "hdr->unk14_15 ="),
                 ("hdr->cells[i][j][0] =", "hdr->unk00[i][j].chr ="),
                 ("hdr->cells[i][j][1] =", "hdr->unk00[i][j].unk1 =")):
        t = sub1(t, a, b)
    return t

CFG = """typedef struct {
    u8 unk0[0x14];
    u32 unk14_0 : 4;
    u32 unk14_4 : 6;
    u32 unk14_10 : 22;
} Cfg720FC;
"""

def s5ED34(e):
    e = sub1(e, "static s32 D_800A3568;\n", "static Unk8009BD24Block *D_800A3568;\n")
    e = sub1(e, "s32 func_8006E534(s32 arg0, s32 arg1, u8 *arg2, u32 arg3) {",
             "s32 func_8006E534(s32 arg0, s32 arg1, Unk8009BD24Block *arg2, u32 arg3) {")
    e = sub1(e, "D_800A35BC = *(s32 *)(arg2 + 0x14) & 0xF;", "D_800A35BC = arg2->unk14_0;")
    e = sub1(e, "D_800A3568 = (s32)arg2;", "D_800A3568 = arg2;")
    e = sub1(e, "*(s32 *)(D_800A3568 + 0x14) & 0x20000", "D_800A3568->unk14_17", 8)
    e = sub1(e, "*(s32 *)((s32)D_800A3568 + 0x14) & 0x20000", "D_800A3568->unk14_17")
    e = sub1(e, "*(s32 *)(D_800A3568 + 0x20) & 1", "D_800A3568->unk20_0")
    e = sub1(e, """            *(s32 *)(D_800A3568 + 0x14) =
                (*(s32 *)(D_800A3568 + 0x14) & ~0x3F0) | ((mode & 0x3F) << 4);
""", "            D_800A3568->unk14_4 = mode;\n")
    # the two replay-record loops: member stores; the `dst` named intermediates (FAKE) go, the
    # member form keeps the target's `addu v0,v0,a0` without them
    e = sub1(e, """            s32 dst = i * 10; /* FAKE: named intermediate for player i's 10-byte replay-record
                               * offset (named-intermediate entry, no-new-park-categories.md).
                               * Mechanism, fixed at RTL expansion: the store's address expands
                               * with EXPAND_SUM, where an inlined `i * 10` comes back as
                               * (mult i 10) (expr.c:5359-5383) and PLUS_EXPR's "put a
                               * multiplication first" (expr.c:5288-5290) swaps it ahead of
                               * D_800A3568; force_operand (expr.c:3744) then emits
                               * (plus i*10 D_800A3568), `addu v0,a0,v0`. dst is a REG, so
                               * nothing is swapped and the add keeps the target's
                               * `addu v0,v0,a0`. */

            *(u8 *)(D_800A3568 + dst) = D_800A3560.rec[i].unk0;
""", "            D_800A3568->unk00[i][0].chr = D_800A3560.rec[i].unk0;\n")
    e = sub1(e, """            s32 dst = i * 10; /* FAKE: named intermediate for player i's 10-byte replay-record
                               * offset; same entry and mechanism as the first loop's
                               * dst (here the `+ 1` store). */

            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560.rec[i].unk2;
""", "            D_800A3568->unk00[i][0].unk1 = D_800A3560.rec[i].unk2;\n")
    e = sub1(e, CFG, "")
    e = sub1(e, "((Cfg720FC *)D_800A3568)->unk14_4", "D_800A3568->unk14_4", 2)
    return e

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(A.write())
    out["51268.c"] = s51268(out["51268.c"])
    out["bb2.h"] = bb2(out["bb2.h"])
    out["64FD8.c"] = s64FD8(out["64FD8.c"])
    out["game.h"] = game(out["game.h"])
    out["5ED34.c"] = s5ED34(A.show("src/main/5ED34.c"))
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    left = [l.strip() for l in out["51268.c"].split(NL) if "D_800A3524" in l and "D_800A3524->" not in l]
    for l in left:
        print("LEFT", l)
    return out

if __name__ == "__main__":
    write()
    print("wrote f20b", sorted(OPT))
