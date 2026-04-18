#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"


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
extern void md_game_check_mode(s32 *, s32);
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
extern void initLoadImage(s32, s16 *, s32, s32);

void PutShadowRmd(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    s32 v0;
    u16 *tbl;
    s16 u, v;
    s32 *pkt;
    s32 *ot;

    v0 = D_800A36AC;
    a2 = v0 & 1;
    if (a2 != D_800A3234) {
        D_800A3378 = (s32)(&D_800A9830 + a2 * 240);
        D_800A3234 = a2;
    }
    v0 = D_800A3234;
    if ((s32 *)D_800A3378 != (s32 *)(&D_800A9920 + v0 * 240)) {
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
        initLoadImage((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (s32 *)D_800A3378;
        ot = (s32 *)D_800A378C;
        *pkt = (*pkt & 0xFF000000) | (ot[0x3FFC / 4] & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & 0xFF000000) | ((s32)pkt & 0xFFFFFF);
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
            PutShadowRmd(a0, 5);
            PutShadowRmd(a0, 6);
            mask = 0;
            break;
        }
        mask = mask & D_80094B48[*(s16 *)(ptr + 8)];
        i = 0;
        do {
            if (mask & 1) {
                PutShadowRmd(a0, 4 - i);
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
extern void AllocRobRmd(s32 *);
extern void rob_life_ctrl(s32 *);
extern void rob_calc_2d_position(s32 *);
extern s32 *func_80045878(s32);
extern void func_8003F824(s32 *, s32);
extern void func_8003FFC4(s32 *);
extern void func_8003E120(void);
s32 *func_80040510(s32 a0) {
    s32 *ptr;
    s32 idx = a0;
    ptr = func_80045878(a0);
    g_player_ptrs[idx] = (s32)ptr;
    AllocRobRmd(ptr);
    do { rob_life_ctrl(ptr); rob_calc_2d_position(ptr); func_8003F824(ptr, 1); } while (0);
    func_8003FFC4(ptr);
    func_80040CB8(ptr);
    func_8003E120();
    return ptr;
}
INCLUDE_ASM("asm/funcs", AllocRobRmd);
/* kengo:HIGH  |  am_rmd/AllocRobRmd  |  220i  |  +3 near-exact */
extern s16 D_80094B96[];
extern s16 D_80094B98[];
extern s16 D_80094B9A[];
extern s16 D_80094B9C[];
extern s16 D_80094C68[];
void rob_life_ctrl(s32 *a0) {
    s16 *tbl;
    s32 count;
    s16 idx;

    tbl = D_80094C68;
    count = 0;
    if (*tbl != -1) {
        tbl++;
        do {
            count++;
        } while (*tbl++ != -1);
    }

    idx = *(s16 *)((u8 *)a0 + 8);
    if (idx < count) {
        *(u16 *)((u8 *)a0 + 0x12) = (u16)D_80094C68[idx];
    } else {
        *(u16 *)((u8 *)a0 + 0x12) = 0x1000;
    }

    {
        u8 *a3 = (u8 *)a0 + 0x2C;
        u8 *base = a3;
        s32 neg1 = -1;
        s32 off = 0;
        u8 *a1 = (u8 *)a0 + 0x30;

        do {
            s16 v1;
            s32 prod;
            *a3 = 0;
            *(a1 - 3) = 0;
            *(s16 *)(a1 + 4) = 0;
            *(s16 *)(a1 - 2) = neg1;
            v1 = *(s16 *)((u8 *)D_80094B96 + off);
            if (v1 != neg1) {
                *(s32 *)(a1 + 8) = (s32)(base + v1 * 104);
            } else {
                *(s32 *)(a1 + 8) = 0;
            }
            *(s32 *)(a1 + 0x48) = ((s32)*(s16 *)((u8 *)D_80094B98 + off) * (s16)*(u16 *)((u8 *)a0 + 0x12)) >> 12;
            *(s32 *)(a1 + 0x4C) = ((s32)*(s16 *)((u8 *)D_80094B9A + off) * (s16)*(u16 *)((u8 *)a0 + 0x12)) >> 12;
            prod = (s32)*(s16 *)((u8 *)D_80094B9C + off) * (s16)*(u16 *)((u8 *)a0 + 0x12);
            *(s16 *)(a1 + 0xC) = 0;
            *(s16 *)(a1 + 0xE) = 0;
            *(s16 *)(a1 + 0x10) = 0;
            *(s16 *)(a1 + 0x2) = 0;
            *(s32 *)(a1 + 0x50) = prod >> 12;
            *(u16 *)(a1 + 6) = *(u16 *)((u8 *)a0 + 0x10);
            *(u16 *)a1 = *(u16 *)((u8 *)a0 + 0x14);
            off += 0xA;
            a3 += 0x68;
            a1 = a3 + 4;
        } while (off < 0xD2);
    }

    func_80040A78((s32)a0);
}
/* kengo:MED  |  my_rob/rob_life_ctrl  |  96i  |  x2 size collision */
void func_80040A78(s32 arg0) {
    register s32 var_a1 asm("a1");
    register s32 var_v1 asm("v1");

    var_a1 = 0;
    var_v1 = arg0 + 0x94;
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
        var_v1 += 0x68;
        var_a1 += 1;
        arg0 += 4;
    } while (var_a1 < 0x14);
}
typedef struct { s32 f0, f1, f2, f3; } Copy16;
typedef struct { s32 f0, f1; } Copy8;
void rob_calc_2d_position(s32 *arg0) {
    s32 seen[18];
    s32 *t5;
    s32 *t7;
    s32 *v1;
    s32 *t3;
    s32 i;
    s32 *ptr;
    u16 a0_val;
    s32 *t4;

    t5 = (s32 *)((u8 *)arg0 + 0x10D4);
    t7 = (s32 *)((u8 *)arg0 + 0x94);
    v1 = *(s32 **)((u8 *)arg0 + 0x1C);
    t3 = (s32 *)((u8 *)v1 + *(s32 *)((u8 *)v1 + 8));

    i = 0x11;
    ptr = &seen[17];
    do {
        *ptr = 0;
        i--;
        ptr--;
    } while (i >= 0);

    i = 0x13;
    ptr = (s32 *)((u8 *)arg0 + 0x4C);
    do {
        *(s32 *)((u8 *)ptr + 0x1A34) = 0;
        i--;
        ptr = (s32 *)((u8 *)ptr - 4);
    } while (i >= 0);

    a0_val = *(u16 *)t3;
    if ((a0_val & 0xFFFF) == 0xFFFF) goto done;

    {
        s32 one = 1;
        t4 = (s32 *)((u8 *)t5 + 0x58);
        t3 = (s32 *)((u8 *)t3 + 2);

        do {
            s32 a3 = *(u16 *)t3;
            s32 *a1;
            s32 t2;

            t3 = (s32 *)((u8 *)t3 + 2);
            a1 = &seen[a3];
            t2 = a0_val & 0xFFFF;

            if (*a1 == 0) {
                s32 *slot = (s32 *)((u8 *)t7 + a3 * 0x68);
                *(s16 *)((u8 *)slot + 2) = (s16)t2;
                *a1 = one;
                *(s32 *)((u8 *)arg0 + (t2 << 2) + 0x1A34) = (s32)slot;
            } else {
                s32 *t0 = t5;
                s32 *a2 = (s32 *)((u8 *)t7 + a3 * 0x68);
                s32 *end = (s32 *)((u8 *)a2 + 0x60);

                do {
                    *(Copy16 *)t0 = *(Copy16 *)a2;
                    a2 = (s32 *)((u8 *)a2 + 0x10);
                    t0 = (s32 *)((u8 *)t0 + 0x10);
                } while (a2 != end);

                *(Copy8 *)t0 = *(Copy8 *)a2;

                *(s16 *)((u8 *)t4 - 0x56) = (s16)t2;
                *(s32 *)t4 = (s32)((u8 *)t7 + a3 * 0x68);
                t4 = (s32 *)((u8 *)t4 + 0x68);
                *(s32 *)((u8 *)arg0 + (t2 << 2) + 0x1A34) = (s32)t5;
                t5 = (s32 *)((u8 *)t5 + 0x68);
            }

            a0_val = *(u16 *)t3;
            t3 = (s32 *)((u8 *)t3 + 2);
        } while (a0_val != 0xFFFF);
        t3 = (s32 *)((u8 *)t3 - 2);
    }
done:
    *(s32 *)((u8 *)t5 + 0x58) = 0;
}
/* kengo:MED  |  my_rob/rob_calc_2d_position  |  93i */
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    register s8 *a2 asm("a2");
    register s32 t1 asm("t1");
    register s16 t4 asm("t4");
    register s16 t3 asm("t3");
    register s16 t2 asm("t2");
    register s32 t0 asm("t0");
    register s16 *a3 asm("a3");
    register s32 a1 asm("a1");

    a2 = (s8 *)arg0 + 0x8B4;
    t1 = 0;
    t4 = -1;
    t3 = 3;
    t2 = 1;
    t0 = (s32)arg0 + 0x94;
    a3 = D_80094B9E;
    a1 = (s32)arg0 + 0x90C;

loop:
    {
        s16 v0 = *a3;
        if (v0 != t4) {
            s16 v1 = v0;
            *(s16 *)(a1 - 0x56) = v1;
            *a2 = t3;
            *(s8 *)(a1 - 0x57) = 0;
            *(s16 *)(a1 - 0x50) = 0;
            *(s32 *)(a1 - 0x4C) = t0;
            *(s16 *)(a1 - 0x52) = t2;
            *(s16 *)(a1 - 0x4E) = 0;
            {
                u16 lhu_result = *(u16 *)((s32)arg0 + 0x16);
                a2 += 0x68;
                *(s32 *)a1 = 0;
                *(s16 *)(a1 - 0x54) = lhu_result;
                a1 += 0x68;
            }
        }
        t0 += 0x68;
        t1++;
        a3 = (s16 *)((s32)a3 + 0xA);
        if (t1 < 0x12) goto loop;
    }
    *(s16 *)((s32)a2 + 2) = -1;
}
INCLUDE_ASM("asm/funcs", func_80040D48);
extern s32 D_80094CFC[];
extern s32 D_800A9A10[];
extern void func_8004A348(s16 *, s32 *);
extern void func_800523E0(s32 *, s32 *, s32, s32);
extern void func_80044DE4(s16 *, s16 *, s32, s32);
void hirahira_w_ctrl(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
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
    register s32 **t0 asm("t0") = D_80015820;
    register s32 t1 asm("t1") = 0;
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
    func_800414FC(a1, (s16 *)((u8 *)s0 + 0x2C), 0x15);
    func_800414FC(a1, (s16 *)((u8 *)s0 + 0x8B4), 0x14);
    func_800414FC(a1, (s16 *)((u8 *)s0 + 0x10D4), 0x14);
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
    {
        register s32 *v1 asm("v1");
        v1 = s0;
        do {
            s32 val = *(s32 *)((u8 *)v1 + 0x1A34);
            if (val) {
                *(s32 *)((u8 *)v1 + 0x1A34) = val + a1;
            }
            i++;
            v1++;
        } while (i < 0x14);
    }
    func_80040A78((s32)s0);
}
void func_800414FC(s32 a0, s16 *a1, s32 a2) {
    volatile s32 sp;
    s32 i = a2 - 1;
    if (a2 == 0) {
        return;
    }
    a2 = -1;
    a1 = (s16 *)((u8 *)a1 + 0xC);
    do {
        s32 val = *(s32 *)a1;
        if (val) {
            *(s32 *)a1 = val + a0;
        }
        i--;
        a1 = (s16 *)((u8 *)a1 + 0x68);
    } while (i != a2);
}
extern s32 g_player_ptrs[];
s32 func_8004153C(s32 a0) {
    return g_player_ptrs[a0];
}
s32 func_80041554(s32 a0) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        return ptr[4];
    }
    return -1;
}
s32 func_80041584(void) {
    s32 i;
    s32 ret = -1;
    for (i = 0; i < 3; i++) {
        if (g_player_ptrs[i] == 0) {
            ret = i;
            break;
        }
    }
    return ret;
}
extern void func_8004016C(s32);
extern void func_80045A50(s32);
void player_Destroy(s32 a0) {
    func_8004016C(a0);
    func_80045A50(a0);
    g_player_ptrs[a0] = 0;
}
extern s32 g_player_char_ids[];
void player_SetCharId(s32 a0, s32 a1) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        if ((val & 0x1F) != a1) {
            ptr[3] = -2;
        }
    }
    g_player_char_ids[a0] = a1;
}
s32 func_80041650(s32 a0) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        return val & 0x1F;
    }
    return -1;
}
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void func_80041398(s32);

