/* REJECTED — s35 rederive. Single-exit result-accumulator skeleton.
 * score 19 (STRICTLY WORSE than floor 9), build_insns 90, rules_dropped 4,
 * cheat_asm_stripped 150.
 *
 * Distinct AST from all six banked rederive forms (HEAD dual-return comma-if,
 * s8 goto/nested-if, s17 inverted-early-return-guard, s18 computed-flag-two-if,
 * s26 ternary, s27 post-increment). A human-writable natural shape (`s32 result
 * = 0; if(cond){...; result=-1;} return result;`) — NOT a cheat.
 *
 * NEW DATA POINT: unlike s17/s18 (exit-coalescing -> build 88 / score 12), the
 * result-accumulator does NOT coalesce exits — it stays at 90 insns but scores
 * 19, WORSE than both the distinct-exit floor build (90/9) AND the
 * exit-coalescing forms (88/12). The shared `result` pseudo forces an extra
 * $v0 move/materialization on the A-false fall-through path (return 0 loads the
 * accumulator instead of `li $v0,0` at the return) plus reshuffles the tail
 * return set, adding divergence on top of the two mechanism-pinned axes.
 * Neither axis A (BF68[0] combine offset-0 fold, combine.c:1458) nor axis B
 * (over-determined sched1 volatile-MEM lock) moved. KILLED.
 */
s32 func_8007DC9C(void) {
    volatile s32 *new_var2;
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    s32 result = 0;
    new_var2 = &D_8009BF7C;
    if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
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
        result = -1;
    }
    return result;
}
