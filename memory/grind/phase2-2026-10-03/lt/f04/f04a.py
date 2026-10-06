#!/usr/bin/env python3
# F04 (+ F15), on top of F13 + F20 as committed (7797854fe): the other resource-file layouts
# after the shared head. SEL / SEL1 / SEL2 (5ED34 D_800A35A8; func_8006EA28) -> Unk8006EA28Rec,
# D_SEL (64FD8 SELWORK->f04; func_80076FF8) -> Unk80076FF8Rec, NAR (64FD8 D_800A35F8;
# func_80077D10) -> Ctx77D94 moved into game.h. Unk8006EACCRec.unk_00 carries SEL (5ED34's handlers)
# or D_SEL (64FD8's / 63D2C's): a union of the two root pointers. func_8006E950 / func_8006E8CC take
# Unk8006E950Head *.
# usage: f04a.py [opt=<name>,...]   writes tmp/p2/f04/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f20"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import f20c as C
sys.argv = _argv
OUT = "tmp/p2/f04/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
# measured holder-first defaults (each IDENTICAL); opt=plain drops them
if "plain" not in OPT:
    OPT |= {"s3p", "basep", "p20FC", "ip", "s0p", "s0u8", "c70"}
    if not OPT & {"imga", "imgb", "imge"}:
        OPT.add("imgc")
sub1 = C.sub1
fn = C.fn
SEL = "Unk8006EA28Rec"
DSEL = "Unk80076FF8Rec"

# ---------------------------------------------------------------- game.h
def cut(g, start, end_incl):
    i = g.index(start)
    j = g.index(end_incl, i) + len(end_incl)
    return g[:i] + g[j:], g[i:j]

LAYOUTS = """
/* SEL.BIN / SEL1.BIN / SEL2.BIN (resource files 3-5), the root 5ED34's func_8006E534 loads at its work
 * area + 0x58 (D_800A35A8; func_8006EACC hands it to the handlers as Unk8006EACCRec.unk_00). After
 * the head:
 * - unk_14: per entry id two TIM pixel addresses, one per column, func_80070F78 loads (LoadImage).
 * - unk_54..unk_74: the nine lists func_8006EA28 relocates (func_8006920C); s32 * as
 *   Unk8006919CRec's lists. unk_78 is a further offset no code reads.
 * - unk_7C: the bytes func_80070F78 uploads per player; unk_80: the bytes func_800720FC reads.
 * - unk_84: TIM pixel addresses func_8006ECF4 loads per character case. */
typedef struct {
    Unk8006E950Head unk_00;
    s32 unk_14[8][2];
    s32 *unk_54;
    s32 *unk_58;
    s32 *unk_5C;
    s32 *unk_60;
    s32 *unk_64;
    s32 *unk_68;
    s32 *unk_6C;
    s32 *unk_70;
    s32 *unk_74;
    s32 *unk_78;
    u8 *unk_7C;
    u8 *unk_80;
    s32 unk_84[5];
} Unk8006EA28Rec;

/* D_SEL.BIN (resource file 6), the root 64FD8's func_800770B8 loads at its work area + 0x58 and
 * keeps in SelWork.f04 (func_80077724 hands it to the handlers as Unk8006EACCRec.unk_00). After the
 * head: unk_14..unk_38, the ten lists func_80076FF8 relocates (unk_20 is indexed by the round count
 * SelWork.f65); unk_3C, the bytes func_80074B18 reads. */
typedef struct {
    Unk8006E950Head unk_00;
    s32 *unk_14;
    s32 *unk_18;
    s32 *unk_1C;
    s32 *unk_20[3];
    s32 *unk_2C;
    s32 *unk_30;
    s32 *unk_34;
    s32 *unk_38;
    u8 *unk_3C;
} Unk80076FF8Rec;

typedef struct {
    s16 on, off;
} Win77D94;

/* NAR.BIN (resource file 0x32), the root 64FD8's func_800784E4 loads at its work area + 0x58
 * (D_800A35F8). After the head: table, hdr18..hdr28 (the five lists func_80077D10 relocates),
 * win2C, in30 / out34 and the five portrait TIMs img38 func_80077D94 uploads. */
typedef struct {
    Unk8006E950Head unk_00;
    s32 table;
    s32 *hdr18;
    s32 *hdr1C;
    s32 *hdr20;
    s32 *hdr24;
    s32 *hdr28;
    Win77D94 *win2C;
    s16 *in30;
    s16 *out34;
    s32 img38[5];
} Ctx77D94;

"""

