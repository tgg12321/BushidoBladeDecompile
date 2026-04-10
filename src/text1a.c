#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"


extern s16 D_800EED10[];
extern s32 D_800EED1C[];
extern s32 D_800EED18;
extern s32 D_800A33AC;
extern s32 D_800A33A0;
extern s32 D_800A33A4;
extern s32 D_800A33A8;
extern u8 D_800A9D10;
extern void func_80049E1C(void);
extern void func_80045294(s32, s32);
extern void func_80052C10(void);
extern void func_80044098(s16);
extern void func_80044010(s32 *, s16);
extern s32 g_snd_bgm_id;
extern s32 g_snd_se_id;
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

extern s32 D_800A36AC;
extern s32 D_800A378C;
extern s32 D_800A3234;
extern s32 D_800A3378;
extern u8 D_800A9830;
extern u8 D_800A9920;
extern u16 D_80094AF4;
extern void func_8007ABB8(s32, s16 *, s32, s32);

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
        func_8007ABB8((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (s32 *)D_800A3378;
        ot = (s32 *)D_800A378C;
        *pkt = (*pkt & 0xFF000000) | (ot[0x3FFC / 4] & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & 0xFF000000) | ((s32)pkt & 0xFFFFFF);
    }
}
INCLUDE_ASM("asm/funcs", func_80040304);
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
INCLUDE_ASM("asm/funcs", rob_life_ctrl);
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
INCLUDE_ASM("asm/funcs", rob_calc_2d_position);
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
INCLUDE_ASM("asm/funcs", hirahira_w_ctrl);
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
void func_800415C4(s32 a0) {
    func_8004016C(a0);
    func_80045A50(a0);
    g_player_ptrs[a0] = 0;
}
extern s32 g_player_char_ids[];
void func_80041604(s32 a0, s32 a1) {
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
INCLUDE_ASM("asm/funcs", func_80041688);
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
INCLUDE_ASM("asm/funcs", func_80041988);
INCLUDE_ASM("asm/funcs", func_80041AC8);
INCLUDE_ASM("asm/funcs", func_80041BF4);
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
INCLUDE_ASM("asm/funcs", func_80041EB0);
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
INCLUDE_ASM("asm/funcs", func_800422BC);
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern void func_8007EFBC(s32, s32, s32);
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
    func_8007EFBC(r, g, b);
}
INCLUDE_ASM("asm/funcs", rob_life_ctrl_2);
/* kengo:MED  |  my_rob/rob_life_ctrl_2  |  96i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", mot_data_set);
/* kengo:MED  |  se_fc/mot_data_set  |  110i */
extern s16 D_800F6650;
void func_8004283C(s32 a0) {
    if (a0) {
        D_800F6650 = 1;
    } else {
        D_800F6650 = 0;
    }
}
extern s16 D_800F6650;
s32 func_80042864(void) {
    return D_800F6650;
}
INCLUDE_ASM("asm/funcs", func_80042874);
INCLUDE_ASM("asm/funcs", func_80042A88);
INCLUDE_ASM("asm/funcs", hirahira_w_ctrl_2);
/* kengo:MED  |  my_hirahira/hirahira_w_ctrl_2  |  132i  |  x2 size collision */
extern void func_8004A348(void);
extern void func_80042874(void);
extern void func_80042A88(void);
extern void hirahira_w_ctrl_2(void);
extern s32 D_800F66A8;
extern s32 D_800F66B0;
extern s32 D_800F66B4;
void func_80042E90(void) {
    g_anim_func_table[0] = (s32)func_8004A348;
    D_800F66A8 = (s32)func_80042874;
    D_800F66B0 = (s32)func_80042A88;
    D_800F66B4 = (s32)hirahira_w_ctrl_2;
}
void func_80042ED8(u16 *a0) {
    u16 t0, t1, t2, t3;
    t0 = a0[6];
    t1 = a0[2];
    t2 = a0[1];
    t3 = a0[5];
    a0[2] = t0;
    t0 = a0[3];
    a0[1] = t0;
    t0 = a0[7];
    a0[6] = t1;
    a0[3] = t2;
    a0[7] = t3;
    a0[5] = t0;
}
extern s16 Judge[];
void func_80042F10(s32 *a0, s32 *a1, s32 a2) {
    s16 sin_val, cos_val;
    s32 x, y;
    s32 sin_x, cos_x, sin_y, cos_y;
    sin_val = Judge[(a2 + 0x400) & 0xFFF];
    x = *a0;
    cos_val = Judge[a2 & 0xFFF];
    y = *a1;
    sin_x = sin_val * x;
    cos_x = cos_val * x;
    sin_y = sin_val * y;
    cos_y = cos_val * y;
    *a1 = (cos_x + sin_y) >> 12;
    *a0 = (sin_x - cos_y) >> 12;
}
INCLUDE_ASM("asm/funcs", func_80042FA0);
INCLUDE_ASM("asm/funcs", func_800430E4);
s32 func_80043244(s32 a0) {
    s32 ret;
    if (a0 > 0x16A09) {
        ret = 3;
    } else if (a0 > (s32)0xB500) {
        ret = 2;
    } else {
        ret = a0 >= 0x5A01;
    }
    return ret;
}
s32 func_80043278(s32 a0) {
    s32 mask = (s32)0xFFFFF000;
    volatile s32 *sp = (volatile s32 *)0x1F800000;
    s32 v0 = a0 >> sp[2];
    s32 a0_new = v0 >> 11;
    v0 = (v0 & 0x7FF) | mask;
    v0 = v0 >> a0_new;
    return v0 & 0xFFF;
}
INCLUDE_ASM("asm/funcs", func_800432A0);
extern void func_800432A0(s16, s16, s16, s16, s16);
void func_80043398(s16 a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    func_800432A0(a0, (s16)(a1 << 6), (s16)(a2 << 8), (s16)(a3 << 6), (s16)(a4 << 8));
}
extern s32 *D_80103608[];
extern u16 D_80103658[];
extern void func_80043454(s32, s16, s16, s16);
void func_800433E4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5) {
    *(s32 *)0x1F800000 = D_80103608[arg0][arg1];
    func_80043454(arg2, arg3, arg4, arg5);
}
INCLUDE_ASM("asm/funcs", func_80043454);
s32 func_80043BD0(u8 *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
  register unsigned int t1 asm("$9");
  register s32 t0 asm("$8");
  register s32 v0 asm("$2");
  register unsigned int v1 asm("$3");

  a1 = (a1 << 16) >> 22;
  v0 = (a2 << 16) >> 24;
  a3 = (a3 << 16) >> 20;
  t1 = *((u16 *) (a0 + 0x16));
  t0 = (t1 & 0xF) + a1;
  t0 &= 0xF;
  v1 = (t1 >> 4) & 1;
  v1 += v0;
  v1 &= 1;
  t1 &= 0xFFE0;
  v1 <<= 4;
  t1 |= v1;
  v0 = *((u8 *) (a0 + 0xD));
  t0 |= t1;
  *((u16 *) (a0 + 0x16)) = t0;
  {
    register s32 la1 asm("$5");
    __asm__ __volatile__("lw\t$5, 16($29)" : "=r"(la1) : "r"(t0));
    a1 = la1;
  }
  v1 = *((u8 *) (a0 + 0x1D));
  v0 += a2;
  *((u8 *) (a0 + 0xD)) = v0;
  v0 = *((u8 *) (a0 + 0x15));
  v1 += a2;
  *((u8 *) (a0 + 0x1D)) = v1;
  v1 = *((u16 *) (a0 + 0xE));
  v0 += a2;
  *((u8 *) (a0 + 0x15)) = v0;
  __asm__ __volatile__("");
  a3 += v1 & 0x3F;
  a3 &= 0x3F;
  a1 += (v1 >> 6) & 0x1FF;
  a1 &= 0x1FF;
  v1 &= 0x8000;
  a1 <<= 6;
  v1 |= a1;
  a3 |= v1;
  *((u16 *) (a0 + 0xE)) = a3;
}
s32 func_80043C7C(u8 *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
  register unsigned int t1 asm("$9");
  register s32 t0 asm("$8");
  register s32 v0 asm("$2");
  register unsigned int v1 asm("$3");

  a1 = (a1 << 16) >> 22;
  v0 = (a2 << 16) >> 24;
  a3 = (a3 << 16) >> 20;
  t1 = *((u16 *) (a0 + 0x16));
  t0 = (t1 & 0xF) + a1;
  t0 &= 0xF;
  v1 = (t1 >> 4) & 1;
  v1 += v0;
  v1 &= 1;
  t1 &= 0xFFE0;
  v1 <<= 4;
  t1 |= v1;
  t0 |= t1;
  *((u16 *) (a0 + 0x16)) = t0;
  {
    register s32 la1 asm("$5");
    __asm__ __volatile__("lw\t$5, 16($29)" : "=r"(la1) : "r"(t0));
    a1 = la1;
  }
  v1 = *((u8 *) (a0 + 0x15));
  v0 = *((u8 *) (a0 + 0xD));
  v1 += a2;
  *((u8 *) (a0 + 0x15)) = v1;
  v1 = *((u8 *) (a0 + 0x25));
  v0 += a2;
  *((u8 *) (a0 + 0xD)) = v0;
  v0 = *((u8 *) (a0 + 0x1D));
  v1 += a2;
  *((u8 *) (a0 + 0x25)) = v1;
  v1 = *((u16 *) (a0 + 0xE));
  v0 += a2;
  *((u8 *) (a0 + 0x1D)) = v0;
  a3 += v1 & 0x3F;
  a3 &= 0x3F;
  a1 += (v1 >> 6) & 0x1FF;
  a1 &= 0x1FF;
  v1 &= 0x8000;
  a1 <<= 6;
  v1 |= a1;
  a3 |= v1;
  *((u16 *) (a0 + 0xE)) = a3;
}
s32 func_80043D34(u8 *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
  register unsigned int t1 asm("$9");
  register s32 t0 asm("$8");
  register s32 v0 asm("$2");
  register unsigned int v1 asm("$3");

  a1 = (a1 << 16) >> 22;
  v0 = (a2 << 16) >> 24;
  a3 = (a3 << 16) >> 20;
  t1 = *((u16 *) (a0 + 0x1A));
  t0 = (t1 & 0xF) + a1;
  t0 &= 0xF;
  v1 = (t1 >> 4) & 1;
  v1 += v0;
  v1 &= 1;
  t1 &= 0xFFE0;
  v1 <<= 4;
  t1 |= v1;
  v0 = *((u8 *) (a0 + 0xD));
  t0 |= t1;
  *((u16 *) (a0 + 0x1A)) = t0;
  {
    register s32 la1 asm("$5");
    __asm__ __volatile__("lw\t$5, 16($29)" : "=r"(la1) : "r"(t0));
    a1 = la1;
  }
  v1 = *((u8 *) (a0 + 0x25));
  v0 += a2;
  *((u8 *) (a0 + 0xD)) = v0;
  v0 = *((u8 *) (a0 + 0x19));
  v1 += a2;
  *((u8 *) (a0 + 0x25)) = v1;
  v1 = *((u16 *) (a0 + 0xE));
  v0 += a2;
  *((u8 *) (a0 + 0x19)) = v0;
  __asm__ __volatile__("");
  a3 += v1 & 0x3F;
  a3 &= 0x3F;
  a1 += (v1 >> 6) & 0x1FF;
  a1 &= 0x1FF;
  v1 &= 0x8000;
  a1 <<= 6;
  v1 |= a1;
  a3 |= v1;
  *((u16 *) (a0 + 0xE)) = a3;
}
s32 func_80043DE0(u8 *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
  register unsigned int t1 asm("$9");
  register s32 t0 asm("$8");
  register s32 v0 asm("$2");
  register unsigned int v1 asm("$3");

  a1 = (a1 << 16) >> 22;
  v0 = (a2 << 16) >> 24;
  a3 = (a3 << 16) >> 20;
  t1 = *((u16 *) (a0 + 0x1A));
  t0 = (t1 & 0xF) + a1;
  t0 &= 0xF;
  v1 = (t1 >> 4) & 1;
  v1 += v0;
  v1 &= 1;
  t1 &= 0xFFE0;
  v1 <<= 4;
  t1 |= v1;
  t0 |= t1;
  *((u16 *) (a0 + 0x1A)) = t0;
  {
    register s32 la1 asm("$5");
    __asm__ __volatile__("lw\t$5, 16($29)" : "=r"(la1) : "r"(t0));
    a1 = la1;
  }
  v1 = *((u8 *) (a0 + 0x19));
  v0 = *((u8 *) (a0 + 0xD));
  v1 += a2;
  *((u8 *) (a0 + 0x19)) = v1;
  v1 = *((u8 *) (a0 + 0x31));
  v0 += a2;
  *((u8 *) (a0 + 0xD)) = v0;
  v0 = *((u8 *) (a0 + 0x25));
  v1 += a2;
  *((u8 *) (a0 + 0x31)) = v1;
  v1 = *((u16 *) (a0 + 0xE));
  v0 += a2;
  *((u8 *) (a0 + 0x25)) = v0;
  a3 += v1 & 0x3F;
  a3 &= 0x3F;
  a1 += (v1 >> 6) & 0x1FF;
  a1 &= 0x1FF;
  v1 &= 0x8000;
  a1 <<= 6;
  v1 |= a1;
  a3 |= v1;
  *((u16 *) (a0 + 0xE)) = a3;
}
void func_80043E98(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    s16 r1;
    r1 = func_80043F80(a0[0], a1, a2);
    a0[0] = r1;
    a0[2] = func_80043FCC(a0[2], a3, a4);
}
extern s16 func_80043F80(s16, s16, s16);
extern s16 func_80043FCC(s16, s16, s32);
void func_80043F0C(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    s16 r1;
    r1 = func_80043F80(a0[3], a1, a2);
    a0[3] = r1;
    a0[1] = func_80043FCC(a0[1], a3, a4);
}
s16 func_80043F80(s16 a0, s16 a1, s16 a2) {
    s32 low = (a0 & 0xF) + (a1 >> 6);
    s32 mid;
    low &= 0xF;
    mid = ((a0 >> 4) & 1) + (a2 >> 8);
    mid &= 1;
    return (s16)(low | ((a0 & ~0x1F) | (mid << 4)));
}
s16 func_80043FCC(s16 a0, s16 a1, s32 a2) {
    s32 low = (a1 >> 4) + (a0 & 0x3F);
    s32 mid;
    low &= 0x3F;
    mid = (a2 + (((u32)(a0 << 17)) >> 23)) & 0x1FF;
    return (s16)(low | ((a0 & (s16)0x8000) | (mid << 6)));
}
void func_80044010(s32 *a0, s16 a1) {
    s32 new_var;
    volatile s32 sp_pad;
    s32 *a2 = a0;
    s32 v1 = *a0;
    unsigned short v0;
    *a0 = (v1 | 0x8000) & 0xFFFF;
    a0 = a0 + 1;
    D_80103608[a1] = a0;
    D_80103658[a1] = v1 & 0x7FFF;
    v0 = v1;
    if (!(v1 & 0x8000)) {
        v1 = v0 & 0xFFFF;
        new_var = v1;
        if (new_var > 0) {
            s32 i = 0;
            do {
                *a0 += (s32)a2;
                i++;
                a0++;
            } while (i < new_var);
        }
    }
}
void func_80044098(s16 a0) {
    register s32 *v1 asm("v1");
    register s32 a4 asm("a0");
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            } while (a4 != -1);
        }
    }
}
void func_80044100(s32 a0, s32 a1) {
    s32 *ptr = D_80103608[a0];
    s32 count = D_80103658[a0];
    D_80103608[a0] = (ptr = ptr + (a1 / 4));
    count--;
    if (count != -1) {
        if (1) {
            do { } while (0);
        }
        do {
            *ptr += a1;
            ptr++;
            count--;
        } while (count != -1);
    }
}
INCLUDE_ASM("asm/funcs", func_80044170);
INCLUDE_ASM("asm/funcs", hirahira_w_frie);
/* kengo:MED  |  my_hirahira/hirahira_w_frie  |  59i */
INCLUDE_ASM("asm/funcs", calc_fc_frame);
/* kengo:MED  |  se_fc/calc_fc_frame  |  72i */
extern s16 D_8010367E;
void func_80044498(void) {
    s32 i = 0x13;
    s16 *p = &D_8010367E;
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}
extern s32 D_800A378C;
void func_800444BC(void) {
    func_80044504(D_800A378C);
}
void func_800444E0(void) {
    func_80044504(D_800A378C);
}
extern s32 D_800A3678;
extern s32 D_80101BD0;
extern s32 D_800A3708;
extern s32 D_800A370C;
extern s32 D_800FF610;
extern s32 D_800A36AC;
extern s32 g_game_timer;
extern s16 D_80095328;
extern s32 D_80102C00;
extern s32 D_800A3820;
extern void func_80042874(s32 *, s32 *);
extern void func_8007EB4C(s32 *, s32 *);
extern void func_8007EC5C(s32 *, s32 *);
extern void func_8007E4DC(s32 *, s32 *, s32 *);
extern void camera_InitMatrix(void);
extern s32 func_8003E2C8(void);
extern s32 func_8003F268(void);
extern s32 game_GetPause(void);
extern void func_8004A4E0(void);
extern void game_SetPause(s32);
void func_80044504(s32 a0) {
    s32 *s0 = &D_80101BD0;
    func_80042874(&D_800A3678, s0);
    func_8007EB4C(s0, (s32 *)(D_800A3708 + 0x18));
    func_8007EC5C((s32 *)(D_800A370C + 0x18), s0);
    func_8007E4DC((s32 *)(D_800A370C + 0x18), (s32 *)(D_800A3708 + 0x18), &D_800FF610);
    if (D_800A36AC & 1) {
        *(s32 *)0x1F800014 = -1;
    } else {
        *(s32 *)0x1F800014 = 0;
    }
    camera_InitMatrix();
    *(s32 *)0x1F80001C = (s32)&D_80095328;
    *(s32 *)0x1F80000C = a0;
    {
        s32 v1;
        if (g_game_timer & 8) {
            v1 = func_8003E2C8();
        } else {
            v1 = 0x7FFFFFFF;
        }
        *(s32 *)0x1F800010 = v1;
    }
    {
        s32 v0 = func_8003F268();
        if (v0 != 0) {
            v0 = 0xBE;
        } else {
            v0 = game_GetPause();
            if (v0 != 0) {
                v0 = 0x182;
            } else {
                v0 = 0xBE;
            }
        }
        *(s32 *)0x1F800018 = v0;
    }
    func_8004A4E0();
    game_SetPause(1);
    D_800A3820 = (s32)&D_80102C00;
}
extern void func_80052C10(void);
void func_80044650(void) {
    func_80052C10();
}
extern s16 D_800A9CF8;
extern s32 D_800A9D00;
extern s32 D_800A9D04;
extern s16 D_800A9CFA;
extern s16 D_800A9CFC;
extern s16 D_800A9CFE;
extern s32 stage_GetId(void);
s32 func_80044670(s16 *a0, s16 a1, s32 a2) {
    s32 v0;
    do { } while (0);
    D_800A9CF8 = a1;
    v0 = *(u16 *)a0;
    a0++;
    D_800A9D00 = (s32)a0;
    D_800A9D04 = a2;
    D_800A9CFA = v0;
    v0 = stage_GetId();
    D_800A9CFC = v0;
    switch ((s16)v0) {
    case 7:
        D_800A9CFE = 0x11;
        break;
    case 4:
        D_800A9CFE = 2;
        break;
    case 0x12:
        D_800A9CFE = 0xA;
        break;
    }
    {
        s32 val = D_800A9CFE;
        return a2 + val * 104;
    }
}
extern s32 func_8003F1D4(void);
extern s32 D_800A9D08;
void func_8004473C(void)
{
  extern s8 *D_800A9D04;
  register s32 temp_v0 asm("v0");
  s32 temp_v0_2;
  register s32 var_a2 asm("a2");
  s8 *var_a1;
  register s8 *var_v1 asm("v1");
  register void *var_a0 asm("a0");
  s8 *var_a3;
  register s32 const4 asm("t1");
  register s32 constneg1 asm("t0");
  s32 _sp_pad[2];
  __asm__ volatile("" : "=m"(_sp_pad[0]));
  temp_v0 = func_8003F1D4();
  var_a2 = 0;
  var_a1 = D_800A9D04;
  *((s32 *) (((s8 *) &D_800A9D08) + 0)) = temp_v0;
  if (D_800A9CFE > 0)
  {
    var_a3 = ((s8 *) &D_800A9D08) - 0x10;
    const4 = 4;
    constneg1 = -1;
    var_a0 = (void *)(temp_v0 + 0x34);
    var_v1 = var_a1 + 0x58;
    do
    {
      *var_a1 = 0;
      *(s8 *)(var_v1 - 0x57) = 0;
      *(s16 *)(var_v1 - 0x56) = 0;
      *(u16 *)(var_v1 - 0x54) = *(u16 *)(var_a3 + 0);
      *(s16 *)(var_v1 - 0x50) = 0;
      *(s32 *)(var_v1 - 0x4C) = 0;
      *(s16 *)(var_v1 - 0x4E) = const4;
      *(s16 *)(var_v1 - 0x48) = 0;
      *(s16 *)(var_v1 - 0x46) = 0;
      *(s16 *)(var_v1 - 0x44) = 0;
      *(s32 *)(var_v1 - 0xC) = *(s32 *)((s8 *)var_a0 - 8);
      var_a1 += 0x68;
      *(s32 *)(var_v1 - 8) = *(s32 *)((s8 *)var_a0 - 4);
      var_a2 += 1;
      temp_v0_2 = *(s32 *)((s8 *)var_a0 + 0);
      var_a0 = (s8 *)var_a0 + 0x68;
      *(s16 *)(var_v1 - 0x52) = 0;
      *(s32 *)(var_v1 + 0) = constneg1;
      *(s32 *)(var_v1 - 4) = temp_v0_2;
      var_v1 += 0x68;
    }
    while (var_a2 < *(s16 *)(var_a3 + 6));
  }
}
INCLUDE_ASM("asm/funcs", efc_rob_set_type_flash);
/* kengo:HIGH  |  is_efc_rob/efc_rob_set_type_flash  |  204i */
INCLUDE_ASM("asm/funcs", func_80044B30);
extern s16 D_800A9CF8;
extern s32 D_800A9D04;
extern s32 D_800A9D00;
extern void func_80044100(s32, s32);
void func_80044C70(s32 a0) {
    func_80044100((s32)D_800A9CF8, a0);
    D_800A9D04 += a0;
    D_800A9D00 += a0;
}
extern s32 func_8007DF20(s32);
extern s32 func_8007DFEC(s32);
void func_80044CCC(s16 *a0, s16 *a1, s32 a2, s32 a3) {
    s32 sp18[3];
    s32 sp28[3];
    s16 angle;
    s32 radius;
    s32 v0;

    sp18[1] = a0[0];
    angle = a0[1];
    radius = a0[2];
    sp18[0] = (func_8007DF20(angle) * radius) >> 12;
    v0 = func_8007DFEC(angle);
    sp18[1] = -sp18[1];
    sp18[2] = (v0 * radius) >> 12;
    sp18[2] = -sp18[2];

    sp28[1] = a1[0];
    angle = a1[1];
    radius = a1[2];
    sp28[0] = (func_8007DF20(angle) * radius) >> 12;
    v0 = func_8007DFEC(angle);
    sp28[1] = -sp28[1];
    sp28[2] = (v0 * radius) >> 12;
    sp28[2] = -sp28[2];

    func_8007E1AC(sp18, sp28, 0x1000 - a2, a2, a3);
}
extern void func_8007E1AC(s32 *, s32 *, s32, s32, s32);
void func_80044DE4(s16 *a0, s16 *a1, s32 a2, s32 a3) {
    s32 sp18[4];
    s32 sp28[4];
    sp18[0] = a0[0];
    a0++;
    sp18[1] = -a0[0];
    sp18[2] = -a0[1];
    sp28[0] = a1[0];
    a1++;
    sp28[1] = -a1[0];
    sp28[2] = -a1[1];
    func_8007E1AC(sp18, sp28, 0x1000 - a2, a2, a3);
}
s32 func_80044E64(void) {
    return 0x25;
}
s32 func_80044E6C(void) {
    return 0x26;
}
extern void func_80036F40(void);
extern void func_80036E34(s32, s32, s32, s32);

