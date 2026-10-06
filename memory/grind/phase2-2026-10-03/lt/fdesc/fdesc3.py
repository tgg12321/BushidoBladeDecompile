#!/usr/bin/env python3
# Descriptor unification, batch (ii-b) (on batch (ii-a), committed as 12e8c26bd): 51268's local descriptor types (S_69AE4,
# S69E18, S_69F80, S_6A880, S_6B120, Env_8006C21C, Env_8006CFBC, EnvA, EnvB) become Unk8007352CEnv;
# the raw `u8 *` descriptor parameters of func_8006A3CC / func_8006A494 / func_8006A564 take it; the
# MOD.BIN sheet lists and func_8006D808's sheet set are typed.
# usage: fdesc3.py [opt=<name>,...]   writes tmp/p2/fdesc3/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import fdesc1 as D
sys.argv = _argv
OUT = "tmp/p2/fdesc3/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1, fn, retype = D.sub1, D.fn, D.retype
H, C, ENV = "Unk8009B0E0Record", "Unk8009B400Record", "Unk8007352CEnv"

BASE_REV = "12e8c26bd"   # batch (ii-a) as committed

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def base():
    out = {"51268.c": show("src/main/51268.c"), "3AB48.c": show("src/main/3AB48.c")}
    for f in ("game.h", "bb2.h"):
        out[f] = show("include/" + f)
    return out

def subs(b, pairs):
    for p in pairs:
        if len(p) == 3 and p[2] is None:
            assert p[0] in b, p[0][:70]
            b = b.replace(p[0], p[1])
        else:
            b = sub1(b, p[0], p[1])
    return b

# ---------------------------------------------------------------- game.h / bb2.h
SET = """/* func_8006D808's sheet set: three frame sheets (unk_00), the name sheet and the per-character name
 * cells (unk_0C / unk_10, three cells per character after a first group), the digit sheet and its
 * cells (unk_14 / unk_18; the walker shifts the sheet's ubase per digit), and per-character x
 * offsets (unk_1C). 3AB48 hands it D_8009B0C0, func_8006DD94 MOD.BIN's unk_3C. */
typedef struct {
    Unk8009B0E0Record *unk_00[3];
    Unk8009B0E0Record *unk_0C;
    Unk8009B400Record *unk_10;
    Unk8009B0E0Record *unk_14;
    Unk8009B400Record *unk_18;
    s16 *unk_1C;
} Unk8006D808Set;

/* MOD.BIN's unk_3C list: func_8006D808's set, then the three sheets func_8006DD94 draws. */
typedef struct {
    Unk8006D808Set unk_00;
    Unk8009B0E0Record *unk_20[3];
} Unk8006DD94List;

"""

def headers(g, b):
    anchor = "/* MOD.BIN (resource file 2)"
    g = sub1(g, anchor, SET + anchor)
    i = g.index(anchor)
    j = g.index("} Unk8006919CRec;", i)
    blk = g[i:j]
    for m in ("14", "18", "1C", "20", "24", "28", "2C", "30", "34", "38", "40"):
        blk = sub1(blk, "    s32 *unk_%s;\n" % m, "    Unk8009B0E0Record **unk_%s;\n" % m)
    blk = sub1(blk, "    s32 *unk_3C;\n", "    Unk8006DD94List *unk_3C;\n")
    blk = sub1(blk, """ *   word, -1 entries skipped). Most entries are sprite sheets, a 12-byte Unk8009B0E0Record header
 *   followed by its 8-byte Unk8009B400Record cells (hence the readers' `+ 0xC`). The lists stay
 *   s32 *: their entries go into the s32 header words of the draw descriptors, which keep that
 *   type until the descriptors are unified.
""", """ *   word, -1 entries skipped). Their entries are sprite sheets (unk_20 pairs each sheet with a -1),
 *   except unk_3C, func_8006DD94's list (Unk8006DD94List); func_8006919C walks the twelve slots
 *   as one run of list pointers.
""")
    g = g[:i] + blk + g[j:]
    b = sub1(b, "extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);", "extern void func_8006D808(s32 *, s32 *, Unk8006D808Set *, s32, s32);")
    return g, b

