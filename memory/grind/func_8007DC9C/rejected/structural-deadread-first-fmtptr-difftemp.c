/* REJECTED (s21, structural) — score 9, IDENTICAL fingerprint (target 91 / build 90,
 * rules_dropped 4, cheat_asm_stripped 150). Neither axis moved.
 *
 * Novel three-lever combination not previously measured as a UNIT:
 *   (1) dead stat-read `new_var` STAGED FIRST (declared + assigned as first stmt),
 *   (2) fmt pointer precomputed into a local `fmt0`,
 *   (3) masked subtraction homed into a local `diff0`.
 * Distinct topology from s2 (each lever individually: fmt-precompute-first,
 * new_var-declared-first, bf78/diff-temp — all score 9) and from s20
 * (fmt-ptr-FIRST + diff-temp, i.e. fmt BEFORE the dead read — score 9).
 *
 * Why it measured dead (predicted by s15): axis B priority is body-shape-INVARIANT —
 * the sched1 decision comes from the volatile-MEM anti-dep REG_DEP_ANTI (dead
 * *g_gpu_stat_reg read -> volatile D_8009BF7C read), which every legitimate body
 * preserves; declaration/LUID topology shifts UIDs uniformly without changing
 * priorities. Axis A is a per-expression combine.c:1458 added_sets_2 offset-0 fold
 * on the single pure D_8009BF68[0] rvalue — untouched by any of these levers.
 */
s32 func_8007DC9C(void) {
    const s32 *fmt0;
    s32 diff0;
    s32 new_var;
    s32 temp_v0;
    s32 temp_v1;
    if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
        new_var = *g_gpu_stat_reg;
        fmt0 = &g_str_gpu_timeout;
        diff0 = (D_8009BF78 - D_8009BF7C) & 0x3F;
        debug_printf(fmt0, diff0, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
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
