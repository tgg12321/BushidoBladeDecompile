/* PsyQ 4.0 LIBAPI C159: SetMem, the BIOS A(0x9F) trampoline. .text 0x80078968..0x80078978, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_A_FUNCTION(SetMem, 0x9F);
