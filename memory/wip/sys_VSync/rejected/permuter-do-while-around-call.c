/*
 * REJECTED (session 4, 2026-06-12) — permuter random-mode finding.
 *
 * Permuter score: 40 (vs base 495). Best non-cheat: 495 (no improvement).
 *
 * The mutation wraps the SECOND call expression statement in a
 * `do { ... } while (0);`. Same family as the inherited inner do-while
 * at HEAD's src/ings2.c line 69 / candidate.c line 30, which session-2's
 * cheat-reviewer explicitly FAILed.
 *
 * Per [[do-while-zero-exception]]: the sanction is NARROW —
 * LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole on NE conditions
 * only. This use is a SCHED1 FENCE (defeats sched1's interleave of the
 * volatile lw with the independent arg setup), explicitly OUT OF SCOPE.
 *
 * Per [[no-new-park-categories]] §"Auto-search tools — output is
 * PROPOSALS, not winners": vetting checklist failures are 1 (no semantic
 * purpose), 4 ("necessary only because permuter said so"), 5 (family
 * check — same family as previously FAILed).
 *
 * Permuter also surfaced 3 other do-while wrap variants (around the
 * first-call block at score 355, around the multi-statement prologue at
 * score 300, around the second call combined with dead-store-via-s0_val
 * at score 50). All same family. All FAIL.
 *
 * Net effect: the random-permuter surface near this candidate is
 * exhausted of legitimate (non-cheat) closing forms. Random-mode
 * mutations don't reach the reorg.c delay-slot-fill decision point
 * (where the session-3 dump localised the remaining gap).
 */

/* The rejected mutation: */
s0_val = *D_800A1510;
do { func_80082A14(g_sys_dma_region + 1, 1); } while (0);
/*
 * Same intent as:
 *   s0_val = *D_800A1510;
 *   func_80082A14(g_sys_dma_region + 1, 1);
 * except the do-while emits NOTE_INSN_LOOP_BEG which sched1 treats as
 * a basic-block boundary, preventing it from interleaving the volatile
 * lw of s0 with the arg-setup loads.
 */
