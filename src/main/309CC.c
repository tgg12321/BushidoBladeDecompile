/* 12 game functions, among them gpu_AddDrawMove. .text 0x800401CC (ROM
 * 0x309CC). Start boundary: G8 (cc1 -G8 by proof). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

extern s32 D_800A3234;
extern u16 D_80094AF4[];
extern u8 D_80094B48[];

/* Q65: this file's statics (.sbss, allocated per file in link order by
 * PSYLINK), in address order. */
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
        /* FAKE: SDK bitfield view of an OT word retains tag length
         * (masked u32 copies, same behaviour: score 18; whole-word copies: 19);
         * SOTN PS1 use: src/main/psxsdk/libgpu/sys.c:288 */
        /* SOTN: include/psxsdk/libgpu.h:88 @db41b28eee52969244a52cc269c8163d1ed8826a */
        ot = (OTag *)D_800A378C;
        pkt->addr = ot[0x3FFC / 4].addr;
        ot[0x3FFC / 4].addr = (u32)pkt;
        D_800A3378++;
    }
}

void func_80040304(s32 a0, s32 a1) {
    Unk80045878Obj *ptr;
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
        mask = mask & D_80094B48[ptr->unk_08];
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

void func_80040400(Unk80045878Node *a0, Unk80045878Node *a1, s16 a2) {
    while (a1->node.unk2 != -1) {
        a1++;
    }
    a1->node.unk2 = 2;
    a1->node.unk0 = 3;
    a1->node.unk1 = 0;
    a1->node.unkC = &a0[6].node;
    a1->node.unk6 = 1;
    a1->node.unk8 = 0;
    a1->node.unkA = 0;
    a1->node.unk4 = a2;
    a1->unk58 = 0;
    a1[1].node.unk2 = -1;
}

Unk80045878Node *func_8004046C(s32 a0, s32 a1) {
    Unk80045878Obj *base = func_8004153C(a0);
    return base->unk_1A34[a1];
}

void func_800404A0(Unk80045878Node *a0, s32 a1) {
    if (a0->node.unk2 == -1) {
        return;
    }
    do {
        a0->unk58 = a1;
        a0++;
    } while (a0->node.unk2 != -1);
}

void func_800404D8(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        g_player_ptrs[i] = 0;
        g_player_char_ids[i] = 0;
    }
}

extern void func_80040594(Unk80045878Obj *);
extern void func_800408F8(Unk80045878Obj *);
extern void func_80040B44(Unk80045878Obj *);
/* Not the definition's spelling: snd_VabFakeOpen takes an s16 second parameter,
 * but this call passes it unextended (an s16 prototype adds sll/sra). */
extern s32 snd_VabFakeOpen(s32, s32);

Unk80045878Obj *func_80040510(s32 a0, s32 a1, s32 a2) {
    Unk80045878Obj *ptr;
    ptr = func_80045878(a0, a1, a2);
    g_player_ptrs[a0] = ptr;
    func_80040594(ptr);
    func_800408F8(ptr);
    func_80040B44(ptr);
    func_8003F824(ptr, 1);
    func_8003FFC4(ptr);
    func_80040CB8(ptr);
    func_8003E120();
    return ptr;
}

