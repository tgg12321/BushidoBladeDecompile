/* PsyQ 4.0 LIBAPI A53: write, the BIOS B(0x35) trampoline. .text 0x80078A08..0x80078A18, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(write, 0x35);
