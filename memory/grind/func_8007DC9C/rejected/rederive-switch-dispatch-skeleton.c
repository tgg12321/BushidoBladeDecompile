/* REJECTED s36 (rederive) — switch-dispatch skeleton. Score 18 / build_insns 95
 * (baseline score 9 / build 90) — STRICTLY WORSE.
 *
 * Idea: dispatch the report body on the computed condition value via
 *   switch ((vcount < VSync(-1)) || (draw_count++ > 0xF0000)) { case 0: return 0; default: ...; return -1; }
 * instead of the HEAD `if (cond) { ...report...; return -1; } return 0;`.
 *
 * Why it fails: a `switch` on the `||` expression forces GCC to MATERIALIZE the
 * short-circuit result into a 0/1 truth value in a register (seqz/compare +
 * jump-on-value), where target/HEAD branch directly on the short-circuit. That
 * adds ~5 instructions to the function BODY (build 90 -> 95), and the extra
 * truth-value materialization lands entirely in the guard region — it does NOT
 * touch either residual axis (axis A = combine offset-0 BF68 fold in the 2nd
 * printf; axis B = sched1 volatile-MEM anti-dep priority in the 1st printf
 * setup). Both axes are in the report body and are provably insensitive to how
 * the guard is reached (s8 control-flow-insensitive proof). So the switch only
 * ADDS a truth-value cluster and never reorders/materializes the target's
 * missing insns.
 *
 * Skeleton-level rederive is now dead at: if/nested-if+goto (s8), ternary (s26),
 * post-increment condition (s27), single-exit accumulator (s35), inverted early
 * return / computed-flag-two-if (banked), and now switch-dispatch (s36, strictly
 * worse). Both axes stay mechanism-pinned dead.
 */
s32 func_8007DC9C(void) {
    volatile s32 *new_var2;
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    new_var2 = &D_8009BF7C;
    switch ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
    case 0:
        return 0;
    default:
        new_var = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
        temp_v0 = motion_make_table(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = *new_var2;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        motion_make_table(D_8009BF88);
        return -1;
    }
}
