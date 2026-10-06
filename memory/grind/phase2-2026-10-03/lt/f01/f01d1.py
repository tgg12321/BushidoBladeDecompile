#!/usr/bin/env python3
# F01 batch d-1, on top of F01c (d8c591f23): D_800A34EC, the work area at 0x1F8000B8
# (Unk1F800000Rec.unkB8), becomes a pointer to Unk1F8000B8Union (game.h; the F02 Unk1F8002B8Union
# model: raw sizes it, one member per layout). This batch adds the layouts of func_80065800 and of the
# func_800678A8 / func_80067D14 / func_80068D88 trio; the other users (func_80061FAC, func_800620B8,
# func_8006295C, func_80063084, func_80063E10, func_800646E8) still convert D_800A34EC as at HEAD
# (later batches). Unk800EFC78Record moves to game.h; D_800A3724 (= &trio.unk1AC) becomes s32 *.
# usage: f01d1.py [measure [func...]] [opt=<name>,...]
#   writes tmp/p2/f01d1/{51268.c,game.h,bb2.h,64FD8.c} (scratch only); with opt=..., tmp/p2/f01d1/opt/
import os, re, shutil, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f01c as C
sys.argv = _argv
OUT = "tmp/p2/f01d1/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = C.sub1
fn = C.fn

BASE_REV = "d8c591f23"   # F01c as committed (equal to f01c.py's output)
HEAD64 = BASE_REV       # its 64FD8.c (unchanged since 643264f38)

def base():
    def show(p):
        return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                              text=True, encoding="utf-8").stdout
    return show("src/main/51268.c"), show("include/game.h"), show("include/bb2.h")

EFC78 = """/* 20-byte record table at 0x800EFC78: 4 rows (arg1) of 48 records. Object
 * model evidence: asm/funcs/func_80067200.s addresses it as
 * base + arg1*0x3C0 + i*20 with halfword stores at +0..+0xC, +0x10, +0x12
 * (+0xE untouched here); 0x3C0 / 20 = 48 = the loop's record count. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
    s16 unk12;
} Unk800EFC78Record;
"""

