/* PsyQ 4.0 LIBGTE MTX_01: ApplyRotMatrixLV. .text 0x8007EA0C..0x8007EB4C, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007EA0C = LIBGTE MTX_01 ApplyRotMatrixLV - verbatim-linked Sony PsyQ
 * 4.0 object. Sibling of ApplyMatrixLV (func_8007E74C):
 * sign-splits input vec into hi/lo halves (arithmetic split), runs mvmva
 * twice (hi 0,0,3,3,0 then lo 1,0,3,3,0), post-scales hi by <<3 with signed
 * preservation, sums and stores. No pure-C form reaches these bytes without
 * register pins or asm rewriting. Canonical body. */
INCLUDE_ASM("asm/funcs", ApplyRotMatrixLV);