void func_80040594(Unk80045878Obj *a0) {
    s32 *rmd;
    s32 *sec;
    s32 count;
    s32 *ptr;
    s32 off;

    if (a0->unk_06 == 0) {
        return;
    }

    a0->unk_06 = 0;

    if (a0->unk_20 != 0) {
        rmd = (s32 *)a0->unk_20;
    } else {
        rmd = (s32 *)a0->unk_1C;
    }

    count = rmd[0];
    sec = (s32 *)((s32)rmd + (((u32)rmd[count] >> 2) << 2));

    if (seq_GetState() == 0)
        goto call_b644;
    if (a0->unk_04 == 1)
        goto after_b644;

call_b644:
    func_8005B644(a0->unk_04);

after_b644:
    if (count < 6)
        goto simple;

    {
        ptr = (s32 *)((s32)rmd + (((u32)rmd[count - 1] >> 2) << 2));
        ptr =
            (s32 *)((s32)ptr +
                    func_8005C2A8(
                        ptr, a0->unk_04 * 3 + 1,
                        (s32 *)((s32)rmd + (((u32)rmd[count + 1] >> 2) << 2))));

        if (ptr == 0) {
            func_80052C10();
        }

        a0->unk_00.word |= 2;
        goto after_select;
    }

simple:
    ptr = sec;

after_select:
    off = (s32)ptr - (s32)rmd;

    if (a0->unk_20 != 0) {
        func_800520B8(a0->unk_20, a0->unk_1C, off);
        rmd = (s32 *)a0->unk_1C;
        if ((a0->unk_00.word >> 1) & 1) {
            snd_VabFakeOpen((s32)rmd - a0->unk_20, a0->unk_04 * 3 + 1);
        }
    }

    {
        /* FAKE: texA / texB computed ahead of the unk_28 store; computed in
         * the calls they move below it and the s-registers re-seat: score 72 */
        s32 *texA = (s32 *)((s32)rmd + (((u32)rmd[1] >> 2) << 2));
        s32 *texB;
        texB = (s32 *)((s32)rmd + (((u32)rmd[4] >> 2) << 2));
        a0->unk_28 = (s32)rmd + (((u32)rmd[3] >> 2) << 2);
        func_80044010(texA, a0->unk_14);
        func_80044010(texB, a0->unk_16);
    }

    {
        if (a0->unk_04 == 1)
            goto case_1;
        if (a0->unk_04 >= 2)
            goto done_cases;
        if (a0->unk_04 != 0)
            goto done_cases;

        func_80047EE8(sec, 0);
        if (func_8003E2A0() != 0)
            goto done_cases;
        func_800432A0(a0->unk_14, 0, 0, -0x140, 0xE8);
        func_800480C0((s32)sec, 0, 0, 0, -0x140, 0xF0);
        goto done_cases;

    case_1:
        func_80047FBC(sec, 0, 0x80, 0);
        if (func_8003E2A0() != 1)
            goto case_1_else;
        func_800432A0(a0->unk_14, 0x80, 0, -0x140, 0xE8);
        func_800480C0((s32)sec, 0, 0x80, 0, -0x140, 0xF0);
        goto case_1_done;

    case_1_else:
        func_80043398(a0->unk_14, 2, 0, 2, 0);

    case_1_done:
        func_80041AC8(a0);
    }

done_cases:
    DrawSync(0);
    func_80041988(
        a0->unk_04, a0->unk_08, g_player_char_ids[a0->unk_04], (s32)sec);

    {
        a0->unk_00.word = (a0->unk_00.word & 0xFFE0FFFF) |
                          ((g_player_char_ids[a0->unk_04] & 0x1F) << 16);
        g_player_char_ids[a0->unk_04] = 0;
    }

    DrawSync(0);
    a0->unk_24 = (void *)(a0->unk_1C + off);
    func_80045A28(a0->unk_04, off);
}

extern Unk80094B96Rec D_80094B96[21];

void func_800408F8(Unk80045878Obj *a0) {
    s16 *tbl;
    s32 count;
    s16 idx;

    tbl = D_80094C68;
    count = 0;
    while (*tbl++ != -1) {
        count++;
    }

    idx = a0->unk_08;
    if (idx < count) {
        a0->unk_12 = D_80094C68[idx];
    } else {
        a0->unk_12 = 0x1000;
    }

    {
        Unk80045878Node *p = a0->unk_2C;
        /* FAKE: base keeps the array start (move t1,a3); &a0->unk_2C[v1]
         * rebuilds it from a0 (addiu 44 + addu) and the mflo temps move to t1
         * (score 9) */
        Unk80045878Node *base = p;
        s32 i = 0;

        do {
            s16 v1;
            p->node.unk0 = 0;
            p->node.unk1 = 0;
            p->node.unk8 = 0;
            p->node.unk2 = -1;
            v1 = D_80094B96[i].unk_00;
            if (v1 != -1) {
                p->node.unkC = &base[v1].node;
            } else {
                p->node.unkC = 0;
            }
            p->node.work.t[0] = (D_80094B96[i].unk_02[0] * a0->unk_12) >> 12;
            p->node.work.t[1] = (D_80094B96[i].unk_02[1] * a0->unk_12) >> 12;
            p->node.work.t[2] = (D_80094B96[i].unk_02[2] * a0->unk_12) >> 12;
            p->node.xf.rot.vx = 0;
            p->node.xf.rot.vy = 0;
            p->node.xf.rot.vz = 0;
            p->node.unk6 = 0;
            p->node.unkA = a0->unk_10;
            p->node.unk4 = a0->unk_14;
            p++;
            i++;
        } while (i < 21);
    }

    func_80040A78(a0);
}

