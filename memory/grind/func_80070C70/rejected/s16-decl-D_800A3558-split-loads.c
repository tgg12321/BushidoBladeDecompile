/* REJECTED (s2, 2026-09-10) - instance kill, measured 101 -> 101 on the floor-101 chassis.
 *
 * WHY IT IS DEAD: the target reads %gp_rel(D_800A3558) with two different signednesses in the
 * same region (lhu $a2 at 80070DF4 / 80070ECC feeding sll 16 / sra 16 / addu in the body, and
 * lh $v0 at 80070E08 / 80070ED0 feeding the loop bound). This form tried to reproduce that pair
 * by re-declaring the symbol as s16 and letting the two use sites differ by cast. GCC 2.7.2's
 * convert_to_integer folds (short)(unsigned short)x to (short)x for an s32-declared AND an
 * s16-declared x, so the body still compiled to a single `lh` and nothing moved. No cast spelling
 * on a single integer declaration of that symbol can split the two loads.
 */
extern s16 D_800A3558;          /* was: extern s32 D_800A3558; */

/* loop entry guard and loop bound - intended to keep the signed `lh` read: */
/*   if ((s32)(D_800A35B0 + (D_800A3558 + 1)) > 0) { ... }                   */
/*   } while (var_s0 < (s32)(D_800A35B0 + (D_800A3558 + 1)));                */

/* body - intended to force lhu + sll 16 + sra 16, but folded back to a single lh: */
/*   if (((D_800A35B0 + (s16)(u16)D_800A3558) != 0) || (D_800A35BC == 2)) {  */
