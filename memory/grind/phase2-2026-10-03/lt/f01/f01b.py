#!/usr/bin/env python3
# F01 batch b1 (on HEAD 7b5212f70): the command block at 0x1F800000 (func_80060A68's local struct Ob)
# becomes game.h's Unk1F800000Unk00, Unk1F800000Rec.unk00's type and D_800F116C's; D_800A3468 /
# D_800A346C / D_800A3470 and func_80060E38's `*(T **)0x1F800004 / 8` seeds are typed. D_800A3478 /
# D_800A347C are batch b2.
# usage: f01b.py [measure] [opt=<name>,...]   (writes tmp/p2/lt/f01/v/{51268.b.c,game.h.b,bb2.h.b})
import os, re, shutil, subprocess, sys
NL = chr(10)
H = "tmp/p2/lt/f01/b/"
HEAD = "7b5212f70"
for _f, _p in (("51268.head.c", "src/main/51268.c"), ("game.head.h", "include/game.h"), ("bb2.head.h", "include/bb2.h")):
    if not os.path.exists(H + _f):   # the base copies: HEAD's files, LF
        os.makedirs(H, exist_ok=True)
        open(H + _f, "wb").write(subprocess.run(["git", "show", "%s:%s" % (HEAD, _p)], capture_output=True, check=True).stdout)
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))

def rd(p):
    return open(p, encoding="utf-8").read()

def sub1(s, a, b, n=1):
    assert s.count(a) == n, (a[:90], s.count(a), n)
    return s.replace(a, b)

# ---------------------------------------------------------------- game.h
TYPE = """/* The 0x2C-byte block D_800A3468 points at (51268.c): Unk1F800000Rec.unk00 (func_80060E38's seed)
 * or D_800F116C, where func_800611A4 .. func_80061EC0 point it before calling func_80060A68.
 * D_800F1198 follows D_800F116C, so that copy ends at +0x2C. Each copy sets one pointer pair:
 * - unk00: one word, stored whole and read whole (bit 21 in func_80060A68, bits 17-18 / 19-20 in
 *   func_80063AF0 / func_80063B34 / func_80065000); func_80060A68 / func_80060B70 also read its
 *   low halfword (`lhu`), their D_800F10D0 / D_8009BA60 index.
 * - unk04 / unk08: the three halfwords / three words func_80060B70 and func_800620B8 copy into the
 *   scratchpad block's unk18 / unk20 (through D_800A346C / D_800A3470). Only the scratchpad block's
 *   are set: func_80060E38 stores its two arguments there.
 * - unk0C / unk10: the three words / three halfwords func_80060A68 copies into unk20 / unk18. Set
 *   only in the D_800F116C block (through D_800A3468, or D_800F1178 / D_800F117C, separate symbols
 *   at its +0x0C / +0x10).
 * - unk14: the byte func_80060A68 / func_80060B70 store the called function's result to (`sb`);
 *   D_800F1180 is the D_800F116C block's +0x14.
 * - unk1E: no access. */
typedef struct {
    union {
        s32 w;
        u16 h;
    } unk00;
    s16 *unk04;
    s32 *unk08;
    s32 *unk0C;
    s16 *unk10;
    u8 *unk14;
    u16 unk18[3];
    u8 unk1E[2];
    s32 unk20[3];
} Unk1F800000Unk00;

"""

def game(g):
    i = g.index("/* 51268's view of the scratchpad from 0x1F800000")
    g = g[:i] + TYPE + g[i:]
    g = sub1(g, """ * - unk00: the command block D_800A3468 points at until a function retargets it (func_80060A68's
 *   Ob layout; D_800A346C / D_800A3470 point at its +0x18 / +0x20), with +0x2C..+0x2F. Typed with
 *   D_800A3468 in a later batch.
""", """ * - unk00: the command block D_800A3468 points at until a function retargets it; D_800A346C /
 *   D_800A3470 point at its unk18 / unk20.
 * - unk2C: no access.
""")
    g = sub1(g, """ * members (51268.c); apart from its own two raw stores to unk00's +0x04 / +0x08, the code reaches
 * them only through those globals:
""", """ * members (51268.c); apart from its own two stores to unk00.unk04 / unk08, the code reaches them only
 * through those globals:
""")
    g = sub1(g, "    u8 unk00[0x30];\n    MATRIX unk30;", "    Unk1F800000Unk00 unk00;\n    u8 unk2C[4];\n    MATRIX unk30;")
    # func_800620B8's u16 reads moved up with the shorter bodies above it (HEAD :989-991)
    g = sub1(g, "so its u16 reads at 51268.c:989-991 go through", "so its u16 reads at 51268.c:953-955 go through")
    return g

