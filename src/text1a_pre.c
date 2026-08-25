#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"
#include "gpu.h"


extern s16 D_800EED10[];
extern s32 D_800EED1C[];
extern s32 D_800EED18;
extern s32 D_800EED14;
extern s32 D_800EED00[];
extern s32 D_800A33AC;
extern s32 D_800A33A0;
extern s32 D_800A33A4;
extern s32 D_800A33A8;
extern u8 D_800A9D10;
extern void func_80049E1C(void);
extern void func_80052C10(void);
extern void func_80044098(s16);
extern void func_80044010(s32 *, s16);
extern s32 D_800A3240;
extern s32 D_800A3398;
extern s32 D_800A3244;
extern s16 D_800963EE;
extern void func_8003EDC0(s32 *, s32);
extern void func_80054410(s32 *);
extern s32 func_800457A0(s32);
extern void func_80041430(s32, s32);
extern s32 func_8004019C(s32 *, s32);
/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */

extern s32 D_800A378C;
extern s32 D_800A3234;
extern s32 D_800A3378;
extern u8 D_800A9830;
extern u8 D_800A9920;
extern u16 D_80094AF4;
extern u8 D_80094B48[];
extern u8 D_80094D40[];
extern void SetDrawMove(s32, s16 *, s32, s32);

