/* s53 REDERIVE: m2c-regvars-derived in-place walk on v0 for arg5 addr.
   Shape (arg5 leg): `v0 <<= 2; v0 += (s32)tbl_125c; arg5 = *(s32*)v0;`
   (t0 leg unchanged from candidate; only arg5's addr computation reuses
   v0 for a THIRD in-place set instead of birthing a fresh p107 temp.)

   Rationale: m2c --reg-vars v0,v1,a0 emits the TARGET asm shape as
   `var_v0 = M2C_FIELD(&D_800A1494, u8*, 1); var_v0 *= 4; var_v0 += &D_800A125C; var_v1 = *var_v0;`
   — the arg5 addr computation SETS v0 in-place. The rederive angle was:
   if I mirror this in C via a triple in-place set on v0, will combine.c
   FAIL to fold the second SET (v0 += tbl) into the first (v0 <<= 2)?
   That would leave insn 121's dest (p_v0 = p101) as multi-set at flow
   time, so birthing_insn_p(121) = FALSE, 121 loses LAUNCH, 111 keeps
   LAUNCH via the mult-expander p106 fresh dest path (s7 confirmed), and
   the pair-order flips to target.

   Result: masked=15, target_insns=160, build_insns=160 (+13 vs h5
   baseline of 2). REGRESSION into an intermediate broken-web basin.
   Confirms s39's earlier finding on the "v0-triple-set-carrier"
   respelling: making v0 multi-set beyond 2 in-place sets spreads the
   s-reg conflict web enough to disrupt the h5 chassis (s0/s2/s4 reg
   allocation for idx_1494/idx_1495/idx_1496 shifts), regressing to a
   novel intermediate basin between h5 (2) and inline-all (14).

   Mechanism KILL: v0 as multi-set carrier for the ADDR (not just the
   value) doesn't reach flow-time as "p107 multi-set" — instead it
   reaches "p101 quadruple-set" (init + shift + add + downstream) which
   breaks the qty-priority arithmetic the h5 chassis depends on. The
   frontier's target (multi-set on the arg5 ADDR pseudo p107 specifically)
   is not spellable via v0 reuse; combine.c's addsi3_internal still
   substitutes the addsi into the shift's producer at that scope.
*/
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    v0 += (s32)tbl_125c;
    arg5 = *(s32 *)v0;