def bb2(h):
    return sub1(h, "extern void func_80061FAC(u16 *, s32, MATRIX *);", "extern void func_80061FAC(u16 *, s32 *, MATRIX *);")

# ---------------------------------------------------------------- 51268.c
ASM = re.compile(r"__asm__\s*(?:__volatile__|volatile)?\s*\(.*?\);", re.S)

def src(s):
    parked = []
    def park(m):
        parked.append(m.group(0))
        return "@@ASM%d@@" % (len(parked) - 1)
    s = ASM.sub(park, s)
    s = _src(s)
    s = re.sub(r"@@ASM(\d+)@@", lambda m: parked[int(m.group(1))], s)
    return s

def span(s, sig):
    i = s.index(sig)
    j = s.index("\n}\n", i) + 3
    return i, j

def fn(s, sig, f):
    i, j = span(s, sig)
    return s[:i] + f(s[i:j]) + s[j:]

OLD_A68 = s_a68 = None

A68_OLD = """/* D_800A3468 holds a pointer to the current object (every store into it is an address: a
 * callee's returned pointer, the scratchpad base 0x1F800000, or &D_800F116C), so this function
 * reaches the object through the struct Ob view below.
 *   - +0x14 always receives a pointer to a byte buffer; this function stores one byte through it
 *     (`sb`), hence `s8 *p14`.
 *   - Offset 0 is written whole as one constant at its other sites (0x210009, 0x210005, 0x210010,
 *     0x210002, 0x210014): the low halfword is the character index loaded here with `lhu`, and
 *     bit 21 (0x200000) is the flag tested at the tail.  One word written whole and read at two
 *     widths is what the union at offset 0 declares.
 * The three tables are the arrays they are (24-entry flag table; per-index offset table;
 * per-character combo-id table), so every access is a member reference or an array subscript.
 *
 * Codegen note: the repeated `lw ?,0x10($v1)` loads and the object-pointer reloads after the call
 * come from cse (each store through the pointer invalidates its memory table,
 * tools/gcc-2.7.2/cse.c:1703-1719), not from the source.  Member references set MEM_IN_STRUCT_P,
 * which lets sched.c `true_dependence` (tools/gcc-2.7.2/sched.c:826-841) disambiguate the
 * offset-0 read from the scalar stores to 0x800A3478 / 0x800A347C; a bare-MEM read of offset 0
 * through an integer cast does not match. */
#define OB ((struct Ob *)D_800A3468)
void func_80060A68(void) {
    struct Ob {
        union { s32 w; u16 h; } id;
        s32 u04;
        s32 u08;
        s32 *p0C;
        u16 *p10;
        s8 *p14;
        u16 m18;
        u16 m1A;
        u16 m1C;
        u16 u1E;
        s32 m20;
        s32 m24;
        s32 m28;
    };
    extern s32 D_800A32BC;



    s32 result;

    D_800F10D0[OB->id.h] = 0;
    OB->m20 = OB->p0C[0];
    OB->m24 = OB->p0C[1];
    OB->m28 = OB->p0C[2];
    OB->m18 = OB->p10[0];
    OB->m1A = OB->p10[1];
    D_800A3478 = (s32)&OB->m18;
    OB->m1C = OB->p10[2];
    D_800A347C = (s32)&OB->m20;

    result = ((s32 (*)(void)) chractar_use_pset_combo_id_table[
                  D_8009BA60[OB->id.h]
                  + D_800F10D0[OB->id.h]])();
    *OB->p14 = result;

    if (OB->id.w & 0x200000) {
        D_800A32BC = 0xA;
    }
}
#undef OB
"""

