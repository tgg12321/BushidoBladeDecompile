/* REJECTED — decomp-permuter seed-1 find output-1290-4.  It is the ONLY form
 * either s4 campaign produced that scored below the honest floor
 * (sandbox 17 at 49 insns vs the floor's 18 at 51) and it is SEMANTICALLY
 * WRONG: the permuter hoisted the third block's `D_80106A73 = val2;` store
 * INSIDE the `if (!c)` arm, so when bit 4 of p[8] is set the flag byte is
 * never written at all, and when it is clear the byte is written with
 * `val | 4` instead of `val`.  Both behaviours contradict the target, which
 * stores unconditionally on the join.  Banked so no later session re-finds
 * this 17 and mistakes it for a floor drop.  It also carries two coercion
 * constructs (`long new_var = 4;` constant holder, `s32 *new_var3 = p;`
 * pointer alias) that would need the named-local / pointer-alias FAKE
 * carve-outs even if the semantics were right.
 *
 * LESSON: the sandbox scores BYTES, not behaviour.  Every permuter find must
 * be read for semantics before its score is credited. */
void func_80034F88(void)
{
  s32 *p;
  u8 *ptr;
  u8 val2;
  s32 c;
  s32 *new_var3;
  long new_var;
  s32 val;
  s32 *new_var4;
  s32 i;
  int new_var2;
  p = func_80077D00();
  new_var3 = p;
  ptr = &D_80106A73;
  *ptr &= 0xF8;
  c = new_var3[8] & 1;
  val = D_80106A73;
  val2 = val | 1;
  if (!c)
  {
    val2 = val;
  }
  new_var = 4;
  D_80106A73 = val2;
  c = new_var3[8] & 2;
  val = D_80106A73;
  val2 = val | 2;
  new_var4 = &new_var3[8];
  if (!c)
  {
    val2 = val;
  }
  D_80106A73 = val2;
  new_var2 = (*new_var4) & new_var;
  c = new_var2;
  val = D_80106A73;
  val2 = val | 4;
  if (!c)
  {
    D_80106A73 = val2;
    val2 = val;
  }
  for (i = 0; i < 3; i++)
  {
    *((&D_80106A70) + i) = *((((u8 *) new_var3) + i) + 0x17);
  }

}
