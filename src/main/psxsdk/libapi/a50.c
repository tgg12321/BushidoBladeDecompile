/* PsyQ 4.0 LIBAPI A50: open, the BIOS B(0x32) trampoline. .text 0x800789E8..0x800789F8, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(open, 0x32);
