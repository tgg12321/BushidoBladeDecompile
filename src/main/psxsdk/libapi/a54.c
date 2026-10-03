/* PsyQ 4.0 LIBAPI A54: close, the BIOS B(0x36) trampoline. .text 0x80078A18..0x80078A28, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(close, 0x36);