def game(g):
    g, head = cut(g, "/* The head of the resource files func_8006E950 loads.", "} Unk8006E950Head;\n\n")
    anchor = "/* Draw context filled by 5ED34 func_8006EACC and 64FD8 func_80077724 / func_8007855C: unk_00 the\n"
    g = sub1(g, anchor, head + LAYOUTS.lstrip("\n") + anchor)
    g = sub1(g, """ * resource root (func_8007855C leaves it unset), then the cursors; unk_24 (pool word +0x20) only
 * func_8006EACC / func_80077724 set. */
typedef struct {
    s32 unk_00;
""", """ * resource root (SEL for func_8006EACC's handlers, D_SEL for func_80077724's; func_8007855C leaves it
 * unset), then the cursors; unk_24 (pool word +0x20) only func_8006EACC / func_80077724 set. */
typedef struct {
    union {
        Unk8006EA28Rec *v8006EA28;
        Unk80076FF8Rec *v80076FF8;
    } unk_00;
""")
    g = sub1(g, "    Unk8009BD24Block *f00;\n    s32 *f04;\n", "    Unk8009BD24Block *f00;\n    Unk80076FF8Rec *f04;\n")
    return g

def bb2(h):
    return sub1(h, "extern void func_8006E950(s32, s32 *);", "extern void func_8006E950(s32, Unk8006E950Head *);")

# ---------------------------------------------------------------- 5ED34
def s5ED34(e):
    e = sub1(e, "static s32 D_800A35A8;\n", "static %s *D_800A35A8;\n" % SEL)
    e = sub1(e, "    D_800A35A8 = arg0 + 0x58;\n", "    D_800A35A8 = (%s *)(arg0 + 0x58);\n" % SEL)
    for k in ("5", "3", "4"):
        e = e.replace("func_8006E950(%s, (s32 *)D_800A356C);" % k, "func_8006E950(%s, (Unk8006E950Head *)D_800A356C);" % k)
    assert e.count("(Unk8006E950Head *)D_800A356C);") == 5
    e = sub1(e, "    sp10.unk_00 = D_800A35A8;\n", "    sp10.unk_00.v8006EA28 = D_800A35A8;\n")
    # func_8006E8CC / func_8006E950: the head
    e = sub1(e, "void func_8006E8CC(s32 *a0) {", "void func_8006E8CC(Unk8006E950Head *a0) {")
    e = sub1(e, "        data = a0[4];\n    } else {\n        data = a0[3];\n",
             "        data = a0->unk_10;\n    } else {\n        data = a0->unk_0C;\n")
    e = sub1(e, "void func_8006E950(s32 a0, s32 *a1) {\n    s32 *s1 = a1;\n",
             "void func_8006E950(s32 a0, Unk8006E950Head *a1) {\n    Unk8006E950Head *s1 = a1;\n")
    e = sub1(e, "    func_8006E440(s1);\n", "    func_8006E440((s32 *)s1);\n")
    e = sub1(e, "    s3 = ((s32 *)((unsigned char *)s1 + 8))[0];\n", "    s3 = s1->unk_08;\n")
    # func_8006ECF4: the SEL root's unk_54 list and unk_84 images
    e = sub1(e, "    v0 = arg0->unk_00;\n    s3 = *(s32 *)(v0 + 0x54);\n",
             "    s3 = (s32)arg0->unk_00.v8006EA28->unk_54;\n")
    for k, off in enumerate((0x84, 0x88, 0x8C, 0x90, 0x94)):
        e = sub1(e, "a2 = *(s32 *)((s32)D_800A35A8 + 0x%X);" % off, "a2 = D_800A35A8->unk_84[%d];" % k)
    # lists
    e = sub1(e, "    base = *(s32 *)(D_800A35A8 + 0x58);\n", "    base = (s32)D_800A35A8->unk_58;\n", 2)
    e = sub1(e, "ctx = *(s32 **)(D_800A35A8 + 0x5C);", "ctx = D_800A35A8->unk_5C;", 2)
    e = sub1(e, "ctx = *(s32 **)(D_800A35A8 + 0x60);", "ctx = D_800A35A8->unk_60;")
    e = sub1(e, "sheets = *(s32 **)(D_800A35A8 + 0x74);", "sheets = D_800A35A8->unk_74;", 4)
    e = sub1(e, "sheets = *(s32 **)(D_800A35A8 + 0x60);", "sheets = D_800A35A8->unk_60;")
    e = sub1(e, "ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);", "ctx_or_var_s2 = (s32)D_800A35A8->unk_64;")
    e = sub1(e, "vram = *(u8 **)(D_800A35A8 + 0x7C);", "vram = D_800A35A8->unk_7C;", 2)
    e = sub1(e, "menu = *(u8 **)(D_800A35A8 + 0x80);", "menu = D_800A35A8->unk_80;")
    e = sub1(e, "tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);", "tim = &D_800A35A8->unk_14[id][sel];", 3)
    e = sub1(e, "    s32 *v0 = (s32 *)D_800A35A8;\n    func_800720FC(a0, v0[0x1A], 0);\n",
             "    func_800720FC(a0, (s32)D_800A35A8->unk_68, 0);\n")
    e = sub1(e, "    s32 *v0 = (s32 *)D_800A35A8;\n    func_800720FC(a0, v0[0x1B], 1);\n",
             "    func_800720FC(a0, (s32)D_800A35A8->unk_6C, 1);\n")
    e = sub1(e, "    s32 *v0 = (s32 *)D_800A35A8;\n    func_800720FC(a0, v0[0x1C], 2);\n",
             "    func_800720FC(a0, (s32)D_800A35A8->unk_70, 2);\n")
    return e

