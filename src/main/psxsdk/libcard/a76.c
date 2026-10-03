/* PsyQ 4.0 LIBCARD A76: StopCARD2, the BIOS B(0x4C) trampoline. .text 0x8007A448..0x8007A458, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(StopCARD2, 0x4C);
