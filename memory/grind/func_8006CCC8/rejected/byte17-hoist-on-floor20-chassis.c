/* REJECTED s6 — re-hoisting `byte17` (single read of *(rec+0x17) before the
 * i==0/else branch, matching target's own unconditional-both-loads-then-select
 * asm shape) on top of the floor-20 chassis (post array-index D_800A34FC
 * addressing fix + for-loop update-clause reorder) REGRESSED the honest floor
 * from 20 to 36 (build_insns 188 -> 184).
 *
 * This is the SAME construct s3 originally introduced (structurally matched
 * target's j-loop insn-for-insn at the time, floor unchanged 77->77) and s5's
 * permuter later dropped in favor of the split-read-into-arms form (39->23).
 * Re-tried here because the floor-20 chassis is otherwise much closer to
 * target than the s3/s5 chassis was, so it seemed worth re-testing whether
 * the earlier trade-off still held. It does: the split-read form (each arm
 * re-reads *(rec+0x17) directly, current candidate.c) remains strictly
 * better on every chassis measured so far in this ledger.
 *
 * Applied to BOTH the field28==3 (+0x1A) and field28==4 (+0x1D) inner
 * for(j...) loops (only the byte17 hoist shown; everything else identical to
 * candidate.c's floor-18 body):
 *
 *     for (j = 0; j < 3; j++) {
 *         rec = (u8 *)D_800A3524 + j;
 *         masked = *(rec + 0x1A) & (nib << fade);
 *         byte17 = *(rec + 0x17);
 *         if (i == 0) {
 *             *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
 *         } else {
 *             *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
 *         }
 *     }
 *
 * measured_on: src/text1b.c HEAD s6, floor-20 chassis (array-index address
 * fix applied, for-loop update clause NOT yet reordered); sandbox --disable
 * all: score 36, build_insns 184 (down from 188, i.e. cc1 DOES fold the two
 * loads into one — matching target's insn count more closely — but the
 * resulting register allocation is measurably WORSE by the levenshtein
 * metric than the split-read form's 188-insn, non-folded shape). Do not
 * re-propose without a new mechanism-level reason to expect a different
 * result on a further-advanced chassis.
 */