A68_NEW = """/* func_80060A68 runs the command in the block D_800A3468 points at (Unk1F800000Unk00, game.h; each
 * caller first points it at D_800F116C). It copies the three words at unk0C and the three
 * halfwords at unk10 into unk20 / unk18 and points D_800A347C / D_800A3478 at those copies. With
 * idx the block's low halfword, it clears D_800F10D0[idx], calls the
 * chractar_use_pset_combo_id_table entry D_8009BA60[idx] + D_800F10D0[idx], stores the result
 * through unk14, and sets D_800A32BC to 0xA when bit 21 (0x200000) of the word is set.
 *
 * Codegen note: the repeated `lw ?,0xC($v1)` / `lw ?,0x10($v1)` loads and the block-pointer reloads
 * after the call and after the `sb` come from cse (each store through the pointer, and the call,
 * invalidates its memory table, tools/gcc-2.7.2/cse.c:1703-1719), not from the source.  Member
 * references set MEM_IN_STRUCT_P, which lets sched.c `true_dependence`
 * (tools/gcc-2.7.2/sched.c:826-841) move the second low-halfword read (the D_8009BA60 index) above
 * the scalar store to D_800A347C before it, as the target does; read through `*(u16 *)D_800A3468`
 * it stays below that store and does not match. */
void func_80060A68(void) {
    extern s32 D_800A32BC;



    s32 result;

    D_800F10D0[D_800A3468->unk00.h] = 0;
    D_800A3468->unk20[0] = D_800A3468->unk0C[0];
    D_800A3468->unk20[1] = D_800A3468->unk0C[1];
    D_800A3468->unk20[2] = D_800A3468->unk0C[2];
    D_800A3468->unk18[0] = D_800A3468->unk10[0];
    D_800A3468->unk18[1] = D_800A3468->unk10[1];
    D_800A3478 = (s32)D_800A3468->unk18;
    D_800A3468->unk18[2] = D_800A3468->unk10[2];
    D_800A347C = (s32)D_800A3468->unk20;

    result = ((s32 (*)(void)) chractar_use_pset_combo_id_table[
                  D_8009BA60[D_800A3468->unk00.h]
                  + D_800F10D0[D_800A3468->unk00.h]])();
    *D_800A3468->unk14 = result;

    if (D_800A3468->unk00.w & 0x200000) {
        D_800A32BC = 0xA;
    }
}
"""

B70_OLD = """    s32 outer;
    u16 *dst_u16;
    s32 dst_s32;
    u16 idx;
    s32 result;

    outer = D_800A3468;
    dst_u16 = (u16 *)D_800A346C;
    dst_u16[0] = *(u16 *)(*(s32 *)(outer + 4) + 0);
    dst_u16[1] = *(u16 *)(*(s32 *)(outer + 4) + 2);
    dst_u16[2] = *(u16 *)(*(s32 *)(outer + 4) + 4);

    dst_s32 = D_800A3470;
    *(s32 *)(dst_s32 + 0) = *(s32 *)(*(s32 *)(outer + 8) + 0);
    *(s32 *)(dst_s32 + 4) = *(s32 *)(*(s32 *)(outer + 8) + 4);
    {
        MATRIX *last_arg = D_800A3474;
        *(s32 *)(dst_s32 + 8) = *(s32 *)(*(s32 *)(outer + 8) + 8);
        func_80061FAC(dst_u16, dst_s32, last_arg);
    }

    idx = *(u16 *)D_800A3468;
    result = ((s32 (*)(void)) chractar_use_pset_combo_id_table[D_8009BA60[idx] + D_800F10D0[idx]])();

    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;
"""
B70_NEW = """    Unk1F800000Unk00 *outer;
    u16 *dst_u16;
    s32 *dst_s32;
    u16 idx;
    s32 result;

    outer = D_800A3468;
    dst_u16 = D_800A346C;
    dst_u16[0] = outer->unk04[0];
    dst_u16[1] = outer->unk04[1];
    dst_u16[2] = outer->unk04[2];

    dst_s32 = D_800A3470;
    dst_s32[0] = outer->unk08[0];
    dst_s32[1] = outer->unk08[1];
    {
        MATRIX *last_arg = D_800A3474;
        dst_s32[2] = outer->unk08[2];
        func_80061FAC(dst_u16, dst_s32, last_arg);
    }

    idx = D_800A3468->unk00.h;
    result = ((s32 (*)(void)) chractar_use_pset_combo_id_table[D_8009BA60[idx] + D_800F10D0[idx]])();

    *D_800A3468->unk14 = result;
"""

