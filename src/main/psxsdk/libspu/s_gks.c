/* PsyQ 4.0 LIBSPU S_GKS: SpuGetKeyStatus. .text 0x8008ACD0..0x8008AD64, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

s32 SpuGetKeyStatus(s32 arg0) {
    volatile SPU_VOICE_REG *voices;
    s32 voice;
    s32 i;
    s32 voice_mask;
    s32 one;
    u16 volumex;

    voice = -1;
    /* FAKE: the bit search's register setup. `i = 0` ahead of `one = 1` rather
     * than in the for header (score 2), and `one` a named 1 rather than a
     * literal (literal: srav/andi, score 4). */
    i = 0;
    one = 1;
    for (; i < 0x18; i++) {
        if (arg0 & (one << i)) {
            voice = i;
            break;
        }
    }
    if (voice == -1) {
        return -1;
    }
    voices = _spu_RXX->rxx.voice;
    volumex = voices[voice].volumex;
    voice_mask = 1 << voice;
    if (_spu_keystat & voice_mask) {
        if (volumex > 0) {
            return 1;
        } else {
            return 3;
        }
    } else if (volumex > 0) {
        return 2;
    } else {
        return 0;
    }
}
