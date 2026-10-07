/* PsyQ 4.0 LIBSND UT_RON: SsUtReverbOn. .text 0x80085FB8..0x80085FD8, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsUtReverbOn(void) { SpuSetReverb(1); }
