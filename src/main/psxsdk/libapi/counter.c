/* PsyQ 4.0 LIBAPI COUNTER: the root counters (SetRCnt, GetRCnt, StartRCnt, StopRCnt, ResetRCnt).
 * .text 0x80078A68..0x80078BE0, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json),
 * Q106 D3. */
#include "common.h"
#include <psxsdk/libapi.h>

/* Declarations from the file this module was split from (src/main/64FD8.c, ex text1b_b.c). */
extern s32 D_8009BD68;
extern s32 D_8009BD6C;
extern s32 D_8009BD70;

s32 SetRCnt(u32 arg0, s32 arg1, s32 arg2) {
    s32 a3;
    s32 t0;
    s32 v0;
    s32 base;
    t0 = arg0 & 0xFFFF;
    a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    base = (t0 * 0x10) + D_8009BD6C;
    *(volatile u16 *) (base + 4) = 0;
    *(volatile u16 *) (base + 8) = arg1;
    if (((u32) t0) < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        v0 = arg2 & 0x1000;
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else {
        v0 = arg2 & 0x1000;
        if (t0 == 2) {
            ;
            if (!(arg2 & 1)) {
                a3 = 0x248;
            }
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    *(volatile u16 *) (((t0 * 0x10) + D_8009BD6C) + 4) = a3;
    return 1;
}
s32 GetRCnt(u32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    return *(volatile u16 *)(D_8009BD6C + v * 0x10);
}
s32 StartRCnt(u32 arg0) {
    s32 v;
    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] | (&D_8009BD70)[v];
    return v < 3;
}
s32 StopRCnt(u32 arg0) {
    s32 v;
    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] & ~(&D_8009BD70)[v];
    return 1;
}
s32 ResetRCnt(u32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    *(volatile u16 *)(D_8009BD6C + v * 0x10) = 0;
    return 1;
}
