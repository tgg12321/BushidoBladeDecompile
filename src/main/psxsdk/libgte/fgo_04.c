/* PsyQ 4.0 LIBGTE FGO_04: RotMatrixX. .text 0x8007F87C..0x8007FA1C, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Hand-written asm in the PsyQ source: uniform multu/mflo pacing, no spills,
 * INT_MIN-guard idiom; same shape as the other FGO rotation modules. */
INCLUDE_ASM("asm/funcs", RotMatrixX);
