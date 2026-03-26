#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

/* Forward declarations */
extern void func_80089D60(s32);
extern void func_800885CC(s32);
extern s32 func_8008AEB0(s32);
extern void func_80084974(s16, s16);
extern s16 func_800880E8(s32, s16, s32, s32);
extern void func_80089A48(s32, s32, s32, s32);

/* Externs for globals */
extern s16 D_800F66F8;
extern s16 D_800FF578;
extern s32 D_800A2D10;
extern s32 D_800A2CDC;

extern void func_80082B20(s32, s32);
extern s32 func_80089024(s32, s32);
extern volatile s32 D_800A2D14;
extern s32 D_800A2874;
extern s32 D_800A2878;
extern s32 D_800A2CF8;
extern u16 D_800A2CF4;
extern s32 D_800A2D04;
extern s32 func_800789B8(void);
extern void func_800789C8(void);
extern void func_8008D050(s32 *);
extern s32 D_800A307C;
extern s32 D_80106F28;
extern s32 func_80078998(s32);
extern void func_80082AC0(void);
extern void func_80088740(s32);
extern void func_8008908C(s32, s32, s32);
extern s32 D_800A287C;
extern s32 D_800A2880;
extern s32 D_800A2884;
extern s32 D_800A288C;
extern s16 D_800A2890;
extern s16 D_800A2892;
extern s32 D_800A2894;
extern s32 D_800A2898;
extern s32 D_800A289C;
extern s32 D_800A28A0;
extern s16 D_800A28D2;
extern s32 D_800A2CD4;
extern s32 D_800A2CF8;
extern s32 D_800A2D38;
extern s32 D_800A2D3C;
extern s32 D_800A2D40;
extern s32 D_800A2D44;
extern s32 func_80088F9C(s32, s32);

/* --- Functions 0x80083BE4 - 0x8008D060 (text4 segment) --- */

extern void func_8008AF9C(void *);
void func_80083BE4(s16 a0, s16 a1) {
    s32 buf[10];
    buf[0] = 3;
    *(s16 *)&buf[1] = (s16)(a0 * 129);
    *((s16 *)&buf[1] + 1) = (s16)(a1 * 129);
    func_8008AF9C(buf);
}
INCLUDE_ASM("asm/funcs", func_80083C34);
INCLUDE_ASM("asm/funcs", func_80083E9C);
INCLUDE_ASM("asm/funcs", func_800841E0);
INCLUDE_ASM("asm/funcs", func_80084500);
INCLUDE_ASM("asm/funcs", func_800848AC);

void func_80084948(s16 a0, s16 a1) {
    func_80084974(a0, a1);
}

INCLUDE_ASM("asm/funcs", func_80084974);
INCLUDE_ASM("asm/funcs", func_80084A7C);
INCLUDE_ASM("asm/funcs", func_80084CC0);
s32 func_80085064(s32 arg0, s16 arg1) {
    s32 result;
    u8 *ptr;
    u8 **base;
    s32 val;
    s32 byte;

    base = (u8 **)((*(s32 *)((u8 *)&D_80106F28 + ((s32)(arg0 << 16) >> 14))) + (arg1 * 0xB0));
    ptr = *base;
    *base = ptr + 1;
    val = *ptr;
    if (val == 0) {
        return 0;
    }
    result = val << 2;
    if (val & 0x80) {
        val &= 0x7F;
        do {
            ptr = *base;
            *base = ptr + 1;
            byte = *ptr;
            val = (val << 7) + (byte & 0x7F);
        } while (byte & 0x80);
    }
    result = val << 2;
    result = (result + val) << 1;
    *(s32 *)((u8 *)base + 0x88) += result;
    return result;
}
void func_80085114(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = (s16)a1 * 0xB0;
    s32 base = *base_ptr;
    u8 *entry = (u8 *)(base + (s16)a1 * 0xB0);
    entry[0x20] = 1;
    entry[0x21] = 0;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~0x100;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~2;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~4;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~0x200;
    *(s32 *)entry = *(s32 *)(entry + 4);
    entry[0x14] = 1;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 1;
}
void func_80085210(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 new_var;
    s32 offset = a1 * 0xB0;
    s32 base = *base_ptr;
    if (1) {
        *(u8 *)((base + offset) + 0x14) = 1;
        base = (new_var = *base_ptr);
        *(s32 *)((offset + base) + 0x98) &= ~8;
    }
}
INCLUDE_ASM("asm/funcs", func_80085270);
INCLUDE_ASM("asm/funcs", func_80085448);
INCLUDE_ASM("asm/funcs", func_80085544);
INCLUDE_ASM("asm/funcs", func_800856B0);
INCLUDE_ASM("asm/funcs", func_800858D0);
extern u8 D_80102A68[];
extern s32 D_80107810[];
s32 func_800859F0(s16 a0) {
    if ((u16)a0 >= 0x11) {
        return -1;
    }
    if (D_80102A68[a0] != 1) {
        return -1;
    }
    return D_80107810[a0];
}
INCLUDE_ASM("asm/funcs", func_80085A40);
extern s32 D_800F5750;
extern s16 D_800F5758;
extern s16 D_800F575A;
extern void func_80089F3C(s32 *);
void func_80085E4C(s16 a0, s16 a1) {
    s32 x = (s16)a0 * 32767 / 127;
    s32 y = (s16)a1 * 32767 / 127;
    s32 *buf = &D_800F5750;
    *buf = 6;
    D_800F5758 = x;
    D_800F575A = y;
    func_80089F3C(buf);
}
extern s32 D_800F5750;
extern s16 D_800F5758;
extern s16 D_800F575A;
INCLUDE_ASM("asm/funcs", func_80085EE4);

