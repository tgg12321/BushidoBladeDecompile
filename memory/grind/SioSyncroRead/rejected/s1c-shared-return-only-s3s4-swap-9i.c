/* REJECTED [s1] 2026-08-25 — single shared return compute (both cleanups
 * `goto return_val;`): r_arg1 stays at 3 flow refs / livelen 102 and loses s3
 * to pkt_len (3 refs / livelen 56) under global.c allocno_compare priority —
 * the rule-era $19<->$20 @0-134 swap. Measured score 9, 160/160 (final chassis
 * minus the F-variant). Solver proof: tmp/ra_solver_work/SioSyncroRead.model.json
 * (p74 disp=20/s4, p78 disp=19/s3). Fixed by the sanctioned
 * duplicated-statement-into-arms ref-lift (return compute duplicated into both
 * cleanup arms; cross-jump re-merges byte-neutrally).
 * Rejected shape:
 */
        DeliverEvent(0xF000000B, 0x8000);
        goto return_val;   /* <- and same in cleanup_B; return_val computes once */
