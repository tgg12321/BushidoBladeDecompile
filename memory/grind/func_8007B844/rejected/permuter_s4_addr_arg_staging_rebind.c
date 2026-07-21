/* s4 permuter find (chassis B "twolocal-random", output-125-1, ~813s after
 * fresh seed, 42k iters total) — REJECTED: sandbox-NEUTRAL (6, build_insns 38).
 *
 * Two mutations layered on the s3-neutral two-local AND-into-mask form:
 *   1. `addr = n;` staged into the debug-call third arg (variable-reuse
 *      family, SOTN-sanctioned as a construct — but measured no-op here).
 *   2. Tail rebind `addr = mask; *ot = addr;` — scalar rebind; copy-prop
 *      folds it exactly like every pointer-rebind form in the bank.
 *
 * Permuter weighted score improved 135 -> 125 (register-weighted raw diffs),
 * but the engine's masked honest distance stayed 6 — the two metrics diverge
 * on this function because the residual is a coupled register-rotation +
 * schedule cascade the mask already normalizes per-insn.
 *
 * Confirms: pre-call pseudo staging (s3) and scalar/pointer rebinding
 * (rounds 1-3, s3) remain fold-transparent even in combination.
 */
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    u32 addr;
    if (g_gpu_debug_level >= 2) {
        addr = n;
        g_gpu_debug_func(&D_80015F98, ot, addr);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    mask = 0xFFFFFF;
    addr = (u32)&g_gpu_ot_end;
    mask = addr & mask;
    addr = mask;
    *ot = addr;
    return ot;
}
