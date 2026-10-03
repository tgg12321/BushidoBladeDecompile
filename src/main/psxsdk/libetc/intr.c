/* PsyQ 4.0 LIBETC INTR: the interrupt dispatcher (ResetCallback .. memclr; $Id: intr.c,v 1.76; SOTN
 * libetc/intr.c). .text 0x80082AC0..0x800831D0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
extern u16 g_sys_vblank_count;
extern volatile u16 *i_mask; /* libetc intr.c i_mask = (u16 *)0x1F801074, I_MASK (MMIO) */
extern s32 *g_sys_irq_vtable;
extern void ChangeClearPAD(s32);
extern void ChangeClearRCnt(s32, s32);

void ResetCallback(void) {
    ((void (*)(void))g_sys_irq_vtable[3])();
}
void InterruptCallback(void) {
    ((void (*)(void))g_sys_irq_vtable[2])();
}

void DMACallback(void) {
    ((void (*)(void))g_sys_irq_vtable[1])();
}
void VSyncCallback(s32 a0) {
    ((void (*)(s32, s32))g_sys_irq_vtable[5])(4, a0);
}

void VSyncCallbacks(void) {
    ((void (*)(void))g_sys_irq_vtable[5])();
}
void StopCallback(void) {
    ((void (*)(void))g_sys_irq_vtable[4])();
}

void RestartCallback(void) {
    ((void (*)(void))g_sys_irq_vtable[6])();
}
u32 CheckCallback(void) {
    return g_sys_vblank_count;
}

u32 GetIntrMask(void) {
    return *i_mask;
}
/* PsyQ 4.0 LIBETC INTR: intr.c v1.76 module state — verbatim-linked Sony
   object; C ref: sotn-decomp src/main/psxsdk/libetc/
   intr.c (intrEnv_t). D_800A1578 = intrEnv; D_800A15B4 = intrEnv.buf[1]
   (JB_SP); i_stat/i_mask/d_pcr (0x800A2604/08/0C) = the module's
   MMIO pointer statics (0x1F801070/74/F0). */
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
extern volatile u16 *i_stat;   /* i_stat = (u16 *)0x1F801070 (MMIO) */
extern volatile s32 *d_pcr;   /* d_pcr  = (s32 *)0x1F8010F0 (MMIO) */
extern intrEnv_t D_800A1578;

extern s32 setjmp(s32 *);
extern void trapIntr(void);
extern void HookEntryInt(s32 *);
extern s32 startIntrVSync();
extern s32 startIntrDMA();
/* FAKE: the BIOS call takes no argument; declared with one for startIntr (see there). */
extern void _96_remove(s32 *);
u16 SetIntrMask(u16 arg0) {
    u16 old = *i_mask;
    *i_mask = arg0;
    return old;
}

/* startIntr (LIBETC intr.c static) */
intrEnv_t *startIntr(void) {
    if (D_800A1578.interruptsInitialized) {
        return 0;
    }
    /* i_mask deref is MMIO (0x1F801074) — volatile is hardware semantics */
    *i_stat = (*i_mask = 0);
    *d_pcr = 0x33333333;
    func_800831A4(&D_800A1578, 0x41A);
    if (setjmp(D_800A1578.buf) != 0) {
        trapIntr();
    }
    D_800A1578.buf[1] = (s32)&D_800A1578.stack[1004];
    HookEntryInt(D_800A1578.buf);
    D_800A1578.interruptsInitialized = 1;
    g_sys_irq_vtable[5] = startIntrVSync();
    g_sys_irq_vtable[1] = startIntrDMA();
    /* FAKE: _96_remove (BIOS A(72h)) takes no argument; passing g_sys_irq_vtable keeps the
     * table pointer live in $a0 into the call as the target does; `_96_remove()` scores 3. */
    _96_remove(g_sys_irq_vtable);
    ExitCriticalSection();
    return &D_800A1578;
}
/* PsyQ 4.0 LIBETC INTR: trapIntr + setIntr + stopIntr + restartIntr + memclr
   — verbatim-linked Sony object intr.c v1.76; C ref:
   sotn-decomp src/main/psxsdk/libetc/intr.c (v1.73; v1.76 deltas measured).
   setIntr/stopIntr/restartIntr are statics referenced only through the
   callbacks vtable raw words at 0x800A25E8/F0/F8 (7D920.data.s). */
