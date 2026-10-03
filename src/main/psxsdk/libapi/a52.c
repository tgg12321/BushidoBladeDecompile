/* PsyQ 4.0 LIBAPI A52: read, the BIOS B(0x34) trampoline. .text 0x800789F8..0x80078A08, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(read, 0x34);
