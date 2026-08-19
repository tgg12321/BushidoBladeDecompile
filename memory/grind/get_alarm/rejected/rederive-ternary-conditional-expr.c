/* REJECTED (s26 rederive) — whole-function ternary/conditional-expression form.
 * The entire report block folded into a comma-expression rvalue inside a `?:`,
 * a structurally-distinct AST from every banked rederive form (HEAD statement-if,
 * s8 goto/nested-if, s17 inverted-early-return, s18 computed-flag-two-if).
 *
 * MEASURED: sandbox --disable all -> score 9, target_insns 91, build_insns 90,
 * rules_dropped 4, cheat_asm_stripped 150 == IDENTICAL floor-9 fingerprint.
 * Neither axis moved. Reaches the 90-insn floor (like HEAD + s8 goto/nested-if),
 * confirming the s17/s18 finding that ONLY exit-coalescing skeletons drop to 88/12;
 * a distinct-exit ternary lands right on the floor. Axis A (combine.c:1458 offset-0
 * fold on the single pure BF68[0] rvalue) and axis B (sched1 over-determined
 * volatile-MEM/load-delay/LUID lock, s24) are both control-flow/skeleton-insensitive
 * (s8/s15/s24/s25), so the conditional-expression skeleton cannot move either. KILLED.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    return ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0)))
        ? (new_var = *g_gpu_stat_reg,
           debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr),
           (void)new_var,
           debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70),
           temp_v0 = motion_make_table(0),
           D_8009BF7C = 0,
           D_8009BF88 = temp_v0,
           D_8009BF78 = D_8009BF7C,
           *g_gpu_dma_chcr = 0x401,
           *D_8009BF64 |= 0x800,
           *g_gpu_stat_reg = 0x02000000,
           *g_gpu_stat_reg = 0x01000000,
           motion_make_table(D_8009BF88),
           -1)
        : 0;
}
