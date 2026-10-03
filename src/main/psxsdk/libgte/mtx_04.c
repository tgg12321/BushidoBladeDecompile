/* PsyQ 4.0 LIBGTE MTX_04: MulMatrix2. .text 0x8007EC5C..0x8007ED6C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* calc_fc_frame_8007EC5C: hand-coded GTE 3x3-mvmva matrix transform.
 * COMPLETED-INLINE-ASM-CANONICAL -- see
 * inline_asm_canonical.txt for justification. Disassembler annotates
 * every cop2 op as a handwritten instruction; final swc2 $11 uses a
 * hardcoded source reg; mvmva/mfc2/mtc2/nop pipeline is hand-scheduled
 * (cycle N+1 setup interleaves with cycle N latency); per-cycle lui $at
 * re-materialization is a hand-coded choice. No pure-C form reaches
 * these bytes. */
INCLUDE_ASM("asm/funcs", MulMatrix2);
