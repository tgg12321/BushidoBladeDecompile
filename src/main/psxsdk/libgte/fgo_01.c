/* PsyQ 4.0 LIBGTE FGO_01: RotMatrix. .text 0x8007F35C..0x8007F5EC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Hand-written asm in the PsyQ source: 3-axis Euler rotation, zero spills,
 * three INT_MIN-guard idioms in succession. */
INCLUDE_ASM("asm/funcs", RotMatrix);
