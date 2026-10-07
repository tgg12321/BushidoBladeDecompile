/* PsyQ 4.0 LIBCARD A78: _card_write, the BIOS B(0x4E) trampoline. .text
 * 0x8007A350..0x8007A360, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(_card_write, 0x4E);