# ---------------------------------------------------------------- 51268
SPMAP = D.SPMAP
S69E18_MAP = {"p0": "header", "p1": "table", "in_tex": "sprt_out", "pad0C": "ft4_out", "zero10": "semi",
              "arg2": "ot_idx", "width": "x", "zero1C": "y", "pad20": "scale_x", "pad24": "scale_y", "byte28": "has_color"}
S6B120_MAP = {"p0": "header", "p1": "table", "chain": "sprt_out", "pad0C": "ft4_out", "flag10": "semi", "n14": "ot_idx",
              "x18": "x", "y1C": "y", "pad20": "scale_x", "pad24": "scale_y", "flag28": "has_color",
              "c29": "col_r", "c2A": "col_g", "c2B": "col_b"}
ENVOUT = {"out": "sprt_out", "pad0C": "ft4_out", "pad20": "scale_x", "pad24": "scale_y"}
RAW = [("*(s32 *)(arg1 + 0)", "arg1->header"), ("*(s32 *)(arg1 + 4)", "arg1->table"), ("*(s32 *)(arg1 + 8)", "arg1->sprt_out"),
       ("*(s32 *)(arg1 + 0xC)", "arg1->ft4_out"), ("*(s32 *)(arg1 + 0x10)", "arg1->semi"), ("*(s32 *)(arg1 + 0x14)", "arg1->ot_idx"),
       ("*(s32 *)(arg1 + 0x18)", "arg1->x"), ("*(s32 *)(arg1 + 0x1C)", "arg1->y"), ("*(s32 *)(arg1 + 0x20)", "arg1->scale_x"),
       ("*(s32 *)(arg1 + 0x24)", "arg1->scale_y"), ("*(s8 *)(arg1 + 0x28)", "arg1->has_color"), ("*(u8 *)(arg1 + 0x29)", "arg1->col_r"),
       ("*(u8 *)(arg1 + 0x2A)", "arg1->col_g"), ("*(u8 *)(arg1 + 0x2B)", "arg1->col_b")]

TAIL = """/* FAKE: frame layout (oversized live object): the descriptor plus its unwritten tail, so the
   locals region reaches the target's frame; mechanism at func_80069F80 / func_8006DD94. */
typedef struct {
    Unk8007352CEnv env;
    s32 tail[4];
} Env_69F80;
"""
TAILB = """/* FAKE: frame layout (oversized live object), as Env_69F80; mechanism at func_8006DD94. */
typedef struct {
    Unk8007352CEnv env;
    s32 tail[2];
} Env_8006DD94;
"""

def tailed(b, oldtype, newtype, mp):
    b = re.sub(r"\b%s s;" % oldtype, "%s s;" % newtype, b)
    b = re.sub(r"\bs\.(\w+)", lambda m: "s.env." + mp.get(m.group(1), m.group(1)), b)
    return b.replace("(s32)&s)", "(s32)&s.env)").replace("(s32)&s.env.header)", "(s32)&s.env)")

def drop_type(t, name, repl=""):
    i = t.rindex("typedef struct", 0, t.index("} %s;\n" % name))
    c = t.rfind("*/\n", 0, i)
    if c != -1 and t[c + 3:i].strip() == "":
        o = t.rindex("/*", 0, c)
        if t[o - 1] == NL:
            i = o
    j = t.index("} %s;\n" % name) + len("} %s;\n" % name)
    return t[:i] + repl + t[j:]

