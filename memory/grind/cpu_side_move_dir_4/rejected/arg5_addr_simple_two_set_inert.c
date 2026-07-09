/* s11 M1-tertiary (isolation probe): arg5_addr = (s32)tbl_125c; arg5_addr += v0;
 * masked=2 INERT (baseline 2), target_insns=160, build_insns=160.
 * MECHANISM ISOLATION: bytes IDENTICAL to h5 baseline. combine.c substitutes
 * the two SETs into a single (set p107 (plus tbl v0)) via addsi3_internal
 * (which recognizes (plus symref reg) - confirmed s7:213). p107 stays
 * single-SET at flow-time; insn 121 LAUNCHES; same pair-swap as h5.
 * This confirms combine folds ANY two-SET C form on arg5_addr that lacks
 * a non-trivially-cancellable operand. Combined with M1-primary (+9) and
 * M1-secondary (+27) results: the frontier's mechanism realization
 * (flow-time reg_n_sets(p107)=2) is not reachable by C-source two-SET
 * variants - combine either folds the sets (INERT) or the fold path
 * disrupts the h5 basin allocation web (regression). Frontier #1 KILLED
 * across all three realizations.
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
    arg5_addr = (s32)tbl_125c;
    arg5_addr += v0;
    arg5 = *(s32 *)arg5_addr;
  }
  debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
}
