/* PsyQ 4.0 LIBSPU S_I: SpuInit. .text 0x800885AC..0x800885CC, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern void _SpuInit(s32);

void SpuInit(void) {
    _SpuInit(0);
}
