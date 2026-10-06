#!/usr/bin/env python3
# Descriptor unification, batch (i) (on the 3AB48 batch, committed as 4290c05ec): one sprite-sheet header
# (Unk8009B0E0Record), one cell (Unk8009B400Record) and one 0x2C draw descriptor (Unk8007352CEnv) in
# game.h; the walkers func_8007352C / func_80073728 / func_80073C78 (63D2C) and the local descriptor
# types of 63D2C / 64FD8 / 5ED34 use them. 51268 / 3AB48's local types are batch (ii).
# usage: fdesc1.py [opt=<name>,...]   writes tmp/p2/fdesc1/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f2b"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import f3ac as P
sys.argv = _argv
OUT = "tmp/p2/fdesc1/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = P.sub1
fn = P.fn

# ---------------------------------------------------------------- game.h
CELL_OLD = """typedef struct {
    s16 unk0;
    s16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
} Unk8009B400Record;"""
CELL_NEW = """typedef struct {
    s16 x;
    s16 y;
    u8 u;
    u8 v;
    u8 w;
    u8 h;
} Unk8009B400Record;"""

HDR_OLD = """typedef struct {
    u16 unk0;
    u8 count;
    u8 unk3;
    s32 unk4;
    s32 unk8;
} Unk8009B0E0Record;"""

def hdr_new():
    if "u8base" in OPT:
        base = "    u8 ubase;\n    u8 unk9;\n    u8 vbase;\n    u8 unkB;\n"
    else:
        base = "    u16 ubase;\n    u16 vbase;\n"
    return ("typedef struct {\n    u8 tp0;\n    u8 tp1;\n    u8 count;\n    u8 unk3;\n    u16 cx;\n    u16 cy;\n" + base +
            "    Unk8009B400Record cells[0];\n} Unk8009B0E0Record;")

ENV = """/* The 0x2C-byte draw descriptor the sprite walkers consume: func_8007352C (one SPRT per cell),
 * func_80073728 / func_80073C78 (one POLY_FT4 per cell, scaled / rotated). header / table: the sprite
 * sheet (Unk8009B0E0Record) and its cells; sprt_out / ft4_out: the SPRT and POLY_FT4 cursors the
 * walkers advance and return; semi: their SetSemiTrans argument; ot_idx: the ordering-table slot
 * (g_gpu_ot_ptr + ot_idx * 4); x / y: the screen offset added to every cell; scale_x / scale_y:
 * the 8.8 cell scales of the POLY_FT4 walkers; has_color / col_r / col_g / col_b: the SetShadeTex
 * switch and the primitive colour. The cursors are s32 because every source of them is an s32
 * word: the draw contexts' primitive cursors (Unk800788B0Rec, 51268's context words). */
typedef struct {
    Unk8009B0E0Record *header;
    Unk8009B400Record *table;
    s32 sprt_out;
    s32 ft4_out;
    s32 semi;
    u32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Unk8007352CEnv;

"""

def cut(g, start, end):
    """remove and return the block from `start` through `end` and the blank line after it."""
    i = g.index(start)
    j = g.index(end, i) + len(end)
    assert g[j] == NL
    return g[:i] + g[j + 1:], g[i:j + 1]

D_SEL_LISTS = ["unk_14", "unk_18", "unk_1C", "unk_20[3]", "unk_2C", "unk_30", "unk_34", "unk_38"]

STAFF = """/* STAFF.BIN (resource file 0x5F), the root 64FD8's func_80078824 loads at its work area + 0x58
 * (D_800A3610). After the head, one offset list (relocated with the head by func_8006E440, so it
 * ends in -1 like the head's own list): the sprite sheets func_80078654 draws: unk_14[10] first,
 * then unk_14[0] onward while the next entry is not -1. */
typedef struct {
    Unk8006E950Head unk_00;
    Unk8009B0E0Record *unk_14[0];
} Unk80078824Rec;

"""

def game(g):
    g = sub1(g, CELL_OLD, CELL_NEW)
    g = sub1(g, HDR_OLD, hdr_new())
    g, cell = cut(g, "/* 8-byte sprite records {s16, s16, u8 x4}.", "} Unk8009B400Record;" + NL)
    g, hdr = cut(g, "/* 0x8009B0E0: table of 9 twelve-byte sprite-sheet headers", "} Unk8009B0E0Record;" + NL)
    anchor = "/* The head of the resource files func_8006E950 loads."
    g = sub1(g, anchor, cell + hdr + anchor)
    g = sub1(g, "} Unk8006E950Head;\n\n", "} Unk8006E950Head;\n\n" + STAFF)
    i = g.index("} Unk8006EA28Rec;")
    j = g.index("} Unk80076FF8Rec;")
    blk = g[i:j]
    for m in D_SEL_LISTS:
        blk = sub1(blk, "    s32 *%s;\n" % m, "    Unk8009B0E0Record **%s;\n" % m)
    g = g[:i] + blk + g[j:]
    g = sub1(g, "    s32 table;\n    s32 *hdr18;\n    s32 *hdr1C;\n    s32 *hdr20;\n    s32 *hdr24;\n    s32 *hdr28;\n",
             "    Unk8009B400Record *table;\n    Unk8009B0E0Record **hdr18;\n    Unk8009B0E0Record **hdr1C;\n"
             "    Unk8009B0E0Record **hdr20;\n    Unk8009B0E0Record **hdr24;\n    Unk8009B0E0Record **hdr28;\n")
    anchor = "/* 0x8009B2BC: three {w, h} menu-frame sizes"
    g = sub1(g, anchor, ENV + anchor)
    return g

