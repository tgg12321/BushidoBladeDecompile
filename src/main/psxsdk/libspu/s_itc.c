/* PsyQ 4.0 LIBSPU S_ITC: SpuIsTransferCompleted. .text 0x8008AEB0..0x8008AF58, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"
#include <psxsdk/libapi.h>

s32 SpuIsTransferCompleted(s32 arg0) {
    s32 var_v0;

    if ((_spu_trans_mode == 1) || (_spu_inTransfer == 1)) {
        return 1;
    }
    var_v0 = TestEvent(_spu_EVdma);
    if (arg0 == 1) {
        while (var_v0 == 0) {
            var_v0 = TestEvent(_spu_EVdma);
        }
        _spu_inTransfer = 1;
        return 1;
    }
    if (var_v0 == 1) {
        _spu_inTransfer = var_v0;
    }
    return var_v0;
}
