#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern void irq_DisableInterrupts(void);
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

s32 sys_SetVsyncMode(s32 a0) {
    s32 old = D_800A14CC;
    D_800A14CC = a0;
    return old;
}

extern s32 D_800A1500;
s32 sys_SetTimer(s32 a0) {
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
void irq_DisableInterrupts(void) {
    ((void (*)(void))D_800A2600[3])();
}
void irq_EnableInterrupts(void) {
    ((void (*)(void))D_800A2600[2])();
}

void irq_AcknowledgeVblank(void) {
    ((void (*)(void))D_800A2600[1])();
}
void irq_SetAlarm(s32 a0) {
    ((void (*)(s32, s32))D_800A2600[5])(4, a0);
}

void irq_ClearAlarm(void) {
    ((void (*)(void))D_800A2600[5])();
}
void irq_Reset(void) {
    ((void (*)(void))D_800A2600[4])();
}

void irq_Dispatch(void) {
    ((void (*)(void))D_800A2600[6])();
}
u32 sys_GetVblankCount(void) {
    return D_800A157A;
}

u32 sys_GetIrqCounter(void) {
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
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_80083220\n"
    "    sw $ra, 0($a0)\n"
    "    sw $gp, 44($a0)\n"
    "    sw $sp, 4($a0)\n"
    "    sw $fp, 8($a0)\n"
    "    sw $s0, 12($a0)\n"
    "    sw $s1, 16($a0)\n"
    "    sw $s2, 20($a0)\n"
    "    sw $s3, 24($a0)\n"
    "    sw $s4, 28($a0)\n"
    "    sw $s5, 32($a0)\n"
    "    sw $s6, 36($a0)\n"
    "    sw $s7, 40($a0)\n"
    "    addu $v0, $zero, $zero\n"
    "    jr $ra\n"
    "    nop\n"
    "    lw $ra, 0($a0)\n"
    "    lw $gp, 44($a0)\n"
    "    lw $sp, 4($a0)\n"
    "    lw $fp, 8($a0)\n"
    "    lw $s0, 12($a0)\n"
    "    lw $s1, 16($a0)\n"
    "    lw $s2, 20($a0)\n"
    "    lw $s3, 24($a0)\n"
    "    lw $s4, 28($a0)\n"
    "    lw $s5, 32($a0)\n"
    "    lw $s6, 36($a0)\n"
    "    lw $s7, 40($a0)\n"
    "    addu $v0, $a1, $zero\n"
    "    jr $ra\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "endlabel func_80083220\n"
    "    .set reorder\n"
    "    .set at\n"
);
INCLUDE_ASM("asm/funcs", func_800832A0);
void sys_MemClear(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}
INCLUDE_ASM("asm/funcs", func_800833C8);
void sys_MemClear2(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}

s32 sys_SetVideoMode(s32 a0) {
    s32 old = D_800A2664;
    D_800A2664 = a0;
    return old;
}

s32 sys_GetVideoMode(void) {
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
extern s32 bios_FileReadRaw(s32, s32, s32, s32);

s32 bios_FileRead(s32 addr, s32 dest, s32 len) {
    register s32 total asm("s2");
    s32 chunk;
    s32 result;

    total = 0;
    if (len != 0) {
        do {
            chunk = len;
            if ((u32)0x8000 < (u32)len) {
                chunk = 0x8000;
            }
            result = bios_FileReadRaw(0, addr, chunk, dest);
            total += result;
            if (result == -1) {
                return -1;
            }
            dest += result;
            len -= result;
            if (result < chunk) {
                break;
            }
        } while (len != 0);
    }
    return total;
}
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_FileReadRaw\n"
    "    .word 0x0000414D\n"
    "    beqz $v0, .L8008394C\n"
    "    addu $v0, $v1, $zero\n"
    "    addiu $v0, $zero, -0x1\n"
    ".L8008394C:\n"
    "    jr $ra\n"
    "    nop\n"
    "endlabel bios_FileReadRaw\n"
    "    .set reorder\n"
    "    .set at\n"
);
extern s32 D_800A26D0;
extern u8 D_800A26DD;
extern u8 D_800A26DE;
extern u8 D_800A26DC;
extern s32 D_800A26D8;
extern void func_800789B8(void);
extern void func_800789C8(void);

void irq_ProcessPending(void) {
    if (D_800A26D0 != 0) {
        return;
    }
    D_800A26DD = 0;
    if (D_800A26DE == 0x7F) {
        return;
    }
    func_800789B8();
    if (D_800A26DC != 0) {
        irq_SetAlarm(0);
        D_800A26DC = 0;
    } else if (D_800A26DE == 0) {
        ((void (*)(s32, s32))irq_EnableInterrupts)(0, D_800A26D8);
        D_800A26D8 = 0;
    } else {
        ((void (*)(s32, s32))irq_EnableInterrupts)(6, 0);
    }
    func_800789C8();
    D_800A26DE = 0x7F;
}

void sys_Shutdown(void) {
    irq_DisableInterrupts();
    func_800885AC();
    func_80083A48();
}

INCLUDE_ASM("asm/funcs", func_80083A48);

void spu_Reset(void) {
    func_800892F8();
}

void spu_SetVolume(s32 a0, s32 a1, s32 a2) {
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