LAYOUTS = """/* 20-byte record table at 0x800EFC78 (51268.c): 4 rows (arg1) of 48 records. Object model evidence:
 * asm/funcs/func_80067200.s addresses it as base + arg1*0x3C0 + i*20 with halfword stores at
 * +0..+0xC, +0x10, +0x12 (+0xE untouched there); 0x3C0 / 20 = 48 = the loop's record count. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
    s16 unk12;
} Unk800EFC78Record;

/* func_80065800's layout of the 51268 work area (Unk1F8000B8Union.v80065800). Its gte_SetTransMatrix
 * operand hands the GTE the area's base as a MATRIX, whose t[] is unk14[0]. */
typedef struct {
    u8 unk00[0x10];                /* no access */
    s32 unk10;                     /* gte_stdp out; not read */
    VECTOR unk14[4];               /* ApplyRotMatrixLV out ([0]); the corner loop's ApplyRotMatrix outs */
    VECTOR unk54;                  /* ApplyRotMatrixLV in */
    SVECTOR unk64;                 /* gte_ldv0 / RotMatrix / ApplyRotMatrix in */
    s16 unk6C;                     /* p_w */
    u8 unk6E[2];                   /* no access */
    s16 unk70;                     /* p_h */
    u8 unk72[2];                   /* no access */
    MATRIX unk74;                  /* RotMatrix out, SetRotMatrix in */
    s16 unk94;                     /* added to the texture u */
    s16 unk96;                     /* added to the texture v */
} Unk1F8000B8_80065800;

/* The layout func_800678A8 / func_80067D14 / func_80068D88 share (Unk1F8000B8Union.v800678A8): each of
 * the wrappers func_800676C8 .. func_8006786C calls the three in turn, and values cross the calls
 * (unk04, unk6C, unk80, the unk8C table; func_800678A8 and func_80067D14 each use unk70 for their own
 * value). func_800678A8 sets it up, func_80067D14 builds the quads at unk80 and records each one's
 * depth (its OT slot index) in unk8C, func_80068D88 links them. */
typedef struct {
    u16 unk00;                     /* func_800678A8: added to the texture u */
    u16 unk02;                     /* func_800678A8: added to the texture v */
    u32 unk04;                     /* func_800678A8 stores 0x895440; func_80067D14 compares against it */
    u8 unk08[0x1C];                /* no access */
    s32 unk24[3];                  /* func_80067D14: gte_stlvnl out */
    u8 unk30[4];                   /* no access */
    VECTOR unk34;                  /* func_80067D14: gte_ldlvl in */
    SVECTOR unk44[3];              /* func_80067D14: gte_ldv0 / gte_ldv3 in */
    VECTOR unk5C;                  /* func_80067D14 */
    s16 unk6C;                     /* func_800678A8 stores it; func_80067D14's loop bound */
    s16 unk6E;                     /* func_80067D14 / func_80068D88 loop index */
    s16 unk70;                     /* func_800678A8: table index; func_80067D14: per entry */
    u8 unk72;                      /* func_80067D14 colours */
    u8 unk73;
    u8 unk74;
    u8 unk75[3];                   /* no access */
    s16 unk78;                     /* func_80067D14 */
    u8 unk7A[2];                   /* no access */
    POLY_FT4 *unk7C;               /* func_80068D88: the end of the quads */
    POLY_FT4 *unk80;               /* the quad cursor */
    Unk800EFC78Record *unk84;      /* func_80067D14 */
    Unk800F0C10Record *unk88;      /* func_80067D14 */
    u16 unk8C[0x90];               /* per quad, its depth, the OT slot index (func_80067D14 stores,
                                      func_80068D88 reads) */
    s32 unk1AC;                    /* func_80067D14 stores rand() values here; D_800A3724 points at it */
} Unk1F8000B8_800678A8;

/* Unk1F800000Rec.unkB8, the 51268 work area D_800A34EC points at (0x1F8000B8 to the end of the
 * scratchpad). Each user lays it out for one call (or, for the func_800678A8 trio, one wrapper call);
 * raw sizes the union. func_80065800 and the trio use their members; func_80061FAC, func_800620B8,
 * func_8006295C, func_80063084, func_80063E10 and func_800646E8 still convert D_800A34EC (later
 * batches). */
typedef union {
    u8 raw[0x400 - 0xB8];
    Unk1F8000B8_80065800 v80065800;
    Unk1F8000B8_800678A8 v800678A8;
} Unk1F8000B8Union;

"""

def game(g):
    a = "/* 51268's view of the scratchpad from 0x1F800000 (SPAD51268 in 51268.c)."
    g = sub1(g, a, LAYOUTS + a)
    g = sub1(g, "    u8 unkB8[0x400 - 0xB8];\n} Unk1F800000Rec;", "    Unk1F8000B8Union unkB8;\n} Unk1F800000Rec;")
    g = sub1(g, " * - unkB8: the work area D_800A34EC points at, laid out differently by its users (later batch);\n *   it runs to the end of the scratchpad.\n",
             " * - unkB8: the work area D_800A34EC points at (Unk1F8000B8Union), laid out differently by its\n *   users; it runs to the end of the scratchpad.\n")
    return g

def src(s):
    s = sub1(s, "static s32 D_800A34EC;\n", "static Unk1F8000B8Union *D_800A34EC;\n")
    s = sub1(s, "    D_800A34EC = 0x1F8000B8;\n", "    D_800A34EC = &SPAD51268->unkB8;\n")
    s = sub1(s, EFC78, "")
    s = sub1(s, "\ns32 D_800A3724;\n", "\ns32 *D_800A3724;\n")
    n = s.count("    extern s32 D_800A3724;\n")
    assert n == 3, n
    s = s.replace("    extern s32 D_800A3724;\n", "    extern s32 *D_800A3724;\n")
    s = fn(s, "func_80065800", b65800)
    s = fn(s, "func_800678A8", b678A8)
    s = fn(s, "func_80067D14", b67D14)
    s = fn(s, "func_80068D88", b68D88)
    return s