void func_80085F98(void) {
    func_80089D60(0);
}

INCLUDE_ASM("asm/funcs", func_80085FB8);
extern s16 D_80102A78[];
extern s16 D_80102A7A[];
extern u8 D_800F65E0[];

extern s16 D_80102A78[];
extern s16 D_80102A7A[];
extern u8 D_800F65E0[];

INCLUDE_ASM("asm/funcs", func_80086130);
INCLUDE_ASM("asm/funcs", func_800861BC);

void func_800863CC(void) {
    D_800F66F8 = 0;
}

INCLUDE_ASM("asm/funcs", func_800863DC);
INCLUDE_ASM("asm/funcs", func_80086818);
INCLUDE_ASM("asm/funcs", func_80086BFC);
INCLUDE_ASM("asm/funcs", func_80086CF8);
INCLUDE_ASM("asm/funcs", func_800871D4);
INCLUDE_ASM("asm/funcs", func_800872A4);
INCLUDE_ASM("asm/funcs", func_80087770);
INCLUDE_ASM("asm/funcs", func_80087CAC);
INCLUDE_ASM("asm/funcs", func_80087DA0);
INCLUDE_ASM("asm/funcs", func_80087E3C);
extern u8 D_80103600;
void func_80087F00(u8 a0) {
    D_80103600 = a0;
}
void func_80087F10(void) {
    D_800FF578 = 1;
}

void func_80087F24(void) {
    D_800FF578 = 0;
}

extern u8 D_80101BCC;
s32 func_80087F34(s32 a0) {
    u8 v = (u8)a0;
    if (v >= 0x19 || v == 0) {
        return 0xFF;
    }
    D_80101BCC = a0;
    return v;
}
extern u8 D_80102A68[];
extern s32 D_80107810[];
extern u16 D_80107808;
extern void func_800899A8(s32);
void func_80087F64(s16 a0) {
    if ((u16)a0 < 0x10) {
        s16 idx = a0;
        if (D_80102A68[idx] == 1) {
            func_800899A8(D_80107810[idx]);
            D_80102A68[idx] = 0;
            D_80107808--;
        }
    }
}
extern u8 D_80102A68[];

s16 func_80087FE8(s16 a0) {
    if ((u16)a0 < 0x11) {
        if (D_80102A68[a0] == 2) {
            func_8008AF58(0);
            D_80102A68[a0] = 1;
            return a0;
        }
    }
    return -1;
}

s16 func_80088058(s32 a0, s16 a1) {
    return func_800880E8(a0, a1, 0, 0);
}

s16 func_80088088(s32 a0, s16 a1, s32 a2) {
    return func_800880E8(a0, a1, 1, a2);
}

s16 func_800880B8(s32 a0, s16 a1, s32 a2) {
    return func_800880E8(a0, a1, 1, a2);
}
INCLUDE_ASM("asm/funcs", func_800880E8);
extern u8 D_80102A68[];
extern s32 D_80107810[];
extern s32 D_801077C8[];
extern s32 func_8008AE24(s32);
extern s32 func_8008ADC4(s32, s32);
s16 func_800884C4(s32 a0, s16 a1) {
    if ((u16)a1 >= 0x11) {
        func_8008AF58(0);
        return -1;
    }
    if (D_80102A68[a1] != 2) {
        func_8008AF58(0);
        return -1;
    }
    {
        s32 s0 = D_80107810[a1];
        func_8008AE7C(0);
        func_8008AE24(s0);
        func_8008ADC4(a0, D_801077C8[a1]);
        D_80102A68[a1] = 1;
    }
    return a1;
}

