#!/usr/bin/env python3
# Worker-2 batch 3: the per-player model object (F08 309CC, F10 func_80040D48, F05's `vehicle`, and every
# other user): one game.h aggregate Unk80045878Obj (0x1A88 bytes, built by func_80045878) with its 0x68-byte
# nodes Unk80045878Node; g_player_ptrs / func_8004153C / func_80045878 return it.
# Chained on w2b2 (lt/f09/w2b2.py, applied in memory on its base).
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f08/w2b3.py [opt=...] [out=DIR | apply]
#   default out=tmp/w2/b3 (scratch copies); `apply` writes the working tree.
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
OPT = {"sc_00f8b"}   # the landed spellings; `opt=` replaces the set
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, span, body, fn = B2.rep, B2.span, B2.body, B2.fn
BASE = "12e8c26bd"   # main with batches 1 and 2 and the descriptor batches (helpers only come from w2b2)


def base(p):
    B2.BASE = BASE
    return B2.show(p)


# ------------------------------------------------------------------ game.h / bb2.h
TYPES = """
/* The per-player model object func_80045878 builds (0x1A88 bytes, its func_80045600 block) and
 * g_player_ptrs[] / func_8004153C hand out. The header (0x00..0x2B) is followed by three arrays of
 * 0x68-byte transform nodes (func_80041430 rebases them as 0x15, 0x14 and 0x14 records), then
 * three 20-entry pointer tables.
 * - unk_00: flag word; func_80040594 sets bit 1 and keeps the character id in bits 16..20,
 *   which player_SetCharId / func_80041650 read as the upper halfword (half[1] & 0x1F).
 * - unk_04: the player index (func_80045878's a0); unk_06: a state func_80040594 clears
 *   (func_80045878 sets 1, player_SetCharId -2); unk_08: func_80045878's a1, indexing the
 *   D_80094C68 / D_80094B48 tables.
 * - unk_10 / unk_14 / unk_16: func_80045878's a0 / a0 / a0 + 3 (unk_14 / unk_16 are the
 *   func_80044010 / func_800432A0 ids); unk_12: the Q12 scale func_800408F8 sets from D_80094C68.
 * - unk_18: written as the word 0x8000 by func_80045878, read as three colour bytes by
 *   func_80041688.
 * - unk_1C / unk_20: the resource blocks func_80045878 stores; unk_24 / unk_28: the scene block
 *   and command cursor (func_8003F824, func_8004019C).
 * - unk_2C[]: node 0 is the root (func_80049718 reads its xf.mat); func_800408F8 builds 0..20,
 *   func_80040CB8 fills unk_8B4[] (unk2 = -1 ends it), func_80040B44 copies duplicates into
 *   unk_10D4[] (unk58 = the source node; 0 ends it).
 * - unk_18F4[] / unk_1994[]: &node.xf.mat of unk_2C[] nodes (func_80040A78); unk_1A34[]: the
 *   node each id selects (func_80040B44); unk_1A84: func_80040D48's arg5. */
typedef struct Unk80045878Node {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;              /* +0x58 */
    s32 unk5C;              /* +0x5C */
    u8 *unk60;              /* +0x60 func_8003FA24's point data */
    s32 unk64;              /* +0x64 */
} Unk80045878Node;          /* 0x68 */

typedef struct Unk80045878Obj {
    union {
        s32 word;
        s16 half[2];
    } unk_00;                        /* +0x0000 */
    s16 unk_04;                      /* +0x0004 */
    s16 unk_06;                      /* +0x0006 */
    s16 unk_08;                      /* +0x0008 */
    u8 pad0A[6];                     /* +0x000A */
    u16 unk_10;                      /* +0x0010 */
    s16 unk_12;                      /* +0x0012 */
    s16 unk_14;                      /* +0x0014 */
    s16 unk_16;                      /* +0x0016 */
    union {
        s32 word;
        u8 byte[4];
    } unk_18;                        /* +0x0018 */
    s32 unk_1C;                      /* +0x001C */
    s32 unk_20;                      /* +0x0020 */
    void *unk_24;                    /* +0x0024 */
    s32 unk_28;                      /* +0x0028 */
    Unk80045878Node unk_2C[21];      /* +0x002C */
    Unk80045878Node unk_8B4[20];     /* +0x08B4 */
    Unk80045878Node unk_10D4[20];    /* +0x10D4 */
    MATRIX *unk_18F4[20];            /* +0x18F4 */
    u8 pad1944[0x50];                /* +0x1944 */
    MATRIX *unk_1994[20];            /* +0x1994 */
    u8 pad19E4[0x50];                /* +0x19E4 */
    Unk80045878Node *unk_1A34[20];   /* +0x1A34 */
    s16 unk_1A84;                    /* +0x1A84 */
    u8 pad1A86[2];                   /* +0x1A86 */
} Unk80045878Obj;                    /* 0x1A88 */
"""


def game_h(s):
    a = "} Unk800A9CF8Entry;         /* 0x68 */\n"
    return rep(s, a, a + TYPES)


def bb2_h(s):
    s = rep(s, "extern s32 *g_player_ptrs[];\n", "extern Unk80045878Obj *g_player_ptrs[];\n")
    s = rep(s, "extern s32 *func_8004153C(s32);\n", "extern Unk80045878Obj *func_8004153C(s32);\n")
    s = rep(s, "extern s16 *func_80045878(s32, s32, s32);\n", "extern Unk80045878Obj *func_80045878(s32, s32, s32);\n")
    for a, c in (("void func_8003F62C(s32 *);", "void func_8003F62C(Unk80045878Obj *);"),
                 ("void func_8003F824(u8 *, s32);", "void func_8003F824(Unk80045878Obj *, s32);"),
                 ("void func_8003FFC4(s32 *);", "void func_8003FFC4(Unk80045878Obj *);"),
                 ("void func_800400B0(s32 *, s32);", "void func_800400B0(Unk80045878Obj *, s32);"),
                 ("void func_800400F8(s32 *);", "void func_800400F8(Unk80045878Obj *);"),
                 ("void func_8004019C(s32 *, s32);", "void func_8004019C(Unk80045878Obj *, s32);"),
                 ("void func_800404A0(s16 *, s32);", "void func_800404A0(Unk80045878Node *, s32);"),
                 ("s32 *func_80040510(s32, s32, s32);", "Unk80045878Obj *func_80040510(s32, s32, s32);"),
                 ("void func_80040A78(s32);", "void func_80040A78(Unk80045878Obj *);"),
                 ("void func_80041430(s32, s32);", "void func_80041430(s32, s32);\nextern void save_vc_ctrl(s32, Unk80045878Node *, s32);")):
        s = rep(s, "extern " + a + "\n", "extern " + c + "\n")
    return s


# ------------------------------------------------------------------ 31D3C.c
def c31d3c(s):
    s = rep(s, "s32 *func_8004153C(s32 a0) {\n", "Unk80045878Obj *func_8004153C(s32 a0) {\n")
    s = rep(s, """    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        return ptr[4];
    }""", """    Unk80045878Obj *ptr = g_player_ptrs[a0];
    if (ptr) {
        return ptr->unk_08;
    }""")
    s = rep(s, """    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        if ((val & 0x1F) != a1) {
            ptr[3] = -2;
        }""", """    Unk80045878Obj *ptr = g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr->unk_00.half[1];
        if ((val & 0x1F) != a1) {
            ptr->unk_06 = -2;
        }""")
    s = rep(s, """    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        return val & 0x1F;""", """    Unk80045878Obj *ptr = g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr->unk_00.half[1];
        return val & 0x1F;""")
    s = fn(s, "func_80041688", b_41688)
    # func_80041AC8: the object (func_80040594 passes it)
    s = rep(s, "void func_80041AC8(s16 *arg0)\n", "void func_80041AC8(Unk80045878Obj *arg0)\n")
    s = rep(s, "  if (arg0[2] != 1)\n", "  if (arg0->unk_04 != 1)\n")
    s = rep(s, "  if (D_80094E08[arg0[4]] == 0xFF)\n", "  if (D_80094E08[arg0->unk_08] == 0xFF)\n")
    s = rep(s, "  id_ptr = &arg0[4];\n  D_800A9A20 = arg0[4];\n", "  id_ptr = &arg0->unk_08;\n  D_800A9A20 = arg0->unk_08;\n")
    if "a_wh" in OPT:        # ablation: the rect size literals
        s = rep(s, "    s32 w = 0x10;\n    s32 h = 1;\n", "")
        s = rep(s, "      rect[2] = w;\n      rect[3] = h;\n", "      rect[2] = 0x10;\n      rect[3] = 1;\n")
    else:
        s = rep(s, "    s32 w = 0x10;\n    s32 h = 1;\n", "    /* FAKE: constant holders for the 16 x 1 rect size; the literals S_WH */\n"
                "    s32 w = 0x10;\n    s32 h = 1;\n")
    # func_80041BF4
    s = rep(s, "  s32 *fp_ptr;\n", "  Unk80045878Obj *fp_ptr;\n")
    s = s.replace("*(((s16 *) fp_ptr) + 4)", "fp_ptr->unk_08")
    return s


