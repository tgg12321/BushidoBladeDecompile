/* REJECTED s3 (structural, 2026-07-21): sandbox 20, build_insns 40 (target 38).
 *
 * Hoisting `mask = 0xFFFFFF;` between the debug arm and the dispatch call
 * makes mask live across the vtable call, so cc1 allocates it to a callee-save
 * register — prologue/epilogue inflate by +2 insns (40 vs 38). Can never match
 * by construction. Confirms the evidence-bank round-1 finding (hoist before
 * BOTH calls = 19) extends to the dispatch-only hoist: the 0xFFFFFF
 * materialization must happen strictly AFTER the last call.
 */
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    mask = 0xFFFFFF;
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    mask = ((u32)&g_gpu_ot_end) & mask;
    *ot = mask;
    return ot;
}
