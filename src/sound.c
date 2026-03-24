#include "common.h"
#include "include_asm.h"

/* Forward declarations for called functions */
extern void func_800453E0(s32);
extern void func_800455AC(s32);
extern void func_80045510(s32, s32);
extern void func_80045230(s32);
extern void func_80054FDC(s32);
extern void func_80078A68(u32, s32, s32);
extern void func_80078B04(u32);
extern void func_80078B3C(u32);
extern void func_8004473C(void);
extern void func_80044800(void);
extern void func_80048F58(s32, s32);
extern void func_80048FFC(s32);
extern void func_8004153C(void);
extern void func_800477E8(void);
extern void func_80047A90(void);

/* Externs for globals */
extern s16 D_80099478;
extern s16 D_8009947A;
extern u8 D_8009947C;
extern s16 D_800F6654;
extern s32 D_800A3790;
extern u8 D_800EEDD0;

/* --- Functions 0x80046780 - 0x80047EC8 --- */

INCLUDE_ASM("asm/funcs", func_80046780);
INCLUDE_ASM("asm/funcs", func_8004678C);

s32 func_80046798(void) {
    return D_80099478;
}

s32 func_800467A8(void) {
    return D_8009947A;
}

INCLUDE_ASM("asm/funcs", func_800467B8);

void func_800468B0(s32 a0) {
    func_80045510(8, a0);
    func_80045230(0);
}

INCLUDE_ASM("asm/funcs", func_800468DC);

void func_80046914(void) {
    func_800453E0(8);
}

void func_80046934(void) {
    func_800455AC(9);
}

void func_80046954(void) {
}

INCLUDE_ASM("asm/funcs", func_8004695C);

void func_800469A0(s32 a0) {
    func_80045510(9, a0);
}

INCLUDE_ASM("asm/funcs", func_800469C4);

void func_80046A60(void) {
    func_800453E0(0xA);
}

void func_80046A80(s32 a0, s32 a1) {
    func_80054FDC(a1);
}

INCLUDE_ASM("asm/funcs", func_80046AA0);

void func_80046AE8(void) {
    func_80078A68(0xF2000001, -1, 0x2000);
    func_80078B3C(0xF2000001);
}

void func_80046B20(void) {
    func_80078B04(0xF2000001);
}

INCLUDE_ASM("asm/funcs", func_80046B44);
INCLUDE_ASM("asm/funcs", func_80046BF4);
INCLUDE_ASM("asm/funcs", func_80046DA8);

s32 func_80046DE4(void) {
    return 0;
}

INCLUDE_ASM("asm/funcs", func_80046DEC);
INCLUDE_ASM("asm/funcs", func_80046E18);

void func_80046E44(void) {
    D_800F6654 = 0;
}

void func_80046E54(s32 a0) {
    if (a0) {
        D_800F6654 = 1;
    } else {
        D_800F6654 = 0;
    }
}

s32 func_80046E7C(void) {
    return D_800F6654;
}

void func_80046E8C(void) {
    D_800A3790 = 0x23;
}

INCLUDE_ASM("asm/funcs", func_80046EA0);
INCLUDE_ASM("asm/funcs", func_80046EDC);

void *func_80046F14(void) {
    return &D_8009947C;
}

INCLUDE_ASM("asm/funcs", func_80046F24);
INCLUDE_ASM("asm/funcs", func_8004700C);
INCLUDE_ASM("asm/funcs", func_800470B0);
INCLUDE_ASM("asm/funcs", func_80047210);

void *func_800472B0(void) {
    return &D_800EEDD0;
}

INCLUDE_ASM("asm/funcs", func_800472C0);
INCLUDE_ASM("asm/funcs", func_80047384);

void func_8004746C(void) {
    func_8004473C();
}

void func_8004748C(void) {
    func_80044800();
}

void func_800474AC(void) {
    func_80048F58(0, 0);
}

void func_800474D0(void) {
    func_80048FFC(0);
}

void func_800474F0(void) {
    func_8004473C();
}

void func_80047510(void) {
    func_80044800();
}

void func_80047530(void) {
    func_800477E8();
}

void func_80047550(void) {
    func_80047A90();
}

INCLUDE_ASM("asm/funcs", func_80047570);
INCLUDE_ASM("asm/funcs", func_800475A4);

void func_80047738(void) {
    func_80048F58(1, 0);
}

void func_8004775C(void) {
    func_80048FFC(0);
}

void func_8004777C(void) {
    func_8004473C();
}

void func_8004779C(void) {
    func_80044800();
}

void func_800477BC(void) {
}

void func_800477C4(void) {
}

void func_800477CC(void) {
}

void func_800477D4(void) {
}

INCLUDE_ASM("asm/funcs", func_800477DC);
INCLUDE_ASM("asm/funcs", func_800477E8);
INCLUDE_ASM("asm/funcs", func_80047A90);
INCLUDE_ASM("asm/funcs", func_80047BE0);
INCLUDE_ASM("asm/funcs", func_80047D94);
INCLUDE_ASM("asm/funcs", func_80047E5C);

s32 func_80047EC8(void) {
    return 0xD00;
}
