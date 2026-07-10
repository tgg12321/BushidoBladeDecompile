#include "common.h"
#include "include_asm.h"
#include "system.h"

/* Forward declarations */
extern void spu_Init(void);
extern void func_80083A48(void);
extern void func_800892F8(void);

/* Externs for globals */
extern s32 g_sys_vsync_mode;
extern s32 g_sys_video_mode;
extern u16 g_sys_vblank_count;
extern u16 *g_sys_irq_counter;
extern s32 *g_sys_irq_vtable;
extern volatile s32 g_sys_dma_region;
extern void func_8008AF9C(s32 *);

/* --- Functions 0x8008289C - 0x80083BE4 --- */

s32 sys_SetVsyncMode(s32 a0) {
    s32 old = g_sys_vsync_mode;
    g_sys_vsync_mode = a0;
    return old;
}

/* PsyQ 4.0 LIBCD cdread.c: CdReadMode — verbatim-linked Sony object (census
   2026-07-09); the "timer" word (0x800A1500) is the +0x30 mode-flag member
   of the volatile cdread module state block (CdlREAD in system.c). */
typedef struct {
    /* 0x00 */ s32 sectors;
    /* 0x04 */ s32 buf;
    /* 0x08 */ s32 p;
    /* 0x0C */ s32 mode;
    /* 0x10 */ s32 size;
    /* 0x14 */ s32 cnt;
    /* 0x18 */ s32 t2;
    /* 0x1C */ s32 t1;
    /* 0x20 */ s32 pos;
    /* 0x24 */ s32 cbsync;
    /* 0x28 */ s32 cbready;
    /* 0x2C */ s32 cbdata;
    /* 0x30 */ s32 tslmode;
} CdlREAD;
extern volatile CdlREAD D_800A14D0;

s32 sys_SetTimer(s32 a0) {
    s32 old = D_800A14D0.tslmode;
    D_800A14D0.tslmode = a0;
    return old;
}

extern volatile s32 *D_800A1510;
extern volatile s32 *D_800A1514;
extern s32 D_800A1518;
extern s32 D_800A151C;
void func_80082A14(s32 a0, s32 a1);

s32 sys_VSync(s32 a0) {
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
            s32 base = D_800A151C - 1;
            frame = base + a0;
        } else {
            frame = D_800A151C;
        }
        count = 0;
        if (a0 > 0) {
            count = a0 - 1;
        }
        func_80082A14(frame, count);
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
extern void bios_ChangeClearPad(s32);
extern void bios_ChangeClearRCnt(s32, s32);
/* PsyQ 4.0 LIBETC VSYNC: v_wait (static) — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libetc/vsync.c.
   FAKE(partial-use volatile array, Ruling 3 2026-07-10): only [0] is
   referenced — SOTN ships the identical `volatile s32 timeout[2]` shape;
   original author idiom. */
void func_80082A14(s32 a0, s32 a1) {
    volatile s32 timeout[2];

    timeout[0] = a1 << 0xF;
    while (g_sys_dma_region < a0) {
        if (timeout[0]-- == 0) {
            tslTm2LoadImage_2(&D_80016318);
            bios_ChangeClearPad(0);
            bios_ChangeClearRCnt(3, 0);
            return;
        }
    }
}
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_ChangeClearRCnt\n"
    "    addiu $t2, $zero, 0xC0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0xA\n"
    "    nop\n"
    "endlabel bios_ChangeClearRCnt\n"
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
/* PsyQ 4.0 LIBETC INTR: intr.c v1.76 module state — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libetc/
   intr.c (intrEnv_t). D_800A1578 = intrEnv; D_800A15B4 = intrEnv.buf[1]
   (JB_SP); D_800A2604/g_sys_irq_counter/D_800A260C = the module's
   i_stat/i_mask/d_pcr MMIO pointer statics (0x1F801070/74/F0). */
typedef struct {
    u16 interruptsInitialized;   /* +0x00 = D_800A1578 */
    u16 inInterrupt;             /* +0x02 */
    void (*handlers[11])(void);  /* +0x04 */
    u16 enabledInterruptsMask;   /* +0x30 */
    u16 savedMask;               /* +0x32 */
    s32 savedPcr;                /* +0x34 */
    s32 buf[12];                 /* +0x38 jmp_buf; [1] = JB_SP = D_800A15B4 */
    s32 stack[1024];             /* +0x68 */
} intrEnv_t;                     /* sizeof 0x1068; memclr count 0x41A words */
extern volatile u16 *D_800A2604;   /* i_stat = (u16 *)0x1F801070 (MMIO) */
extern volatile s32 *D_800A260C;   /* d_pcr  = (s32 *)0x1F8010F0 (MMIO) */
extern intrEnv_t D_800A1578;
extern s32 func_800831A4(u16 *, s32);
extern s32 setjmp(u16 *);
extern void func_80082D34(void);
extern void bios_SetCustomExitFromException(s32 *);
extern s32 func_800832A0();
extern s32 conv_matrix_rotation();
extern void bios_CdRemove_A0(s32 *);
u16 motion_make_table(u16 arg0) {
    u16 *ptr = g_sys_irq_counter;
    u16 old = *ptr;
    *(volatile u16 *)ptr = arg0;
    return old;
}

