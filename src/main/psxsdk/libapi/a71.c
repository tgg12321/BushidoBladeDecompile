/* PsyQ 4.0 LIBAPI A71: AddDrv, the BIOS B(0x47) trampoline. .text 0x8008D050..0x8008D060, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(AddDrv, 0x47);