# ---------------------------------------------------------------- 63D2C walkers
def walkers(m):
    i = m.index("typedef struct EnvA {")
    j = m.index("} EnvA;\n") + len("} EnvA;\n")
    m = m[:i] + m[j:]
    i = m.index("typedef struct SprtHdrA {")
    j = m.index("} SprtEntA;\n") + len("} SprtEntA;\n")
    m = m[:i] + m[j:]
    i = m.index("/* Sprite-sheet header and 8-byte cell record read by func_80073728")
    j = m.index("} EnvF;\n") + len("} EnvF;\n")
    m = m[:i] + m[j:]
    def w52c(b):
        b = sub1(b, "    EnvA *env = (EnvA *)env_addr;\n    SprtHdrA *hdr = (SprtHdrA *)env->header;\n    SPRT *sp = (SPRT *)env->out;\n    SprtEntA *e;\n",
                 "    Unk8007352CEnv *env = (Unk8007352CEnv *)env_addr;\n    Unk8009B0E0Record *hdr = env->header;\n    SPRT *sp = (SPRT *)env->sprt_out;\n    Unk8009B400Record *e;\n")
        b = sub1(b, "        e = (SprtEntA *)env->table + i;\n", "        e = env->table + i;\n")
        return b
    m = fn(m, "func_8007352C", w52c)
    def w728(b):
        b = sub1(b, "    EnvF *env = (EnvF *)env_addr;\n    Ft4Cell *e = env->table;\n    Ft4Sheet *hdr = env->header;\n",
                 "    Unk8007352CEnv *env = (Unk8007352CEnv *)env_addr;\n    Unk8009B400Record *e = env->table;\n    Unk8009B0E0Record *hdr = env->header;\n")
        b = sub1(b, "    p = env->out;\n", "    p = (POLY_FT4 *)env->ft4_out;\n")
        b = sub1(b, "    env->out = p;\n", "    env->ft4_out = (s32)p;\n")
        return b
    m = fn(m, "func_80073728", w728)
    def wc78(b):
        b = sub1(b, "    EnvF *env;\n", "    Unk8007352CEnv *env;\n")
        b = sub1(b, "    Ft4Sheet *hdr;\n", "    Unk8009B0E0Record *hdr;\n")
        b = sub1(b, "    Ft4Cell *e;\n", "    Unk8009B400Record *e;\n")
        b = sub1(b, "    p = env->out;\n", "    p = (POLY_FT4 *)env->ft4_out;\n")
        b = sub1(b, "    env->out = p;\n", "    env->ft4_out = (s32)p;\n")
        return b
    i = m.index("s32 func_80073C78(env, angle, mode)" + NL)
    j = m.index(NL + "}" + NL, i) + 3
    m = m[:i] + wc78(m[i:j]) + m[j:]
    return m

ENVM = ["header", "table", "sprt_out", "ft4_out", "semi", "ot_idx", "x", "y", "scale_x", "scale_y"]
SPMAP = dict(zip(["sp18", "sp1C", "sp20", "sp24", "sp28", "sp2C", "sp30", "sp34", "sp38", "sp3C"], ENVM))
SPMAP.update({"sp40": "has_color", "sp41": "col_r", "sp42": "col_g", "sp43": "col_b"})

def retype(b, var, oldtype, mp, newtype="Unk8007352CEnv"):
    """b: a body; var: the local; oldtype: its declared type; mp: old member -> new member."""
    b = re.sub(r"\b%s(\s+\**%s\b)" % (oldtype, var), newtype + r"\1", b)
    def r(m):
        return m.group(1) + mp.get(m.group(2), m.group(2))
    b = re.sub(r"(\b%s(?:\.|->))(\w+)" % var, r, b)
    return b

def s63D2C_users(m):
    for f in ("func_80074220", "func_80074488"):
        def g(b):
            b = retype(b, "s", "S_80074488", SPMAP)
            b = b.replace("(s32)&s.header", "(s32)&s")
            return b
        m = fn(m, f, g)
    return m

ENVA_MAP = {"out": "sprt_out", "pad0C": "ft4_out", "pad20": "scale_x", "pad24": "scale_y"}
S78654_MAP = dict(zip("a b c d e f g h i j".split(), ENVM))
S78654_MAP.update({"cd_flag": "has_color", "r": "col_r", "g_": "col_g", "b_": "col_b"})

def count_views(b):
    # `*(u8 *)(H + 2)`, the sheet header's cell count, read through the descriptor's header word
    return re.sub(r"\*\(u8 \*\)\((s\.header) \+ 2\)", r"((Unk8009B0E0Record *)\1)->count", b)

def s64FD8(t):
    i = t.index("typedef struct EnvA {")
    j = t.index("} EnvA;\n") + len("} EnvA;\n")
    t = t[:i] + t[j:]
    for name in ("S_80074D2C", "S_753D8", "Env77D94", "S78654"):
        i = t.index("typedef struct {", t.rindex("\n\n", 0, t.index("} %s;" % name)))
        j = t.index("} %s;\n" % name) + len("} %s;\n" % name)
        t = t[:i] + t[j:]
    for f, ty, mp in (("func_80074D2C", "S_80074D2C", SPMAP), ("func_80074E08", "EnvA", ENVA_MAP),
                      ("func_800753D8", "S_753D8", SPMAP), ("func_80075830", "S_80074488", SPMAP),
                      ("func_800759D0", "S_80074488", SPMAP), ("func_8007636C", "S_80074488", SPMAP),
                      ("func_80077D94", "Env77D94", ENVA_MAP), ("func_80078654", "S78654", S78654_MAP)):
        def g(b, ty=ty, mp=mp):
            b = retype(b, "s", ty, mp)
            b = b.replace("(s32)&s.header", "(s32)&s")
            return count_views(b)
        t = fn(t, f, g)
    return t

H = "Unk8009B0E0Record"
C = "Unk8009B400Record"

def sheetify(b, holders=(), lists=(), cellvars=(), d="s.", tb="table"):
    """Typed sheet pointers in a converted body. holders: s32 locals holding one sheet address;
    lists: s32 * locals holding a list of sheet addresses; cellvars: s32 locals holding a cell address;
    d: the descriptor access prefix (`s.`, `s->`, `prim.`); tb: its cell-table member."""
    for v in holders:
        b = re.sub(r"\bs32 %s;" % v, "%s *%s;" % (H, v), b)
        b = re.sub(r"\b%s \+ 0xC\b" % v, "%s->cells" % v, b)
    for v in lists:
        b = re.sub(r"\bs32 \*%s;" % v, "%s **%s;" % (H, v), b)
    for v in cellvars:
        b = re.sub(r"\bs32 %s;" % v, "%s *%s;" % (C, v), b)
    hd, t = d + "header", d + tb
    b = b.replace(hd + " + 0xC", hd + "->cells")
    b = b.replace("((%s *)%s)->count" % (H, hd), hd + "->count")
    b = b.replace("*(u8 *)(%s + 2)" % hd, hd + "->count")
    T, HD = re.escape(t), re.escape(hd)
    b = re.sub(r"(%s \+= %s->count) \* 8;" % (T, HD), r"\1;", b)
    b = re.sub(r"(%s \+= %s->count) << 3;" % (T, HD), r"\1;", b)
    b = re.sub(r"(%s \+= %s->count) \* 16;" % (T, HD), r"\1 * 2;", b)
    b = b.replace("func_8006E480(%s," % hd, "func_8006E480((s32)%s," % hd)
    # multi-header sheets: the cells follow the last header (two: +0x18, three: +0x24)
    b = b.replace(hd + " + 0x18", hd + "[1].cells")
    b = b.replace(hd + " + 0x24", hd + "[2].cells")
    # header 1 + n: the n-th player's highlight header
    b = re.sub(r"%s = %s \+ 12 \+ (\w+) \* 12;" % (HD, HD), hd + r" += 1 + \1;", b)
    b = b.replace(hd + " += 0xC;", hd + "++;")
    return b

