/* PsyQ 4.0 LIBAPI A91: ChangeClearPAD, the BIOS B(0x5B) trampoline. .text 0x80078A58..0x80078A68,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(ChangeClearPAD, 0x5B);
