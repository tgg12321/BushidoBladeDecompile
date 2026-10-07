/* PsyQ 4.0 LIBCARD C171: _card_info, the BIOS A(0xAB) trampoline. .text
 * 0x8007A2F8..0x8007A308, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_A_FUNCTION(_card_info, 0xAB);
