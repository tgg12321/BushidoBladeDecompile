/* REJECTED s5 (permuter, 2026-07-21): named fn-ptr dispatch temp — NEUTRAL at 6.
 *
 * Campaign D (debug-split + goto-end chassis) find output-135-1 at 15s
 * post-launch: temp-for-expr on the dispatch call. Sandbox-measured in
 * src/display.c: score 6, build_insns 38 — byte-identical to the candidate.
 * The fn-ptr temp is copy-prop/fold-transparent like every pointer/scalar
 * rebind in the rejected-forms family; the call's RTL (lw 0x2C; jalr) is
 * unchanged, so the post-call pseudo landscape driving the sched.c
 * return-staging priority is untouched. Not a cheat (ordinary temp), just
 * dead as a lever.
 */
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        void (*fn)(u32 *, s32);
        fn = (void (*)(u32 *, s32))v0[11];
        fn(ot, n);
    }
    mask = 0xFFFFFF;
    mask = ((u32)&g_gpu_ot_end) & mask;
    *ot = mask;
    return ot;
}