def b65800(b):
    b = sub1(b, "    s32 outer;\n", "    Unk1F8000B8_80065800 *outer;\n")
    b = sub1(b, "    outer = D_800A34EC;\n", "    outer = &D_800A34EC->v80065800;\n")
    for old, new in (("p_dp = (s32 *)(outer + 0x10);", "p_dp = &outer->unk10;"),
                     ("p_t = (VECTOR *)(outer + 0x14);", "p_t = outer->unk14;"),
                     ("p_in = (VECTOR *)(outer + 0x54);", "p_in = &outer->unk54;"),
                     ("p_v = (SVECTOR *)(outer + 0x64);", "p_v = &outer->unk64;"),
                     ("p_w = (s16 *)(outer + 0x6C);", "p_w = &outer->unk6C;"),
                     ("p_h = (s16 *)(outer + 0x70);", "p_h = &outer->unk70;"),
                     ("p_mat = (MATRIX *)(outer + 0x74);", "p_mat = &outer->unk74;"),
                     ("p_tw = (s16 *)(outer + 0x94);", "p_tw = &outer->unk94;"),
                     ("p_th = (s16 *)(outer + 0x96);", "p_th = &outer->unk96;")):
        b = sub1(b, old, new)
    assert "(outer + " not in b
    return b

def b678A8(b):
    b = sub1(b, "    s32 outer = D_800A34EC;\n    s16 *p2 = (s16 *)(outer + 2);\n    s16 *p6C = (s16 *)(outer + 0x6C);\n",
             "    Unk1F8000B8_800678A8 *outer = &D_800A34EC->v800678A8;\n    u16 *p2 = &outer->unk02;\n    s16 *p6C = &outer->unk6C;\n")
    b = sub1(b, "    D_800A3724 = outer + 0x1AC;\n", "    D_800A3724 = &outer->unk1AC;\n")
    b = sub1(b, "    *(s32 *)(outer + 0x80) = D_800A37D4;\n", "    outer->unk80 = (POLY_FT4 *)D_800A37D4;\n")
    b = sub1(b, "    *(s32 *)(outer + 4) = 0x895440;\n", "    outer->unk04 = 0x895440;\n")
    b = b.replace("*(s16 *)(outer + 2) = ", "outer->unk02 = ")
    b = b.replace("*(s16 *)(outer + 0) = ", "outer->unk00 = ")
    b = b.replace("*(s16 *)(outer + 0x70)", "outer->unk70")
    b = sub1(b, "*(u16 *)outer + ", "outer->unk00 + ")
    b = sub1(b, "*(u16 *)p2 + ", "*p2 + ")
    assert "(outer + " not in b and "(u16 *)" not in b, b
    if "m678" in OPT:   # measurement: plain member accesses (the whole pointer cluster removed)
        b = sub1(b, "    u16 *p2 = &outer->unk02;\n    s16 *p6C = &outer->unk6C;\n", "")
        b = b.replace("*p2 = ", "outer->unk02 = ").replace("*p2 + ", "outer->unk02 + ").replace("*p6C = ", "outer->unk6C = ")
        return b
    # unk00 / unk02 / unk04 / unk6C / unk80 through plain pointers (FAKE, below)
    b = b.replace("outer->unk00", "*p0").replace("outer->unk04", "*p4").replace("outer->unk80", "*p80")
    b = b.replace("outer->unk02 = ", "*p2 = ")
    b = sub1(b, "    u16 *p2 = &outer->unk02;\n    s16 *p6C = &outer->unk6C;\n",
             FAKE678 + "    u16 *p0 = &outer->unk00;\n    u16 *p2 = &outer->unk02;\n    u32 *p4 = &outer->unk04;\n"
             "    s16 *p6C = &outer->unk6C;\n    POLY_FT4 **p80 = &outer->unk80;\n")
    for k in ("p0", "p2", "p4", "p6C", "p80"):   # measurement: one pointer replaced by member accesses
        if "a678_" + k in OPT:
            m = {"p0": "unk00", "p2": "unk02", "p4": "unk04", "p6C": "unk6C", "p80": "unk80"}[k]
            b = re.sub(r"    [^\n]*[ *]\*%s = &outer->%s;\n" % (k, m), "", b)
            assert "%s = &outer" % k not in b, k
            b = b.replace("*%s" % k, "outer->%s" % m)
    if "a678_new" in OPT:   # measurement: p0 / p4 / p80 (the new ones) replaced
        for k, m in (("p0", "unk00"), ("p4", "unk04"), ("p80", "unk80")):
            b = re.sub(r"    [^\n]*[ *]\*%s = &outer->%s;\n" % (k, m), "", b)
            assert "%s = &outer" % k not in b, k
            b = b.replace("*%s" % k, "outer->%s" % m)
    return b

