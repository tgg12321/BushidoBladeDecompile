/* PsyQ 4.0 LIBSPU S_SNC: SpuSetNoiseClock. .text 0x80089D10..0x80089D60, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

s32 SpuSetNoiseClock(s32 a0) {
    s32 val;
    if (a0 < 0) {
        val = 0;
    } else if (a0 >= 0x40) {
        val = 0x3F;
    } else {
        val = a0;
    }
    _spu_RXX->rxx.spucnt =
        (_spu_RXX->rxx.spucnt & 0xC0FF) | ((val & 0x3F) << 8);
    return val;
}
