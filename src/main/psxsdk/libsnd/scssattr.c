/* PsyQ 4.0 LIBSND SSSATTR: SsSetSerialAttr (SOTN's file for it is libsnd/scssattr.c). .text
 * 0x80083B50..0x80083BE4, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include "libsnd_i.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */
extern void SpuSetCommonAttr(s32 *);

void SsSetSerialAttr(s32 a0, s32 a1, s32 a2) {
    s32 buf[10];

    if ((a0 & 0xFF) == 0) {
        if ((a1 & 0xFF) == 0) {
            buf[0] = 0x200;
            buf[6] = a2 & 0xFF;
        }
        if ((a1 & 0xFF) == 1) {
            buf[0] = 0x100;
            buf[5] = a2 & 0xFF;
        }
    }
    if ((a0 & 0xFF) == 1) {
        if ((a1 & 0xFF) == 0) {
            buf[0] = 0x2000;
            buf[9] = a2 & 0xFF;
        }
        if ((a1 & 0xFF) == 1) {
            buf[0] = 0x1000;
            buf[8] = a2 & 0xFF;
        }
    }
    SpuSetCommonAttr(buf);
}