def s51268(t):
    t = drop_type(t, "S_69AE4")
    t = drop_type(t, "S69E18")
    t = drop_type(t, "S_69F80", TAIL)
    t = drop_type(t, "S_6A880")
    t = drop_type(t, "S_6B120")
    t = drop_type(t, "Env_8006C21C")
    t = drop_type(t, "Env_8006CFBC")
    t = drop_type(t, "EnvA")
    t = drop_type(t, "EnvB", TAILB)

    t = fn(t, "func_8006919C", lambda b: sub1(b, "    s32 **p = &a0->unk_14;\n", "    Unk8009B0E0Record ***p = &a0->unk_14;\n"))

    def ae4(b):
        b = retype(b, "s", "S_69AE4", SPMAP)
        return subs(b, [("    s32 *qbase;\n    s32 *q;\n", "    Unk8009B0E0Record **qbase;\n    Unk8009B0E0Record **q;\n"),
                        ("        s32 v = *q;\n        s.header = v;\n        s.table = v + 0xC;\n",
                         "        Unk8009B0E0Record *v = *q;\n        s.header = v;\n        s.table = v->cells;\n"),
                        ("(s32)&s.header)", "(s32)&s)"),
                        ("        s32 first = qbase[0];\n", "        Unk8009B0E0Record *first = qbase[0];\n"),
                        ("func_8006E480(first, 0)", "func_8006E480((s32)first, 0)")])
    t = fn(t, "func_80069AE4", ae4)

    def e18(b):
        b = retype(b, "s", "S69E18", S69E18_MAP)
        return subs(b, [("    s32 *ptr;\n", "    Unk8009B0E0Record **ptr;\n"), ("    s32 p0;\n    s32 p1;\n", "    Unk8009B0E0Record *p0;\n    Unk8009B400Record *p1;\n"),
                        ("    s.header = (s32 *)ptr[0];\n", "    s.header = ptr[0];\n"),
                        ("    p0 = (s32)s.header;\n    p1 = p0 + 0xC;\n    s.table = (s32 *)p1;\n", "    p0 = s.header;\n    p1 = p0->cells;\n    s.table = p1;\n"),
                        ("    p1 = p0 + 0xC;\n    s.header = (s32 *)p0;\n    s.table = (s32 *)p1;\n", "    p1 = p0->cells;\n    s.header = p0;\n    s.table = p1;\n", None)])
    t = fn(t, "func_80069E18", e18)

    def f80(b, last):
        b = tailed(b, "S_69F80", "Env_69F80", SPMAP)
        b = subs(b, [("    s32 *ptr;\n", "    Unk8009B0E0Record **ptr;\n"),
                     ("    s32 p1;\n    s32 p2;\n    s32 tbl;\n", "    Unk8009B0E0Record *p1;\n    Unk8009B0E0Record *p2;\n    Unk8009B400Record *tbl;\n"),
                     ("        tbl = s.env.header + 0xC;\n", "        tbl = s.env.header->cells;\n"),
                     ("        tbl = p1 + 0xC;\n", "        tbl = p1->cells;\n"),
                     ("func_8006E480(s.env.header, 0)", "func_8006E480((s32)s.env.header, 0)")])
        return last(b)
    t = fn(t, "func_80069F80", lambda b: f80(b, lambda b: sub1(b, "            s.env.header = p2;\n            p2 += 0x14;\n            s.env.table = p2;\n",
                                                               "            s.env.header = p2;\n            s.env.table = &p2->cells[1];\n")))
    t = fn(t, "func_8006A1A0", lambda b: f80(b, lambda b: sub1(b, "            tbl = p2 + 0xC;\n", "            tbl = p2->cells;\n")))

    def raw(b):
        for a, c in RAW:
            b = b.replace(a, c)
        return b
    def a3cc(b):
        b = sub1(b, "(s32 *arg0, u8 *arg1) {", "(s32 *arg0, Unk8007352CEnv *arg1) {")
        b = raw(b)
        return subs(b, [("    arg1->table = arg1->header + 0xC;\n", "    arg1->table = arg1->header->cells;\n"),
                        ("func_8006E480(arg1->header, 0)", "func_8006E480((s32)arg1->header, 0)")])
    t = fn(t, "func_8006A3CC", a3cc)
    t = fn(t, "func_8006A494", a3cc)
    def a564(b):
        b = sub1(b, "void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {", "void func_8006A564(u8 *arg0, Unk8007352CEnv *arg1, s32 arg2) {")
        b = raw(b)
        b = b.replace("(u32)(arg1->col_r)", "(u32)arg1->col_r").replace("(u32)(arg1->col_b)", "(u32)arg1->col_b")
        b = b.replace("((arg1->y) + ", "(arg1->y + ").replace("((arg1->x) + ", "(arg1->x + ").replace("= (arg1->x);", "= arg1->x;")
        b = b.replace("(arg1->ot_idx << 2)", "(arg1->ot_idx << 2)")
        return subs(b, [("    s32 *tbl;\n", "    Unk8009B0E0Record **tbl;\n"),
                        ("        s32 v0, v1;\n        *(s32 *)(arg1 + 0) = tbl[10];\n".replace("*(s32 *)(arg1 + 0)", "arg1->header"),
                         "        Unk8009B0E0Record *v0;\n        Unk8009B400Record *v1;\n"
                         "        /* FAKE: the header store through a pointer to the member: stored as\n"
                         "           arg1->header the member store is scheduled past the D_800A34F8 load\n"
                         "           (MEM_IN_STRUCT_P); score 7. */\n"
                         "        Unk8009B0E0Record **hp = &arg1->header;\n\n        *hp = tbl[10];\n"),
                        ("        v1 = arg1->header;\n        v1 += 0xC;\n", "        v1 = arg1->header->cells;\n"),
                        ("        v1 = v0 + 0xC;\n", "        v1 = v0->cells;\n"),
                        ("func_8006E480(arg1->header, s4)", "func_8006E480((s32)arg1->header, s4)")])
    t = fn(t, "func_8006A564", a564)

    def a880(b):
        b = retype(b, "s", "S_6A880", {})
        return subs(b, [("    s32 *sheets;\n", "    Unk8009B0E0Record **sheets;\n"), ("    s32 cells;\n", "    Unk8009B400Record *cells;\n"),
                        ("cells = s.header + 0xC;", "cells = s.header->cells;", None),
                        ("func_8006E480(s.header, 0)", "func_8006E480((s32)s.header, 0)", None),
                        ("(u8 *)&s", "&s", None)])
    t = fn(t, "func_8006A880", a880)

    def b120(b):
        b = retype(b, "s", "S_6B120", S6B120_MAP)
        return subs(b, [("    s32 *tbl;\n", "    Unk8009B0E0Record **tbl;\n"), ("    s32 p1;\n", "    Unk8009B400Record *p1;\n"),
                        ("p1 = s.header + 0xC;", "p1 = s.header->cells;", None),
                        ("func_8006E480(s.header, 0)", "func_8006E480((s32)s.header, 0)", None)])
    t = fn(t, "func_8006B120", b120)

    def bb68(b):
        b = retype(b, "s", "S69E18", S69E18_MAP)
        return subs(b, [("    s32 *q;\n    s32 p1;\n", "    Unk8009B0E0Record **q;\n    Unk8009B400Record *p1;\n"),
                        ("        s32 p0 = q[0];\n", "        Unk8009B0E0Record *p0 = q[0];\n"),
                        ("        s32 p0 = q[1];\n", "        Unk8009B0E0Record *p0 = q[1];\n"),
                        ("p1 = p0 + 0xC;\n", "p1 = p0->cells;\n", None),
                        ("s.header = (s32 *)p0;\n", "s.header = p0;\n", None),
                        ("s.table = (s32 *)p1;\n", "s.table = p1;\n", None),
                        ("    s.header = (s32 *)q[1];\n", "    s.header = q[1];\n")])
    t = fn(t, "func_8006BB68", bb68)

    def bd28(b):
        b = sub1(b, "S_6A880 *arg2, s32 arg3) {", "Unk8007352CEnv *arg2, s32 arg3) {")
        return subs(b, [("    s32 *sheets;\n", "    Unk8009B0E0Record **sheets;\n"), ("    s32 cells;\n", "    Unk8009B400Record *cells;\n"),
                        ("        if (arg2->header == -1) return;\n", "        if (arg2->header == (Unk8009B0E0Record *)-1) return;\n"),
                        ("            cells = arg2->header + 0xC;\n            arg2->table = cells + j * 8;\n",
                         "            cells = arg2->header->cells;\n            arg2->table = cells + j;\n"),
                        ("func_8006E480(arg2->header, 0)", "func_8006E480((s32)arg2->header, 0)")])
    t = fn(t, "func_8006BD28", bd28)
    t = sub1(t, "void func_8006BD28(s32 arg0, s32 arg1, S_6A880 *arg2, s32 arg3) {", "void func_8006BD28(s32 arg0, s32 arg1, Unk8007352CEnv *arg2, s32 arg3) {") if "S_6A880 *arg2" in t else t
    t = fn(t, "func_8006BEC4", lambda b: sub1(b, "    S_6A880 sp10;\n", "    Unk8007352CEnv sp10;\n"))

    def c21c(b):
        b = retype(b, "s", "Env_8006C21C", ENVOUT)
        return subs(b, [("    s32 *table;\n", "    Unk8009B0E0Record **table;\n"), ("    u8 *cells;\n", "    Unk8009B400Record *cells;\n"),
                        ("s.header = (u8 *)table[", "s.header = table[", None),
                        ("cells = s.header + 0xC;", "cells = s.header->cells;", None),
                        ("       12-byte header (SprtHdrA / SprtEntA, read by func_8007352C); every\n",
                         "       12-byte header (Unk8009B0E0Record / Unk8009B400Record, read by func_8007352C); every\n")])
    t = fn(t, "func_8006C21C", c21c)

    def cfbc(b):
        b = retype(b, "s", "Env_8006CFBC", ENVOUT)
        return subs(b, [("    s32 *table;\n", "    Unk8009B0E0Record **table;\n"), ("    s8 *temp;\n", "    Unk8009B400Record *temp;\n"),
                        ("s.header = (s32 *)table[", "s.header = table[", None),
                        ("temp = (s8 *)s.header + 0xC;", "temp = s.header->cells;", None)])
    t = fn(t, "func_8006CFBC", cfbc)

    def d3dc(b):
        b = retype(b, "s", "EnvA", ENVOUT)
        return subs(b, [("    s32 *q;\n", "    Unk8009B0E0Record **q;\n"), ("    s32 hdr;\n", "    Unk8009B0E0Record *hdr;\n"),
                        ("        s.header = (s32 *)hdr;\n        s.table = (s8 *)(hdr + 0xC);\n", "        s.header = hdr;\n        s.table = hdr->cells;\n")])
    t = fn(t, "func_8006D3DC", d3dc)

    def d808(b):
        b = retype(b, "s", "EnvA", ENVOUT)
        b = sub1(b, "(s32 *arg0, s32 *arg1, s32 *arg2, s32 arg3, s32 arg4) {", "(s32 *arg0, s32 *arg1, Unk8006D808Set *arg2, s32 arg3, s32 arg4) {")
        return subs(b, [("    s.header = (s32 *)arg2[2];\n", "    s.header = arg2->unk_00[2];\n"),
                        ("        s.header = (s32 *)arg2[i];\n        s.table = (s8 *)((s32)s.header + 0xC);\n", "        s.header = arg2->unk_00[i];\n        s.table = s.header->cells;\n"),
                        ("    s.header = (s32 *)arg2[0];\n", "    s.header = arg2->unk_00[0];\n"),
                        ("        s.header = (s32 *)arg2[3];\n", "        s.header = arg2->unk_0C;\n"),
                        ("        s.table = (s8 *)(arg2[4] + 24 + idx * 24);\n", "        s.table = arg2->unk_10 + 3 + idx * 3;\n"),
                        ("        w = ((s16 *)arg2[7])[idx];\n", "        w = arg2->unk_1C[idx];\n"),
                        ("            s.table = (s8 *)arg2[4];\n", "            s.table = arg2->unk_10;\n"),
                        ("    s.header = (s32 *)arg2[3];\n", "    s.header = arg2->unk_0C;\n"),
                        ("    s32 p;\n", "    Unk8009B0E0Record *p;\n"),
                        ("            p = arg2[5];\n            s.header = (s32 *)p;\n            *(s16 *)(p + 8) = d[0] * 24;\n            s.table = (s8 *)arg2[6];\n",
                         "            p = arg2->unk_14;\n            s.header = p;\n            p->ubase = d[0] * 24;\n            s.table = arg2->unk_18;\n"),
                        ("            *(s16 *)((s32)s.header + 8) = d[1] * 24;\n", "            s.header->ubase = d[1] * 24;\n"),
                        ("    s.header = (s32 *)arg2[5];\n", "    s.header = arg2->unk_14;\n"),
                        ("0x2C EnvA descriptor (sp+0x18..0x43)", "0x2C descriptor (sp+0x18..0x43)"),
                        ("       EnvA + a separate 8-aligned s16 array", "       the descriptor + a separate 8-aligned s16 array")])
    t = fn(t, "func_8006D808", d808)

    def dd94(b):
        b = tailed(b, "EnvB", "Env_8006DD94", ENVOUT)
        return subs(b, [("    s32 *q;\n", "    Unk8006DD94List *q;\n"), ("    s32 hdr;\n", "    Unk8009B0E0Record *hdr;\n"),
                        ("        hdr = q[i + 8];\n        s.env.header = (s32 *)hdr;\n        s.env.table = (s8 *)(hdr + 0xC);\n",
                         "        hdr = q->unk_20[i];\n        s.env.header = hdr;\n        s.env.table = hdr->cells;\n"),
                        ("func_8006D808(&arg0[5], &arg0[7], q, s.env.ot_idx, -1);", "func_8006D808(&arg0[5], &arg0[7], &q->unk_00, s.env.ot_idx, -1);"),
                        ("(EnvB = 0x2C + u16 rect[4])", "(the 0x2C descriptor + u16 rect[4])")])
    t = fn(t, "func_8006DD94", dd94)
    return t

