/* PsyQ 4.0 LIBAPI A67: nextfile, the BIOS B(0x43) trampoline. .text 0x80078A48..0x80078A58, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(nextfile, 0x43);