def b_41688(b):
    b = rep(b, "    s32 *player;\n    s32 i;\n    u8 *p;\n    u8 *q;\n",
            "    Unk80045878Obj *player;\n    s32 i;\n    Unk80045878Node *p;\n    Unk80045878Node *q;\n")
    b = rep(b, """    p = (u8 *)player + 0x94;
    if (arg1) {
        p[1] |= 1;
    } else {
        p[1] &= ~1;
    }
""", """    p = &player->unk_2C[1];
    if (arg1) {
        p->node.unk1 |= 1;
    } else {
        p->node.unk1 &= ~1;
    }
""")
    b = rep(b, "    p += 0x68;\n", "    p++;\n")
    b = rep(b, "    b = *(s16 *)(p + 2) >= 0;\n", "    b = p->node.unk2 >= 0;\n")
    b = rep(b, """        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;""", """        if (arg1) p->node.unk1 |= 1;
        else      p->node.unk1 &= ~1;""")
    if "q_raw" in OPT or "q_two" not in OPT and "q_node" not in OPT and "q_while" not in OPT:
        b = rep(b, "    Unk80045878Node *q;\n", "    u8 *q;\n")
    elif "q_two" in OPT:
        b = rep(b, "    Unk80045878Node *q;\n", "    Unk80045878Node *q;\n    u8 *f;\n")
        b = rep(b, """    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;""", """    q = player->unk_10D4;
    f = &q->node.unk1;
loop2:
    if (q->unk58 == 0) goto after2;
    if (arg1) *f |= 1;
    else      *f &= ~1;
    q++;
    f += sizeof(Unk80045878Node);
    goto loop2;""")
    elif "q_while" in OPT:
        b = rep(b, """    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:
""", """    for (q = player->unk_10D4; q->unk58 != 0; q++) {
        if (arg1) q->node.unk1 |= 1;
        else      q->node.unk1 &= ~1;
    }
""")
    else:
        b = rep(b, """    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;""", """    q = player->unk_10D4;
loop2:
    if (q->unk58 == 0) goto after2;
    if (arg1) q->node.unk1 |= 1;
    else      q->node.unk1 &= ~1;
    q++;
    goto loop2;""")
    b = b.replace("*((u8 *)player + 0x18)", "player->unk_18.byte[0]")
    b = b.replace("*((u8 *)player + 0x19)", "player->unk_18.byte[1]")
    b = b.replace("*((u8 *)player + 0x1A)", "player->unk_18.byte[2]")
    return b



# ------------------------------------------------------------------ 31548.c
B_40D48 = """\
void func_80040D48(s32 a0, s32 a1, s32 *a2, s16 *a3, s16 *arg4, s32 arg5) {
    Unk80045878Obj *s4;
    Unk80045878Node *s5;
    Unk80045878Node *s3;
    Unk80045878Node *s2;
    s32 s0;
    s16 *s1;
    Unk80045878Obj *ent;

    ent = g_player_ptrs[a0];
    if (ent == 0) {
        return;
    }
    /* FAKE: s4 copies ent; using ent directly scores 4. */
    s4 = ent;

    s4->unk_2C[0].node.xf.rot.vx = a3[0];
    s4->unk_2C[0].node.xf.rot.vy = a3[1];
    s4->unk_2C[0].node.xf.rot.vz = a3[2];

    s4->unk_2C[0].node.work.t[0] = a2[0];
    /* FAKE: s5 holds s4 + 0x2C here and the linked record pointer in the copy loop below;
     * a fresh loop local scores 35, dropping this first role 46. */
    s5 = s4->unk_2C;
    s4->unk_2C[0].node.work.t[1] = a2[1];
    s3 = &s4->unk_2C[1];
    s4->unk_2C[0].node.work.t[2] = a2[2];

    s2 = &s4->unk_2C[19];

    switch (a1) {
    case 0: {
        s32 *tbl;
        Unk80045878Node *p;
        /* FAKE: s0 is the counter of all four loops in this function; a counter per loop
         * scores 15. */
        s0 = 1;
        tbl = D_80094CFC;
        /* FAKE: s1 copies the parameter arg4; using arg4 directly scores 78. */
        s1 = arg4;

        s3->node.work.t[0] = 0;
        s3->node.work.t[1] = 0;
        s3->node.work.t[2] = 0;
        s3->node.xf.rot.vx = 0;
        s3->node.xf.rot.vy = 0;
        s3->node.xf.rot.vz = 0;

        do {
            s32 idx;
            Unk80045878Node *a4p;
            a4p = &s3[s0];
            idx = *tbl;
            a4p->node.xf.rot.vx = *(u16 *)((u8 *)s1 + idx * 6);
            idx = *tbl;
            a4p->node.xf.rot.vy = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 2);
            idx = *tbl;
            a4p->node.xf.rot.vz = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 4);
            s0++;
            tbl++;
        } while (s0 < 0x12);

        s0 = 0x11;
        p = &s3[17];
        do {
            p->node.unk6 = 0;
            s0--;
            p--;
        } while (s0 >= 0);

        s2->node.work.t[0] = *(s16 *)((u8 *)s1 + 0x6C);
        s2->node.work.t[1] = -(s32)*(s16 *)((u8 *)s1 + 0x6E);
        s2->node.work.t[2] = -(s32)*(s16 *)((u8 *)s1 + 0x70);
        s2->node.xf.rot.vx = *(u16 *)((u8 *)s1 + 0x72);
        s2->node.xf.rot.vy = -(s16)*(u16 *)((u8 *)s1 + 0x74);
        s2->node.xf.rot.vz = -(s16)*(u16 *)((u8 *)s1 + 0x76);

        g_anim_func_table[0](&s2->node.xf.rot, &s2->node.work);

        s2[1].node.work.t[0] = *(s16 *)((u8 *)s1 + 0x78);
        s2[1].node.work.t[1] = -(s32)*(s16 *)((u8 *)s1 + 0x7A);
        s2[1].node.work.t[2] = -(s32)*(s16 *)((u8 *)s1 + 0x7C);
        s2[1].node.xf.rot.vx = *(u16 *)((u8 *)s1 + 0x7E);
        s2[1].node.xf.rot.vy = -(s16)*(u16 *)((u8 *)s1 + 0x80);
        s2[1].node.xf.rot.vz = -(s16)*(u16 *)((u8 *)s1 + 0x82);

        g_anim_func_table[0](&s2[1].node.xf.rot, &s2[1].node.work);
        break;
    }
    case 1:
        s3->node.xf.rot.vx = 0;
        s3->node.xf.rot.vy = 0;
        s3->node.xf.rot.vz = 0;
        s3->node.unk6 = 0;
        s3->node.work.t[0] = 0;
        s3->node.work.t[2] = 0;
        break;
    case 2: break;
    case 3: break;
    case 4: break;
    case 5: break;
    case 6: break;
    }

    {
        s32 scaled;
        Unk80045878Node *s1p;
        scaled = (s3->node.work.t[1] * s4->unk_12) >> 12;
        s0 = 0;
        s1p = s3;
        s3->node.work.t[1] = scaled;
        s5->node.unk6 = 0;
        do {
            func_800417D0(&s1p->node);
            s0++;
            s1p++;
        } while (s0 < 0x12);
    }

    s0 = 1;
    s3->node.unk0 = 0xA;
    s3->unk58 = (s32)s4->unk_18F4;
    {
        s32 *list;
        Unk80045878Node *a4p;
        list = (s32 *)D_800A3820;
        a4p = &s3[1];
        s3->node.unk2 = 0;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)s3;

        do {
            if (a4p->node.unk2 >= 0) {
                s32 *list2;
                list2 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list2 + 1);
                *list2 = (s32)a4p;
            }
            s0++;
            a4p++;
        } while (s0 < 0x12);
    }

    {
        Unk80045878Node *a2p;
COPYLOOP
        a2p = s4->unk_8B4;
        if (s4->unk_8B4[0].node.unk2 != -1) {
            do {
                s32 *list4;
                list4 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list4 + 1);
                *list4 = (s32)a2p;
                a2p++;
            } while (a2p->node.unk2 != -1);
        }
    }

    func_800404A0(s4->unk_8B4, arg5);
    s4->unk_1A84 = arg5;
    func_800400B0(s4, arg5);
    func_8003F62C(s4);
    func_800420E8(a0, (s32)s3->node.xf.mat.t);
}
"""

COPY_40D48 = """        a2p = s4->unk_10D4;
        for (;;) {
            s32 *list3;
            s5 = (Unk80045878Node *)a2p->unk58;
            if (s5 == 0) {
                break;
            }
            a2p->node.xf.mat = s5->node.xf.mat;
            list3 = (s32 *)D_800A3820;
            D_800A3820 = (s32)(list3 + 1);
            *list3 = (s32)a2p;
            a2p++;
        }

"""
D48_58 = {
    "d48_a": "    s3->node.unk0 = 0xA;\n    s3->unk58 = (s32)s4->unk_18F4;\n",
    "d48_v1": "    s3->unk58 = (s32)s4->unk_18F4;\n    s3->node.unk0 = 0xA;\n",
    "d48_v2": "    s3->node.unk0 = 0xA;\n    /* FAKE: unk58 stored through a pointer; the member store lets sched sink it below the\n       D_800A3820 load (score S_P58D). */\n    {\n        s32 *p58 = &s3->unk58;\n        *p58 = (s32)s4->unk_18F4;\n    }\n",
}


