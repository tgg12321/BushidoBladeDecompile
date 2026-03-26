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
extern s32 D_800A33B0;
extern s32 D_800A33B4;
extern s32 D_800A3240;
extern s32 D_800A3398;
extern s32 D_800A3244;
extern s16 D_800963EC[];
extern s16 D_800963EE;
extern void func_8003EDC0(s32 *, s32);
extern void func_80054410(s32 *);
extern s32 func_800457A0(s32);
extern void func_80041430(s32, s32);
extern s32 func_8004019C(s32 *, s32);
/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */

INCLUDE_ASM("asm/funcs", func_800401CC);
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
extern s32 D_800A9A10[];
extern s32 D_80094B88[];
void func_800404D8(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        D_800A9A10[i] = 0;
        D_80094B88[i] = 0;
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
    s32 idx = a0;
    ptr = func_80045878(a0);
    D_800A9A10[idx] = (s32)ptr;
    func_80040594(ptr);
    do { func_800408F8(ptr); func_80040B44(ptr); func_8003F824(ptr, 1); } while (0);
    func_8003FFC4(ptr);
    func_80040CB8(ptr);
    func_8003E120();
    return ptr;
}
INCLUDE_ASM("asm/funcs", func_80040594);
INCLUDE_ASM("asm/funcs", func_800408F8);
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
INCLUDE_ASM("asm/funcs", func_80040B44);
extern s16 D_80094B9E[];
INCLUDE_ASM("asm/funcs", func_80040CB8);
INCLUDE_ASM("asm/funcs", func_80040D48);
INCLUDE_ASM("asm/funcs", func_80041188);
extern s32 *D_80015820[];
extern s32 func_800545F4;
extern s32 D_800545F8;
extern s32 D_800545FC;
extern s32 D_80054600;
INCLUDE_ASM("asm/funcs", func_80041398);
void func_80041430(s32 a0, s32 a1) {
    s32 *base;
    s32 *s0;
    s32 i;
    base = (s32 *)((u8 *)D_800A9A10 + a0 * 4);
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
extern s32 D_800A9A10[];
s32 func_8004153C(s32 a0) {
    return D_800A9A10[a0];
}
s32 func_80041554(s32 a0) {
    s16 *ptr = (s16 *)D_800A9A10[a0];
    if (ptr) {
        return ptr[4];
    }
    return -1;
}
s32 func_80041584(void) {
    s32 i;
    s32 ret = -1;
    for (i = 0; i < 3; i++) {
        if (D_800A9A10[i] == 0) {
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
    D_800A9A10[a0] = 0;
}
extern s32 D_80094B88[];
void func_80041604(s32 a0, s32 a1) {
    s16 *ptr = (s16 *)D_800A9A10[a0];
    if (ptr) {
        s32 val = ptr[1];
        if ((val & 0x1F) != a1) {
            ptr[3] = -2;
        }
    }
    D_80094B88[a0] = a1;
}
s32 func_80041650(s32 a0) {
    s16 *ptr = (s16 *)D_800A9A10[a0];
    if (ptr) {
        s32 val = ptr[1];
        return val & 0x1F;
    }
    return -1;
}
INCLUDE_ASM("asm/funcs", func_80041688);
INCLUDE_ASM("asm/funcs", func_800417D0);
typedef struct { s32 w[4]; } Block16;
extern s32 D_800F66A0[];
void func_800418D0(s32 *a0) {
    s16 sp10[4];
    void (*func)(s16 *, s32 *);
    sp10[0] = -(u16)((u16 *)a0)[8];
    sp10[1] = -(u16)((u16 *)a0)[9];
    sp10[2] = -(u16)((u16 *)a0)[10];
    func = (void (*)(s16 *, s32 *))D_800F66A0[((s16 *)a0)[4]];
    func(sp10, a0 + 14);
    ((Block16 *)(a0 + 6))[0] = ((Block16 *)(a0 + 14))[0];
    ((Block16 *)(a0 + 6))[1] = ((Block16 *)(a0 + 14))[1];
}
INCLUDE_ASM("asm/funcs", func_80041988);
INCLUDE_ASM("asm/funcs", func_80041AC8);
INCLUDE_ASM("asm/funcs", func_80041BF4);
INCLUDE_ASM("asm/funcs", func_80041E10);
INCLUDE_ASM("asm/funcs", func_80041EB0);
extern s16 D_800A3382;
extern s16 D_800A3238;
void func_800420D0(void) {
    extern s16 D_800A3380;
    D_800A3382 = 0;
    D_800A3380 = 0;
    D_800A3238 = -1;
}
extern s16 D_800A3380[];
extern s32 D_800A3384[];
void func_800420E8(s32 a0, s32 a1) {
    if (a0 < 2) {
        D_800A3380[a0] = 1;
        D_800A3384[a0] = a1;
    }
}
extern s32 D_800A3388;
void func_8004211C(void) {
    s32 val = D_800A3380[0] * 2 + D_800A3382;
    switch (val) {
    case 1:
        func_80041EB0(0, D_800A3388);
        break;
    case 2:
        func_80041EB0(D_800A3384[0], 0);
        break;
    case 3:
        func_80041EB0(D_800A3384[0], D_800A3388);
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
extern s32 D_80094E48[];
void func_800421C8(s32 a0) {
    s32 *p = (s32 *)((u8 *)D_80094E48 + a0 * 24);
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
extern void func_80016768(s32, s32, s32, s32);
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
    func_80016768(1, r, g, b);
    func_8007EFBC(r, g, b);
}
INCLUDE_ASM("asm/funcs", func_80042504);
INCLUDE_ASM("asm/funcs", func_80042684);
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
INCLUDE_ASM("asm/funcs", func_80042C80);
extern void func_8004A348(void);
extern void func_80042874(void);
extern void func_80042A88(void);
extern void func_80042C80(void);
extern s32 D_800F66A8;
extern s32 D_800F66B0;
extern s32 D_800F66B4;
void func_80042E90(void) {
    D_800F66A0[0] = (s32)func_8004A348;
    D_800F66A8 = (s32)func_80042874;
    D_800F66B0 = (s32)func_80042A88;
    D_800F66B4 = (s32)func_80042C80;
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
extern s16 D_800973FC[];
void func_80042F10(s32 *a0, s32 *a1, s32 a2) {
    s16 sin_val, cos_val;
    s32 x, y;
    s32 sin_x, cos_x, sin_y, cos_y;
    sin_val = D_800973FC[(a2 + 0x400) & 0xFFF];
    x = *a0;
    cos_val = D_800973FC[a2 & 0xFFF];
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
INCLUDE_ASM("asm/funcs", func_800433E4);
INCLUDE_ASM("asm/funcs", func_80043454);
INCLUDE_ASM("asm/funcs", func_80043BD0);
INCLUDE_ASM("asm/funcs", func_80043C7C);
INCLUDE_ASM("asm/funcs", func_80043D34);
INCLUDE_ASM("asm/funcs", func_80043DE0);
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
INCLUDE_ASM("asm/funcs", func_80044010);
INCLUDE_ASM("asm/funcs", func_80044098);
INCLUDE_ASM("asm/funcs", func_80044100);
INCLUDE_ASM("asm/funcs", func_80044170);
INCLUDE_ASM("asm/funcs", func_8004428C);
INCLUDE_ASM("asm/funcs", func_80044378);
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
INCLUDE_ASM("asm/funcs", func_80044504);
extern void func_80052C10(void);
void func_80044650(void) {
    func_80052C10();
}
INCLUDE_ASM("asm/funcs", func_80044670);
INCLUDE_ASM("asm/funcs", func_8004473C);
INCLUDE_ASM("asm/funcs", func_80044800);
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
INCLUDE_ASM("asm/funcs", func_80044CCC);
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
extern s32 func_800450F4(s32, s32);
void func_80044ED8(s32 a0, s32 a1) {
    if (a0 >= 0x1F) {
        a0 -= 0x1B;
    }
    if (!func_800450F4(a0, a1)) {
        func_80044E74(a0, a1);
    }
}
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
INCLUDE_ASM("asm/funcs", func_80044FA0);
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
INCLUDE_ASM("asm/funcs", func_800450F4);
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
INCLUDE_ASM("asm/funcs", func_80045510);
INCLUDE_ASM("asm/funcs", func_800455AC);
INCLUDE_ASM("asm/funcs", func_80045600);
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
s32 func_800457A0(void) {
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
INCLUDE_ASM("asm/funcs", func_80045AA4);
INCLUDE_ASM("asm/funcs", func_80045B68);
extern void func_8005B6AC(void);
void func_80046020(void) {
    func_800453E0(6);
    func_8005B6AC();
}
extern void func_8005C4C0(s32, s32);
INCLUDE_ASM("asm/funcs", func_80046048);
INCLUDE_ASM("asm/funcs", func_800460E4);
INCLUDE_ASM("asm/funcs", func_800464C4);
INCLUDE_ASM("asm/funcs", func_8004659C);
extern s16 D_80099478;
extern s16 D_8009947A;
extern void func_800453E0(s32);
void func_8004668C(void) {
    func_800453E0(7);
    D_80099478 = -1;
    D_8009947A = 0;
}
INCLUDE_ASM("asm/funcs", func_800466C0);
