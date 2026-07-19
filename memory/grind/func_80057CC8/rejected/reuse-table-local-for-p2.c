/* REJECTED s3 2026-07-19 — KILLED-HARD (score 3 -> 32, build_insns 112).
 * Reusing the function-scope `table` local for BOTH p1 AND p2 (dropping the
 * re-dereference `*(s16**)(arg0+4)` at p2). Forces `table` into a
 * callee-save across the first jal and disturbs everything.
 * Structural bound: p2 MUST use the inline re-dereference form.
 * Do NOT re-derive.
 */
/* Diff from candidate.c: at p2, `(s32)(*(s16**)(arg0+4))` replaced with `(s32)table`. */
