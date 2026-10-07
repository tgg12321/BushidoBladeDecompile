/* PsyQ 4.0 LIBGTE FGO_05: RotMatrixY. .text 0x8007FA1C..0x8007FBBC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Hand-written asm in the PsyQ source: sibling of RotMatrixX with mirrored
 * sin negation and offsets 0..0x10 (vs 6..0x10). */
INCLUDE_ASM("asm/funcs", RotMatrixY);