def b_41188(b):
    b = rep(b, "    s32 base = (s32)g_player_ptrs[a0];\n", "    Unk80045878Obj *base = g_player_ptrs[a0];\n")
    b = rep(b, "    s32 ents;\n", "    Unk80045878Node *ents;\n")
    b = rep(b, "    s32 stptr2;\n    ents = base + 0x94;\n", "    Unk80045878Node *stptr2;\n    ents = &base->unk_2C[1];\n")
    b = rep(b, "func_800523E0(a4, out2, a3, (MATRIX *)(ents + i * 0x68 + 0x38));", "func_800523E0(a4, out2, a3, &ents[i].node.work);")
    b = rep(b, "        *((s16 *) (ents + i * 0x68 + 6)) = 2;\n", "        ents[i].node.unk6 = 2;\n")
    b = rep(b, "    stptr2 = ents + 0x750;\n", "    stptr2 = &ents[18];\n")
    b = rep(b, "func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);", "func_80044DE4((s16 *) a1, (s16 *) a2, a3, (s32)stptr2->node.work.t);")
    b = rep(b, "    func_800523E0(a4, out3, a3, (MATRIX *)(stptr2 + 0x38));\n    *((s16 *) (stptr2 + 6)) = 1;\n    stptr2 += 0x68;\n",
            "    func_800523E0(a4, out3, a3, &stptr2->node.work);\n    stptr2->node.unk6 = 1;\n    stptr2++;\n")
    return b


B_41430 = """void func_80041430(s32 a0, s32 a1) {
    Unk80045878Obj **base;
    Unk80045878Obj *s0;
    s32 i;
    base = &g_player_ptrs[a0];
    s0 = (Unk80045878Obj *)((u8 *)*base + a1);
    *base = s0;
    save_vc_ctrl(a1, s0->unk_2C, 0x15);
    save_vc_ctrl(a1, s0->unk_8B4, 0x14);
    save_vc_ctrl(a1, s0->unk_10D4, 0x14);
    {
        Unk80045878Node *c = s0->unk_10D4;
        do {
            s32 val = c->unk58;
            if (val) {
                c->unk58 = val + a1;
            } else {
                break;
            }
            c++;
        } while (1);
    }
    i = 0;
    do {
        Unk80045878Node *val = s0->unk_1A34[i];
        if (val) {
            s0->unk_1A34[i] = (Unk80045878Node *)((u8 *)val + a1);
        }
        i++;
    } while (i < 0x14);
    func_80040A78(s0);
}
"""


# ------------------------------------------------------------------ 35000.c
B_45878 = """Unk80045878Obj *func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    Unk80045878Obj *s1;
    s32 s0;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (Unk80045878Obj *) ((s32 *) v0)[1];
    } else {
        s1 = (Unk80045878Obj *) func_800455AC(a0);
        func_80045600(a0, sizeof(Unk80045878Obj) + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1->unk_08 = -1;
        s1->unk_06 = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8(s1);
    }
    if (((func_8004574C(s3) != 0) && (s1->unk_08 == a1)) && (s1->unk_06 != (-2))) {
        s1->unk_06 = 0;
    } else {
        s1->unk_20 = a2;
        s0 = (s32) func_800455AC(s3);
        s1->unk_1C = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1->unk_06 = 1;
        s1->unk_24 = 0;
        s1->unk_00.word = 0;
    }
    s1->unk_04 = a0;
    s1->unk_08 = a1;
    s1->unk_14 = a0;
    s1->unk_10 = a0;
    s1->unk_16 = a0 + 3;
    s1->unk_18.word = 0x8000;
    return s1;
}
"""


def c35000(s):
    s = body(s, "func_80045878", B_45878)
    s = rep(s, """    s32 *ptr;
    s32 idx;
    if (a0 < 3) {
        func_80041430(a0, a1);""", """    Unk80045878Obj *ptr;
    s32 idx;
    if (a0 < 3) {
        func_80041430(a0, a1);""")
    s = rep(s, """    ptr = (s32 *)func_800457A0(idx);
    if (ptr == 0) return;
    ptr[7] = ptr[7] + a1;
    func_8004019C(ptr, a1);
    if ((ptr[0] >> 1) & 1) {
        s32 val = *(s16 *)((u8 *)ptr + 4);""", """    ptr = (Unk80045878Obj *)func_800457A0(idx);
    if (ptr == 0) return;
    ptr->unk_1C = ptr->unk_1C + a1;
    func_8004019C(ptr, a1);
    if ((ptr->unk_00.word >> 1) & 1) {
        s32 val = ptr->unk_04;""")
    return s


# ------------------------------------------------------------------ 2B344.c (granted for these users)
def c2b344(s):
    # func_8003E164 / func_8003E22C
    s = rep(s, """    RECT buf;
    s32 *s0;

    if (D_800A3228 == arg0) {""", """    RECT buf;
    Unk80045878Obj *s0;

    if (D_800A3228 == arg0) {""")
    s = rep(s, "        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x140, 0xE8);\n",
            "        func_800432A0(s0->unk_14, 0, 0, -0x140, 0xE8);\n")
    s = rep(s, "        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x1C0, 0xE8);\n",
            "        func_800432A0(s0->unk_14, 0, 0, -0x1C0, 0xE8);\n")
    s = rep(s, """void func_8003E22C(void) {
    s32 *v1;
""", """void func_8003E22C(void) {
    Unk80045878Obj *v1;
""")
    s = rep(s, "                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x140, -0xE8);\n",
            "                func_800432A0(v1->unk_14, 0, 0, 0x140, -0xE8);\n")
    s = rep(s, "                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x1C0, -0xE8);\n",
            "                func_800432A0(v1->unk_14, 0, 0, 0x1C0, -0xE8);\n")
    # func_8003F62C
    s = rep(s, """void func_8003F62C(s32 *a0) {
    Scene *s0;
    s0 = (Scene *)a0[9];
    if (s0 == 0) return;
    if (s0->unk6) {
        func_8004016C(*(s16 *)((u8 *)a0 + 4));
        func_8003F824((u8 *)a0, 0);
    }""", """void func_8003F62C(Unk80045878Obj *a0) {
    Scene *s0;
    s0 = a0->unk_24;
    if (s0 == 0) return;
    if (s0->unk6) {
        func_8004016C(a0->unk_04);
        func_8003F824(a0, 0);
    }""")
    # SceneRec.obj: the node the object's id table selects
    s = rep(s, "    /* 0x14 */ u8 *obj;\n", "    /* 0x14 */ Unk80045878Node *obj;\n")
    s = rep(s, "void func_8003FECC(u8 *a0, SceneRec *rec, s16 *a2);\n", "void func_8003FECC(Unk80045878Obj *a0, SceneRec *rec, s16 *a2);\n")
    # Func8003F6D8Inner.objs: the nodes func_8003FECC records (func_8003F6D8 reads their matrix)
    if "objs_s32" not in OPT:
        s = rep(s, "    /* 0x04 */ s32 *objs[3];\n", "    /* 0x04 */ Unk80045878Node *objs[3];\n")
        s = rep(s, "extern void gte_SetMatrixRotTransIR(s32 *, SVECTOR *, s16 *);\n",
                "extern void gte_SetMatrixRotTransIR(MATRIX *, SVECTOR *, s16 *);\n")
        s = rep(s, "            s32 *obj = in->objs[j] + 6;\n", "            MATRIX *obj = &in->objs[j]->node.xf.mat;\n")
    # func_8003F824
    s = rep(s, """void func_8003F824(u8 *arg0, s32 arg1) {
    Scene *sc;
    s16 *cmds;
    u8 *cur;
    SceneRec *rec;
    u8 *obj;""", """void func_8003F824(Unk80045878Obj *arg0, s32 arg1) {
    Scene *sc;
    s16 *cmds;
    u8 *cur;
    SceneRec *rec;
    Unk80045878Node *obj;""")
    s = rep(s, """    sc = *(Scene **)(arg0 + 0x24);
    if (sc == 0) return;
    cmds = *(s16 **)(arg0 + 0x28);
    cur = sc->data;
    if (*cmds == -3) {
        *(Scene **)(arg0 + 0x24) = 0;""", """    sc = arg0->unk_24;
    if (sc == 0) return;
    cmds = (s16 *)arg0->unk_28;
    cur = sc->data;
    if (*cmds == -3) {
        arg0->unk_24 = 0;""")
    s = rep(s, "            obj = ((u8 **)(arg0 + 0x1A34))[i];\n", "            obj = arg0->unk_1A34[i];\n")
    s = rep(s, "            *obj = 0xD;\n", "            obj->node.unk0 = 0xD;\n")
    s = rep(s, "        func_80045A28(*(s16 *)(arg0 + 4), cur - *(u8 **)(arg0 + 0x1C));\n",
            "        func_80045A28(arg0->unk_04, cur - (u8 *)arg0->unk_1C);\n")
    # func_8003FA24: its record's node
    s = rep(s, "    u8 *obj;\n    u16 *src;\n", "    Unk80045878Node *obj;\n    u16 *src;\n")
    s = rep(s, "    src = (u16 *)D_80103608[*(s16 *)(obj + 4)][*(s16 *)(obj + 2)];\n",
            "    src = (u16 *)D_80103608[obj->node.unk4][obj->node.unk2];\n")
    s = rep(s, "    init.matrix = obj + 0x18;\n", "    init.matrix = &obj->node.xf.mat;\n")
    s = rep(s, "    *(u8 **)(obj + 0x60) = init.point_end;\n", "    obj->unk60 = init.point_end;\n")
    s = rep(s, "    rec->quad.unk4 = obj + 0x18;\n    rec->quad.unk8 = *(u8 **)(obj + 0x60);\n",
            "    rec->quad.unk4 = &obj->node.xf.mat;\n    rec->quad.unk8 = obj->unk60;\n")
    s = rep(s, "    /* 0x04 */ u8 *unk4;\n    /* 0x08 */ u8 *unk8;\n", "    /* 0x04 */ MATRIX *unk4;\n    /* 0x08 */ u8 *unk8;\n")
    # func_8003FECC: the object parameter
    s = rep(s, "void func_8003FECC(u8 *a0, SceneRec *rec, s16 *a2) {\n", "void func_8003FECC(Unk80045878Obj *a0, SceneRec *rec, s16 *a2) {\n")
    if "objs_s32" in OPT:
        s = rep(s, "            in->objs[n] = (s32 *)(a0 + 0x94 + id * 0x68);\n",
                "            in->objs[n] = (s32 *)&a0->unk_2C[id + 1];\n")
    else:
        s = rep(s, "            in->objs[n] = (s32 *)(a0 + 0x94 + id * 0x68);\n", "            in->objs[n] = &a0->unk_2C[id + 1];\n")
    # func_8003FFC4 .. func_8004019C: the scene block at unk_24
    s = rep(s, """void func_8003FFC4(s32 *a0) {
    s16 *v1 = (s16 *)a0[9];""", """void func_8003FFC4(Unk80045878Obj *a0) {
    s16 *v1 = a0->unk_24;""")
    s = rep(s, """    s32 *v0 = func_8004153C(a0);
    if (v0) {
        s16 *v1 = (s16 *)v0[9];""", """    Unk80045878Obj *v0 = func_8004153C(a0);
    if (v0) {
        s16 *v1 = v0->unk_24;""")
    s = rep(s, """void func_800400B0(s32 *a0, s32 a1) {
    s16 *v1 = (s16 *)a0[9];""", """void func_800400B0(Unk80045878Obj *a0, s32 a1) {
    s16 *v1 = a0->unk_24;""")
    s = rep(s, """void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];""", """void func_800400F8(Unk80045878Obj *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = a0->unk_24;""")
    s = rep(s, """void func_8004016C(s32 a0) {
    s32 *v0 = func_8004153C(a0);""", """void func_8004016C(s32 a0) {
    Unk80045878Obj *v0 = func_8004153C(a0);""")
    s = rep(s, """void func_8004019C(s32 *a0, s32 a1) {
    s32 *v1 = (s32 *)a0[9];
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0[9] = (s32)v1;
        a0[10] = a0[10] + a1;""", """void func_8004019C(Unk80045878Obj *a0, s32 a1) {
    s32 *v1 = a0->unk_24;
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0->unk_24 = v1;
        a0->unk_28 = a0->unk_28 + a1;""")
    # the scene block (worker 1's Scene) read by member, not through s16 * / s32 * holders
    if "sc_ffc4" not in OPT:
        s = rep(s, "    s16 *v1 = a0->unk_24;\n    if (v1) {\n        v1[3] = 1;\n",
                "    Scene *v1 = a0->unk_24;\n    if (v1) {\n        v1->unk6 = 1;\n")
    if "sc_ffe0" not in OPT:
        s = rep(s, "        s16 *v1 = v0->unk_24;\n        if (v1) {\n            v1[1] = 1;\n",
                "        Scene *v1 = v0->unk_24;\n        if (v1) {\n            v1->unk2 = 1;\n")
    if "sc_00b0" not in OPT:
        s = rep(s, """    s16 *v1 = a0->unk_24;
    if (v1) {
        s32 i;
        for (i = 0; i < v1[0]; i++) {
            *(s32 *)((u8 *)v1 + i * 0xD0 + 0x34) = a1;""", """    Scene *v1 = a0->unk_24;
    if (v1) {
        s32 i;
        for (i = 0; i < v1->count; i++) {
            v1->recs[i].inner.unk10 = a1;""")
    if "sc_019c" not in OPT:
        s = rep(s, """    s32 *v1 = a0->unk_24;
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0->unk_24 = v1;
        a0->unk_28 = a0->unk_28 + a1;
        *(s16 *)((s32)v1 + 6) = 1;""", """    Scene *v1 = a0->unk_24;
    if (v1) {
        v1 = (Scene *)((u8 *)v1 + a1);
        a0->unk_24 = v1;
        a0->unk_28 = a0->unk_28 + a1;
        v1->unk6 = 1;""")
    if "sc_00f8b" in OPT:
        s = rep(s, """    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = a0->unk_24;
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);""", """    Scene *s2;
    s32 s0;
    s2 = a0->unk_24;
    if (s2 != 0) {
        s0 = 0;
        if (s2->count > s0) {
            do {
                /* FAKE: the id converted to s16 (lh); unconverted it loads lhu (score 1) */
                obj_Clear((s16)s2->recs[s0].unk0);
                s0++;
            } while (s0 < s2->count);""")
        s = rep(s, "/* The variable compare `s2[0] > s0` is load-bearing", "/* The variable compare `s2->count > s0` is load-bearing")
    if "sc_00f8" in OPT:
        s = rep(s, """    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = a0->unk_24;
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);""", """    Scene *s2;
    SceneRec *s1;
    s32 s0;
    s2 = a0->unk_24;
    if (s2 != 0) {
        s0 = 0;
        if (s2->count > s0) {
            s1 = s2->recs;
            do {
                obj_Clear((s16)s1->unk0);
                s1++;
                s0++;
            } while (s0 < s2->count);""")
    return s


