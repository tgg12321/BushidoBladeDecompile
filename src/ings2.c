#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern void irq_DisableInterrupts(void);
extern void func_800885AC(void);
extern void func_80083A48(void);
extern void func_800892F8(void);

/* Externs for globals */
extern s32 g_sys_vsync_mode;
extern s32 g_sys_video_mode;
extern u16 g_sys_vblank_count;
extern u16 *g_sys_irq_counter;
extern s32 *g_sys_irq_vtable;
extern s32 g_sys_dma_region;
extern void func_8008AF9C(s32 *);

/* --- Functions 0x8008289C - 0x80083BE4 --- */

s32 sys_SetVsyncMode(s32 a0) {
    s32 old = g_sys_vsync_mode;
    g_sys_vsync_mode = a0;
    return old;
}

extern s32 g_sys_timer;
s32 sys_SetTimer(s32 a0) {
    s32 *p = &g_sys_timer;
    s32 old = p[0];
    *(volatile s32 *)p = a0;
    return old;
}

extern volatile s32 *D_800A1510;
extern volatile s32 *D_800A1514;
extern s32 D_800A1518;
extern s32 D_800A151C;
void func_80082A14(s32 a0, s32 a1);

s32 func_800828CC(s32 a0) {
    s32 s0_val;
    s32 s1_val;

    s0_val = *D_800A1510;
    s1_val = (*D_800A1514 - D_800A1518) & 0xFFFF;

    if (a0 < 0) {
        return g_sys_dma_region;
    }
    if (a0 == 1) {
        return s1_val;
    }

    {
        s32 frame;
        s32 count;

        if (a0 > 0) {
            frame = (D_800A151C + a0) - 1;
        } else {
            frame = D_800A151C;
        }
        count = 0;
        if (a0 > 0) {
            count = a0 - 1;
        }
        func_80082A14(frame, count);
        do { } while (0);
    }

    s0_val = *D_800A1510;
    func_80082A14(g_sys_dma_region + 1, 1);

    if (s0_val & 0x400000) {
        volatile s32 *ptr = D_800A1510;
        if ((s32)(s0_val ^ *ptr) >= 0) {
            do {
            } while (!((s0_val ^ *ptr) & 0x80000000));
        }
    }

    D_800A151C = g_sys_dma_region;
    D_800A1518 = *D_800A1514;

    return s1_val;
}

extern s32 D_80016318;
extern void tslTm2LoadImage_2(void *);
extern void func_80078A58(s32);
extern void func_80082AB0(s32, s32);
void func_80082A14(s32 a0, s32 a1) {
    volatile s32 counter = a1 << 15;
    asm volatile("" ::: "memory");
    if (g_sys_dma_region < a0) {
        do {
            if (--counter == -1) {
                tslTm2LoadImage_2(&D_80016318);
                func_80078A58(0);
                func_80082AB0(3, 0);
                return;
            }
        } while (g_sys_dma_region < a0);
    }
}
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
    ((void (*)(void))g_sys_irq_vtable[3])();
}
void irq_EnableInterrupts(void) {
    ((void (*)(void))g_sys_irq_vtable[2])();
}

void irq_AcknowledgeVblank(void) {
    ((void (*)(void))g_sys_irq_vtable[1])();
}
void irq_SetAlarm(s32 a0) {
    ((void (*)(s32, s32))g_sys_irq_vtable[5])(4, a0);
}

void irq_ClearAlarm(void) {
    ((void (*)(void))g_sys_irq_vtable[5])();
}
void irq_Reset(void) {
    ((void (*)(void))g_sys_irq_vtable[4])();
}

void irq_Dispatch(void) {
    ((void (*)(void))g_sys_irq_vtable[6])();
}
u32 sys_GetVblankCount(void) {
    return g_sys_vblank_count;
}

u32 sys_GetIrqCounter(void) {
    return *g_sys_irq_counter;
}
extern u16 *D_800A2604;
extern s32 *D_800A260C;
extern u16 D_800A1578;
extern s32 D_800A15B4;
extern s32 func_800831A4(u16 *, s32);
extern s32 func_80083220(u16 *);
extern void func_80082D34(void);
extern void func_80083210(s32 *);
extern s32 func_800832A0(void);
extern s32 conv_matrix_rotation(void);
extern s32 func_800831D8(s32 *);
u16 motion_make_table(u16 arg0) {
    u16 *ptr = g_sys_irq_counter;
    u16 old = *ptr;
    *(volatile u16 *)ptr = arg0;
    return old;
}

