/* PsyQ 4.0 LIBGTE MTX_03: MulMatrix. .text 0x8007EB4C..0x8007EC5C, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007EB4C = LIBGTE MTX_03 MulMatrix â€” verbatim-linked Sony PsyQ 4.0
 * object. In-place variant of the same 3-cycle mvmva
 * transform as func_8007E4DC / calc_fc_frame_8007EC5C: reads matrix + vec
 * from $a0 (out doubles as matrix-input buffer), writes result back to $a0.
 * All the calc_fc_frame (MulMatrix2) hand-coded signals hold. Canonical body.
 */
INCLUDE_ASM("asm/funcs", MulMatrix);
