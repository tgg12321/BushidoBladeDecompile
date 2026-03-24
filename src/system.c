#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern void func_800817A0(void);
extern void func_80080DB0(void);
extern void func_80081030(void);
extern void func_80081718(void);
extern s32 func_80081D1C(void);
extern s32 func_80081E1C(void);
extern void func_80082B20(s32, s32);
extern void func_80081BB0(void);

/* Externs for globals */
extern u8 D_800A11C4;
extern u8 D_800A11D0;
extern u8 D_800A11D4;
extern u8 D_800A11D5;
extern s32 D_800A11B4;
extern s32 D_800A11B8;

/* --- Functions 0x8008008C - 0x800807A8 --- */

INCLUDE_ASM("asm/funcs", func_8008008C);

u32 func_8008009C(void) {
    return D_800A11C4;
}

u32 func_800800AC(void) {
    return D_800A11D4;
}

u32 func_800800BC(void) {
    return D_800A11D5;
}

void *func_800800CC(void) {
    return &D_800A11D0;
}

INCLUDE_ASM("asm/funcs", func_800800DC);

void func_80080148(void) {
    func_800817A0();
}

INCLUDE_ASM("asm/funcs", func_80080168);

void func_800801E8(void) {
    func_80080DB0();
}

void func_80080208(void) {
    func_80081030();
}

s32 func_80080228(s32 a0) {
    s32 old = D_800A11B4;
    D_800A11B4 = a0;
    return old;
}

s32 func_80080240(s32 a0) {
    s32 old = D_800A11B8;
    D_800A11B8 = a0;
    return old;
}

INCLUDE_ASM("asm/funcs", func_80080258);
INCLUDE_ASM("asm/funcs", func_80080390);
INCLUDE_ASM("asm/funcs", func_800804BC);

s32 func_80080600(void) {
    func_80081718();
    return 1;
}

s32 func_80080620(void) {
    return func_80081D1C() == 0;
}

s32 func_80080640(void) {
    return func_80081E1C() == 0;
}

void func_80080660(s32 a0) {
    func_80082B20(3, a0);
}

void func_80080684(void) {
    func_80081BB0();
}

INCLUDE_ASM("asm/funcs", func_800806A4);
