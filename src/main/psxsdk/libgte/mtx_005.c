/* PsyQ 4.0 LIBGTE MTX_005: ApplyRotMatrix. .text 0x8007E8AC..0x8007E8DC, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007E8AC â€” hand-written GTE mvmva vector-transform wrapper
 * (8007Exxx hand-asm cluster, sibling of MulMatrix2 / calc_fc_frame_8007EC5C;
 * owner-authorized per canonical-asm-authorization-recipe). Hand-coded evidence: the
 * lw encodings target $t0/$t1 (unreachable from compiled C without
 * forbidden pins), hand-placed GTE load-delay nop, return-pinned-at-end
 * addu $v0,$a2 pass-through, unfilled jr delay slot. cop2 ops splat-tagged
 * "handwritten instruction". */
INCLUDE_ASM("asm/funcs", ApplyRotMatrix);