/* startIntr (LIBETC intr.c static) */
u16 *func_80082C58(void) {
    if (D_800A1578.interruptsInitialized) {
        return 0;
    }
    /* i_mask deref is MMIO (0x1F801074) — volatile is hardware semantics */
    *D_800A2604 = (*(volatile u16 *)g_sys_irq_counter = 0);
    *D_800A260C = 0x33333333;
    func_800831A4((u16 *)&D_800A1578, 0x41A);
    if (setjmp((u16 *)D_800A1578.buf) != 0) {
        func_80082D34();
    }
    D_800A1578.buf[1] = (s32)&D_800A1578.stack[1004];
    bios_SetCustomExitFromException(D_800A1578.buf);
    D_800A1578.interruptsInitialized = 1;
    g_sys_irq_vtable[5] = func_800832A0();
    {
        /* v1.76 evidence: the compiled Sony object keeps pCallbacks live in
           $a0 INTO the _96_remove call (v1.73's plain `_96_remove();` compiles
           to $v1 here — measured, tmp/closer/intr_test.c); the v1.76 source
           passed the pointer through. */
        s32 r = conv_matrix_rotation();
        s32 *cb = g_sys_irq_vtable;
        cb[1] = r;
        bios_CdRemove_A0(cb);
    }
    ExitCriticalSection();
    return (u16 *)&D_800A1578;
}
/* kengo:HIGH  |  is_motion/motion_make_table  |  62i */
void func_80082D34(void) {}
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_CdRemove_A0\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x72\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "endlabel bios_CdRemove_A0\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_ReturnFromException\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x17\n"
    "    nop\n"
    "endlabel bios_ReturnFromException\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_SetDefaultExitFromException\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x18\n"
    "    nop\n"
    "endlabel bios_SetDefaultExitFromException\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_SetCustomExitFromException\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x19\n"
    "    nop\n"
    "endlabel bios_SetCustomExitFromException\n"
    "    .set reorder\n"
    "    .set at\n"
);
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel setjmp\n"
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
    "endlabel setjmp\n"
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
extern s32 D_800A2640[8];
/* PsyQ 4.0 LIBETC intr_dma.c module state (verbatim-linked Sony object,
   census 2026-07-09; C ref: sotn-decomp src/main/psxsdk/libetc/intr_dma.c).
   D_800A263C holds 0x1F8010F4 (DMA Interrupt Register) — Sony declares it
   `static volatile u_long *` (pointer-to-volatile-MMIO, type-level). */
extern volatile u32 *D_800A263C;
extern u32 *D_800A2660;
extern u8 D_80016394;
extern u8 D_800163B0;

void D_80083418(void);
s32 D_8008359C(s32, s32);

void sys_MemClear(s32 *a0, s32 a1) {
    s32 i;
    for (i = a1 - 1; i != -1; i--) {
        *a0++ = 0;
    }
}
s32 conv_matrix_rotation(void) {
    sys_MemClear2((s32 *)&D_800A2640, 8);
    *D_800A263C = 0;
    ((void (*)(s32, void *))irq_EnableInterrupts)(3, (void *)D_80083418);
    return (s32)D_8008359C;
}

/* PsyQ 4.0 LIBETC INTR_DMA: trapIntrDMA (static) — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp libetc/intr_dma.c */
void D_80083418(void) {
    u32 mask;
    s32 i;

    while ((mask = (*D_800A263C >> 24) & 0x7F) != 0) {
        for (i = 0; mask != 0 && i < 7; i++, mask >>= 1) {
            if (mask & 1) {
                *D_800A263C &= 0xFFFFFF | (1 << (i + 24));
                if (D_800A2640[i] != 0) {
                    ((void (*)(void))D_800A2640[i])();
                }
            }
        }
    }

    if ((*D_800A263C & 0xFF000000) == 0x80000000 || *D_800A263C & 0x8000) {
        debug_printf(&D_80016394, *D_800A263C);
        for (i = 0; i < 7; i++) {
            debug_printf(&D_800163B0, i, D_800A2660[4 * i]);
        }
    }
}

