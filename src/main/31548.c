/* 4 game functions. .text 0x80040D48 (ROM 0x31548). Start boundary: PHASE
 * (rodata-align site 4). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

extern s32 D_80094CFC[];

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
    /* FAKE: s5 holds s4 + 0x2C here and the linked record pointer in the copy
     * loop below; a fresh loop local scores 35, dropping this first role 46. */
    s5 = s4->unk_2C;
    s4->unk_2C[0].node.work.t[1] = a2[1];
    s3 = &s4->unk_2C[1];
    s4->unk_2C[0].node.work.t[2] = a2[2];

    s2 = &s4->unk_2C[19];

    switch (a1) {
    case 0: {
        s32 *tbl;
        Unk80045878Node *p;
        /* FAKE: s0 is the counter of all four loops in this function; a counter
         * per loop scores 15. */
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
            /* FAKE: idx re-read from *tbl before each component, as the target
             * reloads it and recomputes idx * 6; one read computes it once
             * (score 48) */
            idx = *tbl;
            a4p->node.xf.rot.vx = s1[idx * 3];
            idx = *tbl;
            a4p->node.xf.rot.vy = -s1[idx * 3 + 1];
            idx = *tbl;
            a4p->node.xf.rot.vz = -s1[idx * 3 + 2];
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

        s2->node.work.t[0] = s1[0x36];
        s2->node.work.t[1] = -s1[0x37];
        s2->node.work.t[2] = -s1[0x38];
        s2->node.xf.rot.vx = s1[0x39];
        s2->node.xf.rot.vy = -s1[0x3A];
        s2->node.xf.rot.vz = -s1[0x3B];

        g_anim_func_table[0](&s2->node.xf.rot, &s2->node.work);

        s2[1].node.work.t[0] = s1[0x3C];
        s2[1].node.work.t[1] = -s1[0x3D];
        s2[1].node.work.t[2] = -s1[0x3E];
        s2[1].node.xf.rot.vx = s1[0x3F];
        s2[1].node.xf.rot.vy = -s1[0x40];
        s2[1].node.xf.rot.vz = -s1[0x41];

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
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    }

    {
        Unk80045878Node *s1p;
        s0 = 0;
        s1p = s3;
        s3->node.work.t[1] = (s3->node.work.t[1] * s4->unk_12) >> 12;
        s5->node.unk6 = 0;
        do {
            func_800417D0(&s1p->node);
            s0++;
            s1p++;
        } while (s0 < 0x12);
    }

    s0 = 1;
    s3->node.unk0 = 0xA;
    /* FAKE: unk58 stored through a pointer; the member store lets sched sink it
       below the g_draw_queue_cursor load (score 5). */
    {
        s32 *p58 = &s3->unk58;
        *p58 = (s32)s4->unk_18F4;
    }
    {
        void **list;
        Unk80045878Node *a4p;
        list = g_draw_queue_cursor;
        a4p = &s3[1];
        s3->node.unk2 = 0;
        g_draw_queue_cursor = list + 1;
        *list = s3;

        do {
            if (a4p->node.unk2 >= 0) {
                void **list2;
                list2 = g_draw_queue_cursor;
                g_draw_queue_cursor = list2 + 1;
                *list2 = a4p;
            }
            s0++;
            a4p++;
        } while (s0 < 0x12);
    }

    {
        Unk80045878Node *a2p;
        a2p = s4->unk_10D4;
        for (;;) {
            void **list3;
            s5 = (Unk80045878Node *)a2p->unk58;
            if (s5 == 0) {
                break;
            }
            a2p->node.xf.mat = s5->node.xf.mat;
            list3 = g_draw_queue_cursor;
            g_draw_queue_cursor = list3 + 1;
            *list3 = a2p;
            a2p++;
        }

        a2p = s4->unk_8B4;
        if (s4->unk_8B4[0].node.unk2 != -1) {
            do {
                void **list4;
                list4 = g_draw_queue_cursor;
                g_draw_queue_cursor = list4 + 1;
                *list4 = a2p;
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

extern s32 D_80094CFC[];

void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, MATRIX *a4) {
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    Unk80045878Obj *base = g_player_ptrs[a0];
    SVECTOR buf;
    Unk80045878Node *ents;
    MATRIX *out2;
    MATRIX *out3;
    s32 offset;
    u16 *p;
    Unk80045878Node *stptr2;
    ents = &base->unk_2C[1];
    out2 = a4 + 1;
    do {
        offset = (*tbl) * 6;
        p = (u16 *)(offset + (s32)a1);
        buf.vx = p[0];
        buf.vy = -p[1];
        buf.vz = -p[2];
        math_RotMatrixZYX(&buf, a4);
        tbl++;
        /* FAKE: offset becomes the a2-side address (addu s0,s0,s2); a fresh sum
         * loads through v1 (score 4) */
        offset = offset + (s32)a2;
        p = (u16 *)offset;
        buf.vx = p[0];
        buf.vy = -p[1];
        buf.vz = -p[2];
        math_RotMatrixZYX(&buf, out2);
        func_800523E0(a4, out2, a3, &ents[i].node.work);
        ents[i].node.unk6 = 2;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = &ents[18];
    /* FAKE: a second copy of a4 + 1 for the second loop (s3); reusing out2
     * drops the copy and moves lw t0,24(sp) up (score 4) */
    out3 = a4 + 1;
loop2:
    func_80044DE4((s16 *)a1, (s16 *)a2, a3, (s32)stptr2->node.work.t);
    a1 += 6;
    a2 += 6;
    buf.vx = *((u16 *)a1);
    a1 += 2;
    buf.vy = -(*((u16 *)a1));
    a1 += 2;
    buf.vz = -(*((u16 *)a1));
    a1 += 2;
    math_RotMatrixZYX(&buf, a4);
    buf.vx = *((u16 *)a2);
    a2 += 2;
    buf.vy = -(*((u16 *)a2));
    a2 += 2;
    buf.vz = -(*((u16 *)a2));
    a2 += 2;
    math_RotMatrixZYX(&buf, out3);
    func_800523E0(a4, out3, a3, &stptr2->node.work);
    stptr2->node.unk6 = 1;
    stptr2++;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}

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
    Unk80045878Obj **base;
    Unk80045878Obj *s0;
    s32 i;
    base = &g_player_ptrs[a0];
    s0 = (Unk80045878Obj *)((u8 *)*base + a1);
    *base = s0;
    func_800414FC(a1, s0->unk_2C, 0x15);
    func_800414FC(a1, s0->unk_8B4, 0x14);
    func_800414FC(a1, s0->unk_10D4, 0x14);
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