SHEETS63 = {"func_80074220": dict(holders=["v"], lists=["temp_s2"]),
            "func_80074488": dict(holders=["value"], lists=["table"])}
SHEETS64 = {"func_80074D2C": dict(holders=["sheet"], lists=["inner_ptr"]),
            "func_80074E08": dict(lists=["records"], cellvars=["cells"]),
            "func_800753D8": dict(lists=["tbl"], cellvars=["body"]),
            "func_80075830": dict(holders=["temp_v0"]),
            "func_800759D0": dict(lists=["table"], cellvars=["cells"]),
            "func_8007636C": dict(lists=["table"], cellvars=["cells"]),
            "func_80077D94": dict(lists=["hp"], cellvars=["table"]),
            "func_80078654": dict(lists=["var_s0"])}

def sheets(t, spec):
    if "func_80074D2C" in spec:
        t = fn(t, "func_80074D2C", lambda b: re.sub(r"\bsp18_val\b", "sheet", b))
    for f, kw in spec.items():
        t = fn(t, f, lambda b, kw=kw: sheetify(b, **kw))
    return t

# ---------------------------------------------------------------- 5ED34
S46C_MAP = dict(zip("p0 p1 pad08 ret zero10 one14 zero18 zero1C c20 c24 byte28 byte29 byte2A byte2B".split(),
                    ENVM + ["has_color", "col_r", "col_g", "col_b"]))
SPR_MAP = {"hdr": "header", "ent": "table", "unk08": "sprt_out", "ret": "ft4_out", "unk10": "semi",
           "unk14": "ot_idx", "unk28": "has_color"}
DESC_MAP = {"out": "sprt_out", "unk0C": "ft4_out"}
D720_MAP = {"cells": "table"}

def drop(t, start, end):
    i = t.index(start)
    j = t.index(end, i) + len(end)
    return t[:i] + t[j:]

def subs(b, pairs):
    for a, c in pairs:
        b = sub1(b, a, c)
    return b

def s5ED34(t):
    t = drop(t, "typedef struct {\n    u8 unk0[2];\n    u8 count;", "} Spr_8006F100;\n")
    t = drop(t, "/* func_8007352C's draw descriptor (same 0x2C-byte layout as EnvA/EnvB):", "} DescF97C;\n")
    t = re.sub(r"\bDescF97C\b", "Unk8007352CEnv", t)
    t = drop(t, "typedef struct {\n    s32 header;     /* sprite sheet header */", "} Sheets720FC;\n")
    t = drop(t, "typedef struct {\n    s32 sp18, sp1C,", "} S73200;\n")
    t = sub1(t, "void func_800720FC(Unk8006EACCRec *, s32 *, s32);", "void func_800720FC(Unk8006EACCRec *, Unk8009B0E0Record **, s32);")

    def ecf4(b):
        b = retype(b, "s", "S46C", S46C_MAP)
        b = subs(b, [("    s32 *s3;\n    u8 *s0;\n", "    Unk8006ECF4Rec *s3;\n    Unk8009B0E0Record *s0;\n"),
                     ("    s0 = (u8 *)(s3 + 3);\n", "    s0 = s3->unk_0C;\n"),
                     ("s0 + 0x108;", "s0 + 22;"), ("s0 + 0x114;", "s0 + 23;"), ("s0 + 0x120;", "s0 + 24;"),
                     ("s0 + 0x12C;", "s0 + 25;"), ("s0 + 0x138;", "s0 + 26;"),
                     ("s.table = (s32 *)s3[1];", "s.table = s3->unk_00[1];"),
                     ("s.table = (s32 *)s3[i];", "s.table = s3->unk_00[i];")])
        return b.replace("s0 + sel * 12;", "s0 + sel;")
    t = fn(t, "func_8006ECF4", ecf4)

    def spr(b):
        b = retype(b, "s", "Spr_8006F100", SPR_MAP)
        b = subs(b, [("    s32 *base;\n    Obj_8006F100 *obj;\n", "    Unk8009B0E0Record **base;\n    Unk8009B0E0Record *obj;\n"),
                     ("obj = (Obj_8006F100 *)base[", "obj = base["),
                     ("s.header = &obj->hdr[sel];", "s.header = &obj[sel];"),
                     ("s.table = obj->ent;", "s.table = obj[1].cells;")])
        b = b.replace("s.header->unk8 = ", "s.header->ubase = ")
        b = b.replace("obj->ent[0].", "obj[1].cells[0].")
        return b
    t = fn(t, "func_8006F100", spr)
    t = fn(t, "func_80071C4C", spr)

    def f528(b):
        b = retype(b, "s", "S46C", S46C_MAP)
        b = b.replace("        s32 base = ctx[", "        Unk8009B0E0Record *base = ctx[")
        b = b.replace("        s.header = (void *)base;\n        p1 = (s32 *)(base + 0xC);\n",
                      "        s.header = base;\n        p1 = base->cells;\n")
        b = b.replace("    s.table = (s32 *)((s32)s.table + 8);\n", "    s.table++;\n")
        return sheetify(b, lists=["ctx"], cellvars=[])
    t = fn(t, "func_8006F528", f528)
    t = fn(t, "func_8006F528", lambda b: sub1(b, "    s32 *p1;\n", "    Unk8009B400Record *p1;\n"))

    def f97c(b):
        return sheetify(retype(b, "s", "Unk8007352CEnv", DESC_MAP), lists=["ctx"], cellvars=["cells"])
    t = fn(t, "func_8006F97C", f97c)
    def f188(b):
        return sheetify(retype(b, "s", "Unk8007352CEnv", DESC_MAP), lists=["sheets"])
    t = fn(t, "func_80070188", f188)
    def fc70(b):
        b = retype(b, "prim", "Unk8007352CEnv", DESC_MAP)
        b = sub1(b, "    t = g + 0x48;\n", "    t = g[5].cells;\n")
        b = sub1(b, "prim.table += D_800A3590[var_s0] << 4;", "prim.table += D_800A3590[var_s0] * 2;")
        return sheetify(b, holders=["g"], lists=["ctx"], cellvars=["t"], d="prim.")
    t = fn(t, "func_80070C70", fc70)
    def ff78(b):
        b = retype(b, "s", "Unk8007352CEnv", DESC_MAP)
        return sheetify(b, lists=["sheets"], cellvars=["cells"], d="s->")
    t = fn(t, "func_80070F78", ff78)
    def f20fc(b):
        b = retype(b, "s", "Desc720FC", D720_MAP)
        b = sub1(b, "Unk8006EACCRec *arg0, s32 *arg1, s32 mode)", "Unk8006EACCRec *arg0, Unk8009B0E0Record **arg1, s32 mode)")
        for m, k in (("hdr0", 0), ("hdr4", 1), ("hdr8", 2), ("hdr2C", 11), ("hdr30", 12)):
            b = sub1(b, "((Sheets720FC *)arg1)->%s" % m, "arg1[%d]" % k)
        b = sub1(b, "s.table = cells + (mode * 6 + i) * 8;", "s.table = cells + (mode * 6 + i);")
        b = sub1(b, "s.table += ((D_800A359C + mode * 3) * 2 + D_800A3598) * 8;", "s.table += (D_800A359C + mode * 3) * 2 + D_800A3598;")
        return sheetify(b, lists=["sheets"], cellvars=["cells"])
    t = fn(t, "func_800720FC", f20fc)
    def f3200(b):
        b = retype(b, "s", "S73200", SPMAP)
        b = subs(b, [("*(s32 *)((s32)ctx + 0xC)", "ctx[3]"), ("*(s32 *)((s32)ctx + 0x10)", "ctx[4]"),
                     ("*(s32 *)((s32)ctx + 0x14)", "ctx[5]"), ("*(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4)", "ctx[10 + v1 % 4]"),
                     ("s.table = s1 + 8;", "s.table = s1 + 1;"), ("s.table = s1 + 0x10;", "s.table = s1 + 2;"),
                     ("s.table = s1 + 0x18;", "s.table = s1 + 3;"), ("func_8006E480(base1, 0)", "func_8006E480((s32)base1, 0)")])
        b = b.replace("(s32)&s.header)", "(s32)&s)")
        b = sheetify(b, holders=["base1", "base2", "tmp", "idx"], lists=["ctx"], cellvars=["s1"])
        if "one_sheet" in OPT:
            b = sub1(b, "    Unk8009B0E0Record *base1;\n    Unk8009B0E0Record *base2;\n", "    Unk8009B0E0Record *sheet;\n")
            b = sub1(b, "    Unk8009B0E0Record *tmp;\n", "")
            b = sub1(b, "    Unk8009B0E0Record *idx;\n", "")
            b = re.sub(r"\b(base1|base2|tmp|idx)\b", "sheet", b)
        else:
            b = re.sub(r"\bidx\b", "sheet", b)
        return b
    t = fn(t, "func_80073200", f3200)
    return t

