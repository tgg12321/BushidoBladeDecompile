/* PsyQ 4.0 LIBAPI L02: SysEnqIntRP, the BIOS C(0x02) trampoline. .text 0x80078F40..0x80078F50, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_C_FUNCTION(SysEnqIntRP, 0x2);
