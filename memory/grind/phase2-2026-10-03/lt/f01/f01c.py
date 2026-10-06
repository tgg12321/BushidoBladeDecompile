#!/usr/bin/env python3
# F01 batch c, on top of F01b2 (ec8c6dcd2): the retargeted cursors.
# D_800A3488 / D_800A348C become TexRec * (TexRec hoisted to game.h; Unk1F800000Rec.unk50 / unkB0
# TexRec; the 29 D_8009B890..D_8009BA58 table externs TexRec); D_800A34E8 becomes u32 * (the POLY_FT4
# tag word it is aimed at); D_800A34E4 stays u8 * (P5) unless opt=ot32. func_80068D88's and
# func_800620B8's false names are fixed.
# usage: f01c.py [measure [func...]] [opt=<name>,...]
#   writes tmp/p2/f01c/{51268.c,game.h,bb2.h} (scratch only; never the tracked files);
#   with opt=..., tmp/p2/f01c/opt/ (measurement variants).
import os, re, shutil, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f01b2 as B2
sys.argv = _argv
OUT = "tmp/p2/f01c/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))

sub1 = B2.sub1

def span(s, f):
    m = re.search(r"\n[a-z0-9_]+ %s\([^;{]*\)\s*\{" % f, s)   # the definition, not a prototype
    i = m.start() + 1
    j = s.index("\n}\n", i) + 3
    return i, j

def fn(s, f, g):
    i, j = span(s, f)
    return s[:i] + g(s[i:j]) + s[j:]

BASE_REV = "ec8c6dcd2"   # F01b2 as committed (equal to f01b2.py's output)

def base():
    def show(p):
        return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                              text=True, encoding="utf-8").stdout
    return show("src/main/51268.c"), show("include/game.h"), show("include/bb2.h")

TEXREC_51268 = """/* One 8-byte texture record: the CLUT position (PsyQ getClut(x, y) =
   (y << 6) | ((x >> 4) & 0x3F)) and the texture u/v origin. */
typedef struct {
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
} TexRec;
"""
TEXREC_GAME = """/* One 8-byte texture record: the CLUT position (PsyQ getClut(x, y) = (y << 6) | ((x >> 4) & 0x3F))
 * and the texture u/v origin. 51268's D_800A3488 / D_800A348C point at one; the D_8009B890 ..
 * D_8009BA58 tables (asm/data/7D920.data.s) are runs of them. */
typedef struct {
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
} TexRec;

"""

def game(g, s):
    g = sub1(g, "/* The 0x2C-byte block D_800A3468 points at (51268.c)", TEXREC_GAME + "/* The 0x2C-byte block D_800A3468 points at (51268.c)")
    g = sub1(g, "    u8 unk50[8];\n", "    TexRec unk50;\n")
    g = sub1(g, "    u8 unkB0[8];\n", "    TexRec unkB0;\n")
    # the three reads that can see the seeded unk50 / unkB0 (func_800620B8, outside its switch)
    L = s.split(NL)
    n = [k + 1 for k, l in enumerate(L) if "*D_800A3494 = (u16)(((D_800A348C->clut_x >> 4)" in l]
    assert len(n) == 1, n
    a = n[0]
    assert "*D_800A3498 = D_800A3488->u;" in L[a] and "*D_800A34A0 = D_800A3488->v;" in L[a + 1]
    g = sub1(g, """ * - unk50 / unkB0: the initial targets of D_800A3488 / D_800A348C. func_800620B8 retargets them
 *   only in its switch cases 0-3 (unk4 & 7), so its u16 reads at 51268.c:953-955 go through these
 *   seeded values when no earlier record took one of those cases (typed with the globals later).
""", """ * - unk50 / unkB0: the initial targets of D_800A3488 / D_800A348C (TexRec). func_800620B8 retargets
 *   them only in its switch cases 0-3 (unk4 & 7), so its reads at 51268.c:%d-%d go through these
 *   seeded values when no earlier record took one of those cases.
""" % (a, a + 2))
    if "ot32" in OPT:
        g = sub1(g, "    u8 unkA0[8];\n", "    u32 unkA0;\n    u32 unkA4;\n")
        g = sub1(g, " * - unkA0: the initial targets of D_800A34E4 / D_800A34E8, which every user retargets before use.\n",
                 " * - unkA0 / unkA4: the initial targets of D_800A34E4 / D_800A34E8, which every user retargets\n *   before use.\n")
    else:
        g = sub1(g, "    u8 unkA0[8];\n", "    u8 unkA0[4];\n    u32 unkA4;\n")
        g = sub1(g, " * - unkA0: the initial targets of D_800A34E4 / D_800A34E8, which every user retargets before use.\n",
                 " * - unkA0 / unkA4: the initial targets of D_800A34E4 (u8 *) / D_800A34E8 (u32 *), which every\n *   user retargets before use.\n")
    return g