s16 func_80088584(s16 a0) {
    return func_8008AEB0(a0);
}

void func_800885AC(void) {
    func_800885CC(0);
}

void func_800885CC(s32 arg0) {
    u16 *var_v0;
    s32 var_v1;
    s32 val;

    func_80082AC0();
    func_80088740(arg0);
    val = 0xC000;
    if (arg0 == 0) {
        var_v1 = 0x17;
        var_v0 = (u16 *)&D_800A28D2;
        do {
            *var_v0 = val;
            var_v1 -= 1;
            var_v0 -= 1;
        } while (var_v1 >= 0);
    }
    func_800886C4();
    D_800A287C = 0;
    D_800A2880 = 0;
    D_800A288C = 0;
    D_800A2890 = 0;
    D_800A2892 = 0;
    D_800A2894 = 0;
    D_800A2898 = 0;
    D_800A2884 = D_800A2D44;
    func_8008908C(0xD1, D_800A2D44, 0);
    D_800A2D38 = 0;
    D_800A2D3C = 0;
    D_800A2D40 = 0;
    D_800A2878 = 0;
    D_800A2CF8 = 0;
    D_800A2874 = 0;
    D_800A28A0 = 0;
    D_800A289C = 0;
    D_800A2CD4 = 0;
}
extern s32 D_800A2CD8;
extern s32 D_800A2870;
extern s32 D_80088BA0;

void func_800886C4(void) {
    s32 v0;
    if (D_800A2CD8 == 0) {
        D_800A2CD8 = 1;
        func_800789B8();
        func_800892D4((s32)&D_80088BA0);
        v0 = func_80078978((s32)0xF0000009, 0x20, 0x2000, 0);
        D_800A2870 = v0;
        func_800789A8(v0);
        func_800789C8();
    }
}
INCLUDE_ASM("asm/funcs", func_80088740);
INCLUDE_ASM("asm/funcs", func_800889D4);
INCLUDE_ASM("asm/funcs", func_80088D0C);
extern void func_80088D0C(s32, ...);
extern void func_800889D4(s32, s32);
s32 func_80088F9C(s32 a0, s32 a1) {
    if (D_800A2CF8 == 0) {
        func_80088D0C(2, D_800A2CF4 << D_800A2D04);
        func_80088D0C(1);
        func_80088D0C(3, a0, a1);
    } else {
        func_800889D4(a0, a1);
    }
    return a1;
}
s32 func_80089024(s32 a0, s32 a1) {
    func_80088D0C(2, D_800A2CF4 << D_800A2D04);
    func_80088D0C(0);
    func_80088D0C(3, a0, a1);
    return a1;
}
INCLUDE_ASM("asm/funcs", func_8008908C);
INCLUDE_ASM("asm/funcs", func_800890D4);
extern volatile u32 *D_800A2CF0;
void func_80089214(void) {
    *D_800A2CF0 = (*D_800A2CF0 & 0xF0FFFFFF) | 0x20000000;
}
void func_80089240(void) {
    *D_800A2CF0 = (*D_800A2CF0 & 0xF0FFFFFF) | 0x22000000;
}
void func_8008926C(void) {
    volatile s32 i;
    volatile s32 v = 0xD;
    for (i = 0; i < 0x3C; i++) {
        v = v * 13;
    }
}
void func_800892D4(s32 a0) {
    func_80082B20(4, a0);
}
extern s32 D_800A2D18;

extern s32 D_800A2D18;

extern s32 D_800A2D18;

void func_800892F8(void) {
    if (D_800A2CD8 == 1) {
        D_800A2CD8 = 0;
        func_800789B8();
        D_800A2D14 = 0;
        D_800A2D18 = 0;
        func_800892D4(0);
        func_80078988(D_800A2870);
        func_80089374(D_800A2870);
        func_800789C8();
    }
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_80089374\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xD\n"
    "    nop\n"
    "endlabel func_80089374\n"
    ".set reorder\n"
    ".set at\n"
);
extern s32 D_800A2D38;
extern s32 D_800A2D3C;
extern s32 D_800A2D40;

extern s32 D_800A2D38;
extern s32 D_800A2D3C;
extern s32 D_800A2D40;

