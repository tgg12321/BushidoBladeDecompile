/* 12 game functions, among them gpu_AddDrawMove. .text 0x800401CC (ROM 0x309CC). Start boundary:
 * G8 (cc1 -G8 by proof). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include <psxsdk/libsnd.h>
#include "game.h"
#include "code6cac.h"
#include "gpu.h"













extern void func_80052C10(void);

extern void func_80044010(s32 *, s16);









/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */

extern s32 D_800A3234;
extern u16 D_80094AF4[];
extern u8 D_80094B48[];


/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
/* Cursor in the 10-packet bank selected by frame parity. */
static DR_MOVE *D_800A3378;

void gpu_AddDrawMove(s32 a0, s32 a1) {
    s32 parity;
    RECT buf;
    u16 *tbl;
    s16 u, v;
    OTag *pkt;
    OTag *ot;

    parity = D_800A36AC & 1;
    if (parity != D_800A3234) {
        D_800A3378 = D_800A9830[parity];
        D_800A3234 = parity;
    }
    if (D_800A3378 != D_800A9830[D_800A3234] + 10) {
        tbl = D_80094AF4 + a1 * 6;
        buf.x = *tbl++;
        buf.y = *tbl++;
        buf.w = *tbl++;
        buf.h = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf.x = buf.x + 0x80;
            u = u + 0x80;
        }
        SetDrawMove(D_800A3378, &buf, u, v);
        pkt = (OTag *)D_800A3378;
        /* FAKE: SDK bitfield view of an OT word retains tag length;
         * PS1 use: src/main/psxsdk/libgpu/sys.c:288. */
        /* SOTN: include/psxsdk/libgpu.h:88 @db41b28eee52969244a52cc269c8163d1ed8826a */
        ot = (OTag *)D_800A378C;
        pkt->addr = ot[0x3FFC / 4].addr;
        ot[0x3FFC / 4].addr = (u32)pkt;
        D_800A3378++;
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
            gpu_AddDrawMove(a0, 5);
            gpu_AddDrawMove(a0, 6);
            mask = 0;
            break;
        }
        mask = mask & D_80094B48[*(s16 *)(ptr + 8)];
        i = 0;
        do {
            if (mask & 1) {
                gpu_AddDrawMove(a0, 4 - i);
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
            snd_VabFakeOpen((s32)rmd - a0[8], ((s16 *)a0)[2] * 3 + 1);
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
    func_80041988(((s16 *)a0)[2], ((s16 *)a0)[4], g_player_char_ids[((s16 *)a0)[2]], (s32)sec);

    {
        s32 flags = a0[0] & (s32)0xFFE0FFFF;
        s32 bits = (g_player_char_ids[((s16 *)a0)[2]] & 0x1F) << 16;
        a0[0] = flags | bits;
        g_player_char_ids[((s16 *)a0)[2]] = 0;
    }

    DrawSync(0);
    a0[9] = a0[7] + off;
    func_80045A28(((s16 *)a0)[2], off);
}


extern s16 D_80094B96[];
extern s16 D_80094B98[];
extern s16 D_80094B9A[];
extern s16 D_80094B9C[];

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
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s16 id;
    s8 *slot = (s8 *)arg0 + 0x8B4;
    s32 i = 0;
    // FAKE (none/kind/one): the three loop-invariant constants must be held in
    // registers across the loop, as target holds them in $t4/$t3/$t2. The
    // goto-loop body carries no LICM (its loop region is rejected as phony), so
    // writing -1/3/1 as literals cannot reproduce them.
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
    // With plain declaration order the scheduler sinks all three.
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
    // register assignment rotates.
    // `ent` is initialised INSIDE the region on purpose: that makes the region
    // start on a non-label insn, so loop.c rejects it as phony and its
    // strength reduction cannot invent a third induction pointer for the
    // -0x57..-0x4C displacement cluster (as a real for-loop does).
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

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A3234 = -1;
