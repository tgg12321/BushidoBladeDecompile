/* REJECTED (s3, 2026-09-10) - measured 99 -> 99 on the floor-99 chassis, three spellings.
 * HYPOTHESIS (s2's F2): target's two differently-signed reads of %gp_rel(D_800A3558) - `lhu $a2`
 * at 80070DF4 / 80070ECC feeding `sll 16 / sra 16 / addu` in the loop body, and `lh $v0` at
 * 80070E08 / 80070ED0 feeding the loop bound - come from the symbol being a u16 OBJECT rather
 * than from casts on an s32/s16 declaration (s2's K1 killed the cast route).
 * PROBE (tmp/grind/func_80070C70/s3/probe2.py):
 *   w1  extern u16 D_800A3558;  bound `(s32)(D_800A35B0 + ((s16)D_800A3558 + 1))`,
 *                               body  `(D_800A35B0 + (s16)D_800A3558)`              -> 99
 *   w2  extern u16 D_800A3558;  bound `(s32)(D_800A35B0 + (D_800A3558 + 1))`,
 *                               body  `(D_800A35B0 + (s16)D_800A3558)`              -> 99
 *   w3  extern s16 D_800A3558;  body `(D_800A35B0 + (s16)(u16)D_800A3558)`          -> 99
 * RESULT: no spelling of a single integer declaration of D_800A3558 splits the read into the
 * target's lhu-plus-sign-extend / lh pair; all three are byte-neutral against the s32 declaration
 * the chassis carries. Together with s2's K1 this closes the "re-type the declaration" avenue for
 * this pair on this chassis. It remains a 2-3 insn shape issue invisible in the score while the
 * callee-saved rotation saturates it.
 */
