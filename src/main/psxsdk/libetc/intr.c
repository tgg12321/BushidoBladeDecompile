/* PsyQ 4.0 LIBETC INTR: the interrupt dispatcher (ResetCallback .. memclr; $Id:
 * intr.c,v 1.76; SOTN libetc/intr.c). .text 0x80082AC0..0x800831D0, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libapi.h>
#include "libetc_internal.h"
#include <psxsdk/libc.h>

/* .rodata 0x80016328..0x80016394: the module's rcsid (pointed at only by INTR's
 * callbacks table) and trapIntr's two messages; every C reader is in this file
 * (Q106 D4). */

const char D_80016328[52] =
    "$Id: intr.c,v 1.76 1997/02/12 12:45:05 makoto Exp $\0";

const char D_8001635C[28] = "unexpected interrupt(%04x)\n\0";

const char D_80016378[28] = "intr timeout(%04x:%04x)\n\0\0\0\0";

/* g_InterruptMask = (u16 *)0x1F801074, I_MASK (MMIO) */
extern volatile u16 *g_InterruptMask;
extern s32 *pCallbacks;

/* intr.c module state (SOTN libetc/intr.c:28, a static intrEnv_t);
 * i_stat/g_InterruptMask/d_pcr are the module's MMIO pointer statics
 * (0x1F801070/74/F0). */
typedef struct {
    u16 interruptsInitialized;  /* +0x00 = intrEnv */
    u16 inInterrupt;            /* +0x02 */
    void (*handlers[11])(void); /* +0x04 */
    u16 enabledInterruptsMask;  /* +0x30 */
    u16 savedMask;              /* +0x32 */
    s32 savedPcr;               /* +0x34 */
    s32 buf[12];                /* +0x38 jmp_buf; [1] = JB_SP = D_800A15B4 */
    s32 stack[1024];            /* +0x68 */
} intrEnv_t;                    /* sizeof 0x1068; memclr count 0x41A words */

extern volatile u16 *i_stat; /* i_stat = (u16 *)0x1F801070 (MMIO) */
extern volatile s32 *d_pcr;  /* d_pcr  = (s32 *)0x1F8010F0 (MMIO) */
extern intrEnv_t intrEnv;

s32 ResetCallback(void) { return ((s32(*)(void))pCallbacks[3])(); }

void *InterruptCallback(s32 irq, void (*func)()) {
    return ((void *(*)(s32, void (*)()))pCallbacks[2])(irq, func);
}

void *DMACallback(s32 dma, void (*func)()) {
    return ((void *(*)(s32, void (*)()))pCallbacks[1])(dma, func);
}

void VSyncCallback(s32 a0) { ((void (*)(s32, s32))pCallbacks[5])(4, a0); }

void VSyncCallbacks(void) { ((void (*)(void))pCallbacks[5])(); }

s32 StopCallback(void) { return ((s32(*)(void))pCallbacks[4])(); }

s32 RestartCallback(void) { return ((s32(*)(void))pCallbacks[6])(); }

s32 CheckCallback(void) { return intrEnv.inInterrupt; }

u32 GetIntrMask(void) { return *g_InterruptMask; }

extern void trapIntr(void);
/* FAKE: the BIOS call takes no argument; declared with one for startIntr
 * (a no-argument call there: score 3). */
extern void _96_remove(s32 *);

u16 SetIntrMask(u16 arg0) {
    u16 old = *g_InterruptMask;
    *g_InterruptMask = arg0;
    return old;
}

