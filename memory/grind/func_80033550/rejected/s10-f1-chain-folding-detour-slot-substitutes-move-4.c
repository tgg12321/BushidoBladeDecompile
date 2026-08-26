/* s10 2026-08-25 — F1 chain-extender, mode B (folding detour 's32 *p = arg0 + 1;'
 * feeding all three loads through p[-1]/p[0]/p[1]).
 * BYTE-NEUTRAL IN COUNT BUT NOT IN BYTES: combine rebases all three lw offsets
 * and the detour REPLACES the entry copy — build emits 'addiu a1,a0,4' where
 * target has 'addu a3,a0,zero'. Model moves (pseudo 79 livelen 16->17, pri
 * 6250->5882) but nrefs stays 5 and hard conflicts stay {2,3,4,29}, so
 * find_reg's ascending scan still returns $a1. sandbox = 4, 34/34. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  s32 *p = arg0 + 1;
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
  do { idx = new_var; w0 = p[-1]; } while (0);
  w1 = p[0];
  w2 = p[1];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
