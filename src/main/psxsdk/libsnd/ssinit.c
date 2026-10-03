/* PsyQ 4.0 LIBSND SSINIT: _SsInit (SOTN libsnd/ssinit.c). .text 0x80083A48..0x80083B30, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

extern u16 D_800A269C;
extern u16 D_800A26AC;
extern s32 _SsMarkCallback[32][16];

/* PsyQ 4.0 LIBSND ssinit: _SsInit — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/ssinit.c */
void _SsInit(void) {
    u16 *var_a2;
    int i, j;

    var_a2 = (u16 *)0x1F801C00;
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 8; j++) {
            *var_a2++ = (&D_800A269C)[j];
        }
    }

    var_a2 = (u16 *)0x1F801D80;
    for (i = 0; i < 16; i++) {
        *var_a2++ = (&D_800A26AC)[i];
    }

    _SsVmInit(0x18);

    for (j = 0; j < 32; j++) {
        for (i = 0; i < 16; i++) {
            _SsMarkCallback[j][i] = 0;
        }
    }

    VBLANK_MINUS = 60;
    _snd_openflag = 0;
    _snd_ev_flag = 0;
}
