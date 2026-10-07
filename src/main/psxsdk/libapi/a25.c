/* PsyQ 4.0 LIBAPI A25: HookEntryInt, the BIOS B(0x19) trampoline. .text
 * 0x80083210..0x80083220, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_B_FUNCTION(HookEntryInt, 0x19);
