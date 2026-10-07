/* PsyQ 4.0 LIBGTE RATAN: ratan2. .text 0x8007FD5C..0x8007FEDC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libgte.h>

extern s16 ratan_tbl[];

/* PsyQ LIBGTE ratan: ratan2 â€” verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp psxsdk (table-lookup atan2) */
s32 ratan2(s32 arg0, s32 arg1) {
    s32 var_v1;
    s32 var_a0;
    s32 var_a1;
    s32 var_a2;
    s32 var_a3;
    s32 idx;

    var_a0 = arg0;
    var_a1 = arg1;
    var_a2 = 0;
    var_a3 = 0;
    if (var_a1 < 0) {
        var_a2 = 1;
        var_a1 = -var_a1;
    }
    if (var_a0 < 0) {
        var_a3 = 1;
        var_a0 = -var_a0;
    }
    if (var_a1 == 0 && var_a0 == 0) {
        return 0;
    }
    if (var_a0 < var_a1) {
        if (var_a0 & 0x7FE00000) {
            idx = var_a0 / (var_a1 >> 0xA);
        } else {
            idx = (var_a0 << 0xA) / var_a1;
        }
        var_v1 = ratan_tbl[idx];
    } else {
        if (var_a1 & 0x7FE00000) {
            idx = var_a1 / (var_a0 >> 0xA);
        } else {
            idx = (var_a1 << 0xA) / var_a0;
        }
        var_v1 = 0x400 - ratan_tbl[idx];
    }
    if (var_a2 != 0) {
        var_v1 = 0x800 - var_v1;
    }
    if (var_a3 != 0) {
        var_v1 = -var_v1;
    }
    return var_v1;
}
