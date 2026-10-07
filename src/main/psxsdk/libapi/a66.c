/* PsyQ 4.0 LIBAPI A66: firstfile, the BIOS B(0x42) trampoline. .text
 * 0x80078A38..0x80078A48, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(firstfile, 0x42);
