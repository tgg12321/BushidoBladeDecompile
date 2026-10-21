/* PsyQ 4.0 LIBSND SSINIT: _SsInit (SOTN libsnd/ssinit.c). .text
 * 0x80083A48..0x80083B30, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* SSINIT .data (0x800A2694..0x800A26CC, 56 bytes; the module's .data in
 * LIBSND.LIB, no XDEF, no relocation). The first 8 bytes are read by no code;
 * the library object holds other values there (0x15107350, 0x0040B19C). */
u32 D_800A2694[2] = {0x17107350, 0x404097E4};
/* the 8 SPU voice registers written to each of the 24 voices: volume L/R 0,
 * pitch 0x1000, start address 0x3000, ADSR 0x00BF / 0 */
u16 default_voice[8] = {0, 0, 0x1000, 0x3000, 0xBF, 0, 0, 0};
/* the 16 SPU control registers from 0x1F801D80: main volume L/R 0x3FFF */
u16 default_state[16] = {0x3FFF, 0x3FFF};
extern s32 _SsMarkCallback[32][16];

/* PsyQ 4.0 LIBSND ssinit: _SsInit — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/ssinit.c */
void _SsInit(void) {
    u16 *var_a2;
    int i, j;

    var_a2 = (u16 *)0x1F801C00;
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 8; j++) {
            *var_a2++ = default_voice[j];
        }
    }

    var_a2 = (u16 *)0x1F801D80;
    for (i = 0; i < 16; i++) {
        *var_a2++ = default_state[i];
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
