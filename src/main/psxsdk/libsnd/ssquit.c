/* PsyQ 4.0 LIBSND SSQUIT: SsQuit (SOTN libsnd/ssquit.c;
 * docs/naming/libscan/ambiguous_resolutions.md). .text 0x80083B30..0x80083B50, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
extern void SpuQuit(void);

void SsQuit(void) {
    SpuQuit();
}
