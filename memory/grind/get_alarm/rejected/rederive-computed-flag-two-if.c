/* REJECTED (s18, rederive modality) — computed-boolean-flag skeleton.
 * sandbox --disable all -> score 12, build_insns 88 (TWO FEWER than the 90-insn
 * floor build; target 91). WORSE than floor.
 *
 * De-comma's HEAD's short-circuit comma-expression guard into an explicit
 * boolean accumulator `do_report` computed in two separate `if` statements.
 * Structurally distinct from HEAD (comma-expr, 90/floor 9), s8 (goto/nested-if,
 * 90/floor 9), s17 (inverted early-return-guard hoisted to fn scope, 88/score 12).
 *
 * Why it worsens: splitting the guard into a flag + two ifs lets GCC MERGE the
 * two `return 0` exit paths (the fall-through after the first `if (!do_report)`
 * block and the final `return 0`), collapsing to 88 insns — moving AWAY from
 * target's 91. Lands at the SAME 88/12 fingerprint as s17's inverted-return:
 * ANY control-flow shape that lets GCC coalesce the two exit paths drops to 88.
 * Only HEAD's comma-expression guard (and s8's goto/nested-if, which keeps the
 * report block nested and the paths separate) reaches the 90-insn floor; none
 * reaches target's 91.
 *
 * Both pinned axes (axis A combine.c:1458 offset-0 fold; axis B sched1
 * volatile-MEM anti-dep REG_DEP_ANTI priority) unmoved — consistent with the
 * s8/s15/s17 proof that both axes are control-flow-insensitive.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    s32 do_report;
    do_report = (g_gpu_vcount < sys_VSync(-1));
    if (!do_report) {
        temp_v1 = g_gpu_draw_count;
        g_gpu_draw_count = temp_v1 + 1;
        do_report = (temp_v1 > 0xF0000);
    }
    if (do_report) {
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
    return 0;
}
