/* PsyQ 4.0 LIBAPI C67: Exec, the BIOS A(0x43) trampoline. .text
 * 0x80078948..0x80078958, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_A_FUNCTION(Exec, 0x43);
