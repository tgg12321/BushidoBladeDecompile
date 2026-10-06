/* PsyQ 4.0 LIBSND UT_REV: SsUtSetReverbType and SsUtGetReverbType. .text 0x80085EE4..0x80085F98, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

s16 SsUtSetReverbType(s16 a0) {
    s32 neg = 0;
    s16 v1 = a0;
    if (a0 < 0) {
        neg = 1;
        v1 = -a0;
    }
    if ((u16)v1 < 0xA) {
        _svm_rattr.mask = 1;
        if (neg) {
            _svm_rattr.mode = v1 | 0x100;
        } else {
            _svm_rattr.mode = v1;
        }
        if (v1 == 0) {
            SpuSetReverb(0);
        }
        SpuSetReverbModeParam(&_svm_rattr);
        return v1;
    }
    return -1;
}
s16 SsUtGetReverbType(void) {
    return _svm_rattr.mode;
}
