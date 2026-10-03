/* PsyQ 4.0 LIBGTE MTX_004: ApplyMatrixLV. .text 0x8007E74C..0x8007E8AC, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007E74C = LIBGTE MTX_004 ApplyMatrixLV â€” verbatim-linked Sony PsyQ 4.0
 * object. Local-vector transform with pre-scaling via sign-
 * split (hi=x>>15, lo=x&0x7FFF), two mvmva cycles (hi 0,0,3,3,0 then lo
 * 1,0,3,3,0), post-scale hi result by 8 via signed <<3, sum + store. Hand-
 * coded evidence: the `sra $tN,$tM,15` split idiom and branch forms that
 * compiled C reaches only with register pins or asm rewriting. Canonical
 * body. */
INCLUDE_ASM("asm/funcs", ApplyMatrixLV);
