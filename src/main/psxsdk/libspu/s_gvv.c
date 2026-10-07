/* PsyQ 4.0 LIBSPU S_GVV: SpuGetVoiceVolume. .text 0x8008BD88..0x8008BDE8, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* Form of psyz decomp/src/libspu/s_gvv.c (Xeeynamo/psyz @ 973fa3460). */
static inline void assign(u16 val, s16 *out) {
    /* FAKE: 0x8000 held in a u32 so the adjustment is a subu; inline, uval -
     * 0x8000 becomes an addu of the same 0x8000 register (same low 16 bits):
     * score 2 */
    u32 offset = 0x8000;
    u32 uval = val;

    if (uval >= 0x4000) {
        *out = uval - offset;
    } else {
        *out = val;
    }
}

void SpuGetVoiceVolume(s32 arg0, s16 *arg1, s16 *arg2) {
    u16 left = _spu_RXX->raw[arg0 * 8];
    /* FAKE: read ahead of the left channel's store (the two lhu together); read
     * at its assign: score 14 */
    u16 right = _spu_RXX->raw[arg0 * 8 + 1];

    assign(left, arg1);
    assign(right, arg2);
}
