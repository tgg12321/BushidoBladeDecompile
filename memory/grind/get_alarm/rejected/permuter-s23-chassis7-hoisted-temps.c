/* REJECTED — s23 permuter chassis-7 base (second-printf reads hoisted to
 * block-top named temps rep_a/rep_b/rep_c, lengthening their live ranges
 * across the first-printf axis-B cluster). A structurally-distinct HIGH-
 * register-pressure regime (base compiles to 98 insns vs target 91; the three
 * temps spill across the intervening debug_printf/motion_make_table calls, so
 * the hoist adds ~8 insns of spill code). base_score 2826.
 *
 * WHY DEAD: 36,103-iteration campaign (-j8). The permuter descended the bloated
 * basin from 2826 to a plateau min of 1186 — never approaching the ~630
 * legitimate floor, and produced ZERO sub-floor find (this seed cannot even
 * reach the banked volatile-BF68 coercion region other chassis hit). Axis A
 * (BF68[0] 3-insn materialization) never legitimately materialized; axis B
 * (8-op fmt-vs-deadread sched1 cluster) never legitimately reordered. The
 * hoist is also non-improving by construction: rep_a=D_8009BF68[0] is still a
 * single pure offset-0 rvalue read → combine.c:1458 folds it to 2-insn exactly
 * as HEAD does. Permuter now 7x-confirmed dead (chassis 1-7, ~189k total iters).
 */
typedef signed int s32;
extern s32 sys_VSync(s32);
extern volatile unsigned int *g_gpu_stat_reg;
extern volatile unsigned int *g_gpu_dma_chcr;
extern unsigned int *g_gpu_dma_madr;
extern s32 g_gpu_vcount, g_gpu_draw_count;
extern s32 motion_make_table(s32);
extern volatile s32 D_8009BF7C;
extern s32 D_8009BF78, D_8009BF88, g_str_gpu_timeout, D_80016044;
extern volatile int *D_8009BF64;
extern s32 D_8009BF68[], D_8009BF6C, D_8009BF70;
extern s32 debug_printf();

s32 func_8007DC9C(void) {
    s32 temp_v0, temp_v1, new_var;
    s32 rep_a, rep_b, rep_c;
    if ((g_gpu_vcount < sys_VSync(-1)) || (temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, ((temp_v1 > 0xF0000) != 0))) {
        rep_a = D_8009BF68[0];        /* hoisted — still folds (single pure read) */
        rep_b = D_8009BF6C;
        rep_c = D_8009BF70;
        new_var = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        debug_printf(&D_80016044, rep_a, rep_b, rep_c);
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
