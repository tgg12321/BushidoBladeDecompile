/* PsyQ 4.0 LIBSPU S_IT: _spu_setInTransfer and _spu_getInTransfer. .text
 * 0x8008AF58..0x8008AF9C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

void _spu_setInTransfer(s32 a0) {
    if (a0 == 1) {
        _spu_inTransfer = 0;
    } else {
        _spu_inTransfer = 1;
    }
}

s32 _spu_getInTransfer(void) { return _spu_inTransfer != 1; }
