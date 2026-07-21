/* REJECTED (measured 5, s1): idx = i*3 with s32-array indexing
 * ((&D_80107850)[idx]) so the <<2 belongs to the store addresses.
 * combine canonicalizes to the identical sll/addu/sll chain — output
 * byte-identical to clean-no-wrap form. Spelling axis is dead here. */
void func_80033550(s32 *arg0)
{
  s32 i; s32 w0; s32 w1; s32 w2; s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  idx = i * 3;
  w0 = arg0[0];
  w1 = arg0[1];
  w2 = arg0[2];
  (&D_80107850)[idx] = w0;
  (&D_80107854)[idx] = w1;
  (&D_80107858)[idx] = w2;
}