FAKE678 = """    /* FAKE: unk00 / unk02 / unk04 / unk6C / unk80 are reached through the plain pointers p0 / p2 /
       p4 / p6C / p80 (p2 / p6C were locals already): a member store is MEM_IN_STRUCT_P, and
       sched.c moves scalar accesses across it (abl_d1.py, each pointer spelled as members): p0,
       in three of the four branches the D_800A34A8 load rises above the unk00 store (score 6);
       p4, the D_800A3490 load rises above the unk04 store and its load-delay nop goes (2); p80,
       the D_800A3724 store sinks below the unk80 store (2); p2, the unk02 stores sink below the
       D_800A34A8 loads, p2's $t0 (outer + 2) goes and $a0 takes a copy of outer (24); p6C, p6C's
       $t1 (outer + 0x6C) goes, arg0 moves from $a3 to $t0, p2 to $a3, and $a0 takes a copy of
       outer (38); all five: 49. */
"""

def b67D14(b):
    b = sub1(b, "    s32 outer = D_800A34EC;\n", "    Unk1F8000B8_800678A8 *outer = &D_800A34EC->v800678A8;\n")
    # its +0x8C table holds each quad's depth, the OT slot index: the TU's other drawers (and, since
    # F01c, func_80068D88) call that table zbuf; p_ot was false
    b = sub1(b, "    s16 *p_ot;\n", "    u16 *p_ot;\n")
    b = sub1(b, "    D_800A3724 = outer + 0x1AC;\n", "    D_800A3724 = &outer->unk1AC;\n")
    for old, new in (("p_seed = (s32 *)(outer + 0x1AC);", "p_seed = &outer->unk1AC;"),
                     ("p_rad = (u32 *)(outer + 4);", "p_rad = &outer->unk04;"),
                     ("p_out = (s32 *)(outer + 0x24);", "p_out = outer->unk24;"),
                     ("p_work = (VECTOR *)(outer + 0x34);", "p_work = &outer->unk34;"),
                     ("p_vert = (SVECTOR *)(outer + 0x44);", "p_vert = outer->unk44;"),
                     ("p_tv = (VECTOR *)(outer + 0x5C);", "p_tv = &outer->unk5C;"),
                     ("p_count = (s16 *)(outer + 0x6C);", "p_count = &outer->unk6C;"),
                     ("p_idx = (s16 *)(outer + 0x6E);", "p_idx = &outer->unk6E;"),
                     ("p_life = (s16 *)(outer + 0x70);", "p_life = &outer->unk70;"),
                     ("p_r = (u8 *)(outer + 0x72);", "p_r = &outer->unk72;"),
                     ("p_g = (u8 *)(outer + 0x73);", "p_g = &outer->unk73;"),
                     ("p_b = (u8 *)(outer + 0x74);", "p_b = &outer->unk74;"),
                     ("p_n = (s16 *)(outer + 0x78);", "p_n = &outer->unk78;"),
                     ("p_prim = (POLY_FT4 **)(outer + 0x80);", "p_prim = &outer->unk80;"),
                     ("p_ent = (Unk800EFC78Record **)(outer + 0x84);", "p_ent = &outer->unk84;"),
                     ("p_tgt = (Unk800F0C10Record **)(outer + 0x88);", "p_tgt = &outer->unk88;"),
                     ("p_ot = (s16 *)(outer + 0x8C);", "p_ot = outer->unk8C;")):
        b = sub1(b, old, new)
    assert "(outer + " not in b
    assert not re.search(r"\bzbuf\b", b)
    b = re.sub(r"\bp_ot\b", "zbuf", b)
    return b

