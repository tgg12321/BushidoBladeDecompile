/* PsyQ 4.0 LIBGTE MTX_000: MulMatrix0. .text 0x8007E4DC..0x8007E5EC, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007E4DC = LIBGTE MTX_000 MulMatrix0 â€” verbatim-linked Sony PsyQ 4.0
 * object. 3x3-mvmva matrix transform sibling of
 * MulMatrix2 (calc_fc_frame_8007EC5C). All the same hand-coded
 * signals: splat-tagged every cop2 op "handwritten instruction", hardcoded
 * `swc2 $11, 16($a2)` source reg, hand-scheduled cycle-N+1-mfc2 during
 * cycle-N-mvmva latency, per-cycle `lui $at, 0xFFFF` re-materialization,
 * addu $v0,$a2 pass-through-at-end. Canonical body. */
INCLUDE_ASM("asm/funcs", MulMatrix0);
