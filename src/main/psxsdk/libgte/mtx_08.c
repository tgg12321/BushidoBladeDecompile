/* PsyQ 4.0 LIBGTE MTX_08: ScaleMatrix. .text 0x8007EDBC..0x8007EEEC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* ScaleMatrix: scales the columns of a 3x3 matrix in place (9 packed s16
 * values, each x coef[k%3] from arg1, >>12 Q12, repacked). Hand-written asm in
 * the original: it keeps a redundant `andi 0xFFFF` before `sll 16; sra 16`
 * that no C form makes GCC (ours or cc1psx) emit without an extra instruction
 * the target lacks. */
INCLUDE_ASM("asm/funcs", ScaleMatrix);
