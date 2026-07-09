/* s12 P1 — pointer-walker arg5_addr form (s32 *ap staging).
 * Applied to h5 candidate: replaced `arg5 = *(s32*)(v0 + (s32)tbl_125c);`
 * with a pointer-typed two-SET walker:
 *   s32 *ap = (s32 *)(s32)tbl_125c;
 *   ap = (s32 *)((s32)ap + v0);
 *   arg5 = *ap;
 * Sandbox --disable all: masked=2 INERT vs h5 baseline (target_insns=160,
 * build_insns=160, bytes identical).
 * Verdict: KILLED. RTL is type-agnostic for the address arithmetic;
 * combine.c substitutes (plus symref reg) through addsi3_internal on the
 * pointer-typed intermediate exactly as it did on the s32 intermediate
 * (s11 M1-tertiary), collapsing to the single-SET p107 h5 baseline RTL.
 * Pointer-typing does NOT reach flow-time with reg_n_sets(p107)=2. */

    v0 <<= 2;
    {
      s32 *ap = (s32 *)(s32)tbl_125c;
      ap = (s32 *)((s32)ap + v0);
      arg5 = *ap;
    }
