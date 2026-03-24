#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern void func_80082AC0(void);
extern void func_800885AC(void);
extern void func_80083A48(void);
extern void func_800892F8(void);

/* Externs for globals */
extern s32 D_800A14CC;
extern s32 D_800A2664;
extern u16 D_800A157A;
extern u16 *D_800A2608;
extern s32 *D_800A2600;
extern s32 D_800A2634;
extern void func_8008AF9C(s32 *);

/* --- Functions 0x8008289C - 0x80083BE4 --- */

s32 func_8008289C(s32 a0) {
    s32 old = D_800A14CC;
    D_800A14CC = a0;
    return old;
}

INCLUDE_ASM("asm/funcs", func_800828B4);

INCLUDE_ASM("asm/funcs", func_800828CC);
INCLUDE_ASM("asm/funcs", func_80082A14);
INCLUDE_ASM("asm/funcs", func_80082AB0);
void func_80082AC0(void) {
    ((void (*)(void))D_800A2600[3])();
}
void func_80082AF0(void) {
    ((void (*)(void))D_800A2600[2])();
}

void func_80082B20(void) {
    ((void (*)(void))D_800A2600[1])();
}
void func_80082B50(s32 a0) {
    ((void (*)(s32, s32))D_800A2600[5])(4, a0);
}

void func_80082B84(void) {
    ((void (*)(void))D_800A2600[5])();
}
void func_80082BB4(void) {
    ((void (*)(void))D_800A2600[4])();
}

void func_80082BE4(void) {
    ((void (*)(void))D_800A2600[6])();
}
u32 func_80082C14(void) {
    return D_800A157A;
}

u32 func_80082C24(void) {
    return *D_800A2608;
}
INCLUDE_ASM("asm/funcs", func_80082C3C);
INCLUDE_ASM("asm/funcs", func_80082D34);
INCLUDE_ASM("asm/funcs", func_800831D8);
INCLUDE_ASM("asm/funcs", func_800831F0);
INCLUDE_ASM("asm/funcs", func_80083200);
INCLUDE_ASM("asm/funcs", func_80083210);
INCLUDE_ASM("asm/funcs", func_80083220);
INCLUDE_ASM("asm/funcs", func_800832A0);
void func_8008339C(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}
INCLUDE_ASM("asm/funcs", func_800833C8);
void func_80083644(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}

s32 func_80083670(s32 a0) {
    s32 old = D_800A2664;
    D_800A2664 = a0;
    return old;
}

s32 func_80083688(void) {
    return D_800A2664;
}

INCLUDE_ASM("asm/funcs", func_80083698);
INCLUDE_ASM("asm/funcs", func_800836B8);
INCLUDE_ASM("asm/funcs", func_800836C8);
INCLUDE_ASM("asm/funcs", func_80083794);
INCLUDE_ASM("asm/funcs", func_8008386C);
INCLUDE_ASM("asm/funcs", func_8008387C);
INCLUDE_ASM("asm/funcs", func_8008393C);
INCLUDE_ASM("asm/funcs", func_80083954);

void func_80083A18(void) {
    func_80082AC0();
    func_800885AC();
    func_80083A48();
}

INCLUDE_ASM("asm/funcs", func_80083A48);

void func_80083B30(void) {
    func_800892F8();
}

void func_80083B50(s32 a0, s32 a1, s32 a2) {
    s32 buf[10];

    if ((a0 & 0xFF) == 0) {
        if ((a1 & 0xFF) == 0) {
            buf[0] = 0x200;
            buf[6] = a2 & 0xFF;
        }
        if ((a1 & 0xFF) == 1) {
            buf[0] = 0x100;
            buf[5] = a2 & 0xFF;
        }
    }
    if ((a0 & 0xFF) == 1) {
        if ((a1 & 0xFF) == 0) {
            buf[0] = 0x2000;
            buf[9] = a2 & 0xFF;
        }
        if ((a1 & 0xFF) == 1) {
            buf[0] = 0x1000;
            buf[8] = a2 & 0xFF;
        }
    }
    func_8008AF9C(buf);
}
