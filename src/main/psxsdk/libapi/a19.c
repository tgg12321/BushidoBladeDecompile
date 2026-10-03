/* PsyQ 4.0 LIBAPI A19: StartPAD2, the BIOS B(0x13) trampoline. .text 0x80078F10..0x80078F20, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(StartPAD2, 0x13);
