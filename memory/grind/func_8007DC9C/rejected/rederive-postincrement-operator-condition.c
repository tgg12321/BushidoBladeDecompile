/* REJECTED (s27, rederive modality) — post-increment operator condition form.
 *
 * Structurally-distinct EXPRESSION-level rederivation: the A-false guard's
 * draw-count side effect is written with the native post-increment operator
 * `g_gpu_draw_count++ > 0xF0000` instead of HEAD's comma-expression temp dance
 * `(temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0)`.
 * This eliminates temp_v1 entirely AND drops the new_var2 volatile-ptr indirection,
 * giving a genuinely different AST at the exact axis-B surrounding region — a form
 * a human would idiomatically write, distinct from every banked skeleton
 * (HEAD comma-if, s8 goto/nested-if, s17 inverted-early-return, s18 computed-flag-
 * two-if, s26 ternary; all skeleton-level, not expression-operator-level).
 *
 * MEASURED: sandbox --disable all -> score 9, target_insns 91, build_insns 90,
 * rules_dropped 4, cheat_asm_stripped 150 == IDENTICAL floor-9 fingerprint.
 * Reaches the 90-insn floor (like HEAD / s8 goto-nested-if / s26 ternary), an
 * exit-distinct skeleton. Neither axis moved: post-increment lowers to the same
 * read-then-increment RTL as the comma-expression temp, so axis A (combine.c:1458
 * offset-0 fold on the single pure BF68[0] rvalue, s25) and axis B (sched1
 * over-determined volatile-MEM anti-dep / load-delay / LUID lock, s24) are both
 * insensitive to the increment operator's spelling — corroborates the per-expression
 * (axis A) / whole-block volatile-order (axis B) insensitivity from a new angle. KILLED.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 new_var;
    if ((g_gpu_vcount < sys_VSync(-1)) || (g_gpu_draw_count++ > 0xF0000)) {
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
