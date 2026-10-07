/* PsyQ 4.0 LIBGTE MTX_04: MulMatrix2. .text 0x8007EC5C..0x8007ED6C, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Hand-written GTE 3x3 mvmva matrix transform: the mvmva/mfc2/mtc2/nop
 * pipeline is hand-scheduled, so it has no C form. */
INCLUDE_ASM("asm/funcs", MulMatrix2);