typedef void (*IntrCallback)(void);
extern u8 D_8001635C;  /* "unexpected interrupt(%04x)\n" */
extern u8 D_80016378;  /* "intr timeout(%04x:%04x)\n" */
extern s32 D_800A2610; /* trapMissedCount */
extern void ReturnFromException(void);
extern void ResetEntryInt(void);

/* trapIntr */
void trapIntr(void) {
    s32 i;
    u16 mask;

    if (!D_800A1578.interruptsInitialized) {
        printf(&D_8001635C, *i_stat);
        ReturnFromException();
    }
    D_800A1578.inInterrupt = 1;
    while ((mask = (D_800A1578.enabledInterruptsMask & *i_stat) &
                   *i_mask) != 0) {
        for (i = 0; mask && i < 11; ++i, mask >>= 1) {
            if (mask & 1) {
                *i_stat = ~(1 << i);
                if (D_800A1578.handlers[i] != 0) {
                    D_800A1578.handlers[i]();
                }
            }
        }
    }
    if (*i_stat & *i_mask) {
        if (D_800A2610++ > 0x800) {
            printf(&D_80016378, *i_stat,
                         *i_mask);
            D_800A2610 = 0;
            *i_stat = 0;
        }
    } else {
        D_800A2610 = 0;
    }
    D_800A1578.inInterrupt = 0;
    ReturnFromException();
}

/* setIntr (LIBETC intr.c static, vtable slot 0x800A25E8) */
static IntrCallback setIntr(s32 irq, IntrCallback handler) {
    IntrCallback prevHandler;
    s32 mask;

    prevHandler = D_800A1578.handlers[irq];
    if (handler != prevHandler && D_800A1578.interruptsInitialized) {
        mask = *i_mask;
        *i_mask = 0;
        if (handler != 0) {
            D_800A1578.handlers[irq] = handler;
            mask = mask | (1 << irq);
            D_800A1578.enabledInterruptsMask |= 1 << irq;
        } else {
            D_800A1578.handlers[irq] = 0;
            mask = mask & ~(1 << irq);
            D_800A1578.enabledInterruptsMask &= ~(1 << irq);
        }
        if (irq == 0) {
            ChangeClearPAD(handler == 0);
            ChangeClearRCnt(3, handler == 0);
        }
        if (irq == 4) {
            ChangeClearRCnt(0, handler == 0);
        }
        if (irq == 5) {
            ChangeClearRCnt(1, handler == 0);
        }
        if (irq == 6) {
            ChangeClearRCnt(2, handler == 0);
        }
        *i_mask = mask;
    }
    return prevHandler;
}

/* stopIntr (LIBETC intr.c static, vtable slot 0x800A25F0) */
static intrEnv_t *stopIntr(void) {
    if (!D_800A1578.interruptsInitialized) {
        return 0;
    }
    EnterCriticalSection();
    D_800A1578.savedMask = *i_mask;
    D_800A1578.savedPcr = *d_pcr;
    *i_stat = (*i_mask = 0);
    *d_pcr &= 0x77777777;
    ResetEntryInt();
    D_800A1578.interruptsInitialized = 0;
    return &D_800A1578;
}

/* restartIntr (LIBETC intr.c static, vtable slot 0x800A25F8) */
static intrEnv_t *restartIntr(void) {
    if (D_800A1578.interruptsInitialized) {
        return 0;
    }
    HookEntryInt(D_800A1578.buf);
    D_800A1578.interruptsInitialized = 1;
    *i_mask = D_800A1578.savedMask;
    *d_pcr = D_800A1578.savedPcr;
    ExitCriticalSection();
    return &D_800A1578;
}

/* memclr (LIBETC intr.c) */
void func_800831A4(u16 *ptr, s32 size) {
    s32 *e = (s32 *)ptr;
    s32 i;

    for (i = size - 1; i != -1; i--) {
        *e++ = 0;
    }
}
