/* PsyQ 4.0 LIBAPI SENDPAD: _SendPAD, _send_pad and the BIOS patch stub func_800790A4 (data-as-code:
 * the 4 patch words, up to D_800790B4, that _send_pad writes into the BIOS pad code at B0[0x5B] +0x3D8
 * and +0x4E0; then the module's 3 pad words).
 * .text 0x80079000..0x800790C0, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json),
 * Q106 D3; split from C68 by owner ruling Q108 (docs/naming/libscan/boundary_fixes.md). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", _SendPAD);
INCLUDE_ASM("asm/funcs", _send_pad);
INCLUDE_ASM("asm/funcs", func_800790A4);