def holders5(e):
    if "s3p" in OPT:   # func_8006ECF4's s3 holds the unk_54 list
        def g(b):
            b = sub1(b, "    s32 s3;\n", "    s32 *s3;\n")
            b = sub1(b, "    s3 = (s32)arg0->unk_00.v8006EA28->unk_54;\n    s0 = s3 + 0xC;\n",
                     "    s3 = arg0->unk_00.v8006EA28->unk_54;\n    s0 = (s32)(s3 + 3);\n")
            b = sub1(b, "s.p1 = (s32 *)*(s32 *)(s3 + 4);", "s.p1 = (s32 *)s3[1];")
            b = sub1(b, "s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);", "s.p1 = (s32 *)s3[i];")
            return b
        e = fn(e, "func_8006ECF4", g)
    if "s0u8" in OPT:   # func_8006ECF4's s0: the byte address of the list's 12-byte records
        def g(b):
            b = sub1(b, "    s32 s0;\n", "    u8 *s0;\n")
            b = sub1(b, "    s0 = (s32)(s3 + 3);\n", "    s0 = (u8 *)(s3 + 3);\n")
            b = re.sub(r"s\.p0 = \(void \*\)\(s0 \+ ([^;]+)\);", r"s.p0 = s0 + \1;", b)
            return b
        e = fn(e, "func_8006ECF4", g)
    if "c70" in OPT:   # func_80070C70's context holder takes the unk_64 list
        def g(b):
            b = sub1(b, "    s32 ctx_or_var_s2;\n", "    s32 *ctx;\n")
            b = sub1(b, "    ctx_or_var_s2 = (s32)D_800A35A8->unk_64;\n", "    ctx = D_800A35A8->unk_64;\n")
            b = sub1(b, "*(s32 *)(ctx_or_var_s2 + 4)", "ctx[1]")
            b = sub1(b, "*(s32 *)(ctx_or_var_s2 + 8)", "ctx[2]")
            b = sub1(b, "*(s32 *)(ctx_or_var_s2)", "ctx[0]", 2)
            assert "ctx_or_var_s2" not in b
            return b
        e = fn(e, "func_80070C70", g)
    if "basep" in OPT:   # base holds the unk_58 list
        for f in ("func_8006F100", "func_80071C4C"):
            def g(b):
                b = sub1(b, "    s32 base;\n", "    s32 *base;\n")
                b = sub1(b, "    base = (s32)D_800A35A8->unk_58;\n", "    base = D_800A35A8->unk_58;\n")
                b = sub1(b, "obj = *(Obj_8006F100 **)(base + D_800A3560.rec[i].unk2 * 4);",
                         "obj = (Obj_8006F100 *)base[D_800A3560.rec[i].unk2];")
                return b
            e = fn(e, f, g)
    if "p20FC" in OPT:   # func_800720FC takes the list as s32 *
        e = sub1(e, "void func_800720FC(Unk8006EACCRec *, s32, s32);", "void func_800720FC(Unk8006EACCRec *, s32 *, s32);")
        e = sub1(e, "void func_800720FC(Unk8006EACCRec *arg0, s32 arg1, s32 mode) {", "void func_800720FC(Unk8006EACCRec *arg0, s32 *arg1, s32 mode) {")
        for k in ("68", "6C", "70"):
            e = sub1(e, "(s32)D_800A35A8->unk_%s" % k, "D_800A35A8->unk_%s" % k)
        e = sub1(e, "((s32 *)arg1 + (i + i))[j + 3]", "(arg1 + (i + i))[j + 3]")
    return e

