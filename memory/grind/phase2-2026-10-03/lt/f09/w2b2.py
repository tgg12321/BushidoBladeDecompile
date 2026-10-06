#!/usr/bin/env python3
# Worker-2 batch 2 (F09 28708 replay records + their 9F9C consumers; F08 309CC; F10 func_80040D48;
# F18 func_8002EECC), on top of p2/w2 d4e00362c (batch 1 rebased on main 7797854fe).
# F09: the replay buffer D_800A36EC holds frames of two 0x1C-byte Rec1C (game.h's record, its
#   layout corrected from the accesses); the two event-slot tables D_800F68E0 (0xB4 x 0x10) and
#   D_80101BF0 (0x20 x 0x10) get their record types.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f09/w2b2.py [opt=...] [out=DIR | apply]
#   default out=tmp/w2/b2 (scratch copies); `apply` writes the working tree.
import os, re, subprocess, sys

BASE = "6b2859480"
NL = chr(10)
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
MAIN_GIT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/.git"   # WSL in a Windows worktree


def show(p):
    for pre in (["git"], ["git", "--git-dir=" + MAIN_GIT]):
        r = subprocess.run(pre + ["show", "%s:%s" % (BASE, p)], capture_output=True, text=True,
                           encoding="utf-8")
        if r.returncode == 0:
            return r.stdout
    raise SystemExit("git show %s:%s failed" % (BASE, p))


def rep(s, old, new, n=1):
    c = s.count(old)
    if c != n:
        raise SystemExit("expected %d x %r, found %d" % (n, old[:80], c))
    return s.replace(old, new)


def span(s, name):
    m = re.search(r"\n[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{" % name, s)
    i = m.start() + 1
    return i, s.index("\n}\n", i) + 3


def body(s, name, new):
    i, j = span(s, name)
    return s[:i] + new + s[j:]


def fn(s, name, g):
    i, j = span(s, name)
    return s[:i] + g(s[i:j]) + s[j:]


# ------------------------------------------------------------------ game.h / bb2.h
REC1C_OLD = """/* 0x1C-byte record: func_8003993C walks an array of these at D_800A36EC with element
 * stride 0x1C (base + i*0x38 + {0,0x1C} + D_800A3748*0x1C, asm/funcs/func_8003993C.s:57-89)
 * and passes two of them to func_8001BAE4 / func_8001BBD8. */
typedef struct Rec1C {
    s16 h0; s16 h2; s16 h4; s16 h6; s16 h8; s16 hA; s16 hC; s16 hE;
    s32 w10; s32 w14; s32 w18;
} Rec1C;
"""
REC1C_NEW = """/* 0x1C-byte record: one player's state in one frame of the replay buffer D_800A36EC (frames
 * of two, Rec1C[2] = 0x38 bytes; func_80039680 writes player a0->index's record of frame
 * D_800A36F8 from the character record, func_8003993C reads both back and passes two of them
 * to func_8001BAE4 / func_8001BBD8, func_8001F1C4 reads b14..b16 / b18).
 * - w0: the character's current move (Unk80101EC8Record.unk_50); func_8003993C reads its unk_04.
 * - h4 / h6 / h8: unk_F4 x / y / z; hA: unk_1C8.vy & 0xFFF with unk_B3 in the top four bits;
 *   hC: unk_148; hE / h10 / h12: unk_64 / unk_66 / unk_68.
 * - b14..b16: unk_1E6 / unk_1E8 / unk_1EA >> 2, read back signed; b17: bit 0 = unk_60 != 0,
 *   bit 1 = unk_61 != 0; b18: unk_62; b19: unk_40. */
typedef struct Rec1C {
    MoveScript *w0;
    s16 h4; s16 h6; s16 h8;
    u16 hA;
    s16 hC; s16 hE; s16 h10; s16 h12;
    s8 b14; s8 b15; s8 b16;
    u8 b17; u8 b18; u8 b19;
    u8 pad1A[2];
} Rec1C;

/* The 0xB4 0x10-byte records of the event table D_800F68E0 (func_800392C8 clears unk_0 to -1,
 * func_80039320 ages them, func_800393C8 adds or refreshes one, func_8003993C replays them).
 * unk_0: -1 = free, else a frame counter; unk_2: frames seen; unk_3: func_800393C8's arg1;
 * unk_4: the s16 x / y / z of arg2; unk_A: arg3[0] & 0xFFF with arg0 in the top four bits;
 * unk_C / unk_E: arg3[1] / arg3[2]. */
typedef struct Unk800F68E0Rec {
    s16 unk_0;
    u8 unk_2;
    u8 unk_3;
    s16 unk_4[3];
    u16 unk_A;
    s16 unk_C;
    s16 unk_E;
} Unk800F68E0Rec;

/* The 0x20 0x10-byte records of the event table D_80101BF0 (func_800392C8 sets unk_0 to 0xFF =
 * free, func_80039320 frees the current frame's, func_800395B4 fills one, func_8003993C replays
 * those whose unk_0 is the frame index). unk_0: the frame (D_800A36F8); unk_1 / unk_2:
 * func_800395B4's arg0 / arg1; unk_4: the s16 x / y / z of arg2; unk_A: arg3[0..2]. */
typedef struct Unk80101BF0Rec {
    u8 unk_0;
    u8 unk_1;
    u8 unk_2;
    u8 pad3;
    s16 unk_4[3];
    u16 unk_A[3];
} Unk80101BF0Rec;
"""