s32 func_80089384(s32 a0, s32 *a1) {
    int new_var2;
    int new_var;
    s32 v0 = a0;
    if (v0 <= 0) {
        new_var2 = 1;
        if (new_var2) {
            return 0;
        }
    }
    new_var = 0x10000 << D_800A2D04;
    *a1 = 0x40001010;
    D_800A2D40 = (s32)a1;
    D_800A2D3C = 0;
    D_800A2D38 = v0;
    a1[1] = new_var - 0x1010;
    return v0;
}
INCLUDE_ASM("asm/funcs", func_800893D8);
INCLUDE_ASM("asm/funcs", func_800896A0);
INCLUDE_ASM("asm/funcs", func_800899A8);

void func_80089A24(s32 a0, s32 a1) {
    func_80089A48(a0, a1, 0xCA, 0xCB);
}

INCLUDE_ASM("asm/funcs", func_80089A48);
s32 func_80089D10(s32 a0) {
    s32 val;
    if (a0 < 0) {
        val = 0;
    } else if (a0 >= 0x40) {
        val = 0x3F;
    } else {
        val = a0;
    }
    {
        volatile u16 *ptr = (volatile u16 *)(D_800A2CDC + 0x1AA);
        u16 tmp = *ptr;
        *ptr = (tmp & 0xC0FF) | ((val & 0x3F) << 8);
    }
    return val;
}
INCLUDE_ASM("asm/funcs", func_80089D60);
INCLUDE_ASM("asm/funcs", func_80089EB0);
INCLUDE_ASM("asm/funcs", func_80089F3C);
INCLUDE_ASM("asm/funcs", func_8008A434);

void func_8008A904(s32 a0, s32 a1) {
    func_80089A48(a0, a1, 0xCC, 0xCD);
}

INCLUDE_ASM("asm/funcs", func_8008A928);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008AAC4\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xA\n"
    "    nop\n"
    "endlabel func_8008AAC4\n"
    ".set reorder\n"
    ".set at\n"
);
INCLUDE_ASM("asm/funcs", func_8008AAD4);
INCLUDE_ASM("asm/funcs", func_8008ACD0);
s32 func_8008AD64(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    func_80089024(a0, a1);
    if (D_800A2D14 == 0) {
        D_800A2D10 = 0;
    }
    return a1;
}
s32 func_8008ADC4(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    func_80088F9C(a0, a1);
    if (D_800A2D14 == 0) {
        D_800A2D10 = 0;
    }
    return a1;
}
extern s32 func_800890D4(s32, s32);
s32 func_8008AE24(s32 a0) {
    s32 v0;
    if ((u32)(a0 - 0x1010) > (u32)0x7EFE8) {
        return 0;
    }
    v0 = func_800890D4(-1, a0);
    D_800A2CF4 = (u16)v0;
    return (u32)(u16)v0 << D_800A2D04;
}
void func_8008AE7C(s32 a0) {
    int new_var;
    s32 v0;
    new_var = 1;
    if (a0 == 0) {
        v0 = 0;
    } else if (new_var == a0) {
        v0 = new_var;
    } else {
        v0 = 0;
        v0++;
        v0--;
    }
    D_800A2878 = a0;
    D_800A2CF8 = v0;
}
s32 func_8008AEB0(s32 arg0) {
    s32 var_v0;

    if ((D_800A2878 == 1) || (D_800A2D10 == 1)) {
        return 1;
    }
    var_v0 = func_80078998(D_800A2870);
    if (arg0 == 1) {
        if (var_v0 == 0) {
            do {
                var_v0 = func_80078998(D_800A2870);
            } while (var_v0 == 0);
        }
        var_v0 = 1;
        goto block_8;
    }
    if (var_v0 == 1) {
block_8:
        D_800A2D10 = var_v0;
    }
    return var_v0;
}
void func_8008AF58(s32 a0) {
    if (a0 == 1) {
        D_800A2D10 = 0;
    } else {
        D_800A2D10 = 1;
    }
}

s32 func_8008AF84(void) {
    return D_800A2D10 != 1;
}

INCLUDE_ASM("asm/funcs", func_8008AF9C);
void func_8008B400(u8 *a0) {
    s32 limit = 24;
    s32 i = 0;
    s32 one = 1;
    s32 three = 3;
    s32 two = 2;
    u8 *buf = a0;
    do {
        s32 off = i << 4;
        u16 data;
        s32 bit;
        data = *((u16 *)((off + D_800A2CDC) + 0xC));
        bit = D_800A2874 & (one << i);
        if (bit) {
            if (data != 0) {
                *buf = one;
            } else {
                *buf = three;
            }
        } else if (data != 0) {
            *buf = two;
        } else {
            *buf = 0;
        }
        i++;
        buf++;
    } while (i < limit);
}
INCLUDE_ASM("asm/funcs", func_8008B488);
INCLUDE_ASM("asm/funcs", func_8008BB24);
INCLUDE_ASM("asm/funcs", func_8008BD88);

