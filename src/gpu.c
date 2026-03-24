#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern void func_8007A448(void);
extern void func_8007A458(void);

/* Externs for globals */
extern u8 D_8009BE74;
extern u8 D_8009BE76;

/* --- Functions 0x8007A28C - 0x8007B234 --- */

INCLUDE_ASM("asm/funcs", func_8007A28C);
INCLUDE_ASM("asm/funcs", func_8007A2F8);
INCLUDE_ASM("asm/funcs", func_8007A308);
INCLUDE_ASM("asm/funcs", func_8007A318);
INCLUDE_ASM("asm/funcs", func_8007A350);
INCLUDE_ASM("asm/funcs", func_8007A360);
INCLUDE_ASM("asm/funcs", func_8007A370);
INCLUDE_ASM("asm/funcs", func_8007A3C8);

void func_8007A400(void) {
    func_8007A448();
    func_8007A458();
}

INCLUDE_ASM("asm/funcs", func_8007A428);
INCLUDE_ASM("asm/funcs", func_8007A438);
INCLUDE_ASM("asm/funcs", func_8007A448);
INCLUDE_ASM("asm/funcs", func_8007A458);
INCLUDE_ASM("asm/funcs", func_8007A4D8);
INCLUDE_ASM("asm/funcs", func_8007A5C4);
INCLUDE_ASM("asm/funcs", func_8007A62C);
INCLUDE_ASM("asm/funcs", func_8007A694);
INCLUDE_ASM("asm/funcs", func_8007A74C);
u32 func_8007A788(s32 a0, s32 a1, s32 a2, s32 a3) {
    return ((a0 & 3) << 7) | ((a1 & 3) << 5) | ((a3 & 0x100) >> 4) | ((a2 & 0x3FF) >> 6) | ((a3 & 0x200) << 2);
}
u32 func_8007A7C4(s32 a0, s32 a1) {
    return ((a1 << 6) | ((a0 >> 4) & 0x3F)) & 0xFFFF;
}
INCLUDE_ASM("asm/funcs", func_8007A7DC);
INCLUDE_ASM("asm/funcs", func_8007A83C);
u32 func_8007A87C(u32 *a0) {
    return (*a0 & 0xFFFFFF) | 0x80000000;
}

u32 func_8007A898(u32 *a0) {
    return (*a0 & 0xFFFFFF) == 0xFFFFFF;
}
INCLUDE_ASM("asm/funcs", func_8007A8B4);
INCLUDE_ASM("asm/funcs", func_8007A8F0);
void func_8007A92C(u32 *a0, u32 a1) {
    *a0 = (*a0 & 0xFF000000) | (a1 & 0xFFFFFF);
}
void func_8007A950(u32 *a0) {
    *a0 |= 0xFFFFFF;
}
void func_8007A968(u8 *a0, s32 a1) {
    if (a1) {
        a0[7] |= 2;
    } else {
        a0[7] &= ~2;
    }
}

void func_8007A990(u8 *a0, s32 a1) {
    if (a1) {
        a0[7] |= 1;
    } else {
        a0[7] &= ~1;
    }
}

void func_8007A9B8(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x20;
}

void func_8007A9CC(u8 *p) {
    p[3] = 0x7;
    p[7] = 0x24;
}

void func_8007A9E0(u8 *p) {
    p[3] = 0x6;
    p[7] = 0x30;
}

void func_8007A9F4(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x34;
}

void func_8007AA08(u8 *p) {
    p[3] = 0x5;
    p[7] = 0x28;
}

void func_8007AA1C(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x2C;
}

void func_8007AA30(u8 *p) {
    p[3] = 0x8;
    p[7] = 0x38;
}

void func_8007AA44(u8 *p) {
    p[3] = 0xC;
    p[7] = 0x3C;
}

void func_8007AA58(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x74;
}

void func_8007AA6C(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x7C;
}

void func_8007AA80(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x64;
}

void func_8007AA94(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x68;
}

void func_8007AAA8(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x70;
}

void func_8007AABC(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x78;
}

void func_8007AAD0(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x60;
}

void func_8007AAE4(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x40;
}

void func_8007AAF8(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x50;
}

void func_8007AB0C(u8 *p) {
    p[3] = 0x5;
    p[7] = 0x48;
    *(u32 *)(p + 0x14) = 0x55555555;
}

void func_8007AB2C(u8 *p) {
    p[3] = 0x7;
    p[7] = 0x58;
    *(u32 *)(p + 0x1C) = 0x55555555;
}

void func_8007AB4C(u8 *p) {
    p[3] = 0x6;
    p[7] = 0x4C;
    *(u32 *)(p + 0x18) = 0x55555555;
}

void func_8007AB6C(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x5C;
    *(u32 *)(p + 0x24) = 0x55555555;
}

INCLUDE_ASM("asm/funcs", func_8007AB8C);
INCLUDE_ASM("asm/funcs", func_8007ABB8);
INCLUDE_ASM("asm/funcs", func_8007AC18);
INCLUDE_ASM("asm/funcs", func_8007AC84);
INCLUDE_ASM("asm/funcs", func_8007ACBC);
INCLUDE_ASM("asm/funcs", func_8007ADD0);
INCLUDE_ASM("asm/funcs", func_8007AE7C);
INCLUDE_ASM("asm/funcs", func_8007B000);
INCLUDE_ASM("asm/funcs", func_8007B114);
INCLUDE_ASM("asm/funcs", func_8007B178);

u32 func_8007B224(void) {
    return D_8009BE74;
}

u32 func_8007B234(void) {
    return D_8009BE76;
}