READ = {0: "clut_x", 1: "clut_y", 2: "u", 3: "v"}

def reads(s):
    for g in ("D_800A3488", "D_800A348C"):
        s = re.sub(r"\(\(u16 \*\)%s\)\[([0-3])\]" % g, lambda m: "%s->%s" % (g, READ[int(m.group(1))]), s)
        s = s.replace("((TexRec *)%s)->" % g, "%s->" % g)
    return s

TABLES_1 = ["D_8009B958", "D_8009B960", "D_8009B968", "D_8009B970", "D_8009B940", "D_8009B948", "D_8009B950",
            "D_8009B8C8", "D_8009B8D0", "D_8009B8D8", "D_8009B8E0", "D_8009B978", "D_8009B980", "D_8009B988",
            "D_8009B990", "D_8009B9D8", "D_8009B9E0", "D_8009B9E8", "D_8009B9F0", "D_8009B890", "D_8009B8B0",
            "D_8009B998", "D_8009B9B8"]

def src(s):
    # TexRec moves to game.h
    s = sub1(s, TEXREC_51268, "")
    s = sub1(s, "static s32 D_800A3488;\nstatic s32 D_800A348C;\n", "static TexRec *D_800A3488;\nstatic TexRec *D_800A348C;\n")
    s = sub1(s, "    D_800A3488 = 0x1F800050;\n", "    D_800A3488 = &SPAD51268->unk50;\n")
    s = sub1(s, "    D_800A348C = 0x1F8000B0;\n", "    D_800A348C = &SPAD51268->unkB0;\n")
    # table externs: u16 [] -> TexRec []
    for t in TABLES_1:
        n = s.count("extern u16 %s[];\n" % t)
        assert n == 1, (t, n)
        s = s.replace("extern u16 %s[];\n" % t, "extern TexRec %s[];\n" % t)
    s = sub1(s, "    extern u16 D_8009BA00[6][4];\n    extern u16 D_8009BA30[4][4];\n    extern u16 D_8009BA50[4];\n    extern u16 D_8009BA58[4];\n",
             "    extern TexRec D_8009BA00[6];\n    extern TexRec D_8009BA30[4];\n    extern TexRec D_8009BA50[1];\n    extern TexRec D_8009BA58[1];\n")
    s = sub1(s, "    extern u16 D_8009B920[][4];\n", "    extern TexRec D_8009B920[];\n")
    # stores
    for t in TABLES_1:
        s = s.replace("(s32)%s;" % t, "%s;" % t)
        s = s.replace("(s32)%s :" % t, "%s :" % t)
    s = sub1(s, "    D_800A3488 = (s32)D_8009B920[*D_800A3480];\n", "    D_800A3488 = &D_8009B920[*D_800A3480];\n")
    s = sub1(s, "            D_800A3488 = (s32)&D_8009B8E8[*frame];\n", "            D_800A3488 = &D_8009B8E8[*frame];\n")
    s = sub1(s, "            D_800A3488 = (s32)&D_8009B9B8[*(s16 *)(outer + 0x70) * 4];\n", "            D_800A3488 = &D_8009B9B8[*(s16 *)(outer + 0x70)];\n")
    s = sub1(s, "            D_800A3488 = (s32)&D_8009B998[*(s16 *)(outer + 0x70) * 4];\n", "            D_800A3488 = &D_8009B998[*(s16 *)(outer + 0x70)];\n")
    s = reads(s)
    s = fn(s, "func_800620B8", b620B8)
    s = fn(s, "func_800678A8", b678A8)
    s = cursors(s)
    s = fn(s, "func_80068D88", b68D88)
    left = [l for l in s.split(NL) if re.search(r"\((s32|u16|TexRec)\s*\*?\)\s*&?D_800A348[8C]\b|\(s32\)&?D_8009B[89A]", l)]
    assert not left, left
    return s

