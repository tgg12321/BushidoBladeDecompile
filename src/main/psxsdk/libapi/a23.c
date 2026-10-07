/* PsyQ 4.0 LIBAPI A23: ReturnFromException, the BIOS B(0x17) trampoline. .text
 * 0x800831F0..0x80083200, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(ReturnFromException, 0x17);
