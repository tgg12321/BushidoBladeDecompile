/* PsyQ 4.0 LIBETC INTR_DMA: the DMA interrupt hooks (startIntrDMA, trapIntrDMA, setIntrDMA and the
 * module's memclr, sys_MemClear2; SOTN libetc/intr_dma.c). .text 0x800833C8..0x80083670, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libetc_internal.h"

/* .rodata 0x80016394..0x800163C0: trapIntrDMA's bus-error report (moved from
 * src/text1a_b_tail_rodata.c, Q106 D4: every reader is in this file, in link order). */

/* D_80016394: 1 string(s), 28B @ 0x80016394 */
const char D_80016394[28] =
    "DMA bus error: code=%08x\n\0\0\0"
    ;

/* D_800163B0: 1 string(s), 16B @ 0x800163B0 */
const char D_800163B0[16] =
    "MADR[%d]=%08x\n\0\0"
    ;

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
void InterruptCallback(void);

extern s32 D_800A2640[8];
/* PsyQ 4.0 LIBETC intr_dma.c module state (verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libetc/intr_dma.c).
   D_800A263C holds 0x1F8010F4 (DMA Interrupt Register) — Sony declares it
   `static volatile u_long *` (pointer-to-volatile-MMIO, type-level). */
extern volatile u32 *D_800A263C;
extern u32 *D_800A2660;

void trapIntrDMA(void);
s32 setIntrDMA(s32, s32);
s32 startIntrDMA(void) {
    sys_MemClear2((s32 *)&D_800A2640, 8);
    *D_800A263C = 0;
    ((void (*)(s32, void *))InterruptCallback)(3, (void *)trapIntrDMA);
    return (s32)setIntrDMA;
}

/* PsyQ 4.0 LIBETC INTR_DMA: trapIntrDMA (static) — verbatim-linked Sony
   object; C ref: sotn-decomp libetc/intr_dma.c */
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

/* PsyQ 4.0 LIBETC INTR_DMA: setIntrDMA (static) — verbatim-linked Sony
   object; C ref: sotn-decomp libetc/intr_dma.c */
s32 setIntrDMA(s32 a0, s32 a1) {
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
