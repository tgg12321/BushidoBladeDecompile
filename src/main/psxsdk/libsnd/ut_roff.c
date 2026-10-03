/* PsyQ 4.0 LIBSND UT_ROFF: SsUtReverbOff. .text 0x80085F98..0x80085FB8, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libspu.h>

void SsUtReverbOff(void) {
    SpuSetReverb(0);
}