def game_h(s):
    return rep(s, REC1C_OLD, REC1C_NEW)


def bb2_h(s):
    s = rep(s, "extern s16 D_800F68E0[];\n", "extern Unk800F68E0Rec D_800F68E0[0xB4];\n")
    s = rep(s, "extern u8 D_80101BF0;\n", "extern Unk80101BF0Rec D_80101BF0[0x20];\n")
    s = rep(s, "extern void func_8001F1C4(Unk80101EC8Record *, u8 *, MotionFrame *, MotionFrame *);\n",
            "extern void func_8001F1C4(Unk80101EC8Record *, Rec1C *, MotionFrame *, MotionFrame *);\n")
    return s


# ------------------------------------------------------------------ 28708.c
B_392C8 = """void func_800392C8(void) {
    /* FAKE: constant holder for the 0xFF fill; with the literal, `li 255` is scheduled after `li 0x1F0` (score 2) */
    u8 fill;
    s32 i;
    s32 j;

    fill = 0xFF;
    D_800A36EC = (Rec1C (*)[2])D_800F33D8;
    D_800A36F8 = 0;
    D_800A3782 = 0;
    for (i = 0x1F; i >= 0; i--) {
        D_80101BF0[i].unk_0 = fill;
    }
    for (j = 0xB3; j >= 0; j--) {
        D_800F68E0[j].unk_0 = -1;
    }
}
"""

B_39320 = """void func_80039320(void) {
    extern u8 D_800A379C;
    extern s16 D_800A3714;
    s32 i;
    Unk80101BF0Rec *p;
    Unk800F68E0Rec *q;
    s16 val;
    s16 newval;
    /* FAKE: constant holder for the 0xFF free marker; the literal moves li 255 below the table
       address (score S_FILL) */
    u8 fill;

    i = 0;
    fill = 0xFF;
    p = D_80101BF0;

    do {
        if (p->unk_0 == D_800A36F8) {
            p->unk_0 = fill;
        }
        i++;
        p++;
    } while (i < 0x20);

    q = D_800F68E0;
    i = 0;
    do {
        val = q->unk_0;
        if (val != -1) {
            newval = val + 1;
            q->unk_0 = newval;
            if ((s16)newval - q->unk_2 >= 0x101) {
                q->unk_0 = -1;
            }
        }
        i++;
        q++;
    } while (i < 0xB4);

    D_800A379C = 0;
    D_800A3714 = 0;
}
"""

