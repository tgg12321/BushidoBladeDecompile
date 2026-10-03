/* PsyQ 4.0 LIBSND VS_VTC: SsVabTransCompleted. .text 0x80088584..0x800885AC, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libspu.h>

s16 SsVabTransCompleted(s16 a0) {
    return SpuIsTransferCompleted(a0);
}
