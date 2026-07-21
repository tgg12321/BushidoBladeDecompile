/* s3 KILLED family — DImode (s64/u64) temp as a single paired-reg a1+a2
 * occupant. Both s64 t = i (score 22) and u64 t = (u32)i (score 22, the
 * cheapest possible: zero high half) emit unavoidable pair-half insns.
 * .greg PROVES the mechanism would work if byte-free: the DImode pseudo's
 * conflict set is exactly {v0,v1,a0}+ptr, so it would pair-home a1+a2 and
 * push ptr to a3 — but GCC 2.7.2 always emits both half-sets; no C
 * spelling makes a DImode pseudo byte-free. Representative body (Z1). */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s64 t;
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
  t = i;
  new_var = (s32) t * 12;
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
