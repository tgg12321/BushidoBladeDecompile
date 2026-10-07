/* PsyQ 4.0 LIBGTE MTX_005: ApplyRotMatrix. .text 0x8007E8AC..0x8007E8DC, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Hand-written GTE mvmva vector-transform wrapper (owner-authorized per
 * canonical-asm-authorization-recipe): lw into $t0/$t1, GTE load-delay nop,
 * $a2 returned at the end, unfilled jr delay slot. */
INCLUDE_ASM("asm/funcs", ApplyRotMatrix);