# ---------------------------------------------------------------- carried-construct sweep (checklist B4)
SWEEP = [
    ("func_80069AE4", [("        Unk8009B0E0Record *v = *q;\n        s.header = v;\n        s.table = v->cells;\n", "        s.header = *q;\n        s.table = s.header->cells;\n"),
        ("        Unk8009B0E0Record *first = qbase[0];\n        s.header = first;\n", "        s.header = qbase[0];\n"),
        ("func_8006E480((s32)first, 0)", "func_8006E480((s32)s.header, 0)")]),
    ("func_8006D3DC", [("    Unk8009B0E0Record *hdr;\n", ""),
        ("        hdr = q[i];\n        s.header = hdr;\n        s.table = hdr->cells;\n", "        s.header = q[i];\n        s.table = s.header->cells;\n"),
        ("    s32 c;\n", ""),
        ("            c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n            s.col_r = s.col_g = s.col_b = c;\n",
         "            s.col_r = s.col_g = s.col_b = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n")]),
    ("func_8006DD94", [("    Unk8009B0E0Record *hdr;\n", ""),
        ("        hdr = q->unk_20[i];\n        s.env.header = hdr;\n        s.env.table = hdr->cells;\n", "        s.env.header = q->unk_20[i];\n        s.env.table = s.env.header->cells;\n"),
        ("    s32 c;\n", ""),
        ("            c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n            s.env.col_r = s.env.col_g = s.env.col_b = c;\n",
         "            s.env.col_r = s.env.col_g = s.env.col_b = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;\n")]),
    ("func_8006D808", [("    s16 v;\n", ""),
        ("            v = d[1] / 10;\n            d[0] = d[0] % 10;\n            d[1] = v % 10;\n", "            d[0] = d[0] % 10;\n            d[1] = d[1] / 10 % 10;\n"),
        ("    Unk8009B0E0Record *p;\n", ""),
        ("            p = arg2->unk_14;\n            s.header = p;\n            p->ubase = d[0] * 24;\n", "            s.header = arg2->unk_14;\n            s.header->ubase = d[0] * 24;\n")]),
]

