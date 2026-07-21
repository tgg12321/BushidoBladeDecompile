/* func_80033550 — best honest form, sessions 1-4 (floor 4, zero cheat-asm).
 * s2 (structural, 24 variants): tail geometry closed. s3 (structural, 14
 * variants): loop-region census-invariance proven (6 spellings, identical
 * .greg), REG_EQUIV const-pointer pseudos deleted pre-RA, DImode pair
 * occupant mechanism confirmed but never byte-free. s4 (permuter, 4
 * campaigns, ~104k iters): whole-function stochastic search measured
 * near-dead — every basin (random, directed-staged, v07-flip-neighborhood,
 * c3-20-neighborhood) converges to the score-20 ptr=a1 attractor; zero
 * sub-20 finds. Remaining axes: cc1 forensics (post-conflict-build
 * deletions), cc1psx calibration check.
 * The do-while(0) wrap is sanctioned per do-while-zero-exception (2026-07-06
 * owner ruling, any codegen effect, single-level, FAKE-annotated).
 * Residual 4 = arg0's pointer pseudo homed in $a1 (build) vs $a3 (target):
 * move + 3 lw base regs. See evidence.md for the RTL conflict analysis. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  /* FAKE: single-level wrap; loop notes fence sched1 so the final sll of
   * idx stays ahead of the three lw's, matching target order. */
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
