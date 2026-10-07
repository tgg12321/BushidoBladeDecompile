/* PsyQ 4.0 LIBAPI C73: GPU_cw, the BIOS A(0x49) trampoline. .text
 * 0x8007DF10..0x8007DF20, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_A_FUNCTION(GPU_cw, 0x49);
