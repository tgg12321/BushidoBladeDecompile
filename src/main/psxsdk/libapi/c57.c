/* PsyQ 4.0 LIBAPI C57: InitHeap, the BIOS A(0x39) trampoline. .text
 * 0x8008386C..0x8008387C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_A_FUNCTION(InitHeap, 0x39);
