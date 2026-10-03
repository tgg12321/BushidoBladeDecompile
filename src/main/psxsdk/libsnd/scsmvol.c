/* PsyQ 4.0 LIBSND SSSMV: SsSetMVol. .text 0x80083BE4..0x80083C34, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern void SpuSetCommonAttr(void *);

void SsSetMVol(s16 a0, s16 a1) {
    s32 buf[10];
    buf[0] = 3;
    *(s16 *)&buf[1] = (s16)(a0 * 129);
    *((s16 *)&buf[1] + 1) = (s16)(a1 * 129);
    SpuSetCommonAttr(buf);
}