void func_80041688(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) {
        p[1] |= 1;
    } else {
        p[1] &= ~1;
    }

    i = 1;
loop1:
    p += 0x68;
    if (*(s16 *)(p + 2) >= 0) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        func_80041398((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        func_80041398(b | ((r << 16) | (g << 8)));
    }
    (void)sp10;
}
typedef struct { s32 w[4]; } Block16;
extern s32 g_anim_func_table[];
extern void func_80052930(void *, void *, void *);
extern void func_80052A88(void *, void *, void *);
void func_800417D0(s32 *a0) {
    void (*func)(s32 *, s32 *);
    s32 *child;

    if (((s16 *)a0)[3] == 1) {
        return;
    }
    if (((s16 *)a0)[3] != 2) {
        func = (void (*)(s32 *, s32 *))g_anim_func_table[((s16 *)a0)[4]];
        func(a0 + 4, a0 + 14);
    }
    child = (s32 *)a0[3];
    if (child != 0) {
        if (((s16 *)child)[3] == 0) {
            func_800417D0(child);
        }
        func_80052930((void *)((u8 *)((s32 *)a0[3]) + 0x18), (void *)((u8 *)a0 + 0x38), (void *)((u8 *)a0 + 0x18));
        func_80052A88((void *)((u8 *)((s32 *)a0[3]) + 0x18), (void *)((u8 *)a0 + 0x4C), (void *)((u8 *)a0 + 0x2C));
    } else {
        ((Block16 *)(a0 + 6))[0] = ((Block16 *)(a0 + 14))[0];
        ((Block16 *)(a0 + 6))[1] = ((Block16 *)(a0 + 14))[1];
    }
    ((s16 *)a0)[3] = 1;
}
typedef struct { s32 w[4]; } Block16;
extern s32 g_anim_func_table[];
void func_800418D0(s32 *a0) {
    s16 sp10[4];
    void (*func)(s16 *, s32 *);
    sp10[0] = -(u16)((u16 *)a0)[8];
    sp10[1] = -(u16)((u16 *)a0)[9];
    sp10[2] = -(u16)((u16 *)a0)[10];
    func = (void (*)(s16 *, s32 *))g_anim_func_table[((s16 *)a0)[4]];
    func(sp10, a0 + 14);
    ((Block16 *)(a0 + 6))[0] = ((Block16 *)(a0 + 14))[0];
    ((Block16 *)(a0 + 6))[1] = ((Block16 *)(a0 + 14))[1];
}
void func_80041988(s32 a0, s32 a1, s32 a2, s32 a3) {
    u8 mask_table;
    s32 bit;
    s32 i;
    s32 id;

    if ((u32)a0 >= 2) {
        return;
    }
    mask_table = D_80094D40[a1];
    bit = 0x10;
    i = 0;
    id = 1;
    do {
        if (!(mask_table & bit) || !(a2 & bit)) {
            goto next;
        }
        if (a0 == 0) {
            goto case0;
        }
        if (a0 == 1) {
            goto case1;
        }
        goto shift;
    case0:
        if (func_8003E2A0() == 0) {
            func_800480C0(a3, id, 0, 0, -0x140, 0xE8);
        } else {
            func_80047EE8(a3, id);
        }
        goto shift;
    case1:
        if (func_8003E2A0() == a0) {
            func_800480C0(a3, id, 0x80, 0, -0x140, 0xE8);
        } else {
            func_80047FBC(a3, id, 0x80, 0);
        }
    next:
    shift:
        bit >>= 1;
        i++;
        id++;
    } while (i < 5);
}