SEL_LISTS = ["unk_58", "unk_5C", "unk_60", "unk_64", "unk_68", "unk_6C", "unk_70", "unk_74"]
ECF4 = """/* SEL.BIN's unk_54 block, func_8006ECF4's: two cell tables (unk_00[i], one per player), a 0 word,
 * then the 12-byte sheet headers the cells are drawn with (unk_0C[sel], plus five special cases). */
typedef struct {
    Unk8009B400Record *unk_00[2];
    s32 unk_08;
    Unk8009B0E0Record unk_0C[0];
} Unk8006ECF4Rec;

"""

def game_sel(g):
    i = g.index("/* SEL.BIN / SEL1.BIN / SEL2.BIN")
    g = g[:i] + ECF4 + g[i:]
    i = g.index("} Rec_8006C21C;")
    j = g.index("} Unk8006EA28Rec;")
    blk = g[i:j]
    blk = sub1(blk, "    s32 *unk_54;\n", "    Unk8006ECF4Rec *unk_54;\n")
    for m in SEL_LISTS:
        blk = sub1(blk, "    s32 *%s;\n" % m, "    Unk8009B0E0Record **%s;\n" % m)
    return g[:i] + blk + g[j:]

# ---------------------------------------------------------------- comments
CELL_DOC = """/* The 8-byte sprite cell: x / y, its offset from the descriptor's screen position; u / v, its texel
 * offset from the sheet's ubase / vbase; w / h, its size (func_8007352C: x0 = x + env x,
 * u0 = u + ubase, w / h copied). A sheet's cells follow its header(s) (Unk8009B0E0Record.cells).
 * The 0x8009B400 tables hold such cells. Object model evidence from the original binary:
"""
HDR_DOC = """/* The 12-byte sprite-sheet header the walkers read (Unk8007352CEnv.header): tp0 / tp1, the
 * texture-page bits (func_80073728: (tp0 & 0xFE1F) + (tp1 << 7), func_8006E480); count, the cell
 * count; cx / cy, the CLUT position (GetClut); ubase / vbase, the texel origin the cells' u / v add
 * to; cells, the cell table that follows. A resource sheet may carry two or three headers (the
 * plain one, then one highlight per player: func_8006F97C, func_800759D0) ahead of one shared cell
 * table, which then starts at the last header's cells (hdr[2].cells).
 * 0x8009B0E0: table of 9 twelve-byte sprite-sheet headers (0x8009B0E0..0x8009B14B),
 * the record func_8007352C reads through Unk8007352CEnv.header. Object
"""

def game_docs(g):
    g = sub1(g, "/* 8-byte sprite records {s16, s16, u8 x4}. Object model evidence from the\n * original binary:\n", CELL_DOC)
    g = sub1(g, "/* 0x8009B0E0: table of 9 twelve-byte sprite-sheet headers (0x8009B0E0..0x8009B14B),\n"
                " * the record func_8007352C reads through EnvA.header (cell count at +2). Object\n", HDR_DOC)
    g = sub1(g, " * - unk_54..unk_74: the nine lists func_8006EA28 relocates (func_8006920C); s32 * as\n"
                " *   Unk8006919CRec's lists. unk_78 is a further offset no code reads.\n",
                " * - unk_54..unk_74: the nine lists func_8006EA28 relocates (func_8006920C): unk_54, the\n"
                " *   header block func_8006ECF4 draws (Unk8006ECF4Rec); unk_58..unk_74, sprite-sheet lists.\n"
                " *   unk_78 is a further offset no code reads.\n")
    g = sub1(g, " * head: unk_14..unk_38, the ten lists func_80076FF8 relocates (unk_20 is indexed by the round count\n"
                " * SelWork.f65); unk_3C, the rectangle rows func_80074B18 draws. */\n",
                " * head: unk_14..unk_38, the ten lists func_80076FF8 relocates, each a list of sprite sheets\n"
                " * (unk_20 is indexed by the round count SelWork.f65); unk_3C, the rectangle rows func_80074B18\n"
                " * draws. */\n")
    g = sub1(g, " * (D_800A35F8). After the head: table, hdr18..hdr28 (the five lists func_80077D10 relocates),\n",
                " * (D_800A35F8). After the head: table, the cell table func_80077D94 draws the hdr18..hdr24\n"
                " * sheets with; hdr18..hdr28, the five sheet lists func_80077D10 relocates;\n")
    i = g.index("typedef struct {\n    s32 sp18;\n    s32 sp1C;")
    j = g.index("} S_80074488;\n") + len("} S_80074488;\n")
    return g[:i].rstrip(NL) + NL + g[j:]

