/* REJECTED s2 2026-07-28 — sandbox score 21 (baseline 15), +4 weighted.
 * Narrow-int retyping var_t1:s32->s16, var_t2:s32->u8 (register-alloc-pure-c Lever B).
 * Effect: build_insns rose from 34 to 38 — GCC inserted sign/zero-extension pack
 * instructions for narrower modes at load/compare sites; net loss.
 * KILLED as an RA lever for this function. */
s32 func_80037B00(u8 *arg0) {
    s16 var_t1;
    s32 var_t3;
    u8 var_t2;
    s8 *var_a3, *var_a1, *var_a2, *var_t0;
    s32 var_v1, var_v0;
    /* ... rest of body identical to candidate.c ... */
}
