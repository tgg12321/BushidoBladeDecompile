/* PsyQ 4.0 LIBSPU S_ITC: SpuIsTransferCompleted. .text 0x8008AEB0..0x8008AF58, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 TestEvent(s32);

s32 SpuIsTransferCompleted(s32 arg0) {
    s32 var_v0;

    if ((_spu_trans_mode == 1) || (_spu_inTransfer == 1)) {
        return 1;
    }
    var_v0 = TestEvent(_spu_EVdma);
    if (arg0 == 1) {
        if (var_v0 == 0) {
            do {
                var_v0 = TestEvent(_spu_EVdma);
            } while (var_v0 == 0);
        }
        var_v0 = 1;
        goto block_8;
    }
    if (var_v0 == 1) {
block_8:
        _spu_inTransfer = var_v0;
    }
    return var_v0;
}
