/* PsyQ 4.0 LIBAPI A20: StopPAD2, the BIOS B(0x14) trampoline. .text 0x80078F20..0x80078F30, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(StopPAD2, 0x14);
