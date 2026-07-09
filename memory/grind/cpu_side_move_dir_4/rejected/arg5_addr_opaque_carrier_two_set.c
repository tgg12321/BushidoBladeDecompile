/* s11 M1-secondary: t_alias = idx_1494[0] * 4; arg5_addr = v0 + t_alias; arg5_addr += (s32)tbl_125c - t_alias;
 * masked=29 (+27 vs h5 baseline 2), target_insns=160, build_insns=160.
 * Opaque-carrier is worse than M1-primary (+27 vs +9). The duplicated
 * (mult (lbu idx_1494) 4) computation via a fresh pseudo t_alias adds
 * a second lbu-and-shift chain, destroying h5's t0 launch-suppression
 * alignment and further re-canonicalizing the arg5 chain. cse.c did NOT
 * unify t_alias with t0 (the massive regression indicates the extra
 * insns are emitted); the granularity probe answers "cse keeps them
 * distinct" - but the resulting duplicated chain is far outside the
 * h5 basin. Frontier M1-secondary KILLED.
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
    s32 t_alias;
    s32 arg5_addr;
    t_alias = idx_1494[0] * 4;
    arg5_addr = v0 + t_alias;
    arg5_addr += (s32)tbl_125c - t_alias;
    arg5 = *(s32 *)arg5_addr;
  }
  debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
}
