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
extern void func_80089024(s32, s32);
extern s32 D_800A2D14;
extern s32 func_800789B8(void);
extern void func_800789C8(void);
extern void func_8008D050(s32 *);
extern s32 D_800A307C;
/* --- Functions 0x80083BE4 - 0x8008D060 (text4 segment) --- */

INCLUDE_ASM("asm/funcs", func_80083BE4);
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
INCLUDE_ASM("asm/funcs", func_800859F0);
INCLUDE_ASM("asm/funcs", func_80085A40);
INCLUDE_ASM("asm/funcs", func_80085E4C);
INCLUDE_ASM("asm/funcs", func_80085EE4);

void func_80085F98(void) {
    func_80089D60(0);
}

INCLUDE_ASM("asm/funcs", func_80085FB8);
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
INCLUDE_ASM("asm/funcs", func_80087F00);

void func_80087F24(void) {
    D_800FF578 = 0;
}

INCLUDE_ASM("asm/funcs", func_80087F34);
INCLUDE_ASM("asm/funcs", func_80087F64);
INCLUDE_ASM("asm/funcs", func_80087FE8);

s16 func_80088088(s32 a0, s16 a1, s32 a2) {
    return func_800880E8(a0, a1, 1, a2);
}

s16 func_800880B8(s32 a0, s16 a1, s32 a2) {
    return func_800880E8(a0, a1, 1, a2);
}
INCLUDE_ASM("asm/funcs", func_800880E8);
INCLUDE_ASM("asm/funcs", func_800884C4);

s16 func_80088584(s16 a0) {
    return func_8008AEB0(a0);
}

void func_800885AC(void) {
    func_800885CC(0);
}

INCLUDE_ASM("asm/funcs", func_800885CC);
INCLUDE_ASM("asm/funcs", func_800886C4);
INCLUDE_ASM("asm/funcs", func_80088740);
INCLUDE_ASM("asm/funcs", func_800889D4);
INCLUDE_ASM("asm/funcs", func_80088D0C);
INCLUDE_ASM("asm/funcs", func_80088F9C);
INCLUDE_ASM("asm/funcs", func_80089024);
INCLUDE_ASM("asm/funcs", func_8008908C);
INCLUDE_ASM("asm/funcs", func_800890D4);
INCLUDE_ASM("asm/funcs", func_80089214);
INCLUDE_ASM("asm/funcs", func_80089240);
INCLUDE_ASM("asm/funcs", func_8008926C);
void func_800892D4(s32 a0) {
    func_80082B20(4, a0);
}
INCLUDE_ASM("asm/funcs", func_800892F8);
INCLUDE_ASM("asm/funcs", func_80089374);
INCLUDE_ASM("asm/funcs", func_80089384);
INCLUDE_ASM("asm/funcs", func_800893D8);
INCLUDE_ASM("asm/funcs", func_800896A0);
INCLUDE_ASM("asm/funcs", func_800899A8);

void func_80089A24(s32 a0, s32 a1) {
    func_80089A48(a0, a1, 0xCA, 0xCB);
}

INCLUDE_ASM("asm/funcs", func_80089A48);
INCLUDE_ASM("asm/funcs", func_80089D10);
INCLUDE_ASM("asm/funcs", func_80089D60);
INCLUDE_ASM("asm/funcs", func_80089EB0);
INCLUDE_ASM("asm/funcs", func_80089F3C);
INCLUDE_ASM("asm/funcs", func_8008A434);

void func_8008A904(s32 a0, s32 a1) {
    func_80089A48(a0, a1, 0xCC, 0xCD);
}

INCLUDE_ASM("asm/funcs", func_8008A928);
INCLUDE_ASM("asm/funcs", func_8008AAC4);
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
INCLUDE_ASM("asm/funcs", func_8008ADC4);
INCLUDE_ASM("asm/funcs", func_8008AE24);
INCLUDE_ASM("asm/funcs", func_8008AE7C);
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
INCLUDE_ASM("asm/funcs", func_8008BE4C);
INCLUDE_ASM("asm/funcs", func_8008BEA4);
INCLUDE_ASM("asm/funcs", func_8008BF04);
INCLUDE_ASM("asm/funcs", func_8008C184);
INCLUDE_ASM("asm/funcs", func_8008C1E8);
INCLUDE_ASM("asm/funcs", func_8008C464);
INCLUDE_ASM("asm/funcs", func_8008D050);
INCLUDE_ASM("asm/funcs", func_8008D060);