B_393C8 = """void func_800393C8(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    extern s16 D_800A3714;
    extern u8 D_800A3209;
    Unk800F68E0Rec *slot;
    s32 i;
    s16 idx;
    s16 cur;
    s32 state;
    u8 seen;
    s32 raw;
    s32 rot;

    slot = D_800F68E0;
    i = 0;
    do {
        state = slot->unk_0;
        if (state != -1) {
            seen = slot->unk_2;
            if (seen == state - 1 && seen < 0xFF) {
                raw = slot->unk_A << 16;
                rot = raw >> 16;
                if ((s32)((u32)raw >> 28) == arg0 &&
                    slot->unk_4[0] == arg2[0] &&
                    slot->unk_4[1] == arg2[1] &&
                    slot->unk_4[2] == arg2[2] &&
                    ((rot - arg3[0]) & 0xFFF) == 0 &&
                    ((slot->unk_C - arg3[1]) & 0xFFF) == 0 &&
                    ((slot->unk_E - arg3[2]) & 0xFFF) == 0) {
                    slot->unk_2 = seen + 1;
                    return;
                }
            }
        }
        i++;
        slot++;
    } while (i < 0xB4);

    idx = D_800A3714;
    slot = &D_800F68E0[idx];
    while (idx < 0xB4) {
        if (slot->unk_0 == -1) {
            break;
        }
        cur = idx + 1;
        D_800A3714 = cur;
        slot++;
        idx = cur;
    }

    if (D_800A3714 == 0xB4) {
        D_800A3209++;
        return;
    }

    slot->unk_0 = 0;
    slot->unk_3 = arg1;
    slot->unk_2 = 0;
    slot->unk_4[0] = arg2[0];
    slot->unk_4[1] = arg2[1];
    slot->unk_4[2] = arg2[2];
    slot->unk_A = (arg3[0] & 0xFFF) | (arg0 << 12);
    slot->unk_C = arg3[1];
    slot->unk_E = arg3[2];
}
"""

B_395B4 = """void func_800395B4(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 D_800A3208;
    extern u8 D_800A379C;
    Unk80101BF0Rec *slot;
    u8 idx;
    /* FAKE: constant holder for the 0xFF free marker; with the literal the li is scheduled after
       the lbu and the load-delay nop is lost (score S_SENT) */
    u8 sentinel;

    if (D_800A3208 == 0) {
        idx = D_800A379C;
        slot = &D_80101BF0[idx];
        if (MASK1) {
            sentinel = 0xFF;
loop:
            if (slot->unk_0 != sentinel) {
                D_800A379C = idx + 1;
                idx = idx + 1;
                slot++;
                if (MASK1) {
                    goto loop;
                }
            }
        }
        if (D_800A379C != 0x20) {
            u8 tmp = D_800A36F8;
            slot->unk_1 = arg0;
            slot->unk_2 = arg1;
            slot->unk_0 = tmp;
            slot->unk_4[0] = arg2[0];
            slot->unk_4[1] = arg2[1];
            slot->unk_4[2] = arg2[2];
            if (arg3 != NULL) {
                slot->unk_A[0] = arg3[0];
                slot->unk_A[1] = arg3[1];
                slot->unk_A[2] = arg3[2];
            }
        }
    }
}
"""

B_39680 = """void func_80039680(Unk80101EC8Record *a0) {
    s16 idx;
    Rec1C *dest;

    idx = a0->index;
    dest = &D_800A36EC[D_800A36F8][idx];

    dest->h4 = a0->unk_F4.x;
    dest->h8 = a0->unk_F4.z;
    dest->h6 = a0->unk_F4.y;

    {
        u16 v = a0->unk_1C8.vy;
        u8 b = a0->unk_B3;
        dest->hA = (v & 0xFFF) | (b << 12);
    }

    dest->hC = a0->unk_148;
    dest->b14 = a0->unk_1E6 >> 2;
    dest->b15 = a0->unk_1E8 >> 2;
    dest->b16 = a0->unk_1EA >> 2;
    dest->w0 = a0->unk_50;
    dest->b17 = 0;

    if (a0->unk_60 != 0) {
        dest->b17 = 1;
    }
    if (a0->unk_61 != 0) {
        dest->b17 |= 2;
    }

    dest->hE = a0->unk_64;
    dest->h10 = a0->unk_66;
    dest->h12 = a0->unk_68;
    dest->b18 = a0->unk_62;
    dest->b19 = a0->unk_40;
}
"""


