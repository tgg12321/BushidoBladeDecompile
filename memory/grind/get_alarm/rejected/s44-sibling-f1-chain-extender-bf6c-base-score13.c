/* REJECTED (s44, synthesis / SIBLING SWEEP). Axis A.
 * Transplant of CD_sync's sanctioned F1 "combine-foldable chain-extender"
 * (memory/grind/CD_sync/candidate.c idx_1495 line; owner ruling 2026-07-01)
 * onto get_alarm's &D_8009BF68 address, which is the one thing axis A wants:
 * a SECOND use of the address pseudo so combine's added_sets_2
 * (combine.c:1458) stops folding lui;addiu;lw 0(reg) into lui;lw %lo.
 *
 *   s32 *bf68 = (s32 *)((u8 *)&D_8009BF6C + ((s32)&D_8009BF68 - (s32)&D_8009BF6C));
 *   printf(&D_80016044, bf68[0], D_8009BF6C, D_8009BF70);
 *
 * Measured on the chassis-current floor-9 body: score 13, build_insns 90.
 * build_insns NEVER leaves 90 -- the BF68 slot still folds, so the extender's
 * second symbol reference does not survive as a second use of the ADDRESS
 * PSEUDO by the time combine runs; it only perturbs codegen elsewhere (+4).
 * KILLS the F1-family spelling of axis A on this chassis. */
