/* PsyQ 4.0 LIBSND UT_GVBA: SsUtGetVBaddrInSB. .text 0x800859F0..0x80085A40, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

s32 SsUtGetVBaddrInSB(s16 a0) {
    if ((u16)a0 >= 0x11) {
        return -1;
    }
    if (_svm_vab_used[a0] != 1) {
        return -1;
    }
    return _svm_vab_start[a0];
}