HOLD = "FAKE: single-use holder of the cell table ahead of the descriptor's table store; stored directly: score %d."
LABELS = [
    ("func_80069E18", "    Unk8009B400Record *p1;\n", "    /* " + HOLD % 8 + " */\n    Unk8009B400Record *p1;\n"),
    ("func_80069F80", "       TU) and func_80041BF4 (src/text1a_post.c). */\n", "       TU) and func_80041BF4 (src/text1a_post.c). The plain descriptor: score 12. */\n"),
    ("func_8006A1A0", "       func_80069F80 and func_8006DD94 (this TU). */\n", "       func_80069F80 and func_8006DD94 (this TU). The plain descriptor: score 14. */\n"),
    ("func_8006DD94", "       below the rect (function.c:724). */\n", "       below the rect (function.c:724). The plain descriptor: score 21. */\n"),
    ("func_80069F80", "    Unk8009B400Record *tbl;\n", "    /* " + HOLD % 8 + " */\n    Unk8009B400Record *tbl;\n"),
    ("func_8006A1A0", "    Unk8009B400Record *tbl;\n", "    /* " + HOLD % 8 + " */\n    Unk8009B400Record *tbl;\n"),
    ("func_8006A564", "        Unk8009B400Record *v1;\n", "        /* " + HOLD % 7 + " */\n        Unk8009B400Record *v1;\n"),
    ("func_8006A880", "       +0xC (MOD.BIN census). */\n", "       +0xC (MOD.BIN census). " + HOLD % 15 + " */\n"),
    ("func_8006B120", "    Unk8009B400Record *p1;\n", "    /* " + HOLD % 11 + " */\n    Unk8009B400Record *p1;\n"),
    ("func_8006BB68", "    Unk8009B400Record *p1;\n", "    /* " + HOLD % 11 + " */\n    Unk8009B400Record *p1;\n"),
    ("func_8006BD28", "       j << 3 each iteration. */\n", "       j << 3 each iteration. Inline: score 31. */\n"),
    ("func_8006BD28", "           iteration; sheets[arg0 * 2 + i] and (sheets + arg0 * 2)[i] do not\n           match. */\n",
     "           iteration; sheets[arg0 * 2 + i]: score 33. */\n"),
    ("func_8006C21C", "       the entry block); a literal 0 at every call does not match. Same shape as\n",
     "       the entry block); a literal 0 at every call: score 3. Same shape as\n"),
    ("func_8006C21C", "    /* Holds several values (Ruling 11, ordinary-c-judge-decidable.md): the\n",
     "    /* FAKE: holds several values (Ruling 11, ordinary-c-judge-decidable.md): the\n"),
    ("func_8006C21C", "       `j` for $fp). */\n", "       `j` for $fp); one counter per phase: score 90. */\n"),
    ("func_8006C21C", "       loads `li $v0,0x80` in each else arm. Literal and chained forms do not\n       match. */\n",
     "       loads `li $v0,0x80` in each else arm. The literals: score 8. */\n"),
    ("func_8006C21C", "       the mechanism needs the s16 read. */\n", "       the mechanism needs the s16 read. Literals at every read: score 37. */\n"),
    ("func_8006C21C", "       one meaning, header + 0xC at every write. */\n", "       one meaning, header + 0xC at every write. " + HOLD % 12 + " */\n"),
    ("func_8006CFBC", "     * table[17], [18] or [19] in the last loop. */\n", "     * table[17], [18] or [19] in the last loop. " + HOLD % 8 + " */\n"),
    ("func_8006D3DC", "    s32 semi = 0;\n", "    /* FAKE: constant holder for func_8006E480's 0; the literal: score 8. */\n    s32 semi = 0;\n"),
    ("func_8006D3DC", "    u8 dim = 0x40;\n", "    /* FAKE: constant holder for the dimmed colour; the literal: score 25. */\n    u8 dim = 0x40;\n"),
    ("func_8006DD94", "    s32 semi = 0;\n", "    /* FAKE: constant holder for the semi flag and func_8006E480's 0; the literal: score 8. */\n    s32 semi = 0;\n"),
    ("func_8006D808", "       (u16 rect[4]); same carve-out as the caller func_8006DD94. */\n",
     "       (u16 rect[4]); same carve-out as the caller func_8006DD94. s16 d[2]: score 26. */\n"),
    ("func_8006D808", "    s32 w;\n", "    /* FAKE: holder of the character's x offset for both x stores; read at each: score 27. */\n    s32 w;\n"),
    ("func_80069F80", "sites); sp44/sp48/sp4C/sp50 are this call site's UNWRITTEN PADDING tail.", "sites); tail[0..3] are this call site's UNWRITTEN PADDING tail."),
    ("func_8006A1A0", "the S_69F80 tail sp44/sp48/sp4C/sp50\n", "the Env_69F80 tail tail[0..3]\n"),
    ("func_8006A1A0", "the one declared (S_69F80, shared", "the one declared (Env_69F80, shared"),
    ("func_8006DD94", "pad2C/pad30 are its unwritten tail.", "tail[0..1] are its unwritten tail."),
    ("func_8006DD94", "0x34 (pad2C, pad30) and 0x38 (pad2C, pad30, pad34)\n", "0x34 (two tail words) and 0x38 (three)\n"),
    ("func_8006DD94", "0x30 (pad2C alone) puts", "0x30 (one tail word) puts"),
]

