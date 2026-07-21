/* s3 KILLED family — const-address pointer pseudos (t0/t1/t2 = &tables)
 * never reach RA: byte-identical to base (score 4) with .greg showing only
 * TWO allocnos (ptr, i). cse folds the uses back to symbolic addresses and
 * flow deletes the dead sets PRE-RA — so REG_EQUIV-style pseudos contribute
 * ZERO conflicts. Also measured with the inits inside a do-while(0) fake
 * loop (Z4): same result — loop notes do not keep them alive either.
 * This kills the structural spelling of the "REG_EQUIV-folded pseudo still
 * shaped the conflict scan" frontier hypothesis. Representative body (Z2). */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  u8 *t0;
  u8 *t1;
  u8 *t2;
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
  t0 = (u8 *) &D_80107850;
  t1 = (u8 *) &D_80107854;
  t2 = (u8 *) &D_80107858;
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (t0 + idx)) = w0;
  *((s32 *) (t1 + idx)) = w1;
  *((s32 *) (t2 + idx)) = w2;
}