typedef struct { s16 x; s16 y; } Coord;
extern Coord D_800963EC[];

void func_80044E74(s32 a0, s32 a1) {
    func_80036F40();
    func_80036E34(0, a1, D_800963EC[a0].x, D_800963EC[a0].y);
    func_80036F40();
}
void func_80044ED8(s32 a0, s32 a1) {
    if (a0 >= 0x1F) {
        a0 -= 0x1B;
    }
    if (!func_800450F4(a0, a1)) {
        func_80044E74(a0, a1);
    }
}
extern void func_80044E74(s32, s32);
extern s32 func_800450F4(s32, s32);
extern void func_80044E74(s32, s32);
void func_80044F30(s32 a0) {
    ((void (*)(s32))func_80044E74)(a0 + 0x27);
}
void func_80044F50(s32 a0, s32 a1, s32 a2) {
    if (!a0) {
        func_80044E74(a1 + 0x83, a2);
    } else {
        func_80044E74(a1 + 0x10C, a2);
    }
}
void func_80044F80(s32 a0, s32 a1) {
    func_80044E74(a0 + 0x4D, a1);
}
extern s32 D_800A3240;
extern char D_8001528C[];
s32 func_80044FA0(s32 a0, s32 a1) {
    s32 v0;
    s32 s0;

    s0 = a1 - (s32)func_80045814();
    if (s0 < 0) {
        goto set_from_table;
    }
    v0 = func_80045808();
    if (s0 >= v0) {
        goto set_from_table;
    }
    if (D_800A3240 != 0) {
        s0 = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
    } else {
        s0 = 0;
    }
    v0 = func_800457DC();
    if (v0 < s0) {
        func_80079208(D_8001528C, a0, s0 - v0);
        while (1) {
            func_800164F8();
        }
    }
    goto do_return;
set_from_table:
    s0 = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
do_return:
    func_80044E74(a0, a1);
    return s0;
}
extern s16 D_800963EE;
extern s32 func_800457DC(void);
s32 func_80045080(s32 a0) {
    s32 val = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
    return func_800457DC() - val;
}
void func_800450BC(s32 a0, s32 a1) {
    func_80044E74(a0 + 0x25, a1);
    D_800A3398 = a1;
    D_800A3244 = 1;
}
extern s16 D_800973EC[];
s32 func_800450F4(s32 a0, s32 a1) {
    s32 *a2_ptr;
    s32 v1;
    s32 *v0_ptr;
    if (D_800A3244 == 0) {
        return 0;
    }
    a0 -= 0x1E;
    if ((u32)a0 >= 7) {
        return 0;
    }
    a0 = D_800973EC[a0];
    if (a0 == -1) {
        return 0;
    }
    a2_ptr = (s32 *)D_800A3398;
    if (*a2_ptr != 5) {
        D_800A3244 = 0;
        return 0;
    }
    v0_ptr = (s32 *)(a0 * 4 + (s32)a2_ptr + 4);
    v1 = *v0_ptr;
    {
        s32 a0_new = (s32)a2_ptr + (((u32)v1 >> 2) << 2);
        s32 a2_val = v0_ptr[1] - v1;
        func_800520B8(a0_new, a1, a2_val);
    }
    return 1;
}
extern s32 D_800A3244;
void func_80045188(void) {
    D_800A3244 = 0;
}
s32 func_80045194(void) {
    return D_800A3244;
}
void func_800451A0(void) {
    func_80036E34(1, (s32)D_800963EC, 0, 2);
}
void func_800451D0(void) {
    s32 v1 = -1;
    s32 v0 = 0x90;
L_loop:
    *(s16 *)((u8 *)D_800EED10 + v0) = v1;
    v0 -= 0x10;
    if (v0 >= 0) goto L_loop;
    D_800A33A0 = (s32)&D_800A9D10;
    D_800A33A4 = 0x45000;
    D_800A33AC = 0;
    D_800A33A8 = 0;
    func_80049E1C();
}
void func_80045230(s32 a0) {
    s32 v1;
    if (!a0) {
        a0 = D_800A33A0;
    }
    a0 -= (s32)&D_800A9D10;
    v1 = a0;
    if (a0 < D_800A33A8) {
        v1 = D_800A33A8;
    }
    D_800A33A8 = v1;
    if (a0 > 0x44FFF) {
        func_80052C10();
    }
}
INCLUDE_ASM("asm/funcs", func_80045294);
INCLUDE_ASM("asm/funcs", func_800453E0);
void func_80045510(s32 a0, s32 a1) {
    volatile s32 sp_pad;
    s32 i = 0;
    s32 count = D_800A33AC;
    if (count <= 0) return;
    {
        s16 *s0 = (s16 *)&D_800EED18;
        s32 v1 = 0;
        do {
            if (*(s16 *)((s32)&D_800EED10 + v1) == a0) {
                s32 diff = a1 - *(s32 *)s0;
                if (diff == 0) return;
                func_80045294(i + 1, diff);
                *(s32 *)s0 = a1;
                return;
            }
            s0 = (s16 *)((u8 *)s0 + 0x10);
            count = D_800A33AC;
            i += 1;
            v1 += 0x10;
        } while (i < count);
    }
}
void func_800455AC(s32 a0) {
    register s32 v1 asm("v1");
    s16 (*new_var)[];
    func_800453E0(a0);
    v1 = D_800A33AC;
    new_var = &D_800EED10;
    D_800A33AC = v1 + 1;
    *((s32 *)((u8 *)(*new_var) + v1 * 16 + 4)) = D_800A33A0;
    *((s16 *)((u8 *)(*new_var) + v1 * 16)) = a0;
    *((s32 *)((u8 *)(*new_var) + v1 * 16 + 0xC)) = 0;
}
void func_80045600(s32 a0, s32 a1) {
    volatile s32 sp_pad;
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        do {
            a3 = a2;
            if (*(s16 *)a3 == a0) goto found;
            i++;
            a2 = (s16 *)((u8 *)a3 + 0x10);
        } while (i < count);
    }