def holders64(t):
    if "ip" in OPT:   # func_80074D2C's inner_ptr holds the unk_1C list
        def g(b):
            b = sub1(b, "    s32 inner_ptr;\n", "    s32 *inner_ptr;\n")
            b = sub1(b, "    inner_ptr = (s32)%s->unk_1C;\n" % D, "    inner_ptr = %s->unk_1C;\n" % D)
            b = sub1(b, "sp18_val = *(s32 *)(((arg2 << 16) >> 14) + inner_ptr);", "sp18_val = inner_ptr[(arg2 << 16) >> 16];")
            return b
        t = fn(t, "func_80074D2C", g)
    if "imgc" in OPT or "imge" in OPT:
        if "imgc" in OPT:   # a pointer to the slot, set where j was set
            t = sub1(t, """                /* FAKE: named intermediate for the TIM-pointer slot offset
                   (0x38 + 4i). Mechanism: loop.c strength-reduces it to
                   the target's giv ($s1 = 0x38, += 4). Written inline
                   (`D_800A35F8->img38[i]` or
                   `D_800A35F8 + i * 4 + 0x38`), fold moves 0x38 into the
                   load displacement and the giv is not reduced. */
                j = i * 4 + 0x38;
""", """                /* FAKE: named intermediate for the TIM-pointer slot's address.
                   Mechanism: loop.c strength-reduces it to the target's giv
                   ($s1 = 0x38, += 4, added to D_800A35F8). Written inline
                   (`D_800A35F8->img38[i]`), fold moves 0x38 into the load
                   displacement (`sll; addu; lw a1,56(v0)`) and the giv is not
                   reduced (score 9). */
                img = &D_800A35F8->img38[i];
""")
            t = sub1(t, "LoadImage((s32)&rect, *(s32 *)((u8 *)D_800A35F8 + j) + 0x220);",
                     "LoadImage((s32)&rect, *img + 0x220);")
        else:   # the array base hoisted, indexed per slot
            t = sub1(t, "            rect = D_800A32FC;\n            for (i = 0; i < 5; i++) {\n",
                     "            rect = D_800A32FC;\n            img = D_800A35F8->img38;\n            for (i = 0; i < 5; i++) {\n")
            t = sub1(t, "                j = i * 4 + 0x38;\n", "")
            t = sub1(t, "LoadImage((s32)&rect, *(s32 *)((u8 *)D_800A35F8 + j) + 0x220);",
                     "LoadImage((s32)&rect, img[i] + 0x220);")
        t = sub1(t, "    Win77D94 *w;\n", "    Win77D94 *w;\n    s32 *img;\n")
        t = fn(t, "func_80077D94", lambda b: sub1(b, "    s32 j;\n", ""))
    if "imga" in OPT or "imgb" in OPT:
        i = t.index("                /* FAKE: named intermediate for the TIM-pointer slot offset")
        j = t.index("                j = i * 4 + 0x38;\n") + len("                j = i * 4 + 0x38;\n")
        t = t[:i] + t[j:]
        if "imga" in OPT:
            t = sub1(t, "LoadImage((s32)&rect, *(s32 *)((u8 *)D_800A35F8 + j) + 0x220);",
                     "LoadImage((s32)&rect, D_800A35F8->img38[i] + 0x220);")
        else:
            t = sub1(t, "            rect = D_800A32FC;\n            for (i = 0; i < 5; i++) {\n",
                     "            rect = D_800A32FC;\n            img = D_800A35F8->img38;\n            for (i = 0; i < 5; i++) {\n")
            t = sub1(t, "LoadImage((s32)&rect, *(s32 *)((u8 *)D_800A35F8 + j) + 0x220);",
                     "LoadImage((s32)&rect, *img++ + 0x220);")
            t = sub1(t, "    Win77D94 *w;\n", "    Win77D94 *w;\n    s32 *img;\n")
    if not OPT & {"win", "win2"}:
        t = sub1(t, "        w = (Win77D94 *)(i * 4 + (s32)D_800A35F8->win2C);\n",
                 "        /* FAKE: the window address is an integer sum converted to Win77D94 *: as\n"
                 "           `&D_800A35F8->win2C[i]` win2C comes first in the addu (`addu a0,v0,s0` for\n"
                 "           the target's `addu a0,s0,v0`): score 1; `w = win2C; w += i`, 2. */\n"
                 "        w = (Win77D94 *)(i * 4 + (s32)D_800A35F8->win2C);\n")
    if "win2" in OPT:
        t = sub1(t, "w = (Win77D94 *)(i * 4 + (s32)D_800A35F8->win2C);", "w = D_800A35F8->win2C;\n        w += i;")
    if "win" in OPT:
        t = sub1(t, "w = (Win77D94 *)(i * 4 + (s32)D_800A35F8->win2C);", "w = &D_800A35F8->win2C[i];")
    if "s0p" in OPT:   # func_800784E4's s0 holds the NAR root
        def g(b):
            b = sub1(b, "    s32 s0;\n", "    Ctx77D94 *s0;\n")
            b = sub1(b, "    s0 = arg0 + 0x58;\n", "    s0 = (Ctx77D94 *)(arg0 + 0x58);\n")
            b = sub1(b, "    D_800A35F8 = (Ctx77D94 *)s0;\n", "    D_800A35F8 = s0;\n")
            b = sub1(b, "    func_8006E950(0x32, (Unk8006E950Head *)s0);\n", "    func_8006E950(0x32, &s0->unk_00);\n")
            b = sub1(b, "    r = func_80077D10(s0);\n", "    r = func_80077D10((s32 *)s0);\n")
            return b
        t = fn(t, "func_800784E4", g)
    return t

