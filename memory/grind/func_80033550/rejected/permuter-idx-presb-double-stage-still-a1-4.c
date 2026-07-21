/* REJECTED s4 (permuter): best permuter find across 4 campaigns (~104k iters).
 * Chassis-3 (v07-flip-neighborhood, base 50) descended 50->40->30->20 in
 * <5 min to THIS form: idx staged new_var2->new_var and computed BEFORE the
 * sb, w2 double-staged through new_var. Permuter score 20 = equal to the
 * candidate floor. Honest measurement: sandbox 4 with the IDENTICAL residual
 * (build: move a1,a0 + three lw via a1; target: addu a3,a0,zero + lw via a3)
 * — same ptr=a1 attractor, NOT a new outcome. The staging is also
 * reviewer-hostile (odd-looking chained staging with no semantic purpose).
 * Every basin (candidate-random, directed-staged, v07-neighborhood, and this
 * form's own neighborhood at 30.9k iters) converges to the score-20 ptr=a1
 * attractor class (alias handles / split multiplies / staging shuffles);
 * nothing sub-20 exists in any searched neighborhood. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 new_var2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  new_var2 = i * 12;
  *(&D_800A3918 + i) = 1;
  new_var = new_var2;
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  new_var = arg0[2];
  w2 = new_var;
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