# ------------------------------------------------------------------ 3AB48.c (granted for these users)
def c3ab48(s):
    s = rep(s, """    s16 *t;
    s32 *v;
    s32 n;
""", """    s16 *t;
    Unk80045878Obj *v;
    s32 n;
""")
    s = rep(s, "    s32 *player;\n", "    Unk80045878Obj *player;\n")
    s = rep(s, "            vec.vy = (vec.vy * *(s16 *)((u8 *)player + 0x12)) >> 12;\n",
            "            vec.vy = (vec.vy * player->unk_12) >> 12;\n")
    return s


# ------------------------------------------------------------------ 31CFC.c: VcCtrl was a second layout of the node
def c31cfc(s):
    s = rep(s, '#include "include_asm.h"\n', '#include "include_asm.h"\n#include "game.h"\n')
    s = rep(s, """/* One 0x68-byte record of the three arrays func_80041430 rebases (obj+0x2C x 0x15, +0x8B4 x 0x14,
 * +0x10D4 x 0x14; text1a_pre_tu2.c): +0xC holds a pointer into the moved block, or 0. */
typedef struct VcCtrl {
    u8 unk0[0xC];
    s32 ptr;
    u8 unk10[0x58];
} VcCtrl;

/* Adds delta to every record's non-null pointer (the block it points into moved by delta). */
void save_vc_ctrl(s32 delta, VcCtrl *rec, s32 n) {
    s32 i;

    for (i = n - 1; i != -1; i--) {
        if (rec->ptr != 0) {
            rec->ptr += delta;
        }
        rec++;
    }
}""", """/* Adds delta to the parent pointer (node.unkC) of each of the n nodes that is not null: func_80041430
 * moved the model object, and the three node arrays with it, by delta bytes. */
void save_vc_ctrl(s32 delta, Unk80045878Node *rec, s32 n) {
    s32 i;

    for (i = n - 1; i != -1; i--) {
        if (rec->node.unkC != 0) {
            rec->node.unkC = (Unk80101DF0Record *)((u8 *)rec->node.unkC + delta);
        }
        rec++;
    }
}""")
    return s


# ------------------------------------------------------------------ 368E4.c
def c368e4(s):
    s = rep(s, """void *game_GetPlayerData(s32 a0) {
    void *v0 = func_8004153C(a0);
    if (v0) {
        return (u8 *)v0 + 0x1994;
    }""", """void *game_GetPlayerData(s32 a0) {
    Unk80045878Obj *v0 = func_8004153C(a0);
    if (v0) {
        return v0->unk_1994;
    }""")
    s = rep(s, """void *game_GetPlayerBase(s32 a0) {
    void *v0 = func_8004153C(a0);
    if (v0) {
        return (u8 *)v0 + 0x2C;
    }""", """void *game_GetPlayerBase(s32 a0) {
    Unk80045878Obj *v0 = func_8004153C(a0);
    if (v0) {
        return v0->unk_2C;
    }""")
    s = rep(s, """    s32 temp_v0;
    s32 sound;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = (s32)func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    idx = *(s16 *)(temp_v0 + 8);""", """    Unk80045878Obj *temp_v0;
    s32 sound;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    idx = temp_v0->unk_08;""")
    return s


def c31548(s):
    s = rep(s, "typedef struct { s32 a, b, c, d, e, f, g, h; } Copy8_40D48;\n", "")   # the MATRIX copy replaces it
    b = B_40D48.replace("COPYLOOP\n", COPY_40D48)
    k = ([o for o in OPT if o.startswith("d48_")] or ["d48_v2"])[0]
    b = rep(b, D48_58["d48_a"], D48_58[k])
    s = body(s, "func_80040D48", b)
    s = fn(s, "func_80041188", b_41188)
    s = body(s, "func_80041430", B_41430)
    return s


# ------------------------------------------------------------------ 309CC.c
def b_40594(b):
    b = rep(b, "void func_80040594(s32 *a0)\n", "void func_80040594(Unk80045878Obj *a0)\n")
    b = b.replace("((s16 *)a0)[3]", "a0->unk_06").replace("((s16 *)a0)[2]", "a0->unk_04")
    b = b.replace("((s16 *)a0)[10]", "a0->unk_14").replace("((s16 *)a0)[11]", "a0->unk_16")
    b = b.replace("((s16 *)a0)[4]", "a0->unk_08")
    b = b.replace("a0[8]", "a0->unk_20").replace("a0[7]", "a0->unk_1C").replace("a0[10]", "a0->unk_28")
    b = b.replace("a0[9]", "a0->unk_24").replace("a0[0]", "a0->unk_00.word")
    b = rep(b, "func_80041AC8((s16 *)a0);", "func_80041AC8(a0);")
    b = rep(b, "    a0->unk_24 = a0->unk_1C + off;\n", "    a0->unk_24 = (void *)(a0->unk_1C + off);\n")
    return b


