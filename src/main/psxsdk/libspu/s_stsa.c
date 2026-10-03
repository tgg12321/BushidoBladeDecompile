/* PsyQ 4.0 LIBSPU S_STSA: SpuSetTransferStartAddr. .text 0x8008AE24..0x8008AE7C, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

s32 SpuSetTransferStartAddr(s32 a0) {
    s32 v0;
    if ((u32)(a0 - 0x1010) > (u32)0x7EFE8) {
        return 0;
    }
    v0 = _spu_FsetRXXa(-1, a0);
    _spu_tsa = (u16)v0;
    return (u32)(u16)v0 << _spu_mem_mode_plus;
}