void func_800401CC(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    u16 *tbl;
    s16 u, v;
    OTag *pkt;
    OTag *ot;

    a2 = D_800A36AC & 1;
    if (a2 != D_800A3234) {
        D_800A3378 = (s32)(&D_800A9830 + a2 * 240);
        D_800A3234 = a2;
    }
    if ((s32 *)D_800A3378 != (s32 *)(&D_800A9920 + D_800A3234 * 240)) {
        tbl = &D_80094AF4 + a1 * 6;
        buf[0] = *tbl++;
        buf[1] = *tbl++;
        buf[2] = *tbl++;
        buf[3] = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf[0] = buf[0] + 0x80;
            u = u + 0x80;
        }
        SetDrawMove((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (OTag *)D_800A3378;
        ot = (OTag *)D_800A378C;
        pkt->addr = ot[0x3FFC / 4].addr;
        ot[0x3FFC / 4].addr = (u32)pkt;
        D_800A3378 = (s32)(pkt + 6);
    }
}
void func_80040304(s32 a0, s32 a1) {
    s32 ptr;
    s32 mask;
    s32 i;

    ptr = func_8004153C(a0);
    if (ptr != 0) {
        switch (a1) {
        case 0:
            mask = 0x05;
            break;
        case 1:
            mask = 0x09;
            break;
        case 2:
            mask = 0x11;
            break;
        case 3:
            mask = 0x06;
            break;
        case 4:
            mask = 0x0A;
            break;
        case 5:
            mask = 0x12;
            break;
        case 6:
            func_800401CC(a0, 5);
            func_800401CC(a0, 6);
            mask = 0;
            break;
        }
        mask = mask & D_80094B48[*(s16 *)(ptr + 8)];
        i = 0;
        do {
            if (mask & 1) {
                func_800401CC(a0, 4 - i);
            }
            i++;
            mask >>= 1;
        } while (i < 5);
    }
}
void func_80040400(s32 *a0, s16 *a1, s16 a2) {
    s32 v0;
    if (a1[1] == -1) {
        v0 = 2;
        goto init;
    }
    {
        s32 v1 = -1;
        a1 = (s16 *)((u8 *)a1 + 0x68);
        do {
            v0 = a1[1];
            a1 = (s16 *)((u8 *)a1 + 0x68);
        } while (v0 != v1);
        a1 = (s16 *)((u8 *)a1 - 0x68);
        v0 = 2;
    }
init:
    a1[1] = v0;
    *(u8 *)a1 = 3;
    *(u8 *)((u8 *)a1 + 1) = 0;
    *(s32 *)((u8 *)a1 + 0xC) = (s32)a0 + 0x270;
    a1[3] = 1;
    a1[4] = 0;
    a1[5] = 0;
    a1[2] = a2;
    *(s32 *)((u8 *)a1 + 0x58) = 0;
    *(s16 *)((u8 *)a1 + 0x6A) = -1;
}
s32 func_8004046C(s32 a0, s32 a1) {
    s32 *base = (s32 *)func_8004153C(a0);
    return *(s32 *)((u8 *)base + a1 * 4 + 0x1A34);
}
void func_800404A0(s16 *a0, s32 a1) {
    if (a0[1] == -1) {
        return;
    }
    do {
        *(s32 *)((u8 *)a0 + 0x58) = a1;
        a0 = (s16 *)((u8 *)a0 + 0x68);
    } while (a0[1] != -1);
}
extern s32 g_player_ptrs[];
extern s32 g_player_char_ids[];
void func_800404D8(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        g_player_ptrs[i] = 0;
        g_player_char_ids[i] = 0;
    }
}
extern void func_80040594(s32 *);
extern void func_800408F8(s32 *);
extern void func_80040B44(s32 *);
extern s32 *func_80045878(s32);
extern void func_8003F824(s32 *, s32);
extern void func_8003FFC4(s32 *);
extern void func_8003E120(void);
s32 *func_80040510(s32 a0) {
    s32 *ptr;
    ptr = func_80045878(a0);
    g_player_ptrs[a0] = (s32)ptr;
    func_80040594(ptr);
    /* FAKE: loop-note ref weighting seats ptr in s0 (s0/s1 swap) */
    do { func_800408F8(ptr); func_80040B44(ptr); func_8003F824(ptr, 1); } while (0);
    func_8003FFC4(ptr);
    func_80040CB8(ptr);
    func_8003E120();
    return ptr;
}
extern s32 D_80094B88[];
void func_80040594(s32 *a0)
{
    s32 *rmd;
    s32 *sec;
    s32 count;
    s32 *ptr;
    s32 off;

    if (((s16 *)a0)[3] == 0) {
        return;
    }

    ((s16 *)a0)[3] = 0;

    if (a0[8] != 0) {
        rmd = (s32 *)a0[8];
    } else {
        rmd = (s32 *)a0[7];
    }

    count = rmd[0];
    sec = (s32 *)((s32)rmd + (((u32)rmd[count] >> 2) << 2));

    if (seq_GetState() == 0) goto call_b644;
    if (((s16 *)a0)[2] == 1) goto after_b644;

call_b644:
    func_8005B644(((s16 *)a0)[2]);

after_b644:
    if (count < 6) goto simple;

    {
        s32 prev_off = ((u32)rmd[count - 1] >> 2) << 2;
        s32 next_off = ((u32)rmd[count + 1] >> 2) << 2;
        s32 idx;
        ptr = (s32 *)((s32)rmd + prev_off);
        idx = ((s16 *)a0)[2] * 3 + 1;
        ptr = (s32 *)((s32)ptr + func_8005C2A8(ptr, idx, (s32 *)((s32)rmd + next_off)));

        if (ptr == 0) {
            func_80052C10();
        }

        a0[0] |= 2;
        goto after_select;
    }

simple:
    ptr = sec;

after_select:
    off = (s32)ptr - (s32)rmd;

    if (a0[8] != 0) {
        func_800520B8(a0[8], a0[7], off);
        rmd = (s32 *)a0[7];
        if ((a0[0] >> 1) & 1) {
            func_8005C4C0((s32)rmd - a0[8], ((s16 *)a0)[2] * 3 + 1);
        }
    }

    {
        s32 *texA = (s32 *)((s32)rmd + (((u32)rmd[1] >> 2) << 2));
        s32 *texB;
        s32 fld10 = (s32)rmd + (((u32)rmd[3] >> 2) << 2);
        texB = (s32 *)((s32)rmd + (((u32)rmd[4] >> 2) << 2));
        a0[10] = fld10;
        func_80044010(texA, ((s16 *)a0)[10]);
        func_80044010(texB, ((s16 *)a0)[11]);
    }

    {
        s32 player = ((s16 *)a0)[2];
        if (player == 1) goto case_1;
        if (player >= 2) goto done_cases;
        if (player != 0) goto done_cases;

        func_80047EE8(sec, 0);
        if (func_8003E2A0() != 0) goto done_cases;
        func_800432A0(((s16 *)a0)[10], 0, 0, -0x140, 0xE8);
        func_800480C0(sec, 0, 0, 0, -0x140, 0xF0);
        goto done_cases;

    case_1:
        func_80047FBC(sec, 0, 0x80, 0);
        if (func_8003E2A0() != player) goto case_1_else;
        func_800432A0(((s16 *)a0)[10], 0x80, 0, -0x140, 0xE8);
        func_800480C0(sec, 0, 0x80, 0, -0x140, 0xF0);
        goto case_1_done;

    case_1_else:
        func_80043398(((s16 *)a0)[10], 2, 0, 2, 0);

    case_1_done:
        func_80041AC8((s16 *)a0);
    }

done_cases:
    DrawSync(0);
    func_80041988(((s16 *)a0)[2], ((s16 *)a0)[4], D_80094B88[((s16 *)a0)[2]], (s32)sec);

    {
        s32 flags = a0[0] & (s32)0xFFE0FFFF;
        s32 bits = (D_80094B88[((s16 *)a0)[2]] & 0x1F) << 16;
        a0[0] = flags | bits;
        D_80094B88[((s16 *)a0)[2]] = 0;
    }

    DrawSync(0);
    a0[9] = a0[7] + off;
    func_80045A28(((s16 *)a0)[2], off);
}