def b_408f8(b):
    b = rep(b, "void func_800408F8(s32 *a0) {\n", "void func_800408F8(Unk80045878Obj *a0) {\n")
    b = rep(b, "    idx = *(s16 *)((u8 *)a0 + 8);\n", "    idx = a0->unk_08;\n")
    b = rep(b, "        *(u16 *)((u8 *)a0 + 0x12) = (u16)D_80094C68[idx];\n", "        a0->unk_12 = D_80094C68[idx];\n")
    b = rep(b, "        *(u16 *)((u8 *)a0 + 0x12) = 0x1000;\n", "        a0->unk_12 = 0x1000;\n")
    b = rep(b, """        u8 *p = (u8 *)a0 + 0x2C;
        u8 *base = p;
""", """        Unk80045878Node *p = a0->unk_2C;
        Unk80045878Node *base = p;
""")
    b = rep(b, """            *p = 0;
            *(p + 1) = 0;
            *(s16 *)(p + 8) = 0;
            *(s16 *)(p + 2) = neg1;
""", """            p->node.unk0 = 0;
            p->node.unk1 = 0;
            p->node.unk8 = 0;
            p->node.unk2 = neg1;
""")
    b = rep(b, """                *(s32 *)(p + 0xC) = (s32)(base + v1 * 104);
            } else {
                *(s32 *)(p + 0xC) = 0;
            }
            *(s32 *)(p + 0x4C) = ((s32)*(s16 *)((u8 *)D_80094B98 + off) * (s16)*(u16 *)((u8 *)a0 + 0x12)) >> 12;
            *(s32 *)(p + 0x50) = ((s32)*(s16 *)((u8 *)D_80094B9A + off) * (s16)*(u16 *)((u8 *)a0 + 0x12)) >> 12;
            prod = (s32)*(s16 *)((u8 *)D_80094B9C + off) * (s16)*(u16 *)((u8 *)a0 + 0x12);
            *(s16 *)(p + 0x10) = 0;
            *(s16 *)(p + 0x12) = 0;
            *(s16 *)(p + 0x14) = 0;
            *(s16 *)(p + 0x6) = 0;
            *(s32 *)(p + 0x54) = prod >> 12;
            *(u16 *)(p + 0xA) = *(u16 *)((u8 *)a0 + 0x10);
            *(u16 *)(p + 4) = *(u16 *)((u8 *)a0 + 0x14);
            off += 0xA;
            p += 0x68;
""", """                p->node.unkC = &base[v1].node;
            } else {
                p->node.unkC = 0;
            }
            p->node.work.t[0] = ((s32)*(s16 *)((u8 *)D_80094B98 + off) * a0->unk_12) >> 12;
            p->node.work.t[1] = ((s32)*(s16 *)((u8 *)D_80094B9A + off) * a0->unk_12) >> 12;
            prod = (s32)*(s16 *)((u8 *)D_80094B9C + off) * a0->unk_12;
            p->node.xf.rot.vx = 0;
            p->node.xf.rot.vy = 0;
            p->node.xf.rot.vz = 0;
            p->node.unk6 = 0;
            p->node.work.t[2] = prod >> 12;
            p->node.unkA = a0->unk_10;
            p->node.unk4 = a0->unk_14;
            off += 0xA;
            p++;
""")
    b = rep(b, "    func_80040A78((s32)a0);\n", "    func_80040A78(a0);\n")
    if "a_neg1" in OPT:      # ablation: the -1 literal for the holder
        b = rep(b, "        s32 neg1 = -1;\n", "")
        b = rep(b, "            p->node.unk2 = neg1;\n", "            p->node.unk2 = -1;\n")
        b = rep(b, "            if (v1 != neg1) {\n", "            if (v1 != -1) {\n")
    else:
        b = rep(b, "        s32 neg1 = -1;\n", "        /* FAKE: constant holder for -1 (the parent-less marker); the literal "
                "S_NEG1 */\n        s32 neg1 = -1;\n")
    return b


def b_40a78(b):
    t = b_40a78_text(b)
    return t if "tbl =" in t else rep(t, "    MATRIX **tbl;\n", "")


def b_40a78_text(b):
    return rep(b, b, """void func_80040A78(Unk80045878Obj *arg0) {
    s32 var_a1;
    MATRIX **tbl;
    Unk80045878Node *n;

    var_a1 = 0;
    arg0->unk_18F4[0] = &arg0->unk_2C[6].node.xf.mat;
    arg0->unk_18F4[1] = &arg0->unk_2C[5].node.xf.mat;
    arg0->unk_18F4[2] = &arg0->unk_2C[4].node.xf.mat;
    arg0->unk_18F4[3] = &arg0->unk_2C[10].node.xf.mat;
    arg0->unk_18F4[4] = &arg0->unk_2C[9].node.xf.mat;
    arg0->unk_18F4[5] = &arg0->unk_2C[8].node.xf.mat;
    arg0->unk_18F4[6] = &arg0->unk_2C[3].node.xf.mat;
    arg0->unk_18F4[7] = &arg0->unk_2C[2].node.xf.mat;
    arg0->unk_18F4[8] = &arg0->unk_2C[17].node.xf.mat;
    arg0->unk_18F4[9] = &arg0->unk_2C[16].node.xf.mat;
    arg0->unk_18F4[10] = &arg0->unk_2C[14].node.xf.mat;
    arg0->unk_18F4[11] = &arg0->unk_2C[13].node.xf.mat;
    arg0->unk_18F4[12] = &arg0->unk_2C[12].node.xf.mat;
    arg0->unk_18F4[13] = &arg0->unk_2C[1].node.xf.mat;
    arg0->unk_18F4[14] = &arg0->unk_2C[7].node.xf.mat;
    arg0->unk_18F4[15] = &arg0->unk_2C[11].node.xf.mat;
    arg0->unk_18F4[16] = &arg0->unk_2C[18].node.xf.mat;
    arg0->unk_18F4[17] = &arg0->unk_2C[15].node.xf.mat;
    arg0->unk_18F4[18] = &arg0->unk_2C[19].node.xf.mat;
    arg0->unk_18F4[19] = &arg0->unk_2C[20].node.xf.mat;
    LOOP
}
""".replace("    LOOP\n", A78[([o for o in OPT if o.startswith("a78_")] or ["a78_g"])[0]]))


A78 = {
    "a78_a": """    n = &arg0->unk_2C[1];
    tbl = arg0->unk_1994;
    do {
        *tbl = &n->node.xf.mat;
        var_a1 += 1;
        n = &arg0->unk_2C[1 + var_a1];
        tbl++;
    } while (var_a1 < 0x14);
""",
    "a78_b": """    do {
        arg0->unk_1994[var_a1] = &arg0->unk_2C[var_a1 + 1].node.xf.mat;
        var_a1 += 1;
    } while (var_a1 < 0x14);
""",
    "a78_c": """    n = &arg0->unk_2C[1];
    do {
        arg0->unk_1994[var_a1] = &n->node.xf.mat;
        var_a1 += 1;
        n = &arg0->unk_2C[1] + var_a1;
    } while (var_a1 < 0x14);
""",
    "a78_e": """    n = &arg0->unk_2C[1];
    tbl = arg0->unk_1994;
    do {
        *tbl = &n->node.xf.mat;
        var_a1 += 1;
        n++;
        tbl++;
    } while (var_a1 < 0x14);
""",
    "a78_f": """    n = &arg0->unk_2C[1];
    do {
        arg0->unk_1994[var_a1] = &n->node.xf.mat;
        var_a1 += 1;
        n++;
    } while (var_a1 < 0x14);
""",
    "a78_g": """    n = &arg0->unk_2C[1];
    do {
        arg0->unk_1994[var_a1] = &n->node.xf.mat;
        n++;
        var_a1 += 1;
    } while (var_a1 < 0x14);
""",
    "a78_d": """    for (var_a1 = 0; var_a1 < 0x14; var_a1++) {
        arg0->unk_1994[var_a1] = &arg0->unk_2C[var_a1 + 1].node.xf.mat;
    }
""",
}


B_40B44 = """void func_80040B44(Unk80045878Obj *arg0) {
    s32 seen[18];
    Unk80045878Node *t5;
    Unk80045878Node *t7;
    s32 *v1;
    s32 *t3;
    u16 a0_val;
    s32 i;

    t5 = arg0->unk_10D4;
    t7 = &arg0->unk_2C[1];
    v1 = (s32 *)arg0->unk_1C;
    t3 = (s32 *)((u8 *)v1 + *(s32 *)((u8 *)v1 + 8));

    {
        s32 *p1;
        i = 0x11;
        p1 = &seen[17];
        do {
            *p1 = 0;
            i--;
            p1--;
        } while (i >= 0);
    }
    i = 0x13;
    do {
        arg0->unk_1A34[i] = 0;
        i--;
    } while (i >= 0);

    a0_val = *(u16 *)t3;
    if ((a0_val & 0xFFFF) == 0xFFFF) goto done;

    {
        do {
            s32 a3;
            s32 *a1;
            s32 t2;

            t3 = (s32 *)((u8 *)t3 + 2);
            a3 = *(u16 *)t3;
            t3 = (s32 *)((u8 *)t3 + 2);
            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);
            t2 = a0_val & 0xFFFF;

            if (*a1 == 0) {
SLOT
            } else {
COPY
                t5->node.unk2 = t2;
                t5->unk58 = (s32)&t7[a3];
                arg0->unk_1A34[t2] = t5;
                t5++;
            }

            a0_val = *(u16 *)t3;
        } while (a0_val != 0xFFFF);
    }
done:
    t5->unk58 = 0;
}
"""
SLOT = {
    "slot_a": """                Unk80045878Node *slot = &t7[a3];
                slot->node.unk2 = t2;
                *a1 = 1;
                arg0->unk_1A34[t2] = slot;
""",
    "slot_b": """                Unk80045878Node *slot = t7 + a3;
                slot->node.unk2 = t2;
                *a1 = 1;
                arg0->unk_1A34[t2] = slot;
""",
    "slot_c": """                t7[a3].node.unk2 = t2;
                *a1 = 1;
                arg0->unk_1A34[t2] = &t7[a3];
""",
    "slot_d": """                Unk80045878Node *slot = a3 + t7;
                slot->node.unk2 = t2;
                *a1 = 1;
                arg0->unk_1A34[t2] = slot;
""",
}
COPY_STRUCT = "                *t5 = t7[a3];\n"
COPY_LOOP = """                s32 *t0 = (s32 *)t5;
                s32 *a2 = (s32 *)&t7[a3];
                s32 *end = (s32 *)((u8 *)a2 + 0x60);

                do {
                    *(Copy16 *)t0 = *(Copy16 *)a2;
                    a2 = (s32 *)((u8 *)a2 + 0x10);
                    t0 = (s32 *)((u8 *)t0 + 0x10);
                } while (a2 != end);

                *(Copy8 *)t0 = *(Copy8 *)a2;

"""