void func_80040A78(Unk80045878Obj *arg0) {
    s32 var_a1;
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
    n = &arg0->unk_2C[1];
    do {
        arg0->unk_1994[var_a1] = &n->node.xf.mat;
        n++;
        var_a1 += 1;
    } while (var_a1 < 0x14);
}

void func_80040B44(Unk80045878Obj *arg0) {
    s32 seen[18];
    Unk80045878Node *t5;
    Unk80045878Node *t7;
    s32 *v1;
    u16 *t3;
    u16 a0_val;
    s32 i;

    t5 = arg0->unk_10D4;
    t7 = &arg0->unk_2C[1];
    v1 = (s32 *)arg0->unk_1C;
    t3 = (u16 *)((u8 *)v1 + v1[2]);

    i = 0x11;
    do {
        seen[i] = 0;
        i--;
    } while (i >= 0);
    i = 0x13;
    do {
        arg0->unk_1A34[i] = 0;
        i--;
    } while (i >= 0);

    a0_val = *t3;
    if (a0_val == 0xFFFF)
        goto done;

    {
        do {
            s32 a3;
            s32 *a1;
            s32 t2;

            t3++;
            a3 = *t3;
            t3++;
            /* FAKE: the slot address as integer arithmetic; &seen[a3] swaps the
             * addu operands (score 1) */
            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);
            /* FAKE: t2 copies a0_val ahead of the seen[] test; with a0_val
             * itself the load and the cursor step re-seat (score 9). */
            t2 = a0_val;

            if (*a1 == 0) {
                t7[a3].node.unk2 = t2;
                *a1 = 1;
                arg0->unk_1A34[t2] = &t7[a3];
            } else {
                *t5 = t7[a3];
                t5->node.unk2 = t2;
                t5->unk58 = (s32)&t7[a3];
                arg0->unk_1A34[t2] = t5;
                t5++;
            }

            a0_val = *t3;
        } while (a0_val != 0xFFFF);
    }
done:
    t5->unk58 = 0;
}

void func_80040CB8(Unk80045878Obj *arg0) {
    s16 id;
    Unk80045878Node *slot = arg0->unk_8B4;
    s32 i = 0;
    // FAKE (none/kind/one): constant-holders keep -1/3/1 in $t4/$t3/$t2 across
    // the goto-loop, which gets no LICM; literals: score 23
    s32 none;
    s32 kind;
    s32 one;
    s32 link;
    s16 *tbl;
    s32 ent;

    // FAKE: do-while(0) keeps the three constant loads ahead of the link/tbl
    // initialisers; without it the scheduler sinks them: score 6
    do {
        none = -1;
        kind = 3;
        one = 1;
    } while (0);
    link = (s32)&arg0->unk_2C[1];
    /* FAKE: tbl walks the records' unk_08 members, stepped by a record; a
       record pointer: score 2, D_80094B96[i].unk_08: score 21 */
    tbl = &D_80094B96[0].unk_08;
    // FAKE: do-while(0) raises the loop depth that weights register allocation
    // (seats id in $v1, the cursor in $a1); ent is set inside so the region is
    // not a real loop and gets no extra induction pointer; without: score 21
    do {
        ent = (s32)&arg0->unk_8B4[0].unk58;
    loop:
        id = *tbl;
        if (id != none) {
            *(s16 *)(ent - 0x56) = id;
            slot->node.unk0 = kind;
            *(s8 *)(ent - 0x57) = 0;
            *(s16 *)(ent - 0x50) = 0;
            *(s32 *)(ent - 0x4C) = link;
            *(s16 *)(ent - 0x52) = one;
            *(s16 *)(ent - 0x4E) = 0;
            {
                /* FAKE: w reads unk_16 ahead of the slot step and the *ent
                 * store; read at its store, sw zero moves above it: score 2 */
                u16 w = arg0->unk_16;
                slot++;
                *(s32 *)ent = 0;
                *(s16 *)(ent - 0x54) = w;
                ent += 0x68;
            }
        }
        link += 0x68;
        i++;
        tbl = (s16 *)((u8 *)tbl + sizeof(Unk80094B96Rec));
        if (i < 0x12)
            goto loop;
    } while (0);
    slot->node.unk2 = -1;
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 D_800A3234 = -1;