/* kengo:HIGH  |  am_rmd/AllocRobRmd  |  220i  |  +3 near-exact */
extern s16 D_80094B96[];
extern s16 D_80094B98[];
extern s16 D_80094B9A[];
extern s16 D_80094B9C[];
extern s16 D_80094C68[];
void func_800408F8(s32 *a0) {
    s16 *tbl;
    s32 count;
    s16 idx;

    tbl = D_80094C68;
    count = 0;
    while (*tbl++ != -1) {
        count++;
    }

    idx = *(s16 *)((u8 *)a0 + 8);
    if (idx < count) {
        *(u16 *)((u8 *)a0 + 0x12) = (u16)D_80094C68[idx];
    } else {
        *(u16 *)((u8 *)a0 + 0x12) = 0x1000;
    }

    {
        u8 *p = (u8 *)a0 + 0x2C;
        u8 *base = p;
        s32 neg1 = -1;
        s32 off = 0;

        do {
            s16 v1;
            s32 prod;
            *p = 0;
            *(p + 1) = 0;
            *(s16 *)(p + 8) = 0;
            *(s16 *)(p + 2) = neg1;
            v1 = *(s16 *)((u8 *)D_80094B96 + off);
            if (v1 != neg1) {
                *(s32 *)(p + 0xC) = (s32)(base + v1 * 104);
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
        } while (off < 0xD2);
    }

    func_80040A78((s32)a0);
}
/* kengo:MED  |  my_rob/rob_life_ctrl  |  96i  |  x2 size collision */
void func_80040A78(s32 arg0) {
    s32 var_a1;
    s32 var_v1;
    s32 base_v1;

    var_a1 = 0;
    base_v1 = arg0 + 0x94;
    var_v1 = base_v1;
    *(s32 *)(arg0 + 0x18F4) = arg0 + 0x2B4;
    *(s32 *)(arg0 + 0x18F8) = arg0 + 0x24C;
    *(s32 *)(arg0 + 0x18FC) = arg0 + 0x1E4;
    *(s32 *)(arg0 + 0x1900) = arg0 + 0x454;
    *(s32 *)(arg0 + 0x1904) = arg0 + 0x3EC;
    *(s32 *)(arg0 + 0x1908) = arg0 + 0x384;
    *(s32 *)(arg0 + 0x190C) = arg0 + 0x17C;
    *(s32 *)(arg0 + 0x1910) = arg0 + 0x114;
    *(s32 *)(arg0 + 0x1914) = arg0 + 0x72C;
    *(s32 *)(arg0 + 0x1918) = arg0 + 0x6C4;
    *(s32 *)(arg0 + 0x191C) = arg0 + 0x5F4;
    *(s32 *)(arg0 + 0x1920) = arg0 + 0x58C;
    *(s32 *)(arg0 + 0x1924) = arg0 + 0x524;
    *(s32 *)(arg0 + 0x1928) = arg0 + 0xAC;
    *(s32 *)(arg0 + 0x192C) = arg0 + 0x31C;
    *(s32 *)(arg0 + 0x1930) = arg0 + 0x4BC;
    *(s32 *)(arg0 + 0x1934) = arg0 + 0x794;
    *(s32 *)(arg0 + 0x1938) = arg0 + 0x65C;
    *(s32 *)(arg0 + 0x193C) = arg0 + 0x7FC;
    *(s32 *)(arg0 + 0x1940) = arg0 + 0x864;
    do {
        *(s32 *)(arg0 + 0x1994) = var_v1 + 0x18;
        var_a1 += 1;
        var_v1 = base_v1 + var_a1 * 0x68;
        arg0 += 4;
    } while (var_a1 < 0x14);
}
typedef struct { s32 f0, f1, f2, f3; } Copy16;
typedef struct { s32 f0, f1; } Copy8;
void func_80040B44(s32 *arg0) {
    s32 seen[18];
    s32 *t5;
    s32 *t7;
    s32 *v1;
    s32 *t3;
    u16 a0_val;
    s32 i;

    t5 = (s32 *)((u8 *)arg0 + 0x10D4);
    t7 = (s32 *)((u8 *)arg0 + 0x94);
    v1 = *(s32 **)((u8 *)arg0 + 0x1C);
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
    {
        s32 *p2;
        i = 0x13;
        p2 = (s32 *)((u8 *)arg0 + 0x4C);
        do {
            *(s32 *)((u8 *)p2 + 0x1A34) = 0;
            i--;
            p2 = (s32 *)((u8 *)p2 - 4);
        } while (i >= 0);
    }

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
                s32 *slot = (s32 *)(a3 * 0x68 + (s32)t7);
                *(s16 *)((u8 *)slot + 2) = (s16)t2;
                *a1 = 1;
                *(s32 *)((t2 << 2) + (s32)arg0 + 0x1A34) = (s32)slot;
            } else {
                s32 *t0 = t5;
                s32 *a2 = (s32 *)(a3 * 0x68 + (s32)t7);
                s32 *end = (s32 *)((u8 *)a2 + 0x60);

                do {
                    *(Copy16 *)t0 = *(Copy16 *)a2;
                    a2 = (s32 *)((u8 *)a2 + 0x10);
                    t0 = (s32 *)((u8 *)t0 + 0x10);
                } while (a2 != end);

                *(Copy8 *)t0 = *(Copy8 *)a2;

                *(s16 *)((u8 *)t5 + 2) = (s16)t2;
                *(s32 *)((u8 *)t5 + 0x58) = (s32)((u8 *)t7 + a3 * 0x68);
                *(s32 *)((t2 << 2) + (s32)arg0 + 0x1A34) = (s32)t5;
                t5 = (s32 *)((u8 *)t5 + 0x68);
            }

            a0_val = *(u16 *)t3;
        } while (a0_val != 0xFFFF);
    }
