/* REJECTED (s8, rederive modality) — score 9, build_insns 90 (== floor, no change).
 *
 * Structurally-different control-flow rederivation: the comma-expression
 * condition is dissolved into explicit statements, the `||` short-circuit is
 * re-expressed as an early `goto report` guard + a nested `if (count > 0xF0000)`,
 * and the draw-count increment is split into two named statements. Semantics are
 * identical to HEAD (increment happens ONLY when g_gpu_vcount >= sys_VSync(-1),
 * exactly as the `||` right-operand side effect).
 *
 * RESULT: sandbox --disable all -> score 9, target 91 / build 90, rules_dropped 4,
 * cheat_asm_stripped 150 — the IDENTICAL fingerprint to every prior form. The
 * control-flow shape does not touch either axis:
 *   - Axis A (BF68[0] combine offset-0 fold) is per-expression, independent of
 *     the surrounding control flow -> still folds to the 2-insn form.
 *   - Axis B (sched1 volatile-MEM anti-dep priority of the dead *g_gpu_stat_reg
 *     read vs the fmt load) is set by volatile ordering, not by block structure
 *     -> unchanged.
 * Confirms the s6/s7 forensic KILL from a fresh structural angle: neither axis is
 * reachable by restructuring the C control flow. Rederive modality dead.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 count;
    s32 new_var;
    if (g_gpu_vcount < sys_VSync(-1)) {
        goto report;
    }
    count = g_gpu_draw_count;
    g_gpu_draw_count = count + 1;
    if (count > 0xF0000) {
    report:
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
