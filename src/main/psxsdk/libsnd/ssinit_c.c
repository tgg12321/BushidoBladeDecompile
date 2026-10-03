/* PsyQ 4.0 LIBSND SSINIT_C: SsInit (the cold-start variant; SSINIT_H, SsInitHot, is SOTN's
 * libsnd/ssinit_h.c; docs/naming/libscan/ambiguous_resolutions.md). .text 0x80083A18..0x80083A48, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
void ResetCallback(void);

void SsInit(void) {
    ResetCallback();
    SpuInit();
    _SsInit();
}
