/* PsyQ 4.0 LIBAPI COUNTER: the root counters (SetRCnt, GetRCnt, StartRCnt,
 * StopRCnt, ResetRCnt). .text 0x80078A68..0x80078BE0, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libapi.h>

/* Declarations from the file this module was split from (src/main/64FD8.c, ex
 * text1b_b.c). */
/* One root counter's registers (psx-spx "Timers": 0x1F801100 + n * 0x10). */
typedef struct {
    u16 count; /* +0 current value */
    u16 pad2;
    u16 mode; /* +4 counter mode */
    u16 pad6;
    u16 target; /* +8 counter target */
    u16 padA[3];
} RCnt;

/* .word 0x1F801070 (I_STAT; [1] = I_MASK), asm/data/7D920.data.s */
extern volatile s32 *D_8009BD68;
/* .word 0x1F801100 (root counters 0..2), asm/data/7D920.data.s */
extern volatile RCnt *D_8009BD6C;
extern s32 D_8009BD70[]; /* each counter's I_MASK bit: 0x10, 0x20, 0x40, 0x01 */

s32 SetRCnt(u32 arg0, s32 arg1, s32 arg2) {
    s32 t0 = arg0 & 0xFFFF;
    s32 a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    D_8009BD6C[t0].mode = 0;
    D_8009BD6C[t0].target = arg1;
    if ((u32)t0 < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else if (t0 == 2) {
        if (!(arg2 & 1)) {
            a3 = 0x248;
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    D_8009BD6C[t0].mode = a3;
    return 1;
}

s32 GetRCnt(u32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    return D_8009BD6C[v].count;
}

s32 StartRCnt(u32 arg0) {
    s32 v;
    v = arg0 & 0xFFFF;
    D_8009BD68[1] |= D_8009BD70[v];
    return v < 3;
}

s32 StopRCnt(u32 arg0) {
    s32 v;
    v = arg0 & 0xFFFF;
    D_8009BD68[1] &= ~D_8009BD70[v];
    return 1;
}

s32 ResetRCnt(u32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    D_8009BD6C[v].count = 0;
    return 1;
}