def docs(out):
    out["game.h"] = game_docs(out["game.h"])
    t = out["64FD8.c"]
    t = sub1(t, "/* func_8007352C's draw descriptor: .header = the sprite sheet's SprtHdrA, .table = its\n"
                "   SprtEntA cell array (the s32 form of S_80074488 / DescF97C). */\n\n", "")
    t = sub1(t, "/* 0x2C-byte draw descriptor func_8007352C consumes (EnvA layout). */\n\n", "")
    out["64FD8.c"] = t
    for f in ("64FD8.c", "5ED34.c"):
        t = out[f]
        t = t.replace("SprtEntA", "Unk8009B400Record").replace("SprtHdrA", "Unk8009B0E0Record")
        out[f] = t
    t = out["5ED34.c"]
    t = sub1(t, "header (s->header->cells in the two loop-2 draws, s->header +\n                * 0x24 for the last loop).",
                "header (s->header->cells in the two loop-2 draws, s->header[2].cells,\n                * after three headers, for the last loop).")
    t = sub1(t, "* *(D_800A35A8 + 0x74) (loaded at entry", "* D_800A35A8->unk_74 (loaded at entry")
    t = sub1(t, "selected-slot arm) and *(D_800A35A8 + 0x60) (the table", "selected-slot arm) and D_800A35A8->unk_60 (the table")
    t = sub1(t, "* *(D_800A35A8 + 0x7C) + (i << 6), computed", "* D_800A35A8->unk_7C + (i << 6), computed")
    out["5ED34.c"] = t
    out["bb2.h"] = sub1(out["bb2.h"], "(func_8007352C reads it as SprtEntA {s16 x, y; u8 u, v, w, h})", "(func_8007352C reads it as one cell)")
    return out

# 3AB48's reads of the renamed cell members (its descriptor types are batch (ii))
CELL3AB48 = [("D_8009B184[0].unk0 = ", "D_8009B184[0].x = ", 2), ("D_8009B184[1].unk0 = ", "D_8009B184[1].x = ", 2),
             ("D_8009B164[0][0].unk0 = ", "D_8009B164[0][0].x = ", 1), ("D_8009B164[1][0].unk0 = ", "D_8009B164[1][0].x = ", 1),
             ("D_8009B164[0][1].unk0 = ", "D_8009B164[0][1].x = ", None), ("D_8009B164[1][1].unk0 = ", "D_8009B164[1][1].x = ", None),
             ("s.table->unk6 = ", "s.table->w = ", 2), ("s.table->unk0 = 0x1A2;", "s.table->x = 0x1A2;", 1), ("s.table->unk0 = 0x1D3;", "s.table->x = 0x1D3;", 1),
             ("s.table->unk0 = 0x209;", "s.table->x = 0x209;", 1), ("s.table->unk0 = 0x1F3;", "s.table->x = 0x1F3;", 1),
             ("p->unk0 = 0x50;", "p->x = 0x50;", 1), ("p->unk0 = 0x209;", "p->x = 0x209;", 1),
             ("s.table->unk0 = s.table->unk2 = 0;", "s.table->x = s.table->y = 0;", 1),
             ("s.p1->unk0 = 0x109;", "s.p1->x = 0x109;", 1), ("s.p1->unk0 = 0x113;", "s.p1->x = 0x113;", 1)]

def cells3AB48(t):
    for a, b, n in CELL3AB48:
        if n is None:
            t = t.replace(a, b)
        else:
            assert t.count(a) == n, (a, t.count(a))
            t = t.replace(a, b)
    return t

def staff(t):
    t = sub1(t, "static s32 * D_800A3610;", "static Unk80078824Rec *D_800A3610;")
    def g(b):
        b = sub1(b, "    s.header = D_800A3610[0xF];\n", "    s.header = D_800A3610->unk_14[10];\n")
        b = sub1(b, "    var_s0 = D_800A3610 + 5;\n", "    var_s0 = D_800A3610->unk_14;\n")
        b = sub1(b, "    if (var_s0[1] != -1) goto loop;\n", "    if (var_s0[1] != (Unk8009B0E0Record *)-1) goto loop;\n")
        return b
    t = fn(t, "func_80078654", g)
    def r824(b):
        b = sub1(b, "    s32 s0;\n", "    Unk80078824Rec *s0;\n")
        b = sub1(b, "    s0 = arg0 + 0x58;\n", "    s0 = (Unk80078824Rec *)(arg0 + 0x58);\n")
        return sub1(b, "    func_8006E950(0x5F, s0);\n", "    func_8006E950(0x5F, &s0->unk_00);\n")
    t = fn(t, "func_80078824", r824)
    t = sub1(t, "s32 func_80078628(s32 *a0) {\n    return a0[1];\n}\n", "s32 func_80078628(Unk80078824Rec *a0) {\n    return a0->unk_00.unk_04;\n}\n")
    return t

