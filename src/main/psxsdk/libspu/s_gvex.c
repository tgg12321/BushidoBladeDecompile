/* PsyQ 4.0 LIBSPU S_GVEX: SpuGetVoiceEnvelope. .text 0x8008BDE8..0x8008BE04, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_RXX;

void SpuGetVoiceEnvelope(s32 a0, u16 *a1) {
    a0 = (a0 << 4) + _spu_RXX;
    *a1 = *(u16 *)(a0 + 0xC);
}