FAKE_RT_OLD = "            D_800A348C = D_800A3488 = ((u32)D_800A32B8 % 6) * sizeof(*strip32) + (s32)strip32 - (s32)strip32 + (s32)strip32;\n"
RT = {
    # the round trip in pointer form: (strip32 + n) - strip32 is the ptrdiff n, + strip32 the record
    "rt_ptr": "            D_800A348C = D_800A3488 = strip32 + (u32)D_800A32B8 % 6 - strip32 + strip32;\n",
    "rt_idx": "            D_800A348C = D_800A3488 = &strip32[(u32)D_800A32B8 % 6] - strip32 + strip32;\n",
    "rt_cast": "            D_800A348C = D_800A3488 = (TexRec *)(((u32)D_800A32B8 % 6) * sizeof(*strip32) + (s32)strip32 - (s32)strip32 + (s32)strip32);\n",
    "rt_np": "            D_800A348C = D_800A3488 = (u32)D_800A32B8 % 6 + strip32 - strip32 + strip32;\n",
    "rt_none": "            D_800A348C = D_800A3488 = &strip32[(u32)D_800A32B8 % 6];\n",
}

def b620B8(b):
    b = sub1(b, "    u16 (*strip32)[4]; /* FAKE: alias of D_8009BA00 */\n    u16 *alt32; /* FAKE: alias of D_8009BA50 */\n    u16 (*strip16)[4]; /* FAKE: alias of D_8009BA30 */\n    u16 *alt16; /* FAKE: alias of D_8009BA58 */\n",
             "    TexRec *strip32; /* FAKE: alias of D_8009BA00 */\n    TexRec *alt32; /* FAKE: alias of D_8009BA50 */\n    TexRec *strip16; /* FAKE: alias of D_8009BA30 */\n    TexRec *alt16; /* FAKE: alias of D_8009BA58 */\n")
    rt = [k for k in RT if k in OPT]
    b = sub1(b, FAKE_RT_OLD, RT[rt[0] if rt else RT_DEFAULT])
    if not rt:
        # abl_c.py: the pointer spellings put strip32 first in the addu (score 4); plain &strip32[n] 38
        b = sub1(b, "               the target has it. Without it: 35. */\n",
                 "               the target has it. Without it: 35. The sum is formed in integers and\n"
                 "               converted to TexRec *: in pointer arithmetic (`strip32 + n - strip32 +\n"
                 "               strip32`, `&strip32[n] - strip32 + strip32`) strip32 comes first in the\n"
                 "               addu (`addu v1,s8,a0` for the target's `addu a0,a0,s8`): 4; plain\n"
                 "               `&strip32[n]`: 38. */\n")
    if not [x for x in OPT if x.startswith("sb_")]:
        b = sub1(b, "        sel_b:\n", "        sel_b:\n"
                 "            /* FAKE: the record address is an integer sum converted to TexRec *: as\n"
                 "               `strip16 + n` or `&strip16[n]` strip16 comes first in the addu (`addu\n"
                 "               v0,t0,v0` for the target's `addu v0,v0,t0`): score 1. */\n")
    sb = {"sb_idx": "&strip16[D_800A32B8 & 3]", "sb_ptr": "strip16 + (D_800A32B8 & 3)", "sb_cast": "(TexRec *)((D_800A32B8 & 3) * sizeof(*strip16) + (s32)strip16)", "sb_u": "strip16 + ((u32)D_800A32B8 & 3)", "sb_np": "(D_800A32B8 & 3) + strip16"}
    k = [x for x in sb if x in OPT]
    b = sub1(b, "            D_800A348C = D_800A3488 = (D_800A32B8 & 3) * sizeof(*strip16) + (s32)strip16;\n",
             "            D_800A348C = D_800A3488 = %s;\n" % sb[k[0] if k else "sb_cast"])
    b = sub1(b, "                D_800A348C = (s32)alt32;\n", "                D_800A348C = alt32;\n")
    b = sub1(b, "                D_800A348C = (s32)alt16;\n", "                D_800A348C = alt16;\n")
    # `flag` is RotTransPers' third argument, PsyQ's `long *p` (the depth-cue interpolation value;
    # func_80063084 calls the same slot `interp`); the flag is D_800A34CC
    b = re.sub(r"\bflag\b", "interp", b)
    return b