def b_3993c(b):
    # the carried FAKE / claim comments get their measured scores (abl_w2b2.ps1)
    b = rep(b, "0x88 keeps the frame (MATRIX[2] / MATRIX[4] measured different) */",
            "0x88 keeps the frame (MATRIX[2]: score 58; MATRIX[4] also differs) */")
    b = rep(b, "binding it to a `rec` local does not match. */", "binding it to a `rec` local does not match (score 20). */")
    b = rep(b, "the target adds 1 to the counter first (addiu; subu). */",
            "the target adds 1 to the counter first (addiu; subu); unnamed: score 2. */")
    b = rep(b, "    u8 *p;\n    Unk80101EC8Record *rob;\n    u8 *e;\n",
            "    Rec1C *p;\n    Unk80101EC8Record *rob;\n    Unk80101BF0Rec *e;\n    Unk800F68E0Rec *s;\n")
    b = rep(b, """    func_8001BAE4((u8 *)(D_800A36EC + idx * 56) + D_800A3748 * 28,
                  D_800A3748 == 0 ? (u8 *)(D_800A36EC + idx * 56) + 0x1C : (u8 *)(D_800A36EC + idx * 56), prog);
    func_8001BBD8((u8 *)(D_800A36EC + idx * 56) + D_800A3748 * 28,
                  D_800A3748 == 0 ? (u8 *)(D_800A36EC + idx * 56) + 0x1C : (u8 *)(D_800A36EC + idx * 56), prog);
""", """    func_8001BAE4(&D_800A36EC[idx][D_800A3748],
                  D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], prog);
    func_8001BBD8(&D_800A36EC[idx][D_800A3748],
                  D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], prog);
""")
    if "rec_local" in OPT:   # re-test the comment's claim with the typed frame
        b = rep(b, """    func_8001BAE4(&D_800A36EC[idx][D_800A3748],
                  D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], prog);
    func_8001BBD8(&D_800A36EC[idx][D_800A3748],
                  D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], prog);
""", """    rec = D_800A36EC[idx];
    func_8001BAE4(&rec[D_800A3748], D_800A3748 == 0 ? &rec[1] : &rec[0], prog);
    func_8001BBD8(&rec[D_800A3748], D_800A3748 == 0 ? &rec[1] : &rec[0], prog);
""")
        b = rep(b, "    Rec1C *p;\n", "    Rec1C *p;\n    Rec1C *rec;\n")
    if "a_next" in OPT:      # ablation: the existing named-intermediate FAKE
        b = rep(b, "        s32 next = D_800A37D0 + 1;\n        temp = D_800A36F8 - next;\n",
                "        temp = D_800A36F8 - (D_800A37D0 + 1);\n")
    b = rep(b, "        p = (u8 *)(D_800A36EC + idx * 56) + i * 28;\n", "        p = &D_800A36EC[idx][i];\n")
    b = rep(b, "        func_800198D0((*(s16 *)(p + 0xE) >> 14) & 3, *(s16 *)(p + 0xE) & 0x3FFF, &work[0], sp1C0);\n"
               "        func_800198D0((*(s16 *)(p + 0x10) >> 14) & 3, *(s16 *)(p + 0x10) & 0x3FFF, &work[1], sp1C0);\n",
            "        func_800198D0((p->hE >> 14) & 3, p->hE & 0x3FFF, &work[0], sp1C0);\n"
            "        func_800198D0((p->h10 >> 14) & 3, p->h10 & 0x3FFF, &work[1], sp1C0);\n")
    b = rep(b, "(u8 *)&work[1], *(s16 *)(p + 0x12), (MATRIX *)sp120);", "(u8 *)&work[1], p->h12, (MATRIX *)sp120);")
    if "a_sp120" in OPT:     # ablation: the existing frame-layout FAKE (the 0x40 bytes written)
        b = rep(b, "    s32 sp120[34];\n", "    MATRIX sp120[2];\n")
        b = rep(b, "(MATRIX *)sp120);", "sp120);")
    b = rep(b, """        pos[0] = *(s16 *)(p + 4);
        pos[1] = *(s16 *)(p + 6);
        pos[2] = *(s16 *)(p + 8);
        rot[0] = 0;
        rot[1] = *(u16 *)(p + 0xA);
""", """        pos[0] = p->h4;
        pos[1] = p->h6;
        pos[2] = p->h8;
        rot[0] = 0;
        rot[1] = p->hA;
""")
    b = rep(b, "func_80040D48(i, 1, pos, rot, 0, *(s16 *)(p + 0xC));", "func_80040D48(i, 1, pos, rot, 0, p->hC);")
    b = b.replace("*(u8 *)(p + 0x18)", "p->b18").replace("*(u8 *)(p + 0x17)", "p->b17")
    b = rep(b, "rob->unk_40 = *(u8 *)(p + 0x19);", "rob->unk_40 = p->b19;")
    b = rep(b, "*(u16 *)(*(s32 *)p + 4)", "p->w0->unk_04", 2)
    b = rep(b, "(*(u16 *)(p + 0xA) >> 12) & 7", "(p->hA >> 12) & 7")
    b = rep(b, """    e = &D_80101BF0;
    for (i = 0; i < 0x20; i++, e += 0x10) {
        if (e[0] == idx) {
            pos[0] = *(s16 *)(e + 4);
            pos[1] = *(s16 *)(e + 6);
            pos[2] = *(s16 *)(e + 8);
            rot[0] = *(u16 *)(e + 0xA);
            rot[1] = *(u16 *)(e + 0xC);
            rot[2] = *(u16 *)(e + 0xE);
            D_800A3208 = 1;
            func_80032854(e[1], e[2], pos, rot);
""", """    e = D_80101BF0;
    for (i = 0; i < 0x20; i++, e++) {
        if (e->unk_0 == idx) {
            pos[0] = e->unk_4[0];
            pos[1] = e->unk_4[1];
            pos[2] = e->unk_4[2];
            rot[0] = e->unk_A[0];
            rot[1] = e->unk_A[1];
            rot[2] = e->unk_A[2];
            D_800A3208 = 1;
            func_80032854(e->unk_1, e->unk_2, pos, rot);
""")
    b = rep(b, """    e = (u8 *)D_800F68E0;
    for (i = 0; i < 0xB4; i++, e += 0x10) {
        if (*(s16 *)e >= temp && *(s16 *)e - e[2] <= temp) {
            pos[0] = *(s16 *)(e + 4);
            pos[1] = *(s16 *)(e + 6);
            pos[2] = *(s16 *)(e + 8);
            rot[0] = *(u16 *)(e + 0xA);
            rot[1] = *(u16 *)(e + 0xC);
            rot[2] = *(u16 *)(e + 0xE);
            func_80049718(e[3], 1, pos, rot);
""", """    s = D_800F68E0;
    for (i = 0; i < 0xB4; i++, s++) {
        if (s->unk_0 >= temp && s->unk_0 - s->unk_2 <= temp) {
            pos[0] = s->unk_4[0];
            pos[1] = s->unk_4[1];
            pos[2] = s->unk_4[2];
            rot[0] = s->unk_A;
            rot[1] = s->unk_C;
            rot[2] = s->unk_E;
            func_80049718(s->unk_3, 1, pos, rot);
""")
    return b


