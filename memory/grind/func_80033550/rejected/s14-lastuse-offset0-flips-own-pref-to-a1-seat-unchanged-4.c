/* s14 REJECTED: source order permuted so the offset-0 dereference (w0) is the
   pointer's LAST use.  MEASURED (tmp/grind/func_80033550/s14/findreg_stderr.txt,
   BB2_FINDREG_DEBUG=72): this flips pseudo 72's own_full_prefs from {} to {5} —
   the first direct proof that global.c's set_preference fires for this function
   (the offset-0 load's address is a bare REG, so set_preference names the
   local-alloc'd seat of that load's destination).  The preference names $a1,
   which is the seat find_reg's scan already produced, so the pointer does not
   move: sandbox score 4, 34 insns, 0 rules — score-neutral, RA-neutral.
   Kept as the positive control for the preference mechanism. */
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
  do { idx = new_var; w1 = arg0[1]; } while (0);
  w2 = arg0[2];
  w0 = arg0[0];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}