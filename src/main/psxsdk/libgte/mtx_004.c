/* PsyQ 4.0 LIBGTE MTX_004: ApplyMatrixLV. .text 0x8007E74C..0x8007E8AC, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Local-vector transform: split each component (hi = x >> 15, lo = x & 0x7FFF),
 * two mvmva passes (hi, then lo), scale the hi result by 8 (<<3), sum, store.
 * Hand-written assembly. */
INCLUDE_ASM("asm/funcs", ApplyMatrixLV);