RT_DEFAULT = "rt_cast"

def b678A8(b):
    if "tblkeep" in OPT:
        b = sub1(b, "    u16 *tbl;\n", "    TexRec *tbl;\n")
        b = sub1(b, "    tbl = (u16 *)D_800A3488;\n", "    tbl = D_800A3488;\n")
        b = sub1(b, "(((tbl[0] >> 4) & 0x3F) + (tbl[1] << 6))", "(((tbl->clut_x >> 4) & 0x3F) + (tbl->clut_y << 6))")
    else:   # the alias goes (no FAKE label at HEAD; measured)
        b = sub1(b, "    u16 *tbl;\n", "")
        b = sub1(b, "    tbl = (u16 *)D_800A3488;\n", "")
        b = sub1(b, "(((tbl[0] >> 4) & 0x3F) + (tbl[1] << 6))", "(((D_800A3488->clut_x >> 4) & 0x3F) + (D_800A3488->clut_y << 6))")
    return b

def cursors(s):
    # D_800A34E8: the tag word of the POLY_FT4 it is aimed at (every store is a POLY_FT4)
    s = sub1(s, "static s32 D_800A34E8;\n", "static u32 *D_800A34E8;\n")
    s = sub1(s, "    D_800A34E8 = 0x1F8000A4;\n", "    D_800A34E8 = &SPAD51268->unkA4;\n")
    n = s.count("D_800A34E8 = (s32)prim;")
    assert n == 4, n
    s = s.replace("D_800A34E8 = (s32)prim;", "D_800A34E8 = &prim->tag;")
    n = s.count("*(u32 *)D_800A34E8 = (*(u32 *)D_800A34E8 & 0xFF000000)")
    assert n == 3, n
    s = s.replace("*(u32 *)D_800A34E8 = (*(u32 *)D_800A34E8 & 0xFF000000)", "*D_800A34E8 = (*D_800A34E8 & 0xFF000000)")
    n = s.count("(D_800A34E8 & 0xFFFFFF)")
    assert n == 5, n
    s = s.replace("(D_800A34E8 & 0xFFFFFF)", "((u32)D_800A34E8 & 0xFFFFFF)")
    if "ot32" in OPT:
        s = sub1(s, "static u8 *D_800A34E4;\n", "static u32 *D_800A34E4;\n")
        s = sub1(s, "    D_800A34E4 = (u8 *)0x1F8000A0;\n", "    D_800A34E4 = &SPAD51268->unkA0;\n")
        n = s.count("*(u32 *)D_800A34E4")
        assert n == 12, n
        s = s.replace("*(u32 *)D_800A34E4", "*D_800A34E4")
        for a in ("g_gpu_ot_ptr + *z * 4;", "g_gpu_ot_ptr + zbuf[k] * 4;", "g_gpu_ot_ptr + *zbuf * 4;"):
            s = s.replace("D_800A34E4 = " + a, "D_800A34E4 = (u32 *)(" + a[:-1] + ");")
    else:
        s = sub1(s, "    D_800A34E4 = (u8 *)0x1F8000A0;\n", "    D_800A34E4 = SPAD51268->unkA0;\n")
    return s

