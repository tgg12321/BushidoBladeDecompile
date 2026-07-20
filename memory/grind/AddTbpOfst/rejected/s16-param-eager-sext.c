/* REJECTED (s1, measured 7 both spellings):
 *   (a) s32 AddTbpOfst(s16 a0, s16 a1) with `(a0 & 0xFFFF)` guard
 *   (b) u16 a0 with `((s16)a0 & 0xFFFF)` guard
 * Both DO produce the wanted `andi $2,$4,0xffff` (combine folds
 * sext-shifts + and into and(SI-copy)), BUT cse shares the sign-extend
 * between the guard (block 1) and idx (block 2), hoisting sll/sra ABOVE the
 * andi/branch. Target has andi first, sign-extend after beqz. Also slti
 * instead of sltiu. The sext placement and the andi operand fight each other
 * through cse's extended-basic-block sharing.
 */
s32 AddTbpOfst(s16 a0, s16 a1);
