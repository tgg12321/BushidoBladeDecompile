/* REJECTED (s5, 2026-08-10): permuter find output-65-1 (weighted 65, best of the
 * s5 c5lim campaign; honest display.c-context linediff 18, roles 5/9 -- BETTER
 * than the floor's 19 -- which is exactly why it must be banked as forbidden).
 * TWO fatal defects:
 * 1. SEMANTIC BUG + UB: the narrow dispatch arm computes `lo = lim & 0xFFF;`
 *    where the spec requires `x & 0xFFF`. lim is only assigned `x` in the WIDE
 *    arm; on the narrow path lim holds a stale limit (or is UNINITIALIZED when
 *    both args are negative -- neither clamp body runs). Classic
 *    ub-dead-read-cross-arm family (see rejected/ub-dead-read-cross-arm-family.c),
 *    now spelled through lim instead of a fresh local.
 * 2. `new_var = hi >= 0;` named-boolean intermediate -- round-2 coercion family.
 * DIAGNOSTIC VALUE (why the score drops): lim staying live ACROSS the dispatch
 * branch supplies the missing conflict that flips carrier->a3 and limsave1->a2.
 * Every legitimate spelling of that intent measured dead again this session:
 * lim=x duplicated into both arms = linediff 21 (4/9, cascade shifts limsaves to
 * v1); unconditional pre-dispatch lim=x = 52 insns. s4's kills (dup pkt=x 13,
 * uncond copy 20/52, split-assign 14) corroborate.
 */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
  int new_var;
  s16 x = arg0;
  s16 tx;
  s32 hi;
  s32 lo;
  s32 pkt;
  s16 lim;
  if (arg0 >= 0)
  {
    lim = D_8009BE78;
    if ((lim - 1) < arg0)
    {
      tx = lim - 1;
    }
    else
    {
      tx = x;
    }
  }
  else
  {
    tx = 0;
  }
  x = tx;
  hi = arg1;
  new_var = hi >= 0;
  if (new_var)
  {
    lim = D_8009BE7A;
    if ((lim - 1) < arg1)
    {
      arg1 = lim - 1;
    }
  }
  else
  {
    arg1 = 0;
  }
  tx = (u32) (D_8009BE74 - 1);
  hi = arg1 & 0xFFF;
  if (tx >= 2U)
  {
    hi = arg1 & 0x3FF;
    hi = hi << 10;
    lim = x;
    lo = lim & 0x3FF;
  }
  else
  {
    hi = hi << 12;
    lo = lim & 0xFFF;
  }
  pkt = lo | 0xE3000000;
  return hi | pkt;
}
