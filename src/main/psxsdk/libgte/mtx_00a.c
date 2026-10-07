/* PsyQ 4.0 LIBGTE MTX_00A: ScaleMatrixL. .text 0x8007E8DC..0x8007EA0C, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Original LIBGTE assembly. In-place Q12 fixed-point scale of a 3x3 matrix's
 * columns 0,1,2 by the scalars arg1[0/1/2]; canonical body, sibling of
 * ScaleMatrix (packed-multiply-cluster). */
INCLUDE_ASM("asm/funcs", ScaleMatrixL);