/* startIntr (LIBETC intr.c static) */
intrEnv_t *startIntr(void) {
    if (intrEnv.interruptsInitialized) {
        return 0;
    }
    /* g_InterruptMask deref is MMIO (0x1F801074) — volatile is hardware
     * semantics */
    *i_stat = (*g_InterruptMask = 0);
    *d_pcr = 0x33333333;
    func_800831A4(&intrEnv, 0x41A);
    if (setjmp(intrEnv.buf) != 0) {
        trapIntr();
    }
    intrEnv.buf[1] = (s32)&intrEnv.stack[1004];
    HookEntryInt(intrEnv.buf);
    intrEnv.interruptsInitialized = 1;
    pCallbacks[5] = startIntrVSync();
    pCallbacks[1] = startIntrDMA();
    /* FAKE: _96_remove (BIOS A(72h)) takes no argument; passing the table
     * keeps it live in $a0 into the call; `_96_remove()` scores 3. */
    _96_remove(pCallbacks);
    ExitCriticalSection();
    return &intrEnv;
}

/* setIntr/stopIntr/restartIntr are statics referenced only through the
 * callbacks vtable words at 0x800A25E8/F0/F8. */
typedef void (*IntrCallback)(void);
extern s32 D_800A2610; /* trapMissedCount */

/* trapIntr */
void trapIntr(void) {
    s32 i;
    u16 mask;

    if (!intrEnv.interruptsInitialized) {
        printf(&D_8001635C, *i_stat);
        ReturnFromException();
    }
    intrEnv.inInterrupt = 1;
    while ((mask = (intrEnv.enabledInterruptsMask & *i_stat) &
                   *g_InterruptMask) != 0) {
        for (i = 0; mask && i < 11; ++i, mask >>= 1) {
            if (mask & 1) {
                *i_stat = ~(1 << i);
                if (intrEnv.handlers[i] != 0) {
                    intrEnv.handlers[i]();
                }
            }
        }
    }
    if (*i_stat & *g_InterruptMask) {
        if (D_800A2610++ > 0x800) {
            printf(&D_80016378, *i_stat, *g_InterruptMask);
            D_800A2610 = 0;
            *i_stat = 0;
        }
    } else {
        D_800A2610 = 0;
    }
    intrEnv.inInterrupt = 0;
    ReturnFromException();
}

/* setIntr (LIBETC intr.c static, vtable slot 0x800A25E8) */
static IntrCallback setIntr(s32 irq, IntrCallback handler) {
    IntrCallback prevHandler;
    s32 mask;

    prevHandler = intrEnv.handlers[irq];
    if (handler != prevHandler && intrEnv.interruptsInitialized) {
        mask = *g_InterruptMask;
        *g_InterruptMask = 0;
        if (handler != 0) {
            intrEnv.handlers[irq] = handler;
            mask = mask | (1 << irq);
            intrEnv.enabledInterruptsMask |= 1 << irq;
        } else {
            intrEnv.handlers[irq] = 0;
            mask = mask & ~(1 << irq);
            intrEnv.enabledInterruptsMask &= ~(1 << irq);
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
        *g_InterruptMask = mask;
    }
    return prevHandler;
}

/* stopIntr (LIBETC intr.c static, vtable slot 0x800A25F0) */
static intrEnv_t *stopIntr(void) {
    if (!intrEnv.interruptsInitialized) {
        return 0;
    }
    EnterCriticalSection();
    intrEnv.savedMask = *g_InterruptMask;
    intrEnv.savedPcr = *d_pcr;
    *i_stat = (*g_InterruptMask = 0);
    *d_pcr &= 0x77777777;
    ResetEntryInt();
    intrEnv.interruptsInitialized = 0;
    return &intrEnv;
}

/* restartIntr (LIBETC intr.c static, vtable slot 0x800A25F8) */
static intrEnv_t *restartIntr(void) {
    if (intrEnv.interruptsInitialized) {
        return 0;
    }
    HookEntryInt(intrEnv.buf);
    intrEnv.interruptsInitialized = 1;
    *g_InterruptMask = intrEnv.savedMask;
    *d_pcr = intrEnv.savedPcr;
    ExitCriticalSection();
    return &intrEnv;
}

/* memclr (LIBETC intr.c) */
void func_800831A4(u16 *ptr, s32 size) {
    s32 *e = (s32 *)ptr;
    s32 i;

    for (i = size - 1; i != -1; i--) {
        *e++ = 0;
    }
}