# the 18 functions that point D_800A3468 at D_800F116C (func_800611A4 .. func_80061EC0)
CMD = ["func_800611A4", "func_80061250", "func_8006133C", "func_800613C8", "func_80061454",
       "func_800614E0", "func_8006156C", "func_80061658", "func_80061710", "func_800617C8",
       "func_800618B4", "func_800619A4", "func_800619F0", "func_80061A3C", "func_80061ACC",
       "func_80061C00", "func_80061D74", "func_80061EC0"]

def cmd_body(b):
    b = re.sub(r"s32 \*v1 = \(s32 \*\) ?\(?&D_800F116C\)?;", "Unk1F800000Unk00 *v1 = &D_800F116C;", b)
    b = re.sub(r"D_800A3468 = \(s32\) ?v1;", "D_800A3468 = v1;", b)
    b = b.replace("D_800A3468 = (s32)&D_800F116C;", "D_800A3468 = &D_800F116C;")
    b = re.sub(r"(?m)^(\s+)\*v1 = ([^;]+);", r"\1v1->unk00.w = \2;", b)
    # +0x14 stores: the value is a u8 array address (the (s32) on it goes with the int view)
    b = re.sub(r"\*\(s32 \*\)\(\(s32\)D_800A3468 \+ 0x14\) = \(s32\)\(([^;]+)\);", r"D_800A3468->unk14 = \1;", b)
    b = re.sub(r"\*\(s32 \*\)\(D_800A3468 \+ 0x14\) = \(s32\)\(([^;]+)\);", r"D_800A3468->unk14 = \1;", b)
    b = re.sub(r"\*\(s32 \*\)\(\(s32\)D_800A3468 \+ 0x14\) = \(s32\)([^;]+);", r"D_800A3468->unk14 = \1;", b)
    b = re.sub(r"\*\(s32 \*\)\(D_800A3468 \+ 0x14\) = \(s32\)([^;]+);", r"D_800A3468->unk14 = \1;", b)
    b = re.sub(r"\*\(s32 \*\)D_800A3468 = ([^;]+);", r"D_800A3468->unk00.w = \1;", b)
    b = b.replace("*(s32 *)(D_800A3468 + 0xC) = arg0;", "D_800A3468->unk0C = arg0;")
    b = b.replace("*(s32 *)(D_800A3468 + 0x10) = (s32)&sp10;", "D_800A3468->unk10 = &sp10.vx;")
    if "/* FAKE: local pointer alias to D_800F116C, mechanism" not in b and "Unk1F800000Unk00 *v1" in b:
        # the &D_800F116C alias is not needed typed (abl_b.py: IDENTICAL without); func_800618B4's
        # FAKE alias (its comment: the direct form scores 16) is IDENTICAL without it too
        if "/* FAKE: local pointer alias to D_800F116C (as in the siblings" in b:
            a = b.index("    /* FAKE: local pointer alias to D_800F116C (as in the siblings")
            b = b[:a] + b[b.index("*/\n", a) + 3:]
        b = sub1(b, "    Unk1F800000Unk00 *v1 = &D_800F116C;\n", "")
        b = sub1(b, "D_800A3468 = v1;", "D_800A3468 = &D_800F116C;")
        b = re.sub(r"\bv1->unk00\.w = ", "D_800F116C.unk00.w = ", b)
        assert not re.search(r"\bv1\b", b)
    return b

FAKE_B34 = """    /* FAKE: the word is read through an s32 * to it, not as D_800A3468->unk00.w: a member read
     * is MEM_IN_STRUCT_P, so sched.c true_dependence lets it rise above the store to D_800F10D4,
     * which then fills its load delay (score 6, 16 insns; the target reads it after that store and
     * waits with a nop, 17). */
"""
FAKE_65000 = """    /* FAKE: the word is read through an s32 * to it, not as a member (D_800A3468->unk00.w, directly
     * or through a block-pointer local): a member read is MEM_IN_STRUCT_P, so sched.c
     * true_dependence lets it rise above the store to D_800F10FC, which then fills its load delay
     * (score 9 / 8, 23 / 22 insns; the target reads it after that store and waits with a nop, 23). */
"""

