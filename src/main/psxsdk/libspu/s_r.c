/* PsyQ 4.0 LIBSPU S_R: SpuRead. .text 0x8008AD64..0x8008ADC4, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

s32 SpuRead(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    _spu_Fr(a0, a1);
    if (_spu_transferCallback == NULL) {
        _spu_inTransfer = 0;
    }
    return a1;
}
