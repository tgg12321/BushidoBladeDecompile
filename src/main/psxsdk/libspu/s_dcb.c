/* PsyQ 4.0 LIBSPU S_DCB: _SpuDataCallback. .text 0x800892D4..0x800892F8, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern void DMACallback(s32, s32);

void _SpuDataCallback(s32 a0) {
    DMACallback(4, a0);
}