# ---------------------------------------------------------------- carried-construct sweep (checklist B4)
# Redundant temps / single-use aliases carried from HEAD in the moved bodies whose removal is
# byte-identical (each measured alone, then all together): removed.
SWEEP = [
    ("63D2C", "func_80074220", [("    Unk8009B0E0Record *v;\n", ""),
        ("        v = temp_s2[i];\n        s.header = v;\n        s.table = v->cells;\n", "        s.header = temp_s2[i];\n        s.table = s.header->cells;\n"),
        ("    s32 a3;\n", ""),
        ("    a3 = func_8006E480((s32)s.header, 0);\n    SetDrawMode(arg0->unk_04.unk_14, 1, 0, a3, 0);\n", "    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480((s32)s.header, 0), 0);\n")]),
    ("63D2C", "func_80074488", [("    Unk8009B0E0Record *value;\n", ""),
        ("            value = table[i];\n            s.header = value;\n            s.table = value->cells;\n", "            s.header = table[i];\n            s.table = s.header->cells;\n"),
        ("    SelWork *base;\n", ""), ("    base = SELWORK;\n", ""), ("base->", "SELWORK->", None)]),
    ("64FD8", "func_80074D2C", [("    Unk8009B0E0Record **inner_ptr;\n", ""), ("    inner_ptr = arg0->unk_00.v80076FF8->unk_1C;\n", ""),
        ("sheet = inner_ptr[", "sheet = arg0->unk_00.v80076FF8->unk_1C["), ("var_s1", "ot", None)]),
    ("64FD8", "func_800753D8", [("    SelWork *base;\n", ""), ("    base = SELWORK;\n", ""), ("base->", "SELWORK->", None),
        ("    c = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n    s.col_b = c;\n    s.col_g = c;\n    s.col_r = c;\n",
         "    s.col_r = s.col_g = s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n"), ("    s32 c;\n", "")]),
    ("64FD8", "func_80075830", [("    Unk8009B0E0Record *temp_v0;\n", ""),
        ("    temp_v0 = arg0->unk_00.v80076FF8->unk_14[21];\n    s.header = temp_v0;\n    s.table = temp_v0->cells;\n",
         "    s.header = arg0->unk_00.v80076FF8->unk_14[21];\n    s.table = s.header->cells;\n"),
        ("    s32 temp_v1;\n", ""),
        ("    temp_v1 = ((s32) (rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;\n    s.col_b = temp_v1;\n    s.col_g = temp_v1;\n    s.col_r = temp_v1;\n",
         "    s.col_r = s.col_g = s.col_b = ((s32) (rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;\n")]),
    ("64FD8", "func_80077D94", [("                v = 0x72;\n                s.semi = 1;\n                s.col_r = s.col_g = s.col_b = v;\n",
        "                s.semi = 1;\n                s.col_r = s.col_g = s.col_b = 0x72;\n")]),
    ("64FD8", "func_80078654", [("s.col_r = (s.col_g = (s.col_b = (u8) sv));", "s.col_r = s.col_g = s.col_b = sv;")]),
    ("64FD8", "func_80078824", [("    s32 r;\n", ""), ("    r = func_80078628(s0);\n    func_8006E49C(r, D_800A360C);\n", "    func_8006E49C(func_80078628(s0), D_800A360C);\n")]),
    ("5ED34", "func_8006ECF4", [("        s32 b = D_800A3588[i];\n        s32 c = D_800A358C[i];\n        sel = D_8009BC40[c][b].value;\n",
        "        sel = D_8009BC40[D_800A358C[i]][D_800A3588[i]].value;\n")]),
    ("5ED34", "func_80070C70", [("    Unk8009B0E0Record *g;\n", ""), ("    g = ctx[1];\n    t = g->cells;\n    prim.header = g;\n", "    prim.header = ctx[1];\n    t = prim.header->cells;\n"),
        ("    g = ctx[0];\n    t = g[5].cells;\n    prim.header = g;\n", "    prim.header = ctx[0];\n    t = prim.header[5].cells;\n"),
        ("                g = prim.header;\n                t = g->cells;\n", "                t = prim.header->cells;\n")]),
    ("5ED34", "func_80070188", [("        c = ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3530[i]) + i * 511) * 63) >> 12) - 0x40;\n        s.col_b = c;\n        s.col_g = c;\n        s.col_r = c;\n",
        "        s.col_r = s.col_g = s.col_b = ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3530[i]) + i * 511) * 63) >> 12) - 0x40;\n"), ("    s32 c;\n", "")]),
    ("5ED34", "func_80070F78", [("                c = ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3544[i]) + i * 511) * 63) >> 12) - 0x40;\n                s->col_b = c;\n                s->col_g = c;\n                s->col_r = c;\n",
        "                s->col_r = s->col_g = s->col_b = ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3544[i]) + i * 511) * 63) >> 12) - 0x40;\n"), ("    s32 c;\n", "")]),
    ("5ED34", "func_800720FC", [("    c = ((rsin((D_800A35C4->unk_08 & 0x1F) * 128 + 0x1FF) * 63) >> 12) - 0x40;\n    s.col_b = c;\n    s.col_g = c;\n    s.col_r = c;\n",
        "    s.col_r = s.col_g = s.col_b = ((rsin((D_800A35C4->unk_08 & 0x1F) * 128 + 0x1FF) * 63) >> 12) - 0x40;\n"), ("    s32 c;\n", "")]),
    ("5ED34", "func_80073200", [("    Unk8009B0E0Record *base1;\n    Unk8009B0E0Record *base2;\n", ""), ("    Unk8009B0E0Record *tmp;\n", ""),
        ("    Unk8009B0E0Record *sheet;\n", ""), ("    s32 v1;\n", ""),
        ("    base1 = ctx[3];\n    s.header = base1;\n    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480((s32)base1, 0), 0);\n",
         "    s.header = ctx[3];\n    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480((s32)s.header, 0), 0);\n"),
        ("    base2 = ctx[4];\n    s.header = base2;\n    s1 = base2->cells;\n", "    s.header = ctx[4];\n    s1 = s.header->cells;\n"),
        ("    tmp = ctx[5];\n    s.header = tmp;\n    s1 = tmp->cells;\n", "    s.header = ctx[5];\n    s1 = s.header->cells;\n"),
        ("        v1 = D_800A35C4->unk_08;\n        sheet = ctx[10 + v1 % 4];\n        s.header = sheet;\n        s1 = sheet->cells;\n",
         "        s.header = ctx[10 + D_800A35C4->unk_08 % 4];\n        s1 = s.header->cells;\n")]),
    ("3AB48", "func_8005D814", [("                    s16 tens = digit[0] / 10;\n\n                    digit[0] = tens % 10;\n", "                    digit[0] = digit[0] / 10 % 10;\n", None),
        ("    num_tens = digit[1] / 10;\n    digit[2] = digit[2] % 10;\n    digit[1] = num_tens % 10;\n", "    digit[2] = digit[2] % 10;\n    digit[1] = digit[1] / 10 % 10;\n"), ("    s16 num_tens;\n", "")]),
    ("3AB48", "func_8005E098", [("        v = s.d[0] / 10;\n        s.d[1] = s.d[1] % 10;\n        s.d[0] = v % 10;\n", "        s.d[1] = s.d[1] % 10;\n        s.d[0] = s.d[0] / 10 % 10;\n"), ("    s16 v;\n", "")]),
    ("3AB48", "func_8005F1C8", [("                        s16 tens = s.d[k] / 10;\n\n                        s.d[k] = tens % 10;\n", "                        s.d[k] = s.d[k] / 10 % 10;\n")]),
]

SWEEP += [
    ("64FD8", "func_80074D2C", [("    Unk8009B0E0Record *sheet;\n", ""),
        ("    sheet = arg0->unk_00.v80076FF8->unk_1C[(arg2 << 16) >> 16];\n    s.x = arg1 * 0xF0;\n    s.y = 0;\n    s.header = sheet;\n    s.table = sheet->cells;\n",
         "    s.header = arg0->unk_00.v80076FF8->unk_1C[(arg2 << 16) >> 16];\n    s.x = arg1 * 0xF0;\n    s.y = 0;\n    s.table = s.header->cells;\n")]),
    ("5ED34", "func_80071C4C", [("            s32 mode = func_80071C20();\n\n            D_800A35A0 = 1;\n            D_800A3568->unk14_4 = mode;\n",
        "            D_800A3568->unk14_4 = func_80071C20();\n            D_800A35A0 = 1;\n")]),
    ("63D2C", "func_80074488", [("    s32 color;\n", ""),
        ("                color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n                s.col_b = color;\n",
         "                s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n"),
        ("                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n                    s.col_b = color;\n",
         "                    s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n", None)]),
    ("64FD8", "func_800759D0", [("    s32 color;\n", ""),
        ("    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;\n    s.col_r = s.col_g = s.col_b = color;\n",
         "    s.col_r = s.col_g = s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;\n")]),
    ("64FD8", "func_8007636C", [("    s32 color;\n", ""),
        ("    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;\n    s.col_r = s.col_g = s.col_b = color;\n",
         "    s.col_r = s.col_g = s.col_b = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;\n")]),
    ("5ED34", "func_8006F100", [("    s32 t0;\n", ""),
        ("        t0 = -(D_800A35C8[i] * 800) / 20;\n        t1 = t0;\n", "        t1 = -(D_800A35C8[i] * 800) / 20;\n"),
        ("            t1 = -t0;\n", "            t1 = -t1;\n")]),
    ("5ED34", "func_80073200", [("             * at the join.  Byte-neutral - jump2's find_cross_jump re-merges\n",
        "             * at the join.  No extra instruction - jump2's find_cross_jump re-merges\n")]),
]

