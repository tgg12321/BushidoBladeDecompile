/* Candidate body for get_alarm (formerly func_8007DC9C) — honest floor 9 on the
 * post-migration chassis (measured s41: sandbox --disable all score 9, target_insns 91,
 * build_insns 90, rules_dropped 0). Equivalent to the s3-cleaned form; symbol names
 * refreshed for the current src/display.c naming (sys_VSync -> VSync, debug_printf ->
 * printf, motion_make_table -> SetIntrMask, func_8007DC9C -> get_alarm).
 *
 * Residual 9-op gap = two orthogonal compiler-internal axes with no legitimate pure-C
 * lever (see hypotheses.md): axis A = combine.c:1458 added_sets_2 single-use fold of the
 * &D_8009BF68 address pseudo (target keeps it materialized); axis B = over-determined
 * sched1 lock (volatile-MEM anti-dep priority 2>1 + MIPS-I load-delay hazard + LUID
 * backstop) fixing the dead *g_gpu_stat_reg read before the fmt load.
 */
/* Candidate body for get_alarm — equivalent to HEAD's body (floor unchanged at 9).
 * The volatile-s32-pointer dance (new_var2) is dropped as no-op cleanup; D_8009BF7C
 * is already declared volatile so the indirection through new_var2 is moot.
 */
s32 get_alarm(void) {
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    if ((g_gpu_vcount < VSync(-1)) || (temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, ((temp_v1 > 0xF0000) != 0))) {
        new_var = *g_gpu_stat_reg;
        printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = D_8009BF7C;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}
