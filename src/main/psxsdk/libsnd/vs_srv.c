/* PsyQ 4.0 LIBSND VS_SRV: SsSetReservedVoice. .text 0x80087F34..0x80087F64, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

s32 SsSetReservedVoice(s32 a0) {
    u8 v = (u8)a0;
    if (v >= 0x19 || v == 0) {
        return 0xFF;
    }
    _SsVmMaxVoice = a0;
    return v;
}