# measured ablation scores (abl_w2b2.ps1)
SCORE = {"fill": "2", "sent": "4"}


def c28708(s):
    s = rep(s, "extern s32 D_800A36EC;\n", "extern Rec1C (*D_800A36EC)[2];\n")
    s = rep(s, "\ns32 D_800A36EC;\n", "\nRec1C (*D_800A36EC)[2];\n")
    b392c8 = B_392C8
    if "fill" not in OPT:    # HEAD's constant-holder FAKE ablates to IDENTICAL with the typed loops: removed
        b392c8 = rep(b392c8, "    /* FAKE: constant holder for the 0xFF fill; with the literal, `li 255` is scheduled after `li 0x1F0` (score 2) */\n", "")
        b392c8 = rep(rep(rep(b392c8, "    u8 fill;\n", ""), "    fill = 0xFF;\n", ""), "unk_0 = fill;", "unk_0 = 0xFF;")
    s = body(s, "func_800392C8", b392c8)
    b39320 = B_39320 if "nv_cast" in OPT else rep(B_39320, "if ((s16)newval - q->unk_2", "if (newval - q->unk_2")
    if "a_fill_lit" in OPT:      # ablation: the 0xFF literal at the store
        b39320 = rep(b39320, "    fill = 0xFF;\n", "")
        b39320 = rep(b39320, "            p->unk_0 = fill;\n", "            p->unk_0 = 0xFF;\n")
    if "a_fill_newval" in OPT:   # ablation: HEAD's newval doubling as the holder
        b39320 = rep(b39320, "    fill = 0xFF;\n", "    newval = 0xFF;\n")
        b39320 = rep(b39320, "            p->unk_0 = fill;\n", "            p->unk_0 = newval;\n")
    b39320 = b39320.replace("S_FILL", SCORE["fill"])
    s = body(s, "func_80039320", b39320)
    s = body(s, "func_800393C8", B_393C8)
    b395b4 = B_395B4.replace("MASK1", "(u32)idx < 0x20U" if "mask_u32" in OPT else "idx < 0x20")
    if "a_sent_lit" in OPT:      # ablation: the 0xFF literal in the compare
        b395b4 = rep(b395b4, "            sentinel = 0xFF;\n", "")
        b395b4 = rep(b395b4, "            if (slot->unk_0 != sentinel) {\n", "            if (slot->unk_0 != 0xFF) {\n")
    b395b4 = b395b4.replace("S_SENT", SCORE["sent"])
    s = body(s, "func_800395B4", b395b4)
    s = body(s, "func_80039680", B_39680)
    s = fn(s, "func_8003993C", b_3993c)
    return s


