/* PsyQ 4.0 LIBETC INTR_DMA: the DMA interrupt hooks (startIntrDMA, trapIntrDMA,
 * setIntrDMA and the module's memclr, sys_MemClear2; SOTN libetc/intr_dma.c).
 * .text 0x800833C8..0x80083670, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libetc_internal.h"

/* trapIntrDMA's bus-error report (Q106 D4: every reader is in this file). */
const char D_80016394[28] = "DMA bus error: code=%08x\n\0\0\0";

const char D_800163B0[16] = "MADR[%d]=%08x\n\0\0";

extern s32 D_800A2640[8];
/* D_800A263C holds 0x1F8010F4 (DMA Interrupt Register); Sony declares it
   `static volatile u_long *`. */
extern volatile u32 *D_800A263C;
extern u32 *D_800A2660;

void trapIntrDMA(void);
s32 setIntrDMA(s32, s32);

s32 startIntrDMA(void) {
    sys_MemClear2((s32 *)&D_800A2640, 8);
    *D_800A263C = 0;
    InterruptCallback(3, trapIntrDMA);
    return (s32)setIntrDMA;
}

void trapIntrDMA(void) {
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
        printf(&D_80016394, *D_800A263C);
        for (i = 0; i < 7; i++) {
            printf(&D_800163B0, i, D_800A2660[4 * i]);
        }
    }
}

s32 setIntrDMA(s32 a0, s32 a1) {
    s32 prev = D_800A2640[a0];
    if (a1 != prev) {
        if (a1 != 0) {
            D_800A2640[a0] = a1;
            *D_800A263C =
                (*D_800A263C & 0xFFFFFF) | 0x800000 | (1 << (a0 + 16));
        } else {
            D_800A2640[a0] = 0;
            *D_800A263C =
                ((*D_800A263C & 0xFFFFFF) | 0x800000) & ~(1 << (a0 + 16));
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
