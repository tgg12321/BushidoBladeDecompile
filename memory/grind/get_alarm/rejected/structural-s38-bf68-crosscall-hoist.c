/* REJECTED s38 — structural: isolate axis-A read D_8009BF68[0] into a
 * top-of-block temp `t68` computed BEFORE the first debug_printf, so its
 * value lives across the first call (forcing a callee-save).
 *
 * Hypothesis: a cross-call live range for the BF68 read might change combine's
 * single-use fold and materialize the 3-insn address (axis A).
 *
 * RESULT: sandbox --disable all score 25, build_insns 92 (STRICTLY WORSE than
 * floor 9 / build 90). The hoisted read still folds at its read site (the
 * ADDRESS pseudo remains single-use — combine.c:1458 added_sets_2 is about the
 * number of address uses, not the value's live-range length), AND the cross-call
 * live range forces a callee-save save/restore (+2 insns) plus a schedule
 * disruption. Live-range length is orthogonal to the offset-0 fold. KILLED.
 * Corroborates s2/s3/s6/s7/s9: axis A needs a genuine 2nd ADDRESS use (= coercion
 * single-fn, mechanically impossible cross-fn), not a longer live range.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    s32 t68;
    if ((g_gpu_vcount < sys_VSync(-1)) || (temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, ((temp_v1 > 0xF0000) != 0))) {
        t68 = D_8009BF68[0];
        new_var = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        debug_printf(&D_80016044, t68, D_8009BF6C, D_8009BF70);
        temp_v0 = motion_make_table(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = D_8009BF7C;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        motion_make_table(D_8009BF88);
        return -1;
    }
    return 0;
}
