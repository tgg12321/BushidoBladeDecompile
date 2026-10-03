/* PsyQ 4.0 LIBSND VS_MONO: SsSetMono and SsSetStereo. .text 0x80087F10..0x80087F34, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s16 _svm_stereo_mono;

void SsSetMono(void) {
    _svm_stereo_mono = 1;
}

void SsSetStereo(void) {
    _svm_stereo_mono = 0;
}
