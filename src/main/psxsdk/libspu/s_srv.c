/* PsyQ 4.0 LIBSPU S_SRV: SpuSetReverbVoice. .text 0x8008A904..0x8008A928, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
s32 _SpuSetAnyVoice(s32 on_off, u32 bits, s32 addr1, s32 addr2);

void SpuSetReverbVoice(s32 a0, s32 a1) {
    _SpuSetAnyVoice(a0, a1, 0xCC, 0xCD);
}