/* PsyQ 4.0 LIBETC INTR_DMA: setIntrDMA (static) — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp libetc/intr_dma.c */
s32 D_8008359C(s32 a0, s32 a1) {
    s32 prev = D_800A2640[a0];
    if (a1 != prev) {
        if (a1 != 0) {
            D_800A2640[a0] = a1;
            *D_800A263C = (*D_800A263C & 0xFFFFFF) | 0x800000 | (1 << (a0 + 16));
        } else {
            D_800A2640[a0] = 0;
            *D_800A263C = ((*D_800A263C & 0xFFFFFF) | 0x800000) & ~(1 << (a0 + 16));
        }
    }
    return prev;
}

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
    "glabel md_gview_init\n"
    "    addu $a1, $a0, $zero\n"
    "    .word 0x0000410D\n"
    "    jr $ra\n"
    "    nop\n"
    "endlabel md_gview_init\n"
    "    .set reorder\n"
    "    .set at\n"
);
s32 ang_hosei(s32 a0, s32 a1, s32 a2) {
    (void)a0;
    (void)a1;
    (void)a2;
    return 0;
}
/* kengo:MED  |  common/ang_hosei  |  47i  |  +4 8.5% */
/* motion_Open + motion_Close (paired open/close functions) */
extern s32 D_800A2668;
extern void (*D_8008D070)(void);
extern s32 D_00000000;

void motion_Open(void) {
    register void (**p)(void) asm("s0");
    register s32 count asm("s1");

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                register void (*f)(void) asm("t0") = *p;
                p++;
                __asm__ volatile("jalr %0" :: "r"(f) : "ra", "memory");
                __asm__ volatile("addiu $17, $17, -1" : "=r"(count) : "0"(count));
            } while (count != 0);
        }
    }
}

void motion_Close(void) {
    register void (**p)(void) asm("s0");
    register s32 count asm("s1");

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                register void (*f)(void) asm("t0") = *p;
                p++;
                __asm__ volatile("jalr %0" :: "r"(f) : "ra", "memory");
                __asm__ volatile("addiu $17, $17, -1" : "=r"(count) : "0"(count));
            } while (count != 0);
        }
    }
}
/* kengo:HIGH  |  is_motion/motion_Open  |  54i */
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel bios_InitHeap\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x39\n"
    "    nop\n"
    "endlabel bios_InitHeap\n"
    "    .set reorder\n"
    "    .set at\n"
);
extern s32 bios_FileReadRaw(s32, s32, s32, s32);

s32 bios_FileRead(s32 addr, s32 dest, s32 len) {
    s32 total;
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
                return total;
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
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

void irq_ProcessPending(void) {
    if (D_800A26D0 != 0) {
        return;
    }
    D_800A26DD = 0;
    if (D_800A26DE == 0x7F) {
        return;
    }
    EnterCriticalSection();
    if (D_800A26DC != 0) {
        irq_SetAlarm(0);
        D_800A26DC = 0;
    } else if (D_800A26DE == 0) {
        ((void (*)(s32, s32))irq_EnableInterrupts)(0, D_800A26D8);
        D_800A26D8 = 0;
    } else {
        ((void (*)(s32, s32))irq_EnableInterrupts)(6, 0);
    }
    ExitCriticalSection();
    D_800A26DE = 0x7F;
}

void sys_Shutdown(void) {
    irq_DisableInterrupts();
    spu_Init();
    func_80083A48();
}

extern u16 D_800A269C;
extern u16 D_800A26AC;
extern s32 D_80106FA8[32][16];
extern s32 D_80104E80;
extern s32 D_801027E4;
extern s32 D_800FF630;
extern void md_game_end(s32);

/* PsyQ 4.0 LIBSND ssinit: _SsInit — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/ssinit.c */
void func_80083A48(void) {
    u16 *var_a2;
    int i, j;

    var_a2 = (u16 *)0x1F801C00;
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 8; j++) {
            *var_a2++ = (&D_800A269C)[j];
        }
    }

    var_a2 = (u16 *)0x1F801D80;
    for (i = 0; i < 16; i++) {
        *var_a2++ = (&D_800A26AC)[i];
    }

    md_game_end(0x18);

    for (j = 0; j < 32; j++) {
        for (i = 0; i < 16; i++) {
            D_80106FA8[j][i] = 0;
        }
    }

    D_80104E80 = 60;
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
