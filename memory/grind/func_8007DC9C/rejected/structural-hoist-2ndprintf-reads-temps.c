/* REJECTED (s29, structural) — score 35 / build_insns 95 (worse than floor 9).
 *
 * Novel structural lever (un-banked): hoist the SECOND debug_printf's global
 * reads D_8009BF6C and D_8009BF70 into named temps `arg_c`/`arg_d` computed at
 * the TOP of the if-body, BEFORE the first debug_printf call — extending both
 * live ranges across the axis-B scheduling region to probe whether added
 * register pressure flips the fmt-vs-deadread sched1 tie.
 *
 * Result: STRICTLY WORSE. Holding two values live across the first
 * debug_printf forces them into callee-saved ($sN) registers → extra prologue
 * save/restore + spill, build grows 90 -> 95 insns, score 9 -> 35. Axis B did
 * NOT flip: the dead *g_gpu_stat_reg read's priority is set by the volatile-MEM
 * anti-dep 38->45 (s6/s15), NOT by surrounding register pressure, so adding
 * pressure only degrades allocation without changing the schedule order.
 * Corroborates the s2/s6 axis-B KILL from the opposite direction: the target's
 * schedule uses the natural LOW-pressure allocation, and any structural form
 * that inflates cross-call liveness moves strictly away from it.
 *
 * Legitimate non-cheat form (named temps for values genuinely consumed by the
 * 2nd printf; no dead code, no coercion) — it simply loses.
 */
s32 func_8007DC9C(void) {
    volatile s32 *new_var2;
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    s32 arg_c;
    s32 arg_d;
    new_var2 = &D_8009BF7C;
    if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
        arg_c = D_8009BF6C;
        arg_d = D_8009BF70;
        new_var = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        debug_printf(&D_80016044, D_8009BF68[0], arg_c, arg_d);
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
