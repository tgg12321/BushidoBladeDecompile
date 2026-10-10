/* PsyQ 4.0 LIBGTE GEO_00: rsin and sin_1. .text 0x8007DF20..0x8007DFEC, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libgte.h>

extern s32 sin_1(s32);

s32 rsin(s32 a0) {
    s32 v;
    if (a0 < 0) {
        v = sin_1((-a0) & 0xFFF);
        return -v;
    }
    return sin_1(a0 & 0xFFF);
}

extern s16 rsin_tbl[];
extern s16 D_8009AF94[];

s32 sin_1(s32 a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return rsin_tbl[a0];
        }
        return rsin_tbl[0x800 - a0];
    }
    if (a0 < 0xC01) {
        return -D_8009AF94[a0];
    }
    return -rsin_tbl[0x1000 - a0];
}
