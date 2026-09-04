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
 *
 * s43 (escalation modality) adds the two links UPSTREAM of s42's output dependence, so axis B's
 * chain is now named end to end: insn 36 (lw stat_ptr, icost 2) shadows insn 38 (the dead read),
 * so sched1 hoists insn 54 (lw madr_ptr) between them; that gives pseudo 89 the live range [6,10)
 * overlapping the dead read's [8,9); local-alloc allocates the dead read LAST (qty 2, refs 1 ->
 * qty_compare priority 0) and the only free low reg across its life is $a0 — which is the register
 * the fmt `la` must write. s43 then closed the C-expressible surface in BOTH scheduler passes:
 * pass-1 goals "38 before 54" and "56 before 38" are each unreachable by any of 2583 luid/luid_move
 * atoms, while the SAME goals ARE reachable by add_dep/del_dep/cost atoms (dependence-graph and
 * machine-description edits with no C spelling). The one implicated C form (stage *g_gpu_dma_madr
 * before the dead read) builds at score 9 — ties, closes nothing. This body is unchanged from s42.
 *
 * s44 (synthesis, 5th merge pass) leaves this BODY UNCHANGED and re-measured it
 * chassis-current at score 9 (target 91 / build 90 / rules_dropped 0 /
 * cheat_asm_stripped 147). Two things changed around it:
 *   (1) The statement-order surface is now CLOSED AT ALL DEPTHS, not just at
 *       depth 1. luid/luid_move atoms mutate only the per-node luid field
 *       (tools/sched_solver/perturb.py:239-251) while priorities are recomputed
 *       from the dependence graph alone, so the whole depth-N luid space is a
 *       permutation space; 80,000 random luid permutations (pass 1 and pass 2,
 *       window-only and whole-block) yield ONE emitted order in pass 1 and FOUR
 *       in pass 2, none of them target's. In pass 2 it is a graph fact rather
 *       than a sampling result: deps[60] = [[38,15]] is an OUTPUT dependence,
 *       and a backward list scheduler can never cross a dependence edge on a
 *       tie-break. Do NOT re-open statement reordering.
 *   (2) The mandated sibling sweep measured CD_sync's two devices on this
 *       chassis for the first time. The sanctioned F1 combine-foldable
 *       chain-extender does NOT open axis A (three spellings, build_insns never
 *       leaves 90), and the sanctioned pointer-alias-to-a-global does NOT move
 *       the dead read off $a0 (madr alias 9/90 inert, stat alias 17/92 worse).
 *       s3's axis-A kill rested on a scope the 2026-07-01 F1 ruling widened, so
 *       axis A is now dead on MEASUREMENT rather than on a superseded reading.
 * Live frontier: (a) which pass kills the F1 extender's second reference
 * (cse2 vs combine — a dump read); (b) the unfinished ra_solver inverse local
 * run for the $v0 seat. See tmp/grind/get_alarm/s44/MERGED-ATTACK-s44.md.
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
