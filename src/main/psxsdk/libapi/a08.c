/* PsyQ 4.0 LIBAPI A08: OpenEvent, the BIOS B(0x08) trampoline. .text
 * 0x80078978..0x80078988, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(OpenEvent, 0x8);
