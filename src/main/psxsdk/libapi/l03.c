/* PsyQ 4.0 LIBAPI L03: SysDeqIntRP, the BIOS C(0x03) trampoline. .text 0x80078F50..0x80078F60, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_C_FUNCTION(SysDeqIntRP, 0x3);
