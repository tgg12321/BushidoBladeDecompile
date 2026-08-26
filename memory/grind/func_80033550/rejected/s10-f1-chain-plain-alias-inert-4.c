/* s10 2026-08-25 — F1 chain-extender, mode A (plain pointer alias 's32 *p = arg0;').
 * INERT: cse/jump deletes the copy before flow.c's ref count, so pseudo 72's
 * model is byte-identical to baseline (nrefs 5, livelen 16, pri 6250, hard
 * conflicts {2,3,4,29}, hardreg $a1). sandbox --disable all = 4, 34/34.
 * The chain-extender's named mechanism (extra reg_n_refs recorded by flow.c
 * before the combine fold) is never engaged by this spelling. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  s32 *p = arg0;
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
  do { idx = new_var; w0 = p[0]; } while (0);
  w1 = p[1];
  w2 = p[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
