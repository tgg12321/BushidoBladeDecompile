/* PsyQ 4.0 LIBSND UT_RDEP: SsUtSetReverbDepth. .text 0x80085E4C..0x80085EE4, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libspu.h>

extern s32 _svm_rattr;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;
void SsUtSetReverbDepth(s16 a0, s16 a1) {
    s32 x = (s16)a0 * 32767 / 127;
    s32 y = (s16)a1 * 32767 / 127;
    s32 *buf = &_svm_rattr;
    *buf = 6;
    _svm_rattr_plus_0x8 = x;
    _svm_rattr_plus_0xA = y;
    SpuSetReverbModeParam(buf);
}
