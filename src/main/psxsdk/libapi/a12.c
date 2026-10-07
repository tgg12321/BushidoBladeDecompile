/* PsyQ 4.0 LIBAPI A12: EnableEvent, the BIOS B(0x0C) trampoline. .text
 * 0x800789A8..0x800789B8, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(EnableEvent, 0xC);
