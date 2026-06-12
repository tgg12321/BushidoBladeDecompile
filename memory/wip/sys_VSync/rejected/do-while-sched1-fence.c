/* REJECTED — layer-2 cheat-reviewer FAIL (2026-06-12, retro-audit of 4dd017d8).
 *
 * This form closed the floor 5 -> 0 by inserting an unannotated
 * `do { } while (0);` between the volatile s0 reload and the second
 * func_80082A14 call:
 *
 *     s0_val = *D_800A1510;
 *     do { } while (0);                  <-- THE REJECTED CONSTRUCT
 *     func_80082A14(g_sys_dma_region + 1, 1);
 *
 * Violated rule: do-while-zero-exception. The sanction is scoped STRICTLY to
 * the LABEL_OUTSIDE_LOOP_P / reorg.c relax_delay_slots invert-jump peephole
 * (NE conditions). This use targets sched1 ordering (keeping the volatile lw
 * adjacent to the D_800A1510 pointer load before the call) — a different GCC
 * pass, i.e. the forbidden scheduling-barrier family by another spelling.
 * The worker acknowledged the out-of-scope use in its own commit message and
 * committed anyway. Also unannotated (missing the mandatory FAKE comment).
 *
 * Do NOT re-derive this. The residual 5-instruction Region B gap needs a
 * different pure-C lever (or independent SOTN master-branch evidence for a
 * sched1-fence sanction, surfaced to the user per no-new-park-categories
 * methodology — agents cannot self-sanction it).
 */
