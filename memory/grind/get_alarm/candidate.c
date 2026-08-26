/* Candidate body for get_alarm (formerly func_8007DC9C) — honest floor 9 on the
 * post-migration chassis (re-measured s42: sandbox --disable all score 9, target_insns 91,
 * build_insns 90, rules_dropped 0, cheat_asm_stripped 147).
 *
 * s42 change vs the s41 form: the discarded volatile read of *g_gpu_stat_reg is now spelled
 * as the bare expression statement `(void)*g_gpu_stat_reg;` instead of `new_var = *g_gpu_stat_reg;`
 * + a trailing `(void)new_var;`. Same score (9), same fingerprint, but it removes a dead local
 * whose only purpose was to hold a discarded value — ordinary C for "read a volatile and throw
 * the value away", and one fewer construct for a reviewer to question. Measured, not assumed.
 *
 * Residual 9-op gap = two orthogonal axes (see hypotheses.md):
 *   axis A (1 insn) — combine.c:1458 added_sets_2 single-use fold of the &D_8009BF68 address
 *     pseudo; target keeps it materialized (lui+addiu+lw 0(reg)), we fold to lui+lw %lo.
 *   axis B (8 insns) — RE-ATTRIBUTED s42 by tools/sched_solver: it is a ONE-INSN move of the
 *     format-string `la` (pass-2 UID 60) from emit slot 4 to slot 1, and the exhaustive
 *     single-atom search over 4391 atoms found EXACTLY ONE vector reaching target's order —
 *     `del_dep 60 <- 38`, i.e. deleting the pass-2 OUTPUT DEPENDENCE on hard register $a0
 *     between the dead *g_gpu_stat_reg read (UID 38, greg seats it in $4) and the fmt `la`
 *     (also writes $4). Target seats that dead read in $v0 (lw $v0,0($v1) @ 0x8007DCFC), so
 *     the edge does not exist there. NO luid/statement-order atom reaches the goal. Axis B is
 *     therefore an RA SEAT question, not a scheduler tie — see tmp/grind/get_alarm/s42/solver_report.md.
 */
s32 get_alarm(void) {
    s32 temp_v0;
    s32 temp_v1;
    if ((g_gpu_vcount < VSync(-1)) || (temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, ((temp_v1 > 0xF0000) != 0))) {
        (void)*g_gpu_stat_reg;
        printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
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