def _src(s):
    s = sub1(s, "static s32 D_800A3468;\n", "static Unk1F800000Unk00 *D_800A3468;\n")
    s = sub1(s, "static s32 D_800A346C;\n", "static u16 *D_800A346C;\n")
    s = sub1(s, "static s32 D_800A3470;\n", "static s32 *D_800A3470;\n")
    s = sub1(s, A68_OLD, A68_NEW)
    s = sub1(s, B70_OLD, B70_NEW)
    # func_80060E38's seeds
    s = sub1(s, "    D_800A3468 = 0x1F800000;\n    D_800A346C = 0x1F800018;\n    D_800A3470 = 0x1F800020;\n",
             "    D_800A3468 = &SPAD51268->unk00;\n    D_800A346C = SPAD51268->unk00.unk18;\n    D_800A3470 = SPAD51268->unk00.unk20;\n")
    s = sub1(s, "    *(s16 **)0x1F800004 = arg0;\n    *(s32 **)0x1F800008 = arg1;\n",
             "    SPAD51268->unk00.unk04 = arg0;\n    SPAD51268->unk00.unk08 = arg1;\n")
    # func_80061064
    s = sub1(s, "        *(s32 **)((s32)D_800A3468 + 0x14) = (s32 *)(i + (s32)&D_800F1150);\n",
             "        D_800A3468->unk14 = &D_800F1150[i];\n")
    s = sub1(s, "            *(s32 *)D_800A3468 = i;\n", "            D_800A3468->unk00.w = i;\n")
    # D_800F116C is the block
    s = sub1(s, "extern s32 D_800F116C;\n", "extern Unk1F800000Unk00 D_800F116C;\n")
    for f in CMD:
        s = fn(s, "\nvoid %s(" % f if f not in ("func_8006133C", "func_800613C8", "func_80061454", "func_800614E0") else "\ns32 %s(" % f, cmd_body)
    # the int parameter both store into unk0C is the caller's s32 * (17AFC passes the arg2 it passes
    # func_800617C8 (s32 *); its call is implicit, so the definition alone changes)
    s = sub1(s, "void func_80061C00(s32 arg0, s32 arg1, s32 arg2) {", "void func_80061C00(s32 *arg0, s32 arg1, s32 arg2) {")
    s = sub1(s, "void func_80061D74(s32 arg0, s16 arg1) {", "void func_80061D74(s32 *arg0, s16 arg1) {")
    # func_80061FAC's unused a1: both callers pass the s32 copy (D_800A3470)
    s = sub1(s, "void func_80061FAC(u16 *a0, s32 a1, MATRIX *a2) {", "void func_80061FAC(u16 *a0, s32 *a1, MATRIX *a2) {")
    # func_800620B8
    s = sub1(s, """    s32 outer;
    u16 *dst16;""", """    Unk1F800000Unk00 *outer;
    u16 *dst16;""")
    s = sub1(s, "    dst16 = (u16 *)D_800A346C;\n", "    dst16 = D_800A346C;\n")
    for k in range(3):
        s = sub1(s, "dst16[%d] = (*(u16 **)(outer + 4))[%d];" % (k, k), "dst16[%d] = outer->unk04[%d];" % (k, k))
        s = sub1(s, "dst32[%d] = (*(s32 **)(outer + 8))[%d];" % (k, k), "dst32[%d] = outer->unk08[%d];" % (k, k))
    s = sub1(s, "    dst32 = (s32 *)D_800A3470;\n", "    dst32 = D_800A3470;\n")
    s = sub1(s, "    func_80061FAC(dst16, (s32)dst32, rot);\n", "    func_80061FAC(dst16, dst32, rot);\n")
    # func_80063AF0 reads the word as a member (its `v1` alias goes: IDENTICAL without, and with it
    # retyped); func_80063B34 / func_80065000 read it through an s32 * to it (FAKE: a member read moves)
    s = sub1(s, "    s32 *v1 = (s32 *)D_800A3468;\n    D_800F10D0[0] = 1;\n    D_800A345C[0] = (*v1 >> 17) & 3;\n",
             "    D_800F10D0[0] = 1;\n    D_800A345C[0] = (D_800A3468->unk00.w >> 17) & 3;\n")
    s = sub1(s, "    s32 *v1 = (s32 *)D_800A3468;\n    D_800F10D4 = 1;\n",
             FAKE_B34 + "    s32 *v1 = &D_800A3468->unk00.w;\n    D_800F10D4 = 1;\n")
    s = sub1(s, "    void *q = D_800A3468;\n", FAKE_65000 + "    s32 *q = &D_800A3468->unk00.w;\n")
    s = sub1(s, "D_800A3440 = (*(s32 *)q >> 19) & 3;", "D_800A3440 = (*q >> 19) & 3;")
    # D_800A3470 readers
    n = s.count("((s32 *)D_800A3470)[")
    assert n == 22, n
    s = s.replace("((s32 *)D_800A3470)[", "D_800A3470[")
    for g in ("D_800A3468", "D_800A346C", "D_800A3470"):
        left = [l for l in s.split(NL) if g in l and re.search(r"\(\s*(s32|u16|s16|u8|s8|void)\s*\**\s*\)\s*\(?\s*\(?\s*(s32\)\s*)?" + g, l)]
        # expected: the comment's measured claim; the (s32) to the still-int D_800A3478 / 347C (b2)
        left = [l for l in left if not l.startswith(" * ") and not re.match(r"    D_800A347[8C] = \(s32\)D_800A3468->unk(18|20);", l)]
        assert not left, left
    s = sub1(s, "#define SPAD51268 ((Unk1F800000Rec *)0x1F800000)\nvoid func_80060E38", "#define SPAD51268 ((Unk1F800000Rec *)0x1F800000)\nvoid func_80060E38")
    return opts(s)