# ------------------------------------------------------------------ 9F9C.c (Rec1C consumers)
def c9f9c(s):
    s = rep(s, "void func_8001BAE4(s32 *arg0, s32 *arg1, s32 arg2) {", "void func_8001BAE4(Rec1C *arg0, Rec1C *arg1, s32 arg2) {")
    s = rep(s, """    temp_a2 = ratan2(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4),
                             *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8));""",
            """    temp_a2 = ratan2(arg1->h4 - arg0->h4, arg1->h8 - arg0->h8);""")
    s = rep(s, "    func_8001B748(&D_800F6608, arg0, arg1, (s32 *)arg2, var_s3, (0x500 - temp_a2) - (var_v0 >> 2));",
            "    func_8001B748(&D_800F6608, arg0, arg1, arg2, var_s3, (0x500 - temp_a2) - (var_v0 >> 2));")
    s = rep(s, "void func_8001BBD8(s32 *arg0, s32 *arg1, s32 *arg2) {", "void func_8001BBD8(Rec1C *arg0, Rec1C *arg1, s32 arg2) {")
    s = rep(s, "-0x200 - ratan2(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4), *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8)));",
            "-0x200 - ratan2(arg1->h4 - arg0->h4, arg1->h8 - arg0->h8));")
    s = rep(s, "void func_8001F1C4(Unk80101EC8Record *arg0, u8 *arg1, MotionFrame *arg2, MotionFrame *arg3) {",
            "void func_8001F1C4(Unk80101EC8Record *arg0, Rec1C *arg1, MotionFrame *arg2, MotionFrame *arg3) {")
    s = rep(s, "    if (!(*(u8 *)(arg1 + 0x18) & 0x80)) {", "    if (!(arg1->b18 & 0x80)) {")
    s = rep(s, "*(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);", "arg1->b14 * 4, arg1->b15 * 4, 0);", 2)
    s = rep(s, "*(s8 *)(arg1 + 0x16) * 4;", "arg1->b16 * 4;", 4)
    return s


# ------------------------------------------------------------------ 17AFC.c (F18)
# func_8002EECC: the 3x3 inverse (cofactors / determinant) of the s16 rotation part of a MATRIX
# into another MATRIX; both callers pass MATRIX storage (bb2.h keeps the void * prototype: the
# callers' s32 * / s32[8] holders stay as they are).
MIDX = {0: "m[0][0]", 2: "m[0][1]", 4: "m[0][2]", 6: "m[1][0]", 8: "m[1][1]", 0xA: "m[1][2]",
        0xC: "m[2][0]", 0xE: "m[2][1]", 0x10: "m[2][2]"}


def b_2eecc(b):
    b = rep(b, "void func_8002EECC(void *arg0, void *arg1) {\n",
            "void func_8002EECC(void *arg0, void *arg1) {\n    MATRIX *src = arg0;\n    MATRIX *dst = arg1;\n")
    b = re.sub(r"\*\(s16 \*\)\(\(u8 \*\)arg0 \+ (0x[0-9A-F]+|\d+)\)", lambda m: "src->" + MIDX[int(m.group(1), 0)], b)
    b = re.sub(r"\*\(s16 \*\)\(\(u8 \*\)arg1 \+ (0x[0-9A-F]+|\d+)\)", lambda m: "dst->" + MIDX[int(m.group(1), 0)], b)
    if "arg0 +" in b or "arg1 +" in b:
        raise SystemExit("raw site left in func_8002EECC")
    if "eecc_casts" not in OPT:   # the m2c (s16) / (s32) value casts on the edited lines are no-ops
        b = b.replace("(s16) (", "(").replace("(s32) (", "(")
    return b


def c17afc(s):
    return fn(s, "func_8002EECC", b_2eecc)


FILES = [("include/game.h", game_h), ("include/bb2.h", bb2_h), ("src/main/28708.c", c28708),
         ("src/main/9F9C.c", c9f9c), ("src/main/17AFC.c", c17afc)]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b2"])[0]
    for p, g in FILES:
        s = g(show(p))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b2 wrote %d files to %s %s" % (len(FILES), "the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
