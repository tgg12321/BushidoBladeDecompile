/* PsyQ 4.0 LIBGTE GEO_01: rcos. .text 0x8007DFEC..0x8007E08C, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libgte.h>

/* Declarations from the file this module was split from
 * (src/main/psxsdk/libgpu/sys.c, ex display.c). */
extern s16 rsin_tbl[];
extern s16 g_cos_lut_q2[];
extern s16 g_cos_lut_q4[];

s32 rcos(s32 a0) {
    if (a0 < 0) {
        a0 = -a0;
    }
    a0 = a0 & 0xFFF;
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return rsin_tbl[0x400 - a0];
        }
        return -g_cos_lut_q2[a0];
    }
    if (a0 < 0xC01) {
        return -rsin_tbl[0xC00 - a0];
    }
    return g_cos_lut_q4[a0];
}