def b68D88(b):
    b = sub1(b, """    s32 outer = D_800A34EC;
    s16 *p_idx = (s16 *)(outer + 0x6E);
    s32 *p_end = (s32 *)(outer + 0x7C);
    s32 *p_cur = (s32 *)(outer + 0x80);
    u16 *zbuf = (u16 *)(outer + 0x8C);
    s32 cur_init;
    s32 prev_init;
""", """    Unk1F8000B8_800678A8 *outer = &D_800A34EC->v800678A8;
    s16 *p_idx = &outer->unk6E;
    POLY_FT4 **p_end = &outer->unk7C;
    POLY_FT4 **p_cur = &outer->unk80;
    u16 *zbuf = outer->unk8C;
    POLY_FT4 *cur_init;
    POLY_FT4 *prev_init;
""")
    b = sub1(b, "    D_800A3724 = outer + 0x1AC;\n", "    D_800A3724 = &outer->unk1AC;\n")
    b = sub1(b, "    prev_init = D_800A37D4;\n", "    prev_init = (POLY_FT4 *)D_800A37D4;\n")
    b = sub1(b, "    count = (POLY_FT4 *)cur_init - (POLY_FT4 *)prev_init;\n", "    count = cur_init - prev_init;\n")
    b = sub1(b, "        if ((u32)*p_cur < (u32)*p_end) {\n", "        if (*p_cur < *p_end) {\n")
    b = sub1(b, "            } while ((u32)*p_cur < (u32)*p_end);\n", "            } while (*p_cur < *p_end);\n")
    b = sub1(b, "                p_b = (u32 *)*p_cur;\n", "                p_b = &(*p_cur)->tag;\n")
    b = sub1(b, "                *p_cur += 0x28;\n", "                (*p_cur)++;\n")
    b = sub1(b, "        D_800A37D4 = *p_end;\n", "        D_800A37D4 = (s32)*p_end;\n")
    assert "(outer + " not in b
    return b

def f64(t):
    # a stray unused file-scope `extern s32 D_800A3724;` (64FD8 never names it): it goes rather than
    # contradict the s32 * definition
    return sub1(t, "\n    extern s32 D_800A3724;\n", "\n")

def write():
    d = OUT + ("opt/" if OPT else "")
    os.makedirs(d, exist_ok=True)
    s0, g0, h0 = base()
    s = src(s0); g = game(g0); h = h0
    t = f64(subprocess.run(["git", "show", HEAD64 + ":src/main/64FD8.c"], capture_output=True, check=True, text=True, encoding="utf-8").stdout)
    for n, x in (("51268.c", s), ("game.h", g), ("bb2.h", h), ("64FD8.c", t)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return s, g, h, t

def measure(s, g, h, t, fs=()):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    for n, x in (("include/game.h", g), ("include/bb2.h", h), ("src/main/51268.c", s), ("src/main/64FD8.c", t)):
        open("tmp/p2/wk/" + n, "w", encoding="utf-8", newline=NL).write(x)
    for tu in ("main/51268", "main/64FD8"):
        r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", tu] + (list(fs) if tu == "main/51268" else []), capture_output=True, text=True)
        print(r.stdout + r.stderr)
        r = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", tu], capture_output=True, text=True)
        print(r.stdout + r.stderr)

if __name__ == "__main__":
    s, g, h, t = write()
    if "measure" in sys.argv[1:]:
        measure(s, g, h, t, [a for a in sys.argv[1:] if a.startswith("func_")])
    print("wrote d1", sorted(OPT))