B_40CB8_A = """void func_80040CB8(Unk80045878Obj *arg0) {
    s16 id;
    Unk80045878Node *slot = arg0->unk_8B4;
    s32 i = 0;
    s32 none;
    s32 kind;
    s32 one;
    Unk80045878Node *link;
    s16 *tbl;
    Unk80045878Node *ent;

    do {
        none = -1;
        kind = 3;
        one = 1;
    } while (0);
    link = &arg0->unk_2C[1];
    tbl = D_80094B9E;
    do {
        ent = arg0->unk_8B4;
    loop:
        id = *tbl;
        if (id != none) {
            ent->node.unk2 = id;
            slot->node.unk0 = kind;
            ent->node.unk1 = 0;
            ent->node.unk8 = 0;
            ent->node.unkC = &link->node;
            ent->node.unk6 = one;
            ent->node.unkA = 0;
            {
                u16 w = arg0->unk_16;
                slot++;
                ent->unk58 = 0;
                ent->node.unk4 = w;
                ent++;
            }
        }
        link++;
        i++;
        tbl = (s16 *)((s32)tbl + 0xA);
        if (i < 0x12) goto loop;
    } while (0);
    slot->node.unk2 = -1;
}
"""


def c309cc(s):
    if "b44_loop" not in OPT:   # func_80040B44's node copy is a struct assignment now
        s = rep(s, "typedef struct { s32 f0, f1; } Copy8;\n", "")
    s = rep(s, "extern void func_80040594(s32 *);\nextern void func_800408F8(s32 *);\nextern void func_80040B44(s32 *);\n",
            "extern void func_80040594(Unk80045878Obj *);\nextern void func_800408F8(Unk80045878Obj *);\n"
            "extern void func_80040B44(Unk80045878Obj *);\n")
    # func_80040304
    s = rep(s, """    s32 ptr;
    s32 mask;
    s32 i;

    ptr = (s32)func_8004153C(a0);""", """    Unk80045878Obj *ptr;
    s32 mask;
    s32 i;

    ptr = func_8004153C(a0);""")
    s = rep(s, "        mask = mask & D_80094B48[*(s16 *)(ptr + 8)];\n", "        mask = mask & D_80094B48[ptr->unk_08];\n")
    # func_8004046C
    s = rep(s, """    s32 *base = func_8004153C(a0);
    return *(s32 *)((u8 *)base + a1 * 4 + 0x1A34);""", """    Unk80045878Obj *base = func_8004153C(a0);
    return (s32)base->unk_1A34[a1];""")
    # func_800404A0: walks the unk_8B4[] nodes from the one it is given
    s = rep(s, """void func_800404A0(s16 *a0, s32 a1) {
    if (a0[1] == -1) {
        return;
    }
    do {
        *(s32 *)((u8 *)a0 + 0x58) = a1;
        a0 = (s16 *)((u8 *)a0 + 0x68);
    } while (a0[1] != -1);
}""", """void func_800404A0(Unk80045878Node *a0, s32 a1) {
    if (a0->node.unk2 == -1) {
        return;
    }
    do {
        a0->unk58 = a1;
        a0++;
    } while (a0->node.unk2 != -1);
}""")
    # func_80040510
    s = rep(s, """s32 *func_80040510(s32 a0, s32 a1, s32 a2) {
    s32 *ptr;
    ptr = (s32 *)func_80045878(a0, a1, a2);""", """Unk80045878Obj *func_80040510(s32 a0, s32 a1, s32 a2) {
    Unk80045878Obj *ptr;
    ptr = func_80045878(a0, a1, a2);""")
    s = rep(s, "    do { func_800408F8(ptr); func_80040B44(ptr); func_8003F824((u8 *)ptr, 1); } while (0);\n",
            "    do { func_800408F8(ptr); func_80040B44(ptr); func_8003F824(ptr, 1); } while (0);\n")
    s = fn(s, "func_80040594", b_40594)
    s = fn(s, "func_800408F8", b_408f8)
    s = fn(s, "func_80040A78", b_40a78)
    s = body(s, "func_80040B44", B_40B44.replace("COPY\n", COPY_LOOP if "b44_loop" in OPT else COPY_STRUCT)
             .replace("SLOT\n", SLOT[([o for o in OPT if o.startswith("slot_")] or ["slot_c"])[0]]))
    if "cb_a" in OPT:
        s = body(s, "func_80040CB8", B_40CB8_A)
    else:   # the typed spellings miss (see the debt row): the parameter takes the type, the body stays
        s = rep(s, "void func_80040CB8(void *arg0) {\n", "void func_80040CB8(Unk80045878Obj *arg0) {\n")
    return s


FILES = [("include/game.h", game_h), ("include/bb2.h", bb2_h), ("src/main/31D3C.c", c31d3c),
         ("src/main/309CC.c", c309cc),
         ("src/main/31548.c", c31548), ("src/main/35000.c", c35000),
         ("src/main/368E4.c", c368e4), ("src/main/2B344.c", c2b344), ("src/main/3AB48.c", c3ab48),
         ("src/main/31CFC.c", c31cfc)]

# measured ablation scores (abl_w2b3.py / run_abl_w2b3.ps1 on the applied tree)
SCORES3 = {"S_NEG1": "scores 2 (li t0,-1 moves one slot)", "S_WH": "drop the s5 / s4 seats (score 4)",
           "S_P58D": "5"}

# the carried FAKEs of the moved bodies, re-ablated: anchor -> score
CARRIED = {
    "src/main/2B344.c": [("/* FAKE: identical arms (gouraud", 99), ("/* FAKE: the variable compare `", 16)],
    "src/main/309CC.c": [("// FAKE (none/kind/one)", 23), ("// FAKE: wrap emits NOTE_INSN_LOOP_BEG/END", 6),
                         ("// FAKE: wrap emits loop notes around the body", 21)],
    "src/main/31D3C.c": [("volatile u32 pre_pad[8]; /* !FAKE: target frame 0x38", 6),
                         ("/* FAKE: guard staged through the existing local b", 2),
                         ("/* FAKE: opaque constant-holder for the trailing", 12),
                         ("/* FAKE: oversized locals object - rect[0..3]", 22),
                         ("/* FAKE: single-level do-while(0) wrap on the else-arm", 18),
                         ("/* FAKE: id_ptr spelling", 2)],
    "src/main/368E4.c": [("/* FAKE: the record counter reuses `sound`", 6)],
    "src/main/3AB48.c": [("/* FAKE: second C handle to the global ctrl block (pointer-alias family);", 63),
                         ("/* FAKE: second C handle to the global ctrl block (pointer-alias family),", 157),
                         ("/* FAKE: one variable for the three player objects", 9),
                         ("/* FAKE: one variable for two values", 41)],
}


def add_score(s, anchor, n):
    if s.count(anchor) == 0 and OPT - {"sc_00f8b"}:   # an alternative body without that comment
        return s
    if s.count(anchor) != 1:
        raise SystemExit("anchor %r: %d" % (anchor, s.count(anchor)))
    i = s.index(anchor)
    if anchor.startswith("//"):
        j = s.index("\n", i)
        while s[j + 1:].lstrip(" ").startswith("//"):
            j = s.index("\n", j + 1)
        indent = s[s.rindex("\n", 0, i) + 1:i]
        return s[:j + 1] + indent + "// Ablated (2026-10-06): score %d.\n" % n + s[j + 1:]
    j = s.index("*/", i)
    return s[:j].rstrip() + " Ablated (2026-10-06): score %d. " % n + s[j:]


def _scored(g, p=None):
    def h(s):
        s = g(s)
        if p == "src/main/2B344.c":
            s = rep(s, "/* The variable compare `", "/* FAKE: the variable compare `")
        if p == "src/main/31D3C.c":
            s = rep(s, "  /* id_ptr spelling:", "  /* FAKE: id_ptr spelling:")
        if p == "src/main/309CC.c" and "keep_510" not in OPT:   # its do-while(0) FAKE ablates to 0
            s = rep(s, "    /* FAKE: loop-note ref weighting seats ptr in s0 (s0/s1 swap) */\n"
                    "    do { func_800408F8(ptr); func_80040B44(ptr); func_8003F824(ptr, 1); } while (0);\n",
                    "    func_800408F8(ptr);\n    func_80040B44(ptr);\n    func_8003F824(ptr, 1);\n")
        for a, n in CARRIED.get(p, []):
            s = add_score(s, a, n)
        for k, v in SCORES3.items():
            s = s.replace(k, v)
        return s
    return h


