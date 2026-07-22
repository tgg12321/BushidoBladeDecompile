/* REJECTED (s39, structural) — score 20 (build 90), strictly WORSE than floor 9.
 *
 * Novel un-banked structural spelling: hoist arg1 `diff` to the top of the
 * if-body AND stage the TWO *g_gpu_stat_reg reads as two distinct named temps —
 * the dead read `new_var` (target's early *stat read) and a named `arg3` for the
 * printf-argument read — so the printf's stat read is precomputed BEFORE the
 * call instead of inline at the call site.
 *
 * Result: score 20. build_insns stays 90 (no insn-count change) but 20 insns
 * differ vs target's 91. Naming the printf-arg *stat read into a pre-call temp
 * reorders the two volatile reads and pushes the sched1 order further from
 * target than the baseline. All prior structural probes (s2/s29/s30) nudged the
 * DEAD read and scored 9; this is the first to nudge the LIVE printf-arg read,
 * and it strictly worsens — proving the printf's *stat read is schedule-
 * position-locked in place (must stay inline at the call), corroborating axis B's
 * volatile-MEM anti-dep order lock (s6/s34) from the opposite direction.
 *
 * Two reads preserved (not the score-19 single-read collapse); the worsening is
 * purely the schedule displacement of moving the live read out of the call.
 */
s32 func_8007DC9C(void) {
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    s32 diff;
    s32 arg3;
    if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
        diff = (D_8009BF78 - D_8009BF7C) & 0x3F;
        new_var = *g_gpu_stat_reg;
        arg3 = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, diff, arg3, *g_gpu_dma_chcr, *g_gpu_dma_madr);
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
