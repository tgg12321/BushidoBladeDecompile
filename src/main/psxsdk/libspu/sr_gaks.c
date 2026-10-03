/* PsyQ 4.0 LIBSPU SR_GAKS: SpuRGetAllKeysStatus and SpuGetAllKeysStatus. .text
 * 0x8008B330..0x8008B488, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_keystat;
extern s32 _spu_RXX;

/* PsyQ 4.0 LIBSPU sr_gaks: SpuRGetAllKeysStatus — verbatim-linked Sony object
   (module SR_GAKS spans 0x8008B330..0x8008B488). This
   entry point is UNREFERENCED in BB2 (dead code pulled in by whole-object
   linking) — no glabel exists at 0x8008B330, so it shares func_8008AF9C's
   splat extent. C ref: sotn-decomp src/main/psxsdk/libspu/sr_gaks.c. */
static s32 SpuRGetAllKeysStatus(s32 min, s32 max, s8 *status) {
    s32 voice;
    u16 volumex;

    if (min < 0) {
        min = 0;
    }
    if (min >= 24) {
        return -3;
    }
    if (max >= 24) {
        max = 23;
    }
    if (max < 0 || max < min) {
        return -3;
    }

    max++;
    for (voice = min; voice < max; voice++) {
        s32 off = voice << 4;
        s32 bit;
        volumex = *(u16 *)((off + _spu_RXX) + 0xC);
        bit = _spu_keystat & (1 << voice);
        if (bit) {
            if (volumex != 0) {
                status[voice] = 1;
            } else {
                status[voice] = 3;
            }
        } else {
            if (volumex != 0) {
                status[voice] = 2;
            } else {
                status[voice] = 0;
            }
        }
    }

    return 0;
}
/* PsyQ LIBSPU sr_gaks.c: SpuGetAllKeysStatus — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/sr_gaks.c
   (SpuRGetAllKeysStatus inlined with min=0, max=NUM_SPU_CHANNELS) */
void SpuGetAllKeysStatus(u8 *status) {
    s32 limit = 24;
    s32 voice = 0;

    do {
        s32 off = voice << 4;
        u16 volumex;
        s32 bit;
        volumex = *((u16 *)((off + _spu_RXX) + 0xC));
        bit = _spu_keystat & (1 << voice);
        if (bit) {
            if (volumex != 0) {
                status[voice] = 1;
            } else {
                status[voice] = 3;
            }
        } else if (volumex != 0) {
            status[voice] = 2;
        } else {
            status[voice] = 0;
        }
        voice++;
    } while (voice < limit);
}
