/* PsyQ 4.0 LIBSND VS_VFB: SsVabFakeBody. .text 0x80087FE8..0x80088058, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

extern u8 _svm_vab_used[];

s16 SsVabFakeBody(s16 a0) {
    if ((u16)a0 < 0x11) {
        if (_svm_vab_used[a0] == 2) {
            _spu_setInTransfer(0);
            _svm_vab_used[a0] = 1;
            return a0;
        }
    }
    return -1;
}
