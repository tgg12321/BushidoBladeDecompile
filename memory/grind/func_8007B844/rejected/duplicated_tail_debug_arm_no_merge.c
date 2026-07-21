/* REJECTED s2 (2026-07-21) — duplicated-statement-into-arms probe (the
 * 2026-07-01-sanctioned ref-lift technique, .claude/rules/
 * duplicated-statement-into-arms.md): duplicate dispatch+tail+return into
 * the debug arm so RA sees doubled reg_n_refs on mask/ot before jump2
 * cross-jump re-merges.
 * Score 22, build_insns 44 (target 38): cross-jump did NOT re-merge the
 * duplicated suffix (+6 residual insns) — the form is NOT byte-neutral, so
 * it both regresses massively and fails the sanction's byte-neutrality
 * prerequisite. KILLED. This was the last untried catalog technique
 * applicable to this function's single-flow structure.
 */

u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
        {
            u32 *v0 = g_gpu_dev_table;
            ((void (*)(u32 *, s32))v0[11])(ot, n);
        }
        mask = 0xFFFFFF;
        mask = ((u32)&g_gpu_ot_end) & mask;
        *ot = mask;
        return ot;
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    mask = 0xFFFFFF;
    mask = ((u32)&g_gpu_ot_end) & mask;
    *ot = mask;
    return ot;
}
