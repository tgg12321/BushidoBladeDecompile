#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

/* Forward declarations */
extern void func_80089D60(s32);
extern void func_800885CC(s32);
extern s16 func_8008AEB0(s16);
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
extern s32 D_800A2D14;
extern s32 D_800A2874;
extern s32 D_800A2878;
extern s32 D_800A2CF8;
extern u16 D_800A2CF4;
extern s32 D_800A2D04;
extern s32 func_800789B8(void);
extern void func_800789C8(void);
extern void func_8008D050(s32 *);
extern s32 D_800A307C;
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
INCLUDE_ASM("asm/funcs", func_80085064);
INCLUDE_ASM("asm/funcs", func_80085114);
INCLUDE_ASM("asm/funcs", func_80085210);
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
INCLUDE_ASM("asm/funcs", func_80085E4C);
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

INCLUDE_ASM("asm/funcs", func_800885CC);
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
INCLUDE_ASM("asm/funcs", func_8008926C);
void func_800892D4(s32 a0) {
    func_80082B20(4, a0);
}
extern s32 D_800A2D18;

extern s32 D_800A2D18;

extern s32 D_800A2D18;

INCLUDE_ASM("asm/funcs", func_800892F8);
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

INCLUDE_ASM("asm/funcs", func_80089384);
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
INCLUDE_ASM("asm/funcs", func_8008AEB0);
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
INCLUDE_ASM("asm/funcs", func_8008B400);
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
INCLUDE_ASM("asm/funcs", func_8008BEA4);
INCLUDE_ASM("asm/funcs", func_8008BF04);
INCLUDE_ASM("asm/funcs", func_8008C184);
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