done:
    *(s32 *)((u8 *)t5 + 0x58) = 0;
}
/* kengo:MED  |  my_rob/rob_calc_2d_position  |  93i */
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s16 id;
    s8 *slot = (s8 *)arg0 + 0x8B4;
    s32 i = 0;
    // FAKE (none/kind/one): the three loop-invariant constants must be held in
    // registers across the loop, as target holds them in $t4/$t3/$t2. The
    // goto-loop body carries no LICM (its loop region is rejected as phony), so
    // writing -1/3/1 as literals cannot reproduce them: measured 35 insns /
    // score 23 in grind s1 (rejected/literal-constants-no-licm-in-goto-form.c).
    s32 none;
    s32 kind;
    s32 one;
    s32 link;
    s16 *tbl;
    s32 ent;

    // FAKE: wrap emits NOTE_INSN_LOOP_BEG/END around the three constant loads.
    // Both notes act as cc1 first-pass-scheduler barriers, which keeps the
    // three single-set constant loads ahead of the link/tbl cursor
    // initialisers instead of being sunk below them (target's prologue order).
    // Natural geometry was tried first: with plain declaration order the
    // scheduler sinks all three (measured score 6, s2/p4.txt).
    do {
        none = -1;
        kind = 3;
        one = 1;
    } while (0);
    link = (s32)arg0 + 0x94;
    tbl = D_80094B9E;
    // FAKE: wrap emits loop notes around the body, so flow.c weights every
    // reference inside it by loop_depth 2. That weighting is what seats the
    // id copy in $v1 and the 0x90C cursor in $a1 (and the rest on target);
    // without it the id copy loses its allocno-priority race and the whole
    // register assignment rotates (measured score 13, s1/build4.txt).
    // `ent` is initialised INSIDE the region on purpose: that makes the region
    // start on a non-label insn, so loop.c rejects it as phony and its
    // strength reduction cannot invent a third induction pointer for the
    // -0x57..-0x4C displacement cluster (measured 38 insns / score 25 with a
    // real for-loop, s2/p2.txt).
    do {
        ent = (s32)arg0 + 0x90C;
    loop:
        id = *tbl;
        if (id != none) {
            *(s16 *)(ent - 0x56) = id;
            *slot = kind;
            *(s8 *)(ent - 0x57) = 0;
            *(s16 *)(ent - 0x50) = 0;
            *(s32 *)(ent - 0x4C) = link;
            *(s16 *)(ent - 0x52) = one;
            *(s16 *)(ent - 0x4E) = 0;
            {
                u16 w = *(u16 *)((s32)arg0 + 0x16);
                slot += 0x68;
                *(s32 *)ent = 0;
                *(s16 *)(ent - 0x54) = w;
                ent += 0x68;
            }
        }
        link += 0x68;
        i++;
        tbl = (s16 *)((s32)tbl + 0xA);
        if (i < 0x12) goto loop;
    } while (0);
    *(s16 *)((s32)slot + 2) = -1;
}
typedef void (*FuncPtr_40D48)(s16 *, s16 *);
typedef struct { s32 a, b, c, d, e, f, g, h; } Copy8_40D48;
extern s32 D_800A9A10[];
extern s32 D_80094CFC[];
extern s32 D_800A3820;
extern FuncPtr_40D48 D_800F66A0[];
extern void func_800417D0(s32 *);
extern void func_800400B0(s32 *, s32);
extern void func_8003F62C(s32 *);
extern void func_800420E8(s32, s32);
void func_80040D48(s32 a0, s32 a1, s32 *a2, s16 *a3, s16 *arg4, s32 arg5) {
    register s32 a0_s7 asm("s7") = a0;
    u8 *s4;
    u8 *s5;
    u8 *s3;
    u8 *s2;
    s32 s0;
    s16 *s1;

    s4 = (u8 *)D_800A9A10[a0_s7];
    if (s4 == 0) {
        return;
    }

    *(s16 *)(s4 + 0x3C) = a3[0];
    *(s16 *)(s4 + 0x3E) = a3[1];
    *(s16 *)(s4 + 0x40) = a3[2];

    *(s32 *)(s4 + 0x78) = a2[0];
    s5 = s4 + 0x2C;
    *(s32 *)(s4 + 0x7C) = a2[1];
    s3 = s4 + 0x94;
    *(s32 *)(s4 + 0x80) = a2[2];

    s2 = s4 + 0x7E4;

    switch (a1) {
    case 0: {
        s32 *tbl;
        u8 *a4p;
        u8 *p;
        FuncPtr_40D48 *s0_fn;
        s0 = 1;
        tbl = D_80094CFC;
        s1 = arg4;
        a4p = s3 + 0x68;

        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x50) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;

        do {
            s32 idx;
            idx = *tbl;
            *(s16 *)(a4p + 0x10) = *(u16 *)((u8 *)s1 + idx * 6);
            idx = *tbl;
            *(s16 *)(a4p + 0x12) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 2);
            idx = *tbl;
            s0++;
            tbl++;
            *(s16 *)(a4p + 0x14) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 4);
            a4p += 0x68;
        } while (s0 < 0x12);

        s0 = 0x11;
        p = s3 + 0x6E8;
        do {
            *(s16 *)(p + 6) = 0;
            s0--;
            p -= 0x68;
        } while (s0 >= 0);

        *(s32 *)(s2 + 0x4C) = *(s16 *)((u8 *)s1 + 0x6C);
        *(s32 *)(s2 + 0x50) = -(s32)*(s16 *)((u8 *)s1 + 0x6E);
        *(s32 *)(s2 + 0x54) = -(s32)*(s16 *)((u8 *)s1 + 0x70);
        *(s16 *)(s2 + 0x10) = *(u16 *)((u8 *)s1 + 0x72);
        s0_fn = D_800F66A0;
        *(s16 *)(s2 + 0x12) = -(s16)*(u16 *)((u8 *)s1 + 0x74);
        *(s16 *)(s2 + 0x14) = -(s16)*(u16 *)((u8 *)s1 + 0x76);

        (*s0_fn)((s16 *)(s2 + 0x10), (s16 *)(s2 + 0x38));

        *(s32 *)(s2 + 0xB4) = *(s16 *)((u8 *)s1 + 0x78);
        *(s32 *)(s2 + 0xB8) = -(s32)*(s16 *)((u8 *)s1 + 0x7A);
        *(s32 *)(s2 + 0xBC) = -(s32)*(s16 *)((u8 *)s1 + 0x7C);
        *(s16 *)(s2 + 0x78) = *(u16 *)((u8 *)s1 + 0x7E);
        *(s16 *)(s2 + 0x7A) = -(s16)*(u16 *)((u8 *)s1 + 0x80);
        *(s16 *)(s2 + 0x7C) = -(s16)*(u16 *)((u8 *)s1 + 0x82);

        (*s0_fn)((s16 *)(s2 + 0x78), (s16 *)(s2 + 0xA0));
        break;
    }
    case 1:
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;
        *(s16 *)(s3 + 0x06) = 0;
        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        break;
    case 2: break;
    case 3: break;
    case 4: break;
    case 5: break;
    case 6: break;
    }

    {
        s32 scaled;
        s32 *s1p;
        scaled = (*(s32 *)(s3 + 0x50) * *(s16 *)(s4 + 0x12)) >> 12;
        s0 = 0;
        s1p = (s32 *)s3;
        *(s32 *)(s3 + 0x50) = scaled;
        *(s16 *)(s5 + 6) = 0;
        do {
            func_800417D0(s1p);
            s0++;
            s1p = (s32 *)((u8 *)s1p + 0x68);
        } while (s0 < 0x12);
    }

    s0 = 1;
    *s3 = 0xA;
    *(s32 *)(s3 + 0x58) = (s32)(s4 + 0x18F4);
    {
        s32 *list;
        u8 *a4p;
        list = (s32 *)D_800A3820;
        a4p = s3 + 0x68;
        *(s16 *)(s3 + 2) = 0;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)s3;

        do {
            if (*(s16 *)(a4p + 2) >= 0) {
                s32 *list2;
                list2 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list2 + 1);
                *list2 = (s32)a4p;
            }
            s0++;
            a4p += 0x68;
        } while (s0 < 0x12);
    }

    {
        u8 *a2p;
        u8 *a3p;
        a2p = s4 + 0x10D4;
        a3p = s4 + 0x10EC;
        for (;;) {
            s32 *s5p;
            s5p = *(s32 **)(a3p + 0x40);
            if (s5p == 0) break;

            *(Copy8_40D48 *)a3p = *(Copy8_40D48 *)((u8 *)s5p + 0x18);

            {
                s32 *list3;
                list3 = (s32 *)D_800A3820;
                a3p += 0x68;
                D_800A3820 = (s32)(list3 + 1);
                *list3 = (s32)a2p;
            }
            a2p += 0x68;
        }
    }

    {
        s16 *a2p2;
        a2p2 = (s16 *)(s4 + 0x8B4);
        if (*(s16 *)(s4 + 0x8B6) != -1) {
            do {
                s32 *list4;
                list4 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list4 + 1);
                *list4 = (s32)a2p2;
                a2p2 = (s16 *)((u8 *)a2p2 + 0x68);
            } while (a2p2[1] != -1);
        }
    }

    func_800404A0((s16 *)(s4 + 0x8B4), arg5);
    *(s16 *)(s4 + 0x1A84) = (s16)arg5;
    func_800400B0((s32 *)s4, arg5);
    func_8003F62C((s32 *)s4);
    func_800420E8(a0_s7, (s32)(s3 + 0x2C));
}