def tidy_v0(e):
    # func_8006ECF4's v0 holder has no other use once the root read is a member
    def g(b):
        if re.search(r"\bv0\b", b.replace("    s32 v0;\n", "", 1)) is None:
            b = sub1(b, "    s32 v0;\n", "")
        return b
    return fn(e, "func_8006ECF4", g)

# ---------------------------------------------------------------- 64FD8 / 63D2C
D = "arg0->unk_00.v80076FF8"

def s64FD8(t):
    t = sub1(t, "        t = (u8 *)SELWORK->f04[0x3C / 4];\n", "        t = SELWORK->f04->unk_3C;\n")
    t = sub1(t, "    inner_ptr = *(s32 *)(arg0->unk_00 + 0x1C);\n", "    inner_ptr = (s32)%s->unk_1C;\n" % D)
    t = sub1(t, "records = *(s32 **)(arg0->unk_00 + 0x18);", "records = %s->unk_18;" % D)
    t = sub1(t, "tbl = *(s32 **)(arg0->unk_00 + 0x2C);", "tbl = %s->unk_2C;" % D, 2)
    t = sub1(t, "tbl = *(s32 **)(arg0->unk_00 + 0x14);", "tbl = %s->unk_14;" % D)
    t = sub1(t, "temp_v0 = *(s32 *)(*(s32 *)(arg0->unk_00 + 0x14) + 0x54);", "temp_v0 = %s->unk_14[21];" % D)
    t = sub1(t, "table = *(s32 **)(arg0->unk_00 + 0x14);", "table = %s->unk_14;" % D, 4)
    t = sub1(t, "table = *(s32 **)(arg0->unk_00 + SELWORK->f65 * 4 + 0x20);", "table = %s->unk_20[SELWORK->f65];" % D, 2)
    t = sub1(t, "table = *(s32 **)(arg0->unk_00 + 0x30);", "table = %s->unk_30;" % D, 2)
    t = sub1(t, "    s.unk_00 = (s32)SELWORK->f04;\n", "    s.unk_00.v80076FF8 = SELWORK->f04;\n")
    t = sub1(t, "        s32 *list = work;\n", "        %s *list = work;\n" % DSEL)
    # NAR
    t = sub1(t, """typedef struct {
    s16 on, off;
} Win77D94;

typedef struct {
    u8 pad00[0x14];
    s32 table;
    s32 *hdr18;
    s32 *hdr1C;
    s32 *hdr20;
    s32 *hdr24;
    s32 *hdr28;
    Win77D94 *win2C;
    s16 *in30;
    s16 *out34;
    s32 img38[5];
} Ctx77D94;

""", "")
    t = sub1(t, "static s32 D_800A35F8;\n", "static Ctx77D94 *D_800A35F8;\n")
    t = t.replace("((Ctx77D94 *)D_800A35F8)->", "D_800A35F8->")
    t = sub1(t, "    D_800A35F8 = s0;\n", "    D_800A35F8 = (Ctx77D94 *)s0;\n")
    t = sub1(t, "    func_8006E950(0x32, s0);\n", "    func_8006E950(0x32, (Unk8006E950Head *)s0);\n")
    t = sub1(t, "LoadImage((s32)&rect, *(s32 *)(D_800A35F8 + j) + 0x220);",
             "LoadImage((s32)&rect, *(s32 *)((u8 *)D_800A35F8 + j) + 0x220);")
    t = sub1(t, "        s32 *p_struct = *(s32 **)((s32)D_800A35F8 + 0x34);\n        s32 new_val = D_800A35F0 + 1;\n        int cond = new_val < *(s16 *)((s32)p_struct + 0xA);\n",
             "        s16 *p_struct = D_800A35F8->out34;\n        s32 new_val = D_800A35F0 + 1;\n        int cond = new_val < p_struct[5];\n")
    return t

