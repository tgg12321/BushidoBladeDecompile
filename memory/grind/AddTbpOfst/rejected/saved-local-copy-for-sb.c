/* REJECTED (s1): trying to make the u16 param var single-use so combine can
 * fold the entry copy into the guard zero_extend (-> andi from $4):
 *   (a) `u16 saved = a0; D_801027F1 = (u8)saved;`  -> measured 3, NO change:
 *       cse propagates saved==var and the sb still reads pseudo 72.
 *   (b) `u8 saved = a0; D_801027F1 = saved;`      -> measured 5, 50 insns:
 *       QI truncation materializes an EXTRA move ($2=$4; $7=$2; andi $2,$2).
 * Mechanism note: combine cannot fold insn6 (72=subreg(73)) into insn16
 * (zero_extend(72)) while the sb keeps 72 multi-use; added_sets_2 PARALLEL
 * fails recog. Any successful spelling must give the sb a value cse folds to
 * pseudo 73 WITHOUT an extra insn.
 */
s32 AddTbpOfst(u16 a0, s16 a1);
