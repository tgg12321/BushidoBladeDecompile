/* PsyQ 4.0 LIBGTE MTX_05: ApplyMatrix. .text 0x8007ED6C..0x8007EDBC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007ED6C = LIBGTE MTX_05 ApplyMatrix â€” verbatim-linked Sony PsyQ 4.0
 * object. Loads a 3x3 R matrix (5 packed s32 words) into
 * cop2 controls 0-4, transforms *a1 vec by RT matrix (mvmva 1,0,0,3,0),
 * writes result to *a2. Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", ApplyMatrix);
