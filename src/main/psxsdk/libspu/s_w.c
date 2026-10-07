/* PsyQ 4.0 LIBSPU S_W: SpuWrite. .text 0x8008ADC4..0x8008AE24, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

s32 SpuWrite(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    _spu_Fw(a0, a1);
    if (_spu_transferCallback == NULL) {
        _spu_inTransfer = 0;
    }
    return a1;
}
