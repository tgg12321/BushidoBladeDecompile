/* PsyQ 4.0 LIBGTE MTX_05: ApplyMatrix. .text 0x8007ED6C..0x8007EDBC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Original LIBGTE assembly. Loads the 3x3 matrix (5 packed s32 words) into
 * cop2 controls 0-4, transforms *a1 by it (MVMVA 1,0,0,3,0), writes *a2. */
INCLUDE_ASM("asm/funcs", ApplyMatrix);