extern s32 D_80094CFC[];
extern s32 D_800A9A10[];
extern void func_8004A348(s16 *, s32 *);
extern void func_800523E0(s32 *, s32 *, s32, s32);
extern void func_80044DE4(s16 *, s16 *, s32, s32);
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    register s32 *s7_a4 asm("s7") = a4;
    s32 *tbl = D_80094CFC;
    s32 i = 1;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) s7_a4) + 0x20);
    stptr = base + 0xFC;
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (a1 + offset);
    buf[0] = p[0];
    buf[1] = -p[1];
    tbl++;
    buf[2] = -p[2];
    func_8004A348(buf, s7_a4);
    p = (u16 *) (a2 + offset);
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(s7_a4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    out2 = (s32 *) (((u8 *) s7_a4) + 0x20);
    stptr = saved + 0x750;
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, s7_a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out2);
    func_800523E0(s7_a4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 1;
    stptr += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}

/* kengo:MED  |  my_hirahira/hirahira_w_ctrl  |  132i  |  x2 size collision */
extern s32 *D_80015820[];
extern s32 func_800545F4;
extern s32 D_800545F8;
extern s32 D_800545FC;
extern s32 D_80054600;
void func_80041398(s32 a0) {
    s32 **t0 = D_80015820;
    s32 t1 = 0;
    s32 mask8 = ~0xFF;
    s32 a1 = (a0 >> 16) & 0xFF;
    s32 mask16 = (s32)0xFFFF0000;
    s32 a0lo = a0 & 0xFFFF;
    s32 t3 = (func_800545F4 & mask8) | a1;
    s32 t2 = (D_800545F8 & mask16) | a0lo;
    s32 v1 = (D_800545FC & mask8) | a1;
    s32 a4 = (D_80054600 & mask16) | a0lo;
    do {
        s32 *v0 = *t0;
        t0++;
        t1++;
        v0[0] = t3;
        v0[1] = t2;
        {
            s32 *v0b = *t0;
            v0b[0] = v1;
            v0b[1] = a4;
        }
        t0++;
    } while (t1 < 4);
}
void func_80041430(s32 a0, s32 a1) {
    s32 *base;
    s32 *s0;
    s32 i;
    base = (s32 *)((u8 *)g_player_ptrs + a0 * 4);
    s0 = (s32 *)(*base + a1);
    *base = (s32)s0;
    save_vc_ctrl(a1, (s16 *)((u8 *)s0 + 0x2C), 0x15);
    save_vc_ctrl(a1, (s16 *)((u8 *)s0 + 0x8B4), 0x14);
    save_vc_ctrl(a1, (s16 *)((u8 *)s0 + 0x10D4), 0x14);
    {
        s32 *v1 = (s32 *)((u8 *)s0 + 0x112C);
        do {
            s32 val = *v1;
            if (val) {
                *v1 = val + a1;
            } else {
                break;
            }
            v1 = (s32 *)((u8 *)v1 + 0x68);
        } while (1);
    }
    i = 0;
    do {
        s32 val = *(s32 *)((u8 *)s0 + i * 4 + 0x1A34);
        if (val) {
            *(s32 *)((u8 *)s0 + i * 4 + 0x1A34) = val + a1;
        }
        i++;
    } while (i < 0x14);
    func_80040A78((s32)s0);
}
