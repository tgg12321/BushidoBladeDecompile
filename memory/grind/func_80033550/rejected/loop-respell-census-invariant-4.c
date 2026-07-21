/* s3 KILLED family — loop-region respellings are RA-census-invariant.
 * Six spellings measured (for / do-while / while / goto+flag-temp s32 /
 * goto+flag-temp u8 / per-iteration address-temp p = &D_800A3918 + i):
 * ALL score 4 with .greg conflict set literally identical to base
 * (`72 conflicts: {i,v0,v1,a0}`, 2 allocnos). cse/flow canonicalize every
 * loop spelling to the same RTL before RA — the loop region cannot supply
 * the a1/a2 occupants needed to push ptr to a3.
 * Representative body (L2_dowhile); full set in tmp/grind/func_80033550/s3/. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  do {
    if ((*(&D_800A3918 + i)) == 0) goto found;
    i++;
  } while (i < 6);
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
