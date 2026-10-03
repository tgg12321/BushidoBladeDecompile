/* PsyQ 4.0 LIBSPU S_STM: SpuSetTransferMode. .text 0x8008AE7C..0x8008AEB0, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_trans_mode;
extern s32 _spu_transMode;

/* PsyQ LIBSPU s_stm.c: SpuSetTransferMode — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_stm.c */
s32 SpuSetTransferMode(s32 mode) {
    s32 transMode;

    switch (mode) {
        case 0:
            transMode = 0;
            break;
        case 1:
            transMode = 1;
            break;
        default:
            transMode = 0;
    }
    _spu_trans_mode = mode;
    _spu_transMode = transMode;
    return transMode;
}
