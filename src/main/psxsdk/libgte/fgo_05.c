/* PsyQ 4.0 LIBGTE FGO_05: RotMatrixY. .text 0x8007FA1C..0x8007FBBC, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007FA1C: hand-coded asm in original PSY-Q source.
 * Sibling of func_8007F87C with mirrored sin negation and offsets
 * shifted to 0..0x10 (vs 6..0x10 for func_8007F87C). All 5 strong
 * signals confirmed (uniform multu pacing, front-loaded loads, tight
 * register packing, INT_MIN-guard idiom at .L8007FA38, cluster). Same
 * cluster authorization (commit 39e9bf0). */
INCLUDE_ASM("asm/funcs", RotMatrixY);