found:
    if (i < D_800A33AC) {
        register s32 old_a0 asm("v0") = D_800A33A0;
        s32 old_a4 = D_800A33A4;
        a0 = a1 - old_a0;
        old_a0 = old_a0 + a0;
        old_a4 = old_a4 - a0;
        *(s32 *)((u8 *)a3 + 8) = a0;
        D_800A33A0 = old_a0;
        D_800A33A4 = old_a4;
        return;
    }
not_found:
    func_80052C10();
}
extern s16 D_800EED10[];
extern s32 D_800EED1C[];
extern s32 D_800A33AC;
void func_80045694(s32 a0, s32 a1) {
    s32 i;
    s32 count = D_800A33AC;
    if (count <= 0) return;
    i = 0;
    do {
        if (*(s16 *)((u8 *)D_800EED10 + i) == a0) {
            *(s32 *)((u8 *)D_800EED1C + i) = a1;
            return;
        }
        i += 0x10;
    } while (i < count * 16);
}
void func_800456F0(s32 a0) {
    s32 i;
    s32 count = D_800A33AC;
    if (count <= 0) return;
    i = 0;
    do {
        if (*(s16 *)((u8 *)D_800EED10 + i) == a0) {
            *(s32 *)((u8 *)D_800EED1C + i) = 0;
            return;
        }
        i += 0x10;
    } while (i < count * 16);
}
s32 *func_8004574C(s32 arg0) {
    s32 *var_a1;
    s32 var_v1;
    s32 limit;

    if (D_800A33AC > 0) {
        var_a1 = (s32 *)&D_800EED10;
        var_v1 = 0;
        limit = D_800A33AC << 4;
        do {
            if (*(s16 *)((s32)&D_800EED10 + var_v1) == arg0) {
                return var_a1;
            }
            var_v1 += 0x10;
            var_a1 = (s32 *)((s32)var_a1 + 0x10);
        } while (var_v1 < limit);
    }
    return NULL;
}
s32 func_800457A0(s32 a0) {
    s32 *v0 = func_8004574C();
    if (v0) {
        return v0[1];
    }
    return 0;
}
void func_800457D4(void) {
}
extern s32 D_800A33A4;
s32 func_800457DC(void) {
    return D_800A33A4;
}
extern s32 D_800A33A8;
s32 func_800457E8(void) {
    return 0x45000 - D_800A33A8;
}
extern s32 D_800A33A0;
s32 func_800457FC(void) {
    return D_800A33A0;
}
s32 func_80045808(void) {
    return 0x45000;
}
extern u8 D_800A9D10;
void *func_80045814(void) {
    return &D_800A9D10;
}
extern void func_800520B8(s32, s32, s32);
void func_80045824(s32 a0, s32 a1, s32 a2) {
    func_80045230(a1 + a2);
    func_800520B8(a0, a1, a2);
}
INCLUDE_ASM("asm/funcs", func_80045878);
void func_80045A28(s32 a0, s32 a1) {
    func_80045510(a0 + 3, a1);
    func_80045230(0);
}
extern void func_8005B644(void);
extern void func_800456F0(s32);
void func_80045A50(s32 a0) {
    s32 a0p3 = a0 + 3;
    func_8005B644();
    func_800456F0(a0p3);
    func_800456F0(a0);
    func_800453E0(a0p3);
    func_800453E0(a0);
}
extern void func_80044100(s32, s32);
extern void func_8005C4C0(s32, s32);
void func_80045AA4(s32 a0, s32 a1) {
    s32 *ptr;
    s32 idx;
    if (a0 < 3) {
        func_80041430(a0, a1);
        return;
    }
    idx = a0 - 3;
    func_80044100(idx, a1);
    func_80044100(a0, a1);
    ptr = (s32 *)func_800457A0(idx);
    if (ptr == 0) return;
    ptr[7] = ptr[7] + a1;
    func_8004019C((s32)ptr, a1);
    if ((ptr[0] >> 1) & 1) {
        s32 val = *(s16 *)((u8 *)ptr + 4);
        idx = 3 * val + 1;
        func_8005C4C0(a1, idx);
    }
}
INCLUDE_ASM("asm/funcs", func_80045B68);
extern void func_8005B6AC(void);
void func_80046020(void) {
    func_800453E0(6);
    func_8005B6AC();
}
extern void func_8005C4C0(s32, s32);
void func_80046048(s32 a0, s32 a1) {
    s32 *s0;
    s32 count;
    func_80044100(6, a1);
    s0 = (s32 *)func_800457A0(6);
    if (s0 == NULL) {
        return;
    }
    {
        s32 off = *s0;
        if (off == 0) {
            return;
        }
        s0 = (s32 *)((u8 *)s0 + off);
    }
    count = *s0;
    count = count - 1;
    if (count == -1) {
        return;
    }
    s0++;
    do {
        func_8005C4C0(a1, *s0++);
        count--;
    } while (count != -1);
}
INCLUDE_ASM("asm/funcs", func_800460E4);
extern s16 g_stage_id;
extern s16 g_stage_variant;
void func_800464C4(void) {
    s32 *s0;
    s32 *s1;
    s32 *a0;
    s32 v0;

    if (g_stage_variant == 0) {
        return;
    }
    s0 = (s32 *)func_800457A0(7);
    v0 = ((u32)s0[1] >> 2) << 2;
    a0 = (s32 *)((u8 *)s0 + v0);
    switch (g_stage_id) {
    case 0xD:
        v0 = ((u32)s0[6] >> 2) << 2;
        s1 = (s32 *)((u8 *)s0 + v0);
        break;
    case 3:
    case 0x22:
        v0 = ((u32)s0[5] >> 2) << 2;
        s1 = (s32 *)((u8 *)s0 + v0);
        break;
    }
    func_80044010(a0, 7);
    md_game_check_mode(s1, 7);
    func_80045510(7, (s32)((u8 *)s1 - (u8 *)s0));
    g_stage_variant = 0;
}
void func_8004659C(s32 a0) {
    s32 *s0;
    s32 *s2;
    s32 *s3;
    s32 *s4p;
    s32 *s1p;
    s32 *s0p;
    s32 v0;
    if (a0 < 0) {
        func_800464C4();
        return;
    }
    if (g_stage_variant == 0) {
        return;
    }
    v0 = func_800457A0(7);
    s0 = (s32 *)v0;
    v0 += 4;
    v0 = v0 + a0 * 20;
    s2 = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s3 = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s4p = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s1p = (s32 *)((u8 *)s0 + *(s32 *)v0);
    s0p = (s32 *)((u8 *)s0 + *(s32 *)(v0 + 4));
    func_80044098(7);
    func_80044010(s2, 7);
    func_80054410(s3);
    g_snd_bgm_id = (s32)s4p;
    g_snd_se_id = (s32)s1p;
    md_game_check_mode(s0p, 7);
}
extern s16 g_stage_id;
extern s16 g_stage_variant;
extern void func_800453E0(s32);
void func_8004668C(void) {
    func_800453E0(7);
    g_stage_id = -1;
    g_stage_variant = 0;
}
INCLUDE_ASM("asm/funcs", func_800466C0);
}
