/* PsyQ 4.0 LIBSND SSSATTR: SsSetSerialAttr (SOTN's file for it is
 * libsnd/scssattr.c). .text 0x80083B50..0x80083BE4, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsSetSerialAttr(s32 a0, s32 a1, s32 a2) {
    SpuCommonAttr attr;

    if ((a0 & 0xFF) == 0) {
        if ((a1 & 0xFF) == 0) {
            attr.mask = 0x200;
            attr.cd.mix = a2 & 0xFF;
        }
        if ((a1 & 0xFF) == 1) {
            attr.mask = 0x100;
            attr.cd.reverb = a2 & 0xFF;
        }
    }
    if ((a0 & 0xFF) == 1) {
        if ((a1 & 0xFF) == 0) {
            attr.mask = 0x2000;
            attr.ext.mix = a2 & 0xFF;
        }
        if ((a1 & 0xFF) == 1) {
            attr.mask = 0x1000;
            attr.ext.reverb = a2 & 0xFF;
        }
    }
    SpuSetCommonAttr(&attr);
}
