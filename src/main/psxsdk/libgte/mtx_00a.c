/* PsyQ 4.0 LIBGTE MTX_00A: ScaleMatrixL. .text 0x8007E8DC..0x8007EA0C, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007E8DC = LIBGTE MTX_00A ScaleMatrixL â€” verbatim-linked Sony PsyQ 4.0
 * object. In-place Q12 fixed-point column scale of a 3x3
 * matrix by 3 scalars (columns 0,1,2 x scalars *arg1[0/1/2]). Splat tags the
 * body handwritten; hardcoded $t0..$t5 packed register cadence + hand-scheduled
 * multu/mflo pairing + sw in jr delay slot are hand-coded signatures. Sibling
 * of ScaleMatrix (func_8007EDBC, canonical body per packed-multiply-cluster).
 * Canonical body. */
INCLUDE_ASM("asm/funcs", ScaleMatrixL);