def f528_blocks(b):
    """func_8006F528's `{ Unk8009B0E0Record *base = ctx[N]; ... }` blocks: base read through s.header."""
    def blk(m):
        inner = m.group(2).replace("        s.header = base;\n        p1 = base->cells;\n",
                                   "        s.header = ctx[%s];\n        p1 = s.header->cells;\n" % m.group(1))
        assert "base" not in inner
        return "".join(l[4:] + "\n" for l in inner.split("\n")[:-1])
    b, n = re.subn(r"    \{\n        Unk8009B0E0Record \*base = ctx\[(\d)\];\n\n((?:        .*\n|\n)*?)    \}\n", blk, b)
    assert n == 5, n
    return b

def sweep(out):
    out["5ED34.c"] = fn(out["5ED34.c"], "func_8006F528", f528_blocks)
    for tu, f, reps in SWEEP:
        def g(b, reps=reps, f=f):
            for r in reps:
                old, new = r[0], r[1]
                if len(r) > 2 and r[2] is None:
                    assert old in b, (f, old[:60])
                    b = b.replace(old, new)
                else:
                    b = sub1(b, old, new)
            return b
        out[tu + ".c"] = fn(out[tu + ".c"], f, g)
    return out

# ---------------------------------------------------------------- FAKE labels and scores (checklist B4)
# Every codegen-shaping construct in a moved body carries /* FAKE */ with its measured ablation
# score (engine sandbox scoring of the plain form, everything else as landed).
HOLD = "FAKE: the holder between the header's cells and %s; stored directly: score %d."
LABELS = [
    ("63D2C", "func_80073C78", "    s16 du0, dv0;\n",
     "    /* FAKE: du0 / dv0 stay 0 in this walker (func_80073728 sets them per mirror mode); u / v\n"
     "       written without them: score 33. */\n    s16 du0, dv0;\n"),
    ("64FD8", "func_80074E08", "       three AddPrim calls add. */\n",
     "       three AddPrim calls add. FAKE: one local for both; a second local for the byte\n       offset: score 22. */\n"),
    ("64FD8", "func_80074E08", "       constant offset, written once per sheet (owner Ruling 9). */\n",
     "       constant offset, written once per sheet (owner Ruling 9). " + HOLD % ("s.table", 12) + " */\n"),
    ("64FD8", "func_800753D8", "    Unk8009B400Record *body;\n",
     "    /* " + HOLD % ("s.table", 15) + " */\n    Unk8009B400Record *body;\n"),
    ("64FD8", "func_800753D8", "       SOTN ships this exact shape: src/dra/7879C.c:2067 `s32 zero = 0;`. */\n",
     "       SOTN ships this exact shape: src/dra/7879C.c:2067 `s32 zero = 0;`. The literal\n       at both calls: score 9. */\n"),
    ("64FD8", "func_800759D0", "       func_800753D8 (`zero`) and func_8007636C (`mode`). */\n",
     "       func_800753D8 (`zero`) and func_8007636C (`mode`). The literal at the three\n       calls: score 30. */\n"),
    ("64FD8", "func_800759D0", "       past the sheet into bytes nothing references (owner Ruling 9). */\n",
     "       past the sheet into bytes nothing references (owner Ruling 9).\n       " + HOLD % ("s.table", 25) + " */\n"),
    ("64FD8", "func_800759D0", "         * sites folds the symbol into each load (`lui $at; addu; lbu %lo`). */\n",
     "         * sites folds the symbol into each load (`lui $at; addu; lbu %lo`): score 44. */\n"),
    ("64FD8", "func_8007636C", "       per player) on the highlightable ones (+0x24). */\n",
     "       per player) on the highlightable ones (+0x24). " + HOLD % ("s.table", 6) + " */\n"),
    ("64FD8", "func_8007636C", "     * 20/56/334/356). */\n", "     * 20/56/334/356). The literal: score 3. */\n"),
    ("64FD8", "func_80077D94", "       a goto) does not match. */\n", "       a goto) does not match: score 23. */\n"),
    ("64FD8", "func_80077D94", "               a separate pseudo and costs a `move` at the join. */\n",
     "               a separate pseudo and costs a `move` at the join (shared tail: score 54). */\n"),
    ("64FD8", "func_80077D94", "                   60 - (cnt - off) matches. */\n", "                   60 - (cnt - off) matches (inline: score 16). */\n"),
    ("64FD8", "func_80078654", "       with the literal 0 the function comes out 3 instructions short of\n       the target. */\n",
     "       with the literal 0 the function comes out 3 instructions short of\n       the target (score 7). */\n"),
    ("64FD8", "func_80078654", "       that reaches 13 at the only wrap site that costs no delay slot. */\n",
     "       that reaches 13 at the only wrap site that costs no delay slot. Unwrapped:\n       score 19. */\n"),
    ("5ED34", "func_8006F528", "    Unk8009B400Record *p1;\n", "    /* " + HOLD % ("s.table", 9) + " */\n    Unk8009B400Record *p1;\n"),
    ("5ED34", "func_8006F100", "             * vertical centre 0x9D. */\n", "             * vertical centre 0x9D (scored with `cx`). */\n"),
    ("5ED34", "func_8006F97C", "       SEL.BIN/SEL1.BIN/SEL2.BIN census: pre-slim-2026-10-01:memory/grind/func_8006F97C/evidence.md. */\n",
     "       SEL.BIN/SEL1.BIN/SEL2.BIN census: pre-slim-2026-10-01:memory/grind/func_8006F97C/evidence.md.\n       " + HOLD % ("s.table", 98) + " */\n"),
    ("5ED34", "func_8006F97C", "                 * join). Byte-neutral: the copies re-merge into the one call. */\n",
     "                 * join): score 29. */\n"),
    ("5ED34", "func_80070C70", "    Unk8009B400Record *t;\n", "    /* " + HOLD % ("prim.table", 11) + " */\n    Unk8009B400Record *t;\n"),
    ("5ED34", "func_80070F78", "                * after three headers, for the last loop). Ruling 11, proof in\n                * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md. */\n",
     "                * after three headers, for the last loop). Ruling 11, proof in\n                * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md.\n                * " + HOLD % ("s->table", 9) + " */\n"),
    ("5ED34", "func_800720FC", "                * pre-slim-2026-10-01:memory/grind/func_800720FC/r11/. */\n",
     "                * pre-slim-2026-10-01:memory/grind/func_800720FC/r11/.\n                * " + HOLD % ("s.table", 123) + " */\n"),
    ("5ED34", "func_800720FC", "                        * emits both. dead-store-fake-exception.md. */\n",
     "                        * emits both. dead-store-fake-exception.md. Both removed:\n                        * score 3. */\n"),
    ("5ED34", "func_80073200", "    Unk8009B400Record *s1;\n", "    /* " + HOLD % ("s.table", 15) + " */\n    Unk8009B400Record *s1;\n"),
    ("5ED34", "func_80073200", "             * with sp43 stored once at the join it is emitted first. */\n",
     "             * with col_b stored once at the join it is emitted first: score 2. */\n"),
    ("3AB48", "func_8005C8A8", "       memory for the in-loop `lw 0xBC($sp)` too; (s16)arg1 does not. */\n",
     "       memory for the in-loop `lw 0xBC($sp)` too; (s16)arg1 does not. `sel = arg1`:\n       score 58. */\n"),
    ("3AB48", "func_8005C8A8", "       rematerialised at the return (frame 0x10 short). */\n",
     "       rematerialised at the return (frame 0x10 short). sizeof: score 33. */\n"),
    ("3AB48", "func_8005D814", "    s32 end_off;\n",
     "    /* FAKE: the returned size is the chunk's end minus its start; sizeof(Unk8005D814Rec): score 76. */\n    s32 end_off;\n"),
    ("3AB48", "func_8005E098", "    s32 end_off;\n",
     "    /* FAKE: the returned size is the chunk's end minus its start; sizeof(Unk8005D814Rec): score 53. */\n    s32 end_off;\n"),
    ("3AB48", "func_8005E098", "    Unk8009B400Record *p;\n",
     "    /* FAKE: alias of s.p1 for the digit cell's x store; through s.p1: score 68. */\n    Unk8009B400Record *p;\n"),
    ("3AB48", "func_8005E54C", "    s32 end_off;\n",
     "    /* FAKE: the returned size is the chunk's end minus its start; sizeof(Unk8005E54CRec): score 75. */\n    s32 end_off;\n"),
    ("3AB48", "func_8005E54C", "       unused array with sibling evidence). */\n", "       unused array with sibling evidence). Removed: score 47. */\n"),
    ("3AB48", "func_8005E54C", "    /* One 32-bit store clears the whole pair", "    /* FAKE: one 32-bit store clears the whole pair"),
    ("3AB48", "func_8005E54C", "       local array). */\n", "       local array). `points[0] = points[1] = 0`: score 2. */\n"),
    ("3AB48", "func_8005E54C", "    /* Each arm sets the whole (x0, y0) position:", "    /* FAKE: each arm sets the whole (x0, y0) position:"),
    ("3AB48", "func_8005E54C", "       if/else does not match. */\n", "       if/else: score 10. */\n"),
    ("3AB48", "func_8005E54C", "                if (points[j] > *(j ? &points[0] : &points[1])) {\n",
     "                /* FAKE: the other player's points read through a selected address;\n                   points[j ^ 1] (both sites): score 110. */\n"
     "                if (points[j] > *(j ? &points[0] : &points[1])) {\n"),
    ("3AB48", "func_8005E54C", "            if (points[j] <= *(j ? &points[0] : &points[1])) {\n",
     "            /* FAKE: the other player's points, as above. */\n            if (points[j] <= *(j ? &points[0] : &points[1])) {\n"),
    ("3AB48", "func_8005F1C8", "    s32 end_off;\n",
     "    /* FAKE: the returned size is the chunk's end minus its start; sizeof(Unk8005D814Rec): score 97. */\n    s32 end_off;\n"),
    ("3AB48", "func_8005E54C", "    /* i counts the players (first loop) and then the rounds;", "    /* FAKE: i counts the players (first loop) and then the rounds;"),
    ("3AB48", "func_8005E54C", "       per phase do not match. */\n", "       per phase (a round counter of its own): score 8. */\n"),
    ("3AB48", "func_8005F1C8", "    /* i/row count the win-mark pips' players and rows;", "    /* FAKE: i/row count the win-mark pips' players and rows;"),
    ("3AB48", "func_8005F1C8", "     * single-counter shape. Separate counters per phase do not match. */\n",
     "     * single-counter shape. Separate counters for the later phases: score 11. */\n"),
    ("5ED34", "func_80070F78", "ALL:mechanism at the ==3 `tim` */", "mechanism and score at the ==3 `tim` */"),
    ("5ED34", "func_800720FC", "same dead store as above (asm line 358). */", "same dead store as above (asm line 358; scored there). */"),
]

