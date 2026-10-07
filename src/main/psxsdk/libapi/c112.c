/* PsyQ 4.0 LIBAPI C112: _bu_init, the BIOS A(0x70) trampoline. .text
 * 0x80078958..0x80078968, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_A_FUNCTION(_bu_init, 0x70);
