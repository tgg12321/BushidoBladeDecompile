/* PsyQ 4.0 LIBSND VS_VAB: SsVabClose. .text 0x80087F64..0x80087FE8, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsVabClose(s16 a0) {
    if ((u16)a0 < 0x10) {
        s16 idx = a0;
        if (_svm_vab_used[idx] == 1) {
            SpuFree(_svm_vab_start[idx]);
            _svm_vab_used[idx] = 0;
            _svm_vab_count--;
        }
    }
}
