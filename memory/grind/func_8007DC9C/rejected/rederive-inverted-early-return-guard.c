/* REJECTED (s17, rederive modality) — inverted early-return guard skeleton.
 *
 * Structurally-distinct control-flow rederivation: the `||` short-circuit guard
 * is inverted into `if (g_gpu_vcount >= sys_VSync(-1)) { draw_count++; if (temp
 * <= 0xF0000) return 0; }` with the divergent debug-report block hoisted to
 * straight-line FUNCTION scope (vs s8's goto-report + nested-if form, which kept
 * the block nested). Semantics preserved: draw_count++ runs only on the A-false
 * path, exactly as the comma-expression short-circuit does.
 *
 * MEASURED: sandbox --disable all -> score 12, build_insns 88 (TWO FEWER than the
 * 90-insn floor build; target is 91). WORSE than the floor. Hoisting the block to
 * function scope + inverting the guard lets GCC merge the two return paths and
 * emit 88 insns, moving AWAY from target's 91 rather than toward it.
 *
 * Conclusion: corroborates s8 (control-flow-insensitive for the two pinned axes)
 * from the opposite direction — the ONLY control-flow shape that even reaches the
 * 90-insn floor is HEAD's comma-expression guard; the goto/nested-if form (s8)
 * ties it at 90/score 9, and this inverted-return form worsens to 88/score 12.
 * No control-flow rederivation reaches target's 91. Axis A (combine offset-0 fold)
 * and axis B (sched1 volatile-MEM anti-dep priority) are unmoved. KILLED.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    if (g_gpu_vcount >= sys_VSync(-1)) {
        temp_v1 = g_gpu_draw_count;
        g_gpu_draw_count = temp_v1 + 1;
        if (temp_v1 <= 0xF0000) {
            return 0;
        }
    }
    new_var = *g_gpu_stat_reg;
    debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
    (void)new_var;
    debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
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
