/* s41 permuter find (chassis: count-inlined, mode: PERM_LINESWAP exhaustive 5!=120).
 * Decl order: v1, sum, i, s4, s5.
 * Score=60 (2-insn residual, same basin as baseline).
 * All 120 permutations converged to score 60; no sub-60 basin escape.
 * Rejection reason: equivalent-basin free-axis member (s1/s3/s11/s29/s30
 * free-axis cluster, extended to count-inlined chassis). Adds 11th
 * chassis/mode combo to the empirical closure count; cumulative permuter
 * budget now 137,872 iters across 11 combos, all converging on score 60
 * (except the s31 semantic-break score=10 outlier).
 */
void func_80045294(s32 a0, s32 a1)
{
  s32 v1 = a0 << 4;
  s32 sum = 0;
  s32 i = a0;
  s32 s4 = *((s32 *) (((u8 *) (&D_800EED14)) + v1));
  s32 s5 = s4 + a1;

  if (i < D_800A33AC) {
    do {
      s32 val = *((s32 *) (((u8 *) (&D_800EED18)) + v1));
      v1 += 0x10;
      i += 1;
      sum += val;
    } while (i < D_800A33AC);
  }
  /* ... tail unchanged ... */
}