def sweep_labels(t):
    for f, reps in SWEEP:
        t = fn(t, f, lambda b, reps=reps: subs(b, reps))
    for f, old, new in LABELS:
        t = fn(t, f, lambda b, old=old, new=new: sub1(b, old, new))
    return t

# ---------------------------------------------------------------- rev-fdesc3 fixes (measured by the reviewer, tmp/rev_fdesc3/)
E18 = [("    Unk8009B0E0Record *p0;\n", ""),
       ("    p0 = s.header;\n    p1 = p0->cells;\n", "    p1 = s.header->cells;\n"),
       ("    p0 = ptr[1];\n    p1 = p0->cells;\n    s.header = p0;\n", "    s.header = ptr[1];\n    p1 = s.header->cells;\n"),
       ("    p0 = ptr[2];\n    p1 = p0->cells;\n    s.header = p0;\n", "    s.header = ptr[2];\n    p1 = s.header->cells;\n")]
F80 = [("    Unk8009B0E0Record *p1;\n    Unk8009B0E0Record *p2;\n", ""),
       ("        p1 = ptr[1];\n        tbl = p1->cells;\n        s.env.header = p1;\n", "        s.env.header = ptr[1];\n        tbl = s.env.header->cells;\n"),
       ("            p2 = ptr[6];\n", "            s.env.header = ptr[6];\n"),
       ("            s.env.header = p2;\n            s.env.table = &p2->cells[1];\n", "            s.env.table = &s.env.header->cells[1];\n"),
       ("= (s8)c;", "= c;", None)]