def opts(s):
    # measurement variants (not part of the batch)
    if "a68raw" in OPT:   # the comment's claim: the second idx read through an integer cast
        s = sub1(s, "                  D_8009BA60[D_800A3468->unk00.h]\n", "                  D_8009BA60[*(u16 *)D_800A3468]\n")
    if "a68raw2" in OPT:
        s = sub1(s, "                  + D_800F10D0[D_800A3468->unk00.h]])();\n", "                  + D_800F10D0[*(u16 *)D_800A3468]])();\n")
    if "a68raw0" in OPT:
        s = sub1(s, "    D_800F10D0[D_800A3468->unk00.h] = 0;\n", "    D_800F10D0[*(u16 *)D_800A3468] = 0;\n")
    if "af0v1" in OPT:    # func_80063AF0 with HEAD's v1, retyped
        s = sub1(s, "    D_800F10D0[0] = 1;\n    D_800A345C[0] = (D_800A3468->unk00.w >> 17) & 3;\n",
                 "    Unk1F800000Unk00 *v1 = D_800A3468;\n    D_800F10D0[0] = 1;\n    D_800A345C[0] = (v1->unk00.w >> 17) & 3;\n")
    if "p1150" in OPT:
        s = sub1(s, "D_800A3468->unk14 = &D_800F1150[i];", "D_800A3468->unk14 = D_800F1150 + i;")
    return s

def write(d="tmp/p2/lt/f01/v"):
    if OPT:   # a measurement variant never overwrites the batch files
        d = "tmp/p2/lt/f01/v/opt"
    os.makedirs(d, exist_ok=True)
    s = src(rd(H + "51268.head.c")); g = game(rd(H + "game.head.h")); h = bb2(rd(H + "bb2.head.h"))
    open(d + "/51268.b.c", "w", encoding="utf-8", newline=NL).write(s)
    open(d + "/game.h.b", "w", encoding="utf-8", newline=NL).write(g)
    open(d + "/bb2.h.b", "w", encoding="utf-8", newline=NL).write(h)
    return s, g, h

def wk(s, g, h):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    open("tmp/p2/wk/include/bb2.h", "w", encoding="utf-8", newline=NL).write(h)
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/51268.c", "w", encoding="utf-8", newline=NL).write(s)

def measure(s, g, h, fs=()):
    wk(s, g, h)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268"] + list(fs), capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/51268"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    s, g, h = write()
    if "measure" in sys.argv[1:]:
        measure(s, g, h, [a for a in sys.argv[1:] if a.startswith("func_")])
    print("wrote b1", sorted(OPT))
