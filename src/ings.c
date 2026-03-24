#include "common.h"
#include "include_asm.h"

/* Forward declarations for called functions */
extern void func_8001945C(void);
extern void func_8007AE7C(s32);
extern void func_8007B2A0(s32);
extern void func_8007B4D0(void *, s32, s32, s32);
extern void func_8007B33C(s32);
extern void func_80046AE8(void);

/* Externs for globals */
extern u8 D_80106A73;
extern u8 D_80106A54;
extern u16 D_800A3710;
extern u32 D_80106A50;
extern u32 D_80106A5C;
extern u8 D_800A3716;
extern s32 D_800A38BC;
extern u32 D_800A30CC;
extern u8 D_800F6740[];

/* --- Non-decompiled functions (INCLUDE_ASM) --- */
INCLUDE_ASM("asm/funcs", func_800164AC);
INCLUDE_ASM("asm/funcs", func_800164F8);
INCLUDE_ASM("asm/funcs", func_80016514);
INCLUDE_ASM("asm/funcs", func_800165F8);
INCLUDE_ASM("asm/funcs", func_800166C4);
INCLUDE_ASM("asm/funcs", func_80016768);

/* --- Decompiled functions --- */

u32 func_800167AC(void) {
    return D_80106A73 & 1;
}

u32 func_800167BC(void) {
    return (D_80106A73 >> 1) & 1;
}

u32 func_800167D4(void) {
    return (D_80106A73 >> 2) & 1;
}

INCLUDE_ASM("asm/funcs", func_800167EC);

void func_80016868(void) {
    func_8007AE7C(1);
}

void func_80016888(void) {
    func_8007B2A0(0);
    func_8007AE7C(1);
    func_8007B4D0(&D_800A30CC, 0, 0, 0);
    func_8007B33C(0);
}

void func_800168D0(void) {
    func_8007B2A0(1);
}

void func_800168F0(void) {
}

void func_800168F8(void) {
    func_80046AE8();
}

INCLUDE_ASM("asm/funcs", func_80016918);
INCLUDE_ASM("asm/funcs", func_80016A18);
INCLUDE_ASM("asm/funcs", func_80016A8C);
INCLUDE_ASM("asm/funcs", func_80016C3C);
INCLUDE_ASM("asm/funcs", func_80016C74);
INCLUDE_ASM("asm/funcs", func_80016C80);
INCLUDE_ASM("asm/funcs", func_80016CF8);
INCLUDE_ASM("asm/funcs", func_80016D78);

void func_80016E40(void) {
    func_8007B33C(0);
}

INCLUDE_ASM("asm/funcs", func_80016E60);
INCLUDE_ASM("asm/funcs", func_800171AC);
INCLUDE_ASM("asm/funcs", func_800171B8);
INCLUDE_ASM("asm/funcs", func_80017200);
INCLUDE_ASM("asm/funcs", func_800174F4);
INCLUDE_ASM("asm/funcs", func_80017714);

s32 func_80017738(s32 a0, s32 a1) {
    return (a0 << 6) + (a1 << 4);
}

INCLUDE_ASM("asm/funcs", func_80017748);
INCLUDE_ASM("asm/funcs", func_800177C8);
INCLUDE_ASM("asm/funcs", func_80017848);
INCLUDE_ASM("asm/funcs", func_80017A44);
INCLUDE_ASM("asm/funcs", func_80017D84);
void func_80017E8C(s32 a0) {
    *(s32 *)(D_800F6740 + a0 * 52) = 0;
}
void func_80017EB4(s32 a0, s32 a1) {
    u8 *ptr = D_800F6740 + a0 * 52;
    s32 c = *(s32 *)(ptr + 0xC) + a1;
    *(s32 *)(ptr + 0xC) = c;
    *(s32 *)(ptr + 0x10) = c + (*(s16 *)(ptr + 4) << 6);
}
INCLUDE_ASM("asm/funcs", func_80017EF4);
INCLUDE_ASM("asm/funcs", func_80017F28);
INCLUDE_ASM("asm/funcs", func_80017F5C);

void func_80017F90(void) {
}

void func_80017F98(void) {
}
