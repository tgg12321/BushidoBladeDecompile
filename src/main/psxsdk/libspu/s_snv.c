/* PsyQ 4.0 LIBSPU S_SNV: SpuSetNoiseVoice. .text 0x80089A24..0x80089A48, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _SpuSetAnyVoice(s32, u32, s32, s32);

void SpuSetNoiseVoice(s32 a0, s32 a1) {
    _SpuSetAnyVoice(a0, a1, 0xCA, 0xCB);
}
