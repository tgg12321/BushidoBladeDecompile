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

extern s32 D_800A1500;
s32 func_800828B4(s32 a0) {
    s32 *p = &D_800A1500;
    s32 old = p[0];
    *(volatile s32 *)p = a0;
    return old;
}

INCLUDE_ASM("asm/funcs", func_800828CC);
INCLUDE_ASM("asm/funcs", func_80082A14);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_80082AB0\n"
    "    addiu $t2, $zero, 0xC0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0xA\n"
    "    nop\n"
    "endlabel func_80082AB0\n"
    "    .set reorder\n"
    "    .set at\n"
);
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
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_800831D8\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x72\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "endlabel func_800831D8\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_800831F0\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x17\n"
    "    nop\n"
    "endlabel func_800831F0\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_80083200\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x18\n"
    "    nop\n"
    "endlabel func_80083200\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_80083210\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x19\n"
    "    nop\n"
    "endlabel func_80083210\n"
    "    .set reorder\n"
    "    .set at\n"
);
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

__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_80083698\n"
    "    addu $a2, $a1, $zero\n"
    "    addu $a1, $a0, $zero\n"
    "    .word 0x000040CD\n"
    "    beqz $v0, .L800836B0\n"
    "    addu $v0, $v1, $zero\n"
    "    addiu $v0, $zero, -0x1\n"
    ".L800836B0:\n"
    "    jr $ra\n"
    "    nop\n"
    "endlabel func_80083698\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_800836B8\n"
    "    addu $a1, $a0, $zero\n"
    "    .word 0x0000410D\n"
    "    jr $ra\n"
    "    nop\n"
    "endlabel func_800836B8\n"
    "    .set reorder\n"
    "    .set at\n"
);
INCLUDE_ASM("asm/funcs", func_800836C8);
INCLUDE_ASM("asm/funcs", func_80083794);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_8008386C\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x39\n"
    "    nop\n"
    "endlabel func_8008386C\n"
    "    .set reorder\n"
    "    .set at\n"
);
INCLUDE_ASM("asm/funcs", func_8008387C);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_8008393C\n"
    "    .word 0x0000414D\n"
    "    beqz $v0, .L8008394C\n"
    "    addu $v0, $v1, $zero\n"
    "    addiu $v0, $zero, -0x1\n"
    ".L8008394C:\n"
    "    jr $ra\n"
    "    nop\n"
    "endlabel func_8008393C\n"
    "    .set reorder\n"
    "    .set at\n"
);
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
