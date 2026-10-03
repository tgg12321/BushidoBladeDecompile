/* PsyQ 4.0 LIBAPI L10: ChangeClearRCnt, the BIOS C(0x0A) trampoline. .text 0x80082AB0..0x80082AC0,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bios.h"

BIOS_C_FUNCTION(ChangeClearRCnt, 0xA);
