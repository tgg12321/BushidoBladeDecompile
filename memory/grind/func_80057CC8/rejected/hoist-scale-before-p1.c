/* REJECTED s3 2026-07-19 — KILLED-HARD (score 3 -> 35, build_insns 113).
 * Hoisting `scale = arg0[2] * 40;` before the p1 pointer add adds pressure
 * at the p1 addu window and completely destroys the delay-slot / RA plan.
 * Structural bound: scale MUST stay after ang_prev/ang_next computation.
 * Do NOT re-derive.
 */
/* Diff from candidate.c: `scale = arg0[2] * 40;` moved to just before the
 * `p = ... prev_idx offset ...` line instead of after the ang_mid if/else. */
