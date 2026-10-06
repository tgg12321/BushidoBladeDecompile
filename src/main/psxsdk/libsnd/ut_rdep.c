/* PsyQ 4.0 LIBSND UT_RDEP: SsUtSetReverbDepth. .text 0x80085E4C..0x80085EE4, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsUtSetReverbDepth(s16 a0, s16 a1) {
    _svm_rattr.mask = 6;
    _svm_rattr.depth.left = (s16)a0 * 32767 / 127;
    _svm_rattr.depth.right = (s16)a1 * 32767 / 127;
    SpuSetReverbModeParam(&_svm_rattr);
}
