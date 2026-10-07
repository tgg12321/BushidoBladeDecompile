/* PsyQ 4.0 LIBGTE MTX_03: MulMatrix. .text 0x8007EB4C..0x8007EC5C, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* MulMatrix: in-place matrix multiply, three MVMVA passes; the result is
 * written back over the matrix at $a0. */
INCLUDE_ASM("asm/funcs", MulMatrix);
