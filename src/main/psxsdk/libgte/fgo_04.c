/* PsyQ 4.0 LIBGTE FGO_04: RotMatrixX. .text 0x8007F87C..0x8007FA1C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007F87C: hand-coded asm in original PSY-Q source.
 * Evidence for the hand-coded classification:
 *   - Uniform 2-cycle multu/mflo pacing on EVERY mult/mflo pair
 *   - Front-loaded loads (6 args loaded interleaved with first 2 multus)
 *   - Tight register packing across 75-instruction kernel, no spills
 *   - INT_MIN-guard idiom: empty `if (a<0){}` body at .L8007F898
 *   - Cluster behavior: motutil_GetWalkDir, func_8007F5EC, func_8007FA1C,
 *     func_8007FBBC share the same skeletal shape.
 * Owner-authorized for this cluster. */
INCLUDE_ASM("asm/funcs", RotMatrixX);
