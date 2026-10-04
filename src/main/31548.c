/* 4 game functions. .text 0x80040D48 (ROM 0x31548). Start boundary: PHASE (rodata-align site 4). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

/* Declarations from the file this TU was split from (text1a_pre.c). */
void func_800404A0(s16 *a0, s32 a1);
void func_80040A78(s32 arg0);

typedef struct { s32 a, b, c, d, e, f, g, h; } Copy8_40D48;
extern s32 D_80094CFC[];
extern void func_800417D0(s32 *);
extern void func_800420E8(s32, s32);
void func_80040D48(s32 a0, s32 a1, s32 *a2, s16 *a3, s16 *arg4, s32 arg5) {
    u8 *s4;
    u8 *s5;
    u8 *s3;
    u8 *s2;
    s32 s0;
    s16 *s1;
    s32 ent;

    ent = g_player_ptrs[a0];
    if (ent == 0) {
        return;
    }
    /* FAKE: s4 copies ent as a byte pointer; using ent directly scores 4. */
    s4 = (u8 *)ent;

    *(s16 *)(s4 + 0x3C) = a3[0];
    *(s16 *)(s4 + 0x3E) = a3[1];
    *(s16 *)(s4 + 0x40) = a3[2];

    *(s32 *)(s4 + 0x78) = a2[0];
    /* FAKE: s5 holds s4 + 0x2C here and the linked record pointer in the copy loop below;
     * a fresh loop local scores 35, dropping this first role 46. */
    s5 = s4 + 0x2C;
    *(s32 *)(s4 + 0x7C) = a2[1];
    s3 = s4 + 0x94;
    *(s32 *)(s4 + 0x80) = a2[2];

    s2 = s4 + 0x7E4;

    switch (a1) {
    case 0: {
        s32 *tbl;
        u8 *p;
        /* FAKE: s0 is the counter of all four loops in this function; a counter per loop
         * scores 15. */
        s0 = 1;
        tbl = D_80094CFC;
        /* FAKE: s1 copies the parameter arg4; using arg4 directly scores 78. */
        s1 = arg4;

        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x50) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;

        do {
            s32 idx;
            u8 *a4p;
            a4p = s3 + s0 * 0x68;
            idx = *tbl;
            *(s16 *)(a4p + 0x10) = *(u16 *)((u8 *)s1 + idx * 6);
            idx = *tbl;
            *(s16 *)(a4p + 0x12) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 2);
            idx = *tbl;
            *(s16 *)(a4p + 0x14) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 4);
            s0++;
            tbl++;
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
        *(s16 *)(s2 + 0x12) = -(s16)*(u16 *)((u8 *)s1 + 0x74);
        *(s16 *)(s2 + 0x14) = -(s16)*(u16 *)((u8 *)s1 + 0x76);

        g_anim_func_table[0]((SVECTOR *)(s2 + 0x10), (MATRIX *)(s2 + 0x38));

        *(s32 *)(s2 + 0xB4) = *(s16 *)((u8 *)s1 + 0x78);
        *(s32 *)(s2 + 0xB8) = -(s32)*(s16 *)((u8 *)s1 + 0x7A);
        *(s32 *)(s2 + 0xBC) = -(s32)*(s16 *)((u8 *)s1 + 0x7C);
        *(s16 *)(s2 + 0x78) = *(u16 *)((u8 *)s1 + 0x7E);
        *(s16 *)(s2 + 0x7A) = -(s16)*(u16 *)((u8 *)s1 + 0x80);
        *(s16 *)(s2 + 0x7C) = -(s16)*(u16 *)((u8 *)s1 + 0x82);

        g_anim_func_table[0]((SVECTOR *)(s2 + 0x78), (MATRIX *)(s2 + 0xA0));
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
            s32 *list3;
            s5 = *(u8 **)(a3p + 0x40);
            if (s5 == 0) {
                break;
            }
            *(Copy8_40D48 *)a3p = *(Copy8_40D48 *)(s5 + 0x18);
            list3 = (s32 *)D_800A3820;
            a3p += 0x68;
            D_800A3820 = (s32)(list3 + 1);
            *list3 = (s32)a2p;
            a2p += 0x68;
        }

        a2p = s4 + 0x8B4;
        if (*(s16 *)(s4 + 0x8B6) != -1) {
            do {
                s32 *list4;
                list4 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list4 + 1);
                *list4 = (s32)a2p;
                a2p += 0x68;
            } while (*(s16 *)(a2p + 2) != -1);
        }
    }

    func_800404A0((s16 *)(s4 + 0x8B4), arg5);
    *(s16 *)(s4 + 0x1A84) = (s16)arg5;
    func_800400B0((s32 *)s4, arg5);
    func_8003F62C((s32 *)s4);
    func_800420E8(a0, (s32)(s3 + 0x2C));
}

extern s32 D_80094CFC[];
extern void math_RotMatrixZYX(s16 *, s32 *);
extern void func_800523E0(s32 *, s32 *, s32, s32);
extern void func_80044DE4(s16 *, s16 *, s32, s32);
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = g_player_ptrs[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        math_RotMatrixZYX(buf, a4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        math_RotMatrixZYX(buf, out2);
        func_800523E0(a4, out2, a3, ents + i * 0x68 + 0x38);
        *((s16 *) (ents + i * 0x68 + 6)) = 2;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
    out3 = (s32 *) (((u8 *) a4) + 0x20);
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    math_RotMatrixZYX(buf, a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    math_RotMatrixZYX(buf, out3);
    func_800523E0(a4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
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
    s32 *base;
    s32 *s0;
    s32 i;
    base = &g_player_ptrs[a0];
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
