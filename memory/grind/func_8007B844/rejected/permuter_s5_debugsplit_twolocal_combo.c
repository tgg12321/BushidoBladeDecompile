/* REJECTED s5 (permuter, 2026-07-21): debug-split + two-local tail COMBINED —
 * NEUTRAL at 6.
 *
 * Campaign F (directed debug-arm x tail cross-product) find output-135-1:
 * the joint selection of two individually-neutral s3 levers (u8 lvl debug
 * split; two-local AND-into-mask tail). s3 measured each alone at 6; the
 * COMBINATION was never sandbox-measured until now. Score 6, build_insns 38 —
 * the levers do not interact; both lower to identical RTL, jointly as well
 * as separately. Closes the last unmeasured cell of the neutral-lever
 * cross-product space (s4's campaign C covered dispatch x AND-order x exit).
 */
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    u32 addr;
    {
        u8 lvl;
        lvl = g_gpu_debug_level;
        if (lvl >= 2) {
            g_gpu_debug_func(&D_80015F98, ot, n);
        }
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    mask = 0xFFFFFF;
    addr = (u32)&g_gpu_ot_end;
    mask = addr & mask;
    *ot = mask;
    return ot;
}