A1A0 = [("    Unk8009B0E0Record *p1;\n    Unk8009B0E0Record *p2;\n", ""),
        ("        p1 = ptr[3];\n        tbl = p1->cells;\n        s.env.header = p1;\n", "        s.env.header = ptr[3];\n        tbl = s.env.header->cells;\n"),
        ("            p2 = ptr[6];\n", "            s.env.header = ptr[6];\n"),
        ("            tbl = p2->cells;\n            s.env.header = p2;\n", "            tbl = s.env.header->cells;\n"),
        ("= (s8)c;", "= c;", None)]
BB68 = [("        Unk8009B0E0Record *p0 = q[0];\n        s.semi = 0;\n        p1 = p0->cells;\n        s.header = p0;\n", "        s.header = q[0];\n        s.semi = 0;\n        p1 = s.header->cells;\n"),
        ("        Unk8009B0E0Record *p0 = q[1];\n        p1 = p0->cells;\n        s.header = p0;\n", "        s.header = q[1];\n        p1 = s.header->cells;\n")]
COLOUR = "        /* FAKE: colour staged ahead of the g0 store (held across the arms for b0); col_r read at its use: score 2. */\n"
A564 = [("    Unk8006919CRec *root;\n", ""), ("    root = *(Unk8006919CRec **)(arg0 + 4);\n", ""), ("    tbl = root->unk_1C;\n", "    tbl = (*(Unk8006919CRec **)(arg0 + 4))->unk_1C;\n"),
        ("        Unk8009B0E0Record *v0;\n", ""),
        ("        v0 = tbl[11];\n        v1 = v0->cells;\n        arg1->header = v0;\n", "        arg1->header = tbl[11];\n        v1 = arg1->header->cells;\n"),
        ("arg1->col_r = (u32)arg1->col_r >> 1;", "arg1->col_r = arg1->col_r >> 1;"),
        ("arg1->col_b = (u32)arg1->col_b >> 1;", "arg1->col_b = arg1->col_b >> 1;"),
        ("    {\n        s32 v0;\n", "    {\n" + COLOUR + "        s32 v0;\n", None)]

def rev1(t):
    t = fn(t, "func_80069E18", lambda b: subs(b, E18))
    t = fn(t, "func_80069F80", lambda b: subs(b, F80))
    t = fn(t, "func_8006A1A0", lambda b: subs(b, A1A0))
    t = fn(t, "func_8006BB68", lambda b: subs(b, BB68))
    t = fn(t, "func_8006A564", lambda b: subs(b, A564))
    for f in ("func_80069F80", "func_8006A1A0"):
        t = fn(t, f, lambda b: sub1(b, "func_80073728((s32)&s, 0)", "func_80073728((s32)&s.env, 0)"))
    return t

def s3AB48(t):
    return sub1(t, "extern s32 D_8009B0C0;\n", "extern Unk8006D808Set D_8009B0C0;\n")

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = base()
    out["game.h"], out["bb2.h"] = headers(out["game.h"], out["bb2.h"])
    out["51268.c"] = rev1(sweep_labels(s51268(out["51268.c"])))
    out["3AB48.c"] = s3AB48(out["3AB48.c"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fdesc3", sorted(OPT))