# rev-w2b3 fixes (FAIL B4 / B5 / B6), applied to the generated text; `abl_<x>` options undo one
# construct in place for its ablation. LBL holds the measured label texts.
LBL = {
    "wh": "constant holders for the 16 x 1 rect size; as literals the g_gpu_store_buf address (lui/addiu s1) "
          "is scheduled ahead of li s5,16 / li s4,1 (score 4)",
    "v0val": "v0_val reads var_s0[1] ahead of the rect[2] / rect[3] stores; read at the rect[1] store, "
             "sh s5 / sh s4 move above the load (score 4)",
    "base": "base keeps the array start (move t1,a3); &a0->unk_2C[v1] rebuilds it from a0 (addiu 44 + addu) "
            "and the mflo temps move to t1 (score 9)",
    "a1": "the slot address as integer arithmetic; &seen[a3] swaps the addu operands (score 1)",
    "out3": "a second copy of a4 + 1 for the second loop (s3); reusing out2 drops the copy and moves "
            "lw t0,24(sp) up (score 4)",
    "off": "offset becomes the a2-side address (addu s0,s0,s2); a fresh sum loads through v1 (score 4)",
    "idx": "idx (a0 - 3) is reused for the sound index; a fresh local swaps the s0 / s1 seats (score 8)",
    "n": "n reads each group count signed (lh) and is copied to count; read straight into count the "
         "load is lhu and the copy goes (score 8)",
    "d48idx": "idx re-read from *tbl before each component, as the target reloads it and recomputes "
              "idx * 6; one read computes it once (score 48)",
    "half": "the halfword read into an s32 local (lh); in the masked expression it loads lhu (score 1)",
    "delta": "delta taken before the loop (subu a2 ahead of it); computed at the call, p's registers "
             "shift (score 19)",
    "q": "q = p + 0xA, the target's second cursor (addiu a1,v1,10; stores at -8 / -6 / -9 / 0 from it); "
         "through p the cursor goes (score 11)",
    "vy": "vec.vy set twice; one expression drops the target's spill of frame[0] (sw v1,28(sp)) (score 2)",
}


def F(s, old, new, n=1):
    return rep(s, old, new, n)


def fix_31d3c(s):
    s = F(s, "    /* FAKE: constant holders for the 16 x 1 rect size; the literals drop the s5 / s4 seats (score 4) */\n",
          "    /* FAKE: %s */\n" % LBL["wh"])
    s = F(s, "   * lh arg0[4]) is only", "   * lh arg0->unk_08) is only")
    s = F(s, "        s32 val = ptr->unk_00.half[1];\n", "        /* FAKE: %s */\n        s32 val = ptr->unk_00.half[1];\n" % LBL["half"], 2)
    if "abl_v0val" in OPT:
        s = F(s, "      u16 v0_val;\n", "")
        s = F(s, "      v0_val = (u16) var_s0[1];\n", "")
        s = F(s, "      rect[1] = v0_val + var_s2;\n", "      rect[1] = (u16) var_s0[1] + var_s2;\n")
    else:
        s = F(s, "      u16 v0_val;\n", "      /* FAKE: %s */\n      u16 v0_val;\n" % LBL.get("v0val", "V0VAL"))
    return s


def fix_309cc(s):
    s = F(s, "s32 func_8004046C(s32 a0, s32 a1) {\n    Unk80045878Obj *base = func_8004153C(a0);\n    return (s32)base->unk_1A34[a1];\n",
          "Unk80045878Node *func_8004046C(s32 a0, s32 a1) {\n    Unk80045878Obj *base = func_8004153C(a0);\n    return base->unk_1A34[a1];\n")
    # func_800408F8: prod dropped (0); base labelled
    s = F(s, "            s32 prod;\n", "")
    s = F(s, "            prod = (s32)*(s16 *)((u8 *)D_80094B9C + off) * a0->unk_12;\n",
          "            p->node.work.t[2] = ((s32)*(s16 *)((u8 *)D_80094B9C + off) * a0->unk_12) >> 12;\n")
    s = F(s, "            p->node.work.t[2] = prod >> 12;\n", "")
    if "abl_base" in OPT:
        s = F(s, "        Unk80045878Node *base = p;\n", "")
        s = F(s, "                p->node.unkC = &base[v1].node;\n", "                p->node.unkC = &a0->unk_2C[v1].node;\n")
    else:
        s = F(s, "        Unk80045878Node *base = p;\n", "        /* FAKE: %s */\n        Unk80045878Node *base = p;\n" % LBL.get("base", "BASE"))
    # func_80040B44: the & 0xFFFF masks dropped (0); the a1 address labelled
    s = F(s, "    if ((a0_val & 0xFFFF) == 0xFFFF) goto done;\n", "    if (a0_val == 0xFFFF) goto done;\n")
    s = F(s, "            t2 = a0_val & 0xFFFF;\n", "            t2 = a0_val;\n")
    if "abl_a1" in OPT:
        s = F(s, "            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);\n", "            a1 = &seen[a3];\n")
    else:
        s = F(s, "            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);\n",
              "            /* FAKE: %s */\n            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);\n" % LBL.get("a1", "A1"))
    # func_80040594: fld10, the offset / index temps, the flags / bits temps and the (s32) cast dropped,
    # `!= player` (the value 1 there) spelled as the literal (each 0)
    s = F(s, "        s32 fld10 = (s32)rmd + (((u32)rmd[3] >> 2) << 2);\n", "")
    s = F(s, "        a0->unk_28 = fld10;\n", "        a0->unk_28 = (s32)rmd + (((u32)rmd[3] >> 2) << 2);\n")
    s = F(s, "        if (func_8003E2A0() != player) goto case_1_else;\n", "        if (func_8003E2A0() != 1) goto case_1_else;\n")
    s = F(s, """        s32 flags = a0->unk_00.word & (s32)0xFFE0FFFF;
        s32 bits = (g_player_char_ids[a0->unk_04] & 0x1F) << 16;
        a0->unk_00.word = flags | bits;
""", """        a0->unk_00.word = (a0->unk_00.word & 0xFFE0FFFF) | ((g_player_char_ids[a0->unk_04] & 0x1F) << 16);
""")
    s = F(s, "        s32 prev_off = ((u32)rmd[count - 1] >> 2) << 2;\n        s32 next_off = ((u32)rmd[count + 1] >> 2) << 2;\n"
             "        s32 idx;\n        ptr = (s32 *)((s32)rmd + prev_off);\n        idx = a0->unk_04 * 3 + 1;\n"
             "        ptr = (s32 *)((s32)ptr + func_8005C2A8(ptr, idx, (s32 *)((s32)rmd + next_off)));\n",
          "        ptr = (s32 *)((s32)rmd + (((u32)rmd[count - 1] >> 2) << 2));\n"
          "        ptr = (s32 *)((s32)ptr + func_8005C2A8(ptr, a0->unk_04 * 3 + 1,\n"
          "                                               (s32 *)((s32)rmd + (((u32)rmd[count + 1] >> 2) << 2))));\n")
    # func_80040CB8: the reviewer's typed forms (0); the ent-relative stores stay (narrowed debt row)
    if "cb_a" in OPT:   # the debt row's two-cursor alternative replaces the whole body
        return s
    s = F(s, "    s8 *slot = (s8 *)arg0 + 0x8B4;\n", "    s8 *slot = (s8 *)arg0->unk_8B4;\n")
    s = F(s, "    link = (s32)arg0 + 0x94;\n", "    link = (s32)&arg0->unk_2C[1];\n")
    s = F(s, "        ent = (s32)arg0 + 0x90C;\n", "        ent = (s32)&arg0->unk_8B4[0].unk58;\n")
    s = F(s, "                u16 w = *(u16 *)((s32)arg0 + 0x16);\n", "                u16 w = arg0->unk_16;\n")
    return s


def fix_31548(s):
    # func_80040D48: `scaled` dropped (0); the triple *tbl read labelled
    s = F(s, "        s32 scaled;\n", "")
    s = F(s, "        scaled = (s3->node.work.t[1] * s4->unk_12) >> 12;\n", "")
    s = F(s, "        s3->node.work.t[1] = scaled;\n", "        s3->node.work.t[1] = (s3->node.work.t[1] * s4->unk_12) >> 12;\n")
    s = F(s, "            a4p = &s3[s0];\n            idx = *tbl;\n",
          "            a4p = &s3[s0];\n            /* FAKE: %s */\n            idx = *tbl;\n" % LBL["d48idx"])
    if "abl_out3" in OPT:
        s = F(s, "    MATRIX *out3;\n", "")
        s = F(s, "    out3 = a4 + 1;\n", "")
        s = s.replace("math_RotMatrixZYX(&buf, out3);", "math_RotMatrixZYX(&buf, out2);").replace(
            "func_800523E0(a4, out3, a3, &stptr2->node.work);", "func_800523E0(a4, out2, a3, &stptr2->node.work);")
    else:
        s = F(s, "    out3 = a4 + 1;\n", "    /* FAKE: %s */\n    out3 = a4 + 1;\n" % LBL.get("out3", "OUT3"))
    if "abl_off" in OPT:
        s = F(s, "        offset = offset + (s32) a2;\n        p = (u16 *) offset;\n", "        p = (u16 *) (offset + (s32) a2);\n")
    else:
        s = F(s, "        offset = offset + (s32) a2;\n",
              "        /* FAKE: %s */\n        offset = offset + (s32) a2;\n" % LBL.get("off", "OFF"))
    return s


