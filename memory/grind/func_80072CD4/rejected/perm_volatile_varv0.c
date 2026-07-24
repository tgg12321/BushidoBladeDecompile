/* REJECTED (s4 permuter, cross-block chassis best = permuter-score 308, NOT a match).
 * The permuter coerced the cross-block temp to `volatile char var_v0` to fight the
 * sched1 hoist that forces var_v0 -> $v1. This is a volatile-coercion cheat on a
 * game-state local (expanded cheat catalog, inline-asm-policy.md: "inline volatile
 * casts / declarations to force scheduling/CSE coercion" — the local was never
 * volatile in the original; the ONLY purpose is to change GCC's scheduler view).
 * AND it does not reach a byte match (score 308: the RA rotation fc->$a0,
 * var_v0->$v1 largely persists). Dead on both counts: cheat + non-match.
 *
 * Mechanism: volatile forces every var_v0 access to be an ordered memory op,
 * disturbing sched1's bottom-up placement of the lone `li var_v0`. It does not
 * cleanly reproduce target's "li in the arm delay slot" pin and is a forbidden
 * coercion regardless.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1)
{
  int fc_const;
  volatile char var_v0;            /* CHEAT: volatile coercion, forbidden */
  initPolyG4(arg1);
  gpu_SetSemiTransp(arg1, 0);
  if (arg0 < 4)
  {
    fc_const = 0xFC;
    if ((*((s32 *) (((s32) D_800A35C4) + 8))) & 4) { /* ...C3/1E/C8... */ var_v0 = 0x32; }
    else                                            { /* ...C3/50/DC... */ var_v0 = 0x46; }
    *((u8 *) (((s32) arg1) + 4)) = fc_const;
    *((u8 *) (((s32) arg1) + 0xC)) = fc_const;
    *((u8 *) (((s32) arg1) + 0xE)) = var_v0;
    /* ...rest of merge unchanged... */
  }
  ot_Link(D_800A374C + 0x60, arg1);
  return (s32) (((u8 *) arg1) + 0x24);
}
