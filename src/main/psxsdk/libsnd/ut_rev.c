/* PsyQ 4.0 LIBSND UT_REV: SsUtSetReverbType and SsUtGetReverbType. .text 0x80085EE4..0x80085F98, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 SpuSetReverbModeParam();

extern s32 _svm_rattr;
extern s32 _svm_rattr_plus_0x4;
s16 SsUtSetReverbType(s16 a0) {
    s32 neg = 0;
    s16 v1 = a0;
    s32 s0;
    if ((s32)(a0 << 16) < 0) {
        neg = 1;
        v1 = -a0;
    }
    if ((u16)v1 < 0xA) {
        _svm_rattr = 1;
        if (neg) {
            _svm_rattr_plus_0x4 = (s16)((v1 | 0x100) << 16 >> 16);
        } else {
            _svm_rattr_plus_0x4 = (s16)(v1 << 16 >> 16);
        }
        s0 = (s16)(v1 << 16 >> 16);
        if (s0 == 0) {
            SpuSetReverb(0);
        }
        SpuSetReverbModeParam(&_svm_rattr);
        return s0;
    }
    return -1;
}
s16 SsUtGetReverbType(void) {
    return *(s16 *)&_svm_rattr_plus_0x4;
}
