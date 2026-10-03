/* PsyQ 4.0 LIBAPI A21: PAD_init2, the BIOS B(0x15) trampoline. .text 0x80078F30..0x80078F40, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(PAD_init2, 0x15);