u16 *func_80082C58(void) {
    u16 *s0 = &D_800A1578;
    s32 result;

    if (*s0 != 0) {
        return 0;
    }

    {
        u16 *v1 = D_800A2604;
        u16 *v0_ptr = g_sys_irq_counter;
        s32 a1_val = 0x33333333;

        *(volatile u16 *)v0_ptr = 0;
        {
            u16 val = *(volatile u16 *)v0_ptr;
            *v1 = val;
        }

        *D_800A260C = a1_val;

        func_800831A4(s0, 0x41A);

        if (func_80083220(s0 + 0x1C) != 0) {
            func_80082D34();
        }
    }

    {
        s32 *s0b = &D_800A15B4;

        *s0b = (s32)s0b + 0xFDC;
        func_80083210(s0b - 1);

        asm volatile("" : "=r"(s0b) : "0"(s0b));
        ((s16 *)s0b)[-0x1E] = 1;
        result = func_800832A0();

        {
            s32 *v1 = g_sys_irq_vtable;
            v1[5] = result;
        }
        result = conv_matrix_rotation();
        {
            s32 *a0 = g_sys_irq_vtable;
            a0[1] = result;
            func_800831D8(a0);
        }
        asm volatile("" : "=r"(s0b) : "0"(s0b));
        s0b = (s32 *)((char *)s0b - 0x3C);
        func_800789C8();
        return (u16 *)s0b;
    }
}
/* kengo:HIGH  |  is_motion/motion_make_table  |  62i */
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
extern s32 D_800A2614[8];
extern volatile s32 D_800A2634;
extern s32 *D_800A2638;

void D_800832F8(void);
void D_80083370(s32, s32);

s32 func_800832A0(void) {
    *D_800A2638 = 0x107;
    D_800A2634 = 0;
    sys_MemClear(&D_800A2614[0], 8);
    ((void (*)(s32, void *))irq_EnableInterrupts)(0, (void *)D_800832F8);
    return (s32)D_80083370;
}

void D_800832F8(void) {
    s32 i;
    s32 *p;

    ++D_800A2634;

    i = 0;
    p = &D_800A2614[0];
    for (; i < 8; i++) {
        s32 fp = *p;
        if (fp != 0) {
            ((void (*)(void))fp)();
        }
        p++;
    }
}

void D_80083370(s32 a0, s32 a1) {
    if (a1 != D_800A2614[a0]) {
        D_800A2614[a0] = a1;
    }
}
void sys_MemClear(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}
INCLUDE_ASM("asm/funcs", conv_matrix_rotation);
/* kengo:MED  |  common/conv_matrix_rotation  |  145i  |  -14 9.7% */
void sys_MemClear2(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}

s32 sys_SetVideoMode(s32 a0) {
    s32 old = g_sys_video_mode;
    g_sys_video_mode = a0;
    return old;
}

s32 sys_GetVideoMode(void) {
    return g_sys_video_mode;
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
INCLUDE_ASM("asm/funcs", ang_hosei);
/* kengo:MED  |  common/ang_hosei  |  47i  |  +4 8.5% */
INCLUDE_ASM("asm/funcs", motion_Open);
/* kengo:HIGH  |  is_motion/motion_Open  |  54i */
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

extern u16 D_800A269C;
extern u16 D_800A26AC;
extern s32 D_80106FA8;
extern s32 D_80104E80;
extern s32 D_801027E4;
extern s32 D_800FF630;
extern void md_game_end(s32);

void func_80083A48(void) {
    /* Loop 1: Copy 8 default regs to each of 24 SPU voices */
    {
        register volatile u16 *spu asm("a2") = (volatile u16 *)0x1F801C00;
        register s32 i asm("a0") = 0;
        register u16 *base asm("a3") = &D_800A269C;
    loop1_outer:
        {
            register s32 j asm("a1") = 0;
            register u16 *src asm("v1") = base;
        loop1_inner:
            *spu++ = *src++;
            j++;
            if (j < 8) goto loop1_inner;
        }
        i++;
        if (i < 0x18) goto loop1_outer;
    }

    /* Loop 2: Copy 16 control regs */
    {
        register volatile u16 *spu2 asm("a2") = (volatile u16 *)0x1F801D80;
        register s32 k asm("a0") = 0;
        register u16 *src2 asm("v1") = &D_800A26AC;
    loop2:
        *spu2++ = *src2++;
        k++;
        if (k < 0x10) goto loop2;
    }

    md_game_end(0x18);

    /* Loop 3: Clear 32 blocks of 16 words each */
    {
        register s32 m asm("a1") = 0;
        register s32 *tbl asm("v1") = &D_80106FA8;
    loop3_outer:
        {
            register s32 n asm("a0") = 15;
            s32 *p = tbl + 15;
        loop3_inner:
            *p-- = 0;
            n--;
            if (n >= 0) goto loop3_inner;
        }
        m++;
        if (m < 0x20) {
            tbl += 16;
            goto loop3_outer;
        }
    }

    D_80104E80 = 0x3C;
    D_801027E4 = 0;
    D_800FF630 = 0;
}

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
