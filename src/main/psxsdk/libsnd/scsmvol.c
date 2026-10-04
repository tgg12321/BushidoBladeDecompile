/* PsyQ 4.0 LIBSND SSSMV: SsSetMVol. .text 0x80083BE4..0x80083C34, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsSetMVol(s16 a0, s16 a1) {
    SpuCommonAttr attr;
    attr.mask = 3;
    attr.mvol.left = a0 * 129;
    attr.mvol.right = a1 * 129;
    SpuSetCommonAttr(&attr);
}
