/* PsyQ 4.0 LIBAPI C68: FlushCache, the BIOS A(0x44) trampoline (4 words incl.
 * the module's trailing nop). .text 0x80078FF0..0x80079000, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", FlushCache);
