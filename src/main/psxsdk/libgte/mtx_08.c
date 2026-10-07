/* PsyQ 4.0 LIBGTE MTX_08: ScaleMatrix. .text 0x8007EDBC..0x8007EEEC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007EDBC: hand-coded asm in the original PSY-Q source (display.c packed
 * fixed-point multiply -- 3x3-matrix column scale: 9 packed s16 values each
 * x coef[k%3] from arg1, >>12 Q12, repacked in place).
 * Hand-coded, NOT compiled C:
 *  - Leading `andi $t1,$t0,0xFFFF` before `sll 16; sra 16` is a REDUNDANT mask
 *    (the sll discards exactly the masked bits). GCC combine (combine.c:1458
 *    added_sets_2) elides a single-use redundant mask and keeps the 2nd-use
 *    insn for a multi-use one, so no pure-C form emits this andi without a
 *    stray instruction the target lacks (verified ~38 C forms).
 *  - cc1psx (Sony GCC 2.7.2.SN.1) is byte-identical to our fork here (both
 *    elide it) -- so the original was not compiled from C.
 *  - Cluster: sibling func_8007E8DC (jaccard=0.58) is inline asm; the 8007Exxx
 *    /8007Fxxx display.c region is documented hand-coded asm.
 * Owner-authorized on that evidence (cc1psx proof + combine.c + cluster).
 */
INCLUDE_ASM("asm/funcs", ScaleMatrix);
