/* PsyQ 4.0 LIBSND VS_VTB: SsVabTransBody. .text 0x800884C4..0x80088584, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

s16 SsVabTransBody(s32 a0, s16 a1) {
    if ((u16)a1 >= 0x11) {
        _spu_setInTransfer(0);
        return -1;
    }
    if (_svm_vab_used[a1] != 2) {
        _spu_setInTransfer(0);
        return -1;
    }
    {
        s32 s0 = _svm_vab_start[a1];
        SpuSetTransferMode(0);
        SpuSetTransferStartAddr(s0);
        SpuWrite(a0, _svm_vab_total[a1]);
        _svm_vab_used[a1] = 1;
    }
    return a1;
}
