/* PsyQ 4.0 LIBAPI A07: DeliverEvent, the BIOS B(0x07) trampoline. .text
 * 0x8008008C..0x8008009C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(DeliverEvent, 0x7);
