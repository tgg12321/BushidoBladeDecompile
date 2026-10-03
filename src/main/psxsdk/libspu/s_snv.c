/* PsyQ 4.0 LIBSPU S_SNV: SpuSetNoiseVoice. .text 0x80089A24..0x80089A48, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

void SpuSetNoiseVoice(s32 a0, s32 a1) {
    _SpuSetAnyVoice(a0, a1, 0xCA, 0xCB);
}
