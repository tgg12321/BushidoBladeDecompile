/* REJECTED (s20, structural) — score 9, identical fingerprint (target 91 /
 * build 90 / 4 rules / 150 cheat_asm_stripped). KILLED.
 *
 * WHY DEAD: This is the COMBINATION of two s2 levers that s2 only measured
 * individually (both 9): (a) fmt-pointer precompute declared FIRST
 * (`const s32 *fmt0 = &g_str_gpu_timeout;`) + (b) the first-printf masked
 * subtraction hoisted to a named temp `diff0`. The combination changes the
 * LUID/declaration topology feeding sched1 but does NOT flip axis B: s15
 * proved the fmt-vs-deadread priority is body-shape-invariant (dead-read
 * priority 2 > fmt priority 1 via the volatile-MEM anti-dep REG_DEP_ANTI,
 * identical modulo a uniform UID shift across every body shape). It also does
 * nothing for axis A (BF68[0] stays a single pure offset-0 rvalue -> combine
 * offset-0 fold to 2-insn). Fresh measured negative; no un-banked structural
 * lever remains.
 */
s32 func_8007DC9C(void) {
    volatile s32 *new_var2;
    const s32 *fmt0;
    s32 diff0;
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    fmt0 = &g_str_gpu_timeout;
    new_var2 = &D_8009BF7C;
    if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
        new_var = *g_gpu_stat_reg;
        diff0 = (D_8009BF78 - D_8009BF7C) & 0x3F;
        debug_printf(fmt0, diff0, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
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
    return 0;
}
