/* REJECTED (s2, structural). Axis B: flip the sched1 order of fmt(la) vs the
 * dead *g_gpu_stat_reg read so fmt schedules first (into $a0) and the dead read
 * lands in $v0 (as target does). Every structural lever measured score 9:
 *   - fmt precompute FIRST (const s32 *fmt = &g_str_gpu_timeout; ...)     -> 9
 *   - fmt precompute as first statement (fmt = &...; then dead read)       -> 9
 *   - bare (void)*g_gpu_stat_reg; with fmt precompute                      -> 9
 *   - arg1 = (BF78-BF7C)&0x3F precomputed to a temp (after dead read)      -> 9
 *   - *g_gpu_dma_madr precomputed to a temp                                -> 9
 *   - D_8009BF78 precomputed to a temp                                     -> 9
 *   - new_var declared first                                              -> 9
 *   - split subtraction (a1v=BF78; a1v-=BF7C; a1v&=0x3F)                  -> 12 (worse)
 *   - *g_gpu_dma_chcr cached to a temp                                    -> 19 (collapses a volatile read)
 *
 * Mechanism (from the .greg dump, insn 38 = dead read, insn 60 = fmt):
 *   The dead read `new_var = *g_gpu_stat_reg;` is volatile-ordered BEFORE the
 *   volatile read of D_8009BF7C (insn 45). That memory ordering puts the dead
 *   read on the critical path  dead-read -> BF7C -> subu(arg1) -> call, giving
 *   it high sched1 priority, so it is scheduled early and grabs $a0 (lowest
 *   free reg); fmt (a lone `la`, chain length 1 to the call) then schedules
 *   AFTER it and must reload $a0. Target schedules fmt first, so its dead read
 *   falls to $v0. The order is decided in sched1 by PRIORITY (critical-path
 *   length), NOT by LUID / source order — hence no statement-order lever moves
 *   it. The volatile ordering dead-read<BF7C matches target and cannot be
 *   swapped (it is observable), so the dead read stays high-priority.
 * KILLS H-B1 (statement-order/liveness tie-break). Remaining axis-B lever, if
 * any, is not structural (would need to change critical-path chain lengths
 * without changing volatile semantics). */