extern s32 D_80094DF0[];
extern u8 D_80094E08[];
extern s16 D_800A9A20;
extern u16 D_800A9A24;
void func_80041AC8(s16 *arg0)
{
  s16 rect[4];
  s16 *var_s0;
  u16 *var_s1;
  s32 var_s2;
  s32 var_s3;
  u16 v1_val;
  if (arg0[2] != 1)
  {
    return;
  }
  if (D_80094E08[arg0[4]] == 0xFF)
  {
    return;
  }
  D_800A9A20 = arg0[4];
  var_s0 = (s16 *) D_80094DF0[D_80094E08[arg0[4]]];
  if (func_8003E2A0() != 1)
  {
    goto else_lbl;
  }
  var_s3 = -0x140;
  var_s2 = 0xF0;
  goto after_if;
  else_lbl:
  var_s3 = 0x80;

  var_s2 = 0;
  after_if:
  v1_val = (u16) (*var_s0);

  if ((*var_s0) >= 0)
  {
    var_s1 = &D_800A9A24;
    do
    {
      u16 v0_val;
      rect[0] = v1_val + var_s3;
      v0_val = (u16) var_s0[1];
      rect[2] = 0x10;
      rect[3] = 1;
      rect[1] = v0_val + var_s2;
      gpu_StoreImage(rect, var_s1);
      var_s0 += 2;
      var_s1 += 0x10;
      v1_val = (u16) (*var_s0);
    }
    while ((*var_s0) >= 0);
  }
  gpu_DrawSync(0);
}
INCLUDE_ASM("asm/funcs", saTan4FireDisp);
extern s16 g_anim_select;
extern s16 D_800A323A;
extern s16 D_800A323C;
extern Block16 D_800A9B28;
void func_80041E10(Block16 *a0, s32 a1) {
    g_anim_select = (s16)((((a1 >> 16) & 0xFF) << 12) / 255);
    D_800A323A = (s16)((((a1 >> 8) & 0xFF) << 12) / 255);
    D_800A323C = (s16)(((a1 & 0xFF) << 12) / 255);
    D_800A9B28 = *a0;
}
INCLUDE_ASM("asm/funcs", decBs0);
extern s16 g_anim_counter;
extern s16 g_anim_select;
void func_800420D0(void) {
    extern s16 g_anim_hit_flags;
    g_anim_counter = 0;
    g_anim_hit_flags = 0;
    g_anim_select = -1;
}
extern s16 g_anim_hit_flags[];
extern s32 g_anim_hit_data[];
void func_800420E8(s32 a0, s32 a1) {
    if (a0 < 2) {
        g_anim_hit_flags[a0] = 1;
        g_anim_hit_data[a0] = a1;
    }
}
extern s32 D_800A3388;
void func_8004211C(void) {
    s32 val = g_anim_hit_flags[0] * 2 + g_anim_counter;
    switch (val) {
    case 1:
        func_80041EB0(0, D_800A3388);
        break;
    case 2:
        func_80041EB0(g_anim_hit_data[0], 0);
        break;
    case 3:
        func_80041EB0(g_anim_hit_data[0], D_800A3388);
        break;
    }
}
extern void func_80041EB0(s32, s32);
void func_800421A4(void) {
    func_80041EB0(0, 0);
}
extern void func_800422BC(s32, s32, s32, s32);
extern void func_80042478(s32);
extern s16 D_800F6462;
extern s16 D_800F6342;
extern s16 D_800F62E2;
extern s16 D_800F6460;
extern s16 D_800F6340;
extern s16 D_800F62E0;
extern s32 StageLight[];
void func_800421C8(s32 a0) {
    s32 *p = (s32 *)((u8 *)StageLight + a0 * 24);
    s32 val;
    func_800422BC(a0, *p++, 0, 0);
    func_800422BC(a0, *p++, 0, 1);
    func_800422BC(a0, *p++, 1, 0);
    func_800422BC(a0, *p++, 1, 1);
    val = *p;
    D_800F6462 = val & 0xFFF;
    D_800F6342 = val & 0xFFF;
    D_800F62E2 = val & 0xFFF;
    val = *(s16 *)((u8 *)p + 2);
    D_800F6460 = val & 0xFFF;
    D_800F6340 = val & 0xFFF;
    D_800F62E0 = val & 0xFFF;
    func_80042478(*(s32 *)((u8 *)p + 4));
}
extern s16 D_800F6318;
extern s16 D_800F631E;
extern s16 D_800F6324;
extern u8 D_800F6338;
extern u8 D_800F6339;
extern u8 D_800F633A;
extern s16 D_800F6378;
extern s16 D_800F637E;
extern s16 D_800F6384;
extern u8 D_800F6398;
extern u8 D_800F6399;
extern u8 D_800F639A;
extern s16 D_800F6498;
extern s16 D_800F649E;
extern s16 D_800F64A4;
extern u8 D_800F64B8;
extern u8 D_800F64B9;
extern u8 D_800F64BA;
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void func_8004A1FC(s16 *);
void gnd_land_hit_char_die_main(s32 a0, s32 packed, s32 a2, s32 a3) {
    s32 r = (packed >> 16) & 0xFF;
    s32 g = (packed >> 8) & 0xFF;
    s32 b = packed & 0xFF;
    s16 r2;
    s16 g2;
    s16 *new_var;
    s16 b2;
    if (func_800486FC(a0)) {
        b = func_8004881C(r, g, b);
        g = b;
        r = b;
    }
    if (a3 != 0) {
        goto raw;
    }
    r2 = (r << 12) / 255;
    g2 = (g << 12) / 255;
    b2 = (b << 12) / 255;
    if (a2 != 0) {
        goto alt_scale;
    }
    new_var = &D_800F6318;
    *new_var = r2;
    D_800F631E = g2;
    D_800F6324 = b2;
    func_8004A1FC((s16 *)(((u8 *)new_var) - 0x38));
    D_800F6378 = r2;
    D_800F637E = g2;
    D_800F6384 = b2;
    func_8004A1FC((s16 *)(((u8 *)new_var) + 0x28));
    goto out;
alt_scale:
    {
        s16 *p2 = &D_800F6498;
        *p2 = r2;
        D_800F649E = g2;
        D_800F64A4 = b2;
        func_8004A1FC((s16 *)(((u8 *)p2) - 0x38));
    }
    goto out;
raw:
    if (a2 != 0) {
        goto alt_raw;
    }
    D_800F6338 = r;
    D_800F6339 = g;
    D_800F633A = b;
    D_800F6398 = r;
    D_800F6399 = g;
    D_800F639A = b;
    goto out;
alt_raw:
    D_800F64B8 = r;
    D_800F64B9 = g;
    D_800F64BA = b;
out:;
}
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern void gte_SetFarColor(s32, s32, s32);
void func_80042478(s32 a0) {
    s32 r = (a0 >> 16) & 0xFF;
    s32 g = (a0 >> 8) & 0xFF;
    s32 b = a0 & 0xFF;
    if (func_800486FC(a0)) {
        b = func_8004881C(r, g, b);
        g = b;
        r = b;
    }
    disp_SetFramebufferMode(1, r, g, b);
    gte_SetFarColor(r, g, b);
}
