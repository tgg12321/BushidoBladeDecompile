/* PsyQ 4.0 LIBSPU S_SR: SpuSetReverb. .text 0x80089D60..0x80089E30, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* PsyQ 4.0 LIBSPU s_sr: SpuSetReverb — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_sr.c */
s32 SpuSetReverb(s32 on_off) {
    u16 cnt;
    switch (on_off) {
    case 0:
        cnt = _spu_RXX->rxx.spucnt;
        _spu_rev_flag = 0;
        cnt &= ~0x80;
        _spu_RXX->rxx.spucnt = cnt;
        break;

    case 1:
        if ((_spu_rev_reserve_wa != on_off) &&
            _SpuIsInAllocateArea_(_spu_rev_offsetaddr)) {
            cnt = _spu_RXX->rxx.spucnt;
            _spu_rev_flag = 0;
            cnt &= ~0x80;
            _spu_RXX->rxx.spucnt = cnt;
        } else {
            cnt = _spu_RXX->rxx.spucnt;
            _spu_rev_flag = on_off;
            cnt |= 0x80;
            _spu_RXX->rxx.spucnt = cnt;
        }
        break;
    }

    return _spu_rev_flag;
}
