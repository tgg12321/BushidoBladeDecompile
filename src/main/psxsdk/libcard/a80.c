/* PsyQ 4.0 LIBCARD A80: _new_card, the BIOS B(0x50) trampoline. .text
 * 0x8007A360..0x8007A370, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(_new_card, 0x50);