def labels(out):
    for tu, f, old, new in LABELS:
        if f == "func_80073C78":   # K&R definition
            m = out[tu + ".c"]
            i = m.index("s32 func_80073C78(env, angle, mode)" + NL)
            j = m.index(NL + "}" + NL, i) + 3
            out[tu + ".c"] = m[:i] + sub1(m[i:j], old, new) + m[j:]
            continue
        if old.startswith("ALL:"):
            out[tu + ".c"] = fn(out[tu + ".c"], f, lambda b, old=old[4:], new=new: b.replace(old, new))
            continue
        out[tu + ".c"] = fn(out[tu + ".c"], f, lambda b, old=old, new=new: sub1(b, old, new))
    return out

BASE_REV = "662320305"   # the 3AB48 batch (4290c05ec) and worker 2's batches, as committed

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def base():
    out = {}
    for f in ("63D2C", "64FD8", "5ED34", "3AB48"):
        out[f + ".c"] = show("src/main/%s.c" % f)
    for f in ("game.h", "bb2.h"):
        out[f] = show("include/" + f)
    return out

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = base()
    out["game.h"] = game_sel(game(out["game.h"]))
    out["5ED34.c"] = s5ED34(out["5ED34.c"])
    out["63D2C.c"] = sheets(s63D2C_users(walkers(out["63D2C.c"])), SHEETS63)
    out["64FD8.c"] = staff(sheets(s64FD8(out["64FD8.c"]), SHEETS64))
    out["3AB48.c"] = cells3AB48(out["3AB48.c"])
    sweep(out)
    docs(out)
    labels(out)
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fdesc1", sorted(OPT))
