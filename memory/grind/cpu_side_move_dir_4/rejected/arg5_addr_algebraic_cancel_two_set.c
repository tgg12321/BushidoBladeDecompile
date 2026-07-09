/* s11 M1-primary: arg5_addr = v0 + t0; arg5_addr += (s32)tbl_125c - t0;
 * masked=11 (+9 vs h5 baseline 2), target_insns=160, build_insns=160.
 * cse.simplify_plus_minus or combine either folded the algebraic cancellation
 * OR the two-SET shift on p107 disrupted the h5 basin materially - the +9
 * regression indicates the arg5-chain RTL was re-canonicalized in a way that
 * broke h5's t0 multi-set launch-suppression alignment.
 * Frontier M1-primary KILLED.
 */
{
  s32 arg5;
  s32 t0;
  void **pp;
  t0 = idx_1494[0];
  v0 = idx_1494[1];
  pp = (void **)&D_800F19C0;
  t0 *= 4;
  t0 = (s32)((u8 *)tbl_125c + t0);
  v0 <<= 2;
  {
    s32 arg5_addr;
    arg5_addr = v0 + t0;
    arg5_addr += (s32)tbl_125c - t0;
    arg5 = *(s32 *)arg5_addr;
  }
  debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
}