def fix_35000(s):
    s = F(s, "    v0 = (s16 *) func_8004574C(a0);\n    if (v0 != 0) {\n        s1 = (Unk80045878Obj *) ((s32 *) v0)[1];\n",
          "    v0 = func_8004574C(a0);\n    if (v0 != 0) {\n        s1 = (Unk80045878Obj *)v0[1];\n")
    s = F(s, "Unk80045878Obj *func_80045878(s32 a0, s32 a1, s32 a2) {\n    s32 s3;\n    s16 *v0;\n",
          "Unk80045878Obj *func_80045878(s32 a0, s32 a1, s32 a2) {\n    s32 s3;\n    s32 *v0;\n")
    if "abl_idx" in OPT:
        s = F(s, "        s32 val = ptr->unk_04;\n        idx = 3 * val + 1;\n        snd_VabFakeOpen(a1, idx);\n",
              "        s32 idx2 = 3 * ptr->unk_04 + 1;\n        snd_VabFakeOpen(a1, idx2);\n")
    else:
        s = F(s, "        s32 val = ptr->unk_04;\n        idx = 3 * val + 1;\n",
              "        /* FAKE: %s */\n        idx = 3 * ptr->unk_04 + 1;\n" % LBL.get("idx", "IDX"))
    return s


def fix_2b344(s):
    s = F(s, "typedef struct {\n    /* 0x00 */ u16 unk0;\n    /* 0x02 */ u8 unk2[2];\n",
          "typedef struct {\n    /* 0x00 */ s16 unk0;\n    /* 0x02 */ u8 unk2[2];\n")
    s = F(s, "                /* FAKE: the id converted to s16 (lh); unconverted it loads lhu (score 1) */\n"
             "                obj_Clear((s16)s2->recs[s0].unk0);\n", "                obj_Clear(s2->recs[s0].unk0);\n")
    s = F(s, "            MATRIX *obj = &in->objs[j]->node.xf.mat;\n"
             "            gte_SetMatrixRotTransIR(obj, &in->quads[j][0], in->pairs[j]);\n"
             "            gte_SetMatrixRotTransIR(obj, &in->quads[j][1], in->pairs[j] + 8);\n",
          "            MATRIX *mat = &in->objs[j]->node.xf.mat;\n"
          "            gte_SetMatrixRotTransIR(mat, &in->quads[j][0], in->pairs[j]);\n"
          "            gte_SetMatrixRotTransIR(mat, &in->quads[j][1], in->pairs[j] + 8);\n")
    if "abl_n" in OPT:
        s = F(s, "    for (n = (s16)*src++, count = n; n != 0; n = (s16)*src++, count = n) {\n",
              "    count = *src++;\n    while (count != 0) {\n")
        s = F(s, "        *packet++ = 0;\n    }\n", "        *packet++ = 0;\n        count = *src++;\n    }\n")
        s = F(s, "    s32 n;\n    s16 *packet;\n", "    s16 *packet;\n")
    else:
        s = F(s, "    for (n = (s16)*src++, count = n; n != 0; n = (s16)*src++, count = n) {\n",
              "    /* FAKE: %s */\n    for (n = (s16)*src++, count = n; n != 0; n = (s16)*src++, count = n) {\n" % LBL.get("n", "NLOOP"))
    return s


def fix_368e4(s):
    # func_80048AD0: idx dropped (0); delta and q labelled
    s = F(s, "    idx = temp_v0->unk_08;\n    D_800A33E0 = arg0;\n    sound = (&D_80099BCC)[idx];\n",
          "    D_800A33E0 = arg0;\n    sound = (&D_80099BCC)[temp_v0->unk_08];\n")
    s = F(s, "    s32 sound;\n    s32 idx;\n    u8 *base;\n", "    s32 sound;\n    u8 *base;\n")
    s = F(s, "    delta = (s32)(p - base);\n", "    /* FAKE: %s */\n    delta = (s32)(p - base);\n" % LBL["delta"])
    s = F(s, "    q = p + 0xA;\n", "    /* FAKE: %s */\n    q = p + 0xA;\n" % LBL["q"])
    return s


def fix_3ab48(s):
    s = F(s, "            vec.vy = frame[0];\n            vec.vy = (vec.vy * player->unk_12) >> 12;\n",
          "            /* FAKE: %s */\n            vec.vy = frame[0];\n            vec.vy = (vec.vy * player->unk_12) >> 12;\n" % LBL["vy"])
    return s


FIX3 = {"src/main/31D3C.c": fix_31d3c, "src/main/309CC.c": fix_309cc, "src/main/31548.c": fix_31548,
        "src/main/35000.c": fix_35000, "src/main/2B344.c": fix_2b344, "src/main/368E4.c": fix_368e4,
        "src/main/3AB48.c": fix_3ab48}


# rev-w2b3 re-review (B4): the second sweep for named temps / staged reads ahead of a store or call
# (abl_w2b3fix2.py). Labels go above the anchor line; two holders ablate to 0 and are dropped.
LBL2 = [   # (file, anchor, nth occurrence (0-based), label)
    ("src/main/309CC.c", "        s32 *texA = (s32 *)((s32)rmd + (((u32)rmd[1] >> 2) << 2));\n", 0,
     "texA / texB computed ahead of the unk_28 store; passed in the calls, the address arithmetic moves "
     "below sw unk_28 and the s-registers re-seat (score 72)"),
    ("src/main/309CC.c", "            t2 = a0_val;\n", 0,
     "t2 copies a0_val ahead of the seen[] test; with a0_val itself the load and the cursor step re-seat "
     "(score 9)"),
    ("src/main/309CC.c", "                u16 w = arg0->unk_16;\n", 0,
     "w reads unk_16 ahead of the slot step and the *ent store; read at its store the sw zero moves "
     "above it (score 2)"),
    ("src/main/31D3C.c", "  v1_val = (u16) (*var_s0);\n", 0,
     "v1_val is the loop-rotated read of *var_s0 (here and at the loop's end); read at the rect[0] store "
     "the frame shrinks 0x50 -> 0x40 and the head's lh / lhu pair changes (score 26)"),
    ("src/main/31D3C.c", "    s32 off = idx << 5;\n", 0,
     "off = idx << 5 taken before idx++; inline in LoadImage with idx++ after DrawSync, addiu s1 / sll v0 "
     "swap around the call setup (score 4)"),
    ("src/main/31D3C.c", "        r = player->unk_18.byte[0];\n", 0,
     "r / g / b read into locals ahead of the call; read in the argument expression the frame grows "
     "0x38 -> 0x40 (score 8)"),
    ("src/main/2B344.c", "            obj = arg0->unk_1A34[i];\n", 0,
     "obj read from unk_1A34[i] ahead of the rec stores and kept across the func_8003FA24 call; re-read "
     "at each use the s-registers re-seat (score 53)"),
    ("src/main/2B344.c", "        mode = ((s16)flags >> 3) & 3;\n", 0,
     "mode computed once ahead of the record loop; indexing D_80094AEC with the expression moves the "
     "addiu s0 / andi pair (score 6)"),
    ("src/main/3AB48.c", "        n = (s->unk4 & 0x3F) - 1;\n", 0,
     "n computed ahead of the a6 test; computed in each call the subtraction moves into the call setup "
     "(score 5)"),
    ("src/main/3AB48.c", "    s32 id = a0 + 0x131;\n", 0,
     "id (a0 + 0x131) computed at entry; at its uses the addiu moves below the prologue stores (score 6)"),
]


def fix2(p, s):
    if p == "src/main/309CC.c":   # func_80040594's player holder: 0
        s = F(s, "        s32 player = a0->unk_04;\n        if (player == 1) goto case_1;\n"
                 "        if (player >= 2) goto done_cases;\n        if (player != 0) goto done_cases;\n",
              "        if (a0->unk_04 == 1) goto case_1;\n        if (a0->unk_04 >= 2) goto done_cases;\n"
              "        if (a0->unk_04 != 0) goto done_cases;\n")
    if p == "src/main/31D3C.c":   # func_80041688's grayscale arm reads the bytes in the call: 0
        s = F(s, "        r = player->unk_18.byte[0];\n        g = player->unk_18.byte[1];\n"
                 "        b = player->unk_18.byte[2];\n        v = math_Grayscale3(b, g, r);\n",
              "        v = math_Grayscale3(player->unk_18.byte[2], player->unk_18.byte[1], player->unk_18.byte[0]);\n")
    for f, anchor, nth, text in LBL2:
        if f != p:
            continue
        i = -1
        for _ in range(nth + 1):
            i = s.index(anchor, i + 1)
        ind = anchor[:len(anchor) - len(anchor.lstrip(" "))]
        s = s[:i] + "%s/* FAKE: %s. */\n" % (ind, text) + s[i:]
    return s


def _fixed(g, p):
    def h(s):
        s = g(s)
        s = FIX3[p](s) if p in FIX3 else s
        return fix2(p, s) if "nofix2" not in OPT else s
    return h


FILES = [(p, _fixed(_scored(g, p), p)) for p, g in FILES]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b3"])[0]
    done = set()
    for p, g in FILES:
        s = g(base(p))
        done.add(p)
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    for p, g in []:
        if p not in done:
            dst = p if apply else out + "/" + p
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            open(dst, "w", encoding="utf-8", newline=NL).write(g(B2.show(p)))
    print("w2b3 wrote to %s %s" % ("the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
