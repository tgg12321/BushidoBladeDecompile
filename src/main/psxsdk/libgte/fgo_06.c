/* PsyQ 4.0 LIBGTE FGO_06: RotMatrixZ. .text 0x8007FBBC..0x8007FD5C, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007FBBC: hand-coded asm in original PSY-Q source.
 * Cluster sibling of func_8007F87C (jaccard=1.00 â€” structurally
 * identical, just different stride offsets 0..0xA). All 5 strong
 * signals confirmed by scan_hand_coded: uniform 2-cycle multu pacing,
 * empty-body INT_MIN-guard branch, 0 spills in 102 insns, 6-load burst
 * at insn 25, cluster sibling of two already-authorized functions.
 * Same cluster authorization. */
INCLUDE_ASM("asm/funcs", RotMatrixZ);