void func_8008BDE8(s32 a0, u16 *a1) {
    a0 = (a0 << 4) + D_800A2CDC;
    *a1 = *(u16 *)(a0 + 0xC);
}

void func_8008BE04(void) {
    s32 v0;
    v0 = func_800789B8();
    func_8008D050(&D_800A307C);
    if (v0 == 1) {
        func_800789C8();
    }
}
extern s32 D_800164A8;

void func_8008BE4C(void) {
    s32 v0;
    v0 = func_800789B8();
    func_8008D060(&D_800164A8);
    func_80078FF0();
    if (v0 == 1) {
        func_800789C8();
    }
}

void func_8008BE9C(void) {
}
extern s32 D_800F1AFC;
extern s32 D_800F1B00;
extern s32 D_800F1B04;
extern s32 D_800A3044;
s32 func_8008BEA4(int a0, int a1) {
    s32 *flag = &D_800F1AFC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1B04 = a1;
    if ((flag && flag) && flag) {
        do { } while (0);
    }
    D_800F1B00 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        {
            volatile u16 *ptr = (volatile u16 *)(reg + 0xA);
            *ptr |= 0x800;
            *ptr |= 0x20;
        }
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_8008BF04);
extern s32 D_800F1AEC;
extern s32 D_800F1AF0;
extern s32 D_800F1AF4;
extern s32 D_800F1AF8;
extern s32 D_800A3044;
s32 func_8008C184(int a0, int a1) {
    s32 *flag = &D_800F1AEC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1AF4 = a1;
    if ((flag && flag) && flag) {
        do { } while (0);
    }
    D_800F1AF0 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        D_800F1AF8 = *(u16 *)(reg + 4) & 0x80;
        *(volatile u16 *)(reg + 0xA) |= 0x400;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_8008C1E8);
INCLUDE_ASM("asm/funcs", func_8008C464);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008D050\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x47\n"
    "    nop\n"
    "endlabel func_8008D050\n"
    ".set reorder\n"
    ".set at\n"
);

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008D060\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x48\n"
    "    nop\n"
    ".global D_8008D070\n"
    ".type D_8008D070, @function\n"
    "D_8008D070:\n"
    "    .aent D_8008D070\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    ".global D_8008D090\n"
    ".type D_8008D090, @function\n"
    "D_8008D090:\n"
    "    .aent D_8008D090\n"
    "    .word 0x8001DCB0\n"
    "    .word 0x8001E878\n"
    "    .word 0x80033898\n"
    "    .word 0x80034708\n"
    "    .word 0x800397D4\n"
    "    .word 0x8003993C\n"
    "    .word 0x8003B9D0\n"
    "    .word 0x8003BCB4\n"
    "    .word 0x80035480\n"
    "    .word 0x80035828\n"
    "    .word 0x8003BE10\n"
    "    .word 0x8003BEA8\n"
    "    .word 0x8001EA04\n"
    "    .word 0x8001EA84\n"
    "    .word 0x80035430\n"
    "    .word 0x8003BFC4\n"
    "    .word 0x8001EEB4\n"
    "    .word 0x8001EFA0\n"
    "    .word 0x8003C040\n"
    "    .word 0x8003C2C0\n"
    "    .word 0x8003C42C\n"
    "    .word 0x8003C560\n"
    "    .word 0x8003B870\n"
    "    .word 0x8003B8E4\n"
    "    .word 0x8003C958\n"
    "    .word 0x8003C9A4\n"
    "    .word 0x80035DC8\n"
    "    .word 0x80035E38\n"
    "    .word 0x8003CE18\n"
    "    .word 0x8003CF84\n"
    "    .word 0x8003C714\n"
    "    .word 0x8003C8B4\n"
    "    .word 0x8003CCCC\n"
    "    .word 0x8003CD10\n"
    ".global D_8008D118\n"
    ".type D_8008D118, @function\n"
    "D_8008D118:\n"
    "    .aent D_8008D118\n"
    "    .word 0x0D0B0800\n"
    "    .word 0x15131110\n"
    "endlabel func_8008D060\n"
    ".set reorder\n"
    ".set at\n"
);
