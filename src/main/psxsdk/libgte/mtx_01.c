/* PsyQ 4.0 LIBGTE MTX_01: ApplyRotMatrixLV. .text 0x8007EA0C..0x8007EB4C, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* ApplyRotMatrixLV: splits the input vector into hi/lo halves, runs MVMVA on
 * each, scales hi by <<3 (sign preserved), sums and stores. */
INCLUDE_ASM("asm/funcs", ApplyRotMatrixLV);