def s63D2C(m):
    m = sub1(m, "temp_s2 = *(s32 **)(arg0->unk_00 + 0x38);", "temp_s2 = %s->unk_38;" % D)
    m = sub1(m, "table = *(s32 **)(arg0->unk_00 + 0x34);", "table = %s->unk_34;" % D)
    return m

def s51268(s):
    return sub1(s, "    func_8006E950(2, (s32 *)D_800A3500);\n", "    func_8006E950(2, (Unk8006E950Head *)D_800A3500);\n")

BASE_REV = "7797854fe"   # F13 + F20 as committed (equal to f20c.py's output)

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def base():
    out = {}
    for f in ("51268", "64FD8", "3AB48", "5ED34", "25788", "2B344"):
        out[f + ".c"] = show("src/main/%s.c" % f)
    out["game.h"] = show("include/game.h")
    out["bb2.h"] = show("include/bb2.h")
    out["undefined_syms_auto.txt"] = show("undefined_syms_auto.txt")
    return out

def write():
    extra = sorted(OPT - {"s3p", "basep", "p20FC", "ip", "s0p", "s0u8", "c70", "imgc"})
    d = OUT + ("opt_%s/" % "_".join(extra) if extra else "")
    os.makedirs(d, exist_ok=True)
    out = base()
    out["game.h"] = game(out["game.h"])
    out["bb2.h"] = bb2(out["bb2.h"])
    out["5ED34.c"] = holders5(tidy_v0(s5ED34(out["5ED34.c"])))
    out["64FD8.c"] = holders64(s64FD8(out["64FD8.c"]))
    out["63D2C.c"] = s63D2C(show("src/main/63D2C.c"))
    out["51268.c"] = s51268(out["51268.c"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f04a", sorted(OPT))