def b68D88(b):
    b = sub1(b, "    extern u8 *g_gpu_ot_ptr;\n", "")   # bb2.h declares it
    b = sub1(b, "                p_b = (s32 *)*p_cur;\n                D_800A34E8 = (s32)p_b;\n",
             "                p_b = (u32 *)*p_cur;\n                D_800A34E8 = p_b;\n")
    b = sub1(b, "            s32 *p_b;\n", "            u32 *p_b;\n")
    if "ot32" in OPT:
        b = sub1(b, "                D_800A34E4 = (u8 *)p_a;\n", "                D_800A34E4 = (u32 *)p_a;\n")
        b = sub1(b, "                    s32 *p_a2 = (s32 *)D_800A34E4;\n", "                    u32 *p_a2 = D_800A34E4;\n")
    # names: strength_red is the quad count; var_t3 the return value; p_matrix (+0x8C) the per-quad
    # depth (OT slot index) table, which the TU's other drawers call zbuf (rev-f01c: p_ot was false);
    # p_prev (+0x7C) the end of the quads
    b = sub1(b, "    s32 strength_red;\n    s32 var_t3;\n", "    s32 count;\n    s32 ret;\n")
    if "cnt_raw" in OPT:
        b = sub1(b, "    strength_red = -((cur_init - prev_init) * 0x33333333) >> 3;\n", "    count = -((cur_init - prev_init) * 0x33333333) >> 3;\n")
    else:
        b = sub1(b, "    strength_red = -((cur_init - prev_init) * 0x33333333) >> 3;\n", "    count = (POLY_FT4 *)cur_init - (POLY_FT4 *)prev_init;\n")
    b = sub1(b, "    if (strength_red != 0) {\n", "    if (count != 0) {\n")
    b = re.sub(r"\bvar_t3\b", "ret", b)
    b = re.sub(r"\bp_matrix\b", "zbuf", b)
    if "pot_view" not in OPT:   # the table holds u16 depths (lhu): type the holder, drop the view
        b = sub1(b, "    s16 *zbuf = (s16 *)(outer + 0x8C);\n", "    u16 *zbuf = (u16 *)(outer + 0x8C);\n")
        b = sub1(b, "                u32 entry = *(u16 *)((s32)zbuf + idx_s * 2);\n", "                u32 entry = zbuf[idx_s];\n")
    b = re.sub(r"\bp_prev\b", "p_end", b)
    if "voids" not in OPT:
        b = sub1(b, "    (void)arg0; (void)arg1;\n", "")
    if "idx_u16" in OPT:
        b = sub1(b, "    s16 *p_idx = (s16 *)(outer + 0x6E);\n", "    u16 *p_idx = (u16 *)(outer + 0x6E);\n")
        b = sub1(b, "                *(u16 *)p_idx = *(u16 *)p_idx + 1;\n", "                *p_idx = *p_idx + 1;\n")
    elif "idx_view" not in OPT:   # the u16 view of the s16 index goes: `(*p_idx)++` is IDENTICAL
        b = sub1(b, "                *(u16 *)p_idx = *(u16 *)p_idx + 1;\n", "                (*p_idx)++;\n")
    return b

def write():
    d = OUT + ("opt/" if OPT else "")
    os.makedirs(d, exist_ok=True)
    s0, g0, h0 = base()
    s = src(s0); g = game(g0, s); h = h0
    for n, t in (("51268.c", s), ("game.h", g), ("bb2.h", h)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(t)
    return s, g, h

def measure(s, g, h, fs=()):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    for n, t in (("include/game.h", g), ("include/bb2.h", h), ("src/main/51268.c", s)):
        open("tmp/p2/wk/" + n, "w", encoding="utf-8", newline=NL).write(t)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268"] + list(fs), capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/51268"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    s, g, h = write()
    if "measure" in sys.argv[1:]:
        measure(s, g, h, [a for a in sys.argv[1:] if a.startswith("func_")])
    print("wrote c", sorted(OPT))
