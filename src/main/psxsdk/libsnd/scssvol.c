/* PsyQ 4.0 LIBSND SSSV: SsSetSerialVol. .text 0x80085448..0x80085544, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* PsyQ LIBSND ssvol: SsSetSerialVol — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libsnd/scssvol.c.
   SpuCommonAttr per PsyQ libspu.h (sizeof = 0x28 — matches the frame). */

void SsSetSerialVol(s16 s_num, s16 voll, s16 volr) {
    SpuCommonAttr attr;
    if ((u8)s_num == 0) {
        attr.mask = 0xC0;
        if (voll >= 0x80) {
            voll = 0x7F;
        }
        if (volr >= 0x80) {
            volr = 0x7F;
        }
        attr.cd.volume.left = voll * 258;
        attr.cd.volume.right = volr * 258;
    }
    if ((u8)s_num == 1) {
        attr.mask = 0xC00;
        if (voll >= 0x80) {
            voll = 0x7F;
        }
        if (volr >= 0x80) {
            volr = 0x7F;
        }
        attr.ext.volume.left = voll * 258;
        attr.ext.volume.right = volr * 258;
    }
    SpuSetCommonAttr(&attr);
}
