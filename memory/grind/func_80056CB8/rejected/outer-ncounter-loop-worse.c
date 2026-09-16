/* REJECTED s52 (rederive modality, 2026-09-16).
 * Genuinely-measured version of the "outer 0..1-counter loop restructuring"
 * that s51 attempted but broke via a brace-unbalanced string-replace
 * (recorded there as a NON-MEASUREMENT, build_insns=8, discarded). This
 * session hand-wrote a brace-balanced splice:
 *
 *   for (n = 0; n < 2; n++) {
 *       ... unchanged per-iteration locals ...
 *       i = start + n;
 *       ... rest of loop body unchanged, using i as before ...
 *   }
 *
 * i.e. decoupling the loop-carried index `i` (used for both indexing and
 * the bound compare in candidate.c's `for (i = start; i < limit; i++)`)
 * into a fresh trivially-constant counter `n` (bound test `n < 2`) plus a
 * derived `i = start + n;` computed once per iteration. No `limit` local
 * at all in this form.
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 68/204,
 * build_insns 199 (baseline candidate.c: score 38/204, build_insns 198).
 * WORSE on both axes. The `n < 2` trivially-constant bound did NOT avoid
 * a register-pressure/spill cost -- if anything it's one insn heavier than
 * baseline (199 vs 198), so decoupling the counter did not remove the
 * reg-11 spill the s51 forensics session traced to the loop-invariant
 * `limit = start + 2;` pseudo; it just relocates the same kind of pressure
 * onto a different pseudo (`start + n` computed fresh per iteration is
 * itself a value that must be held live for both the index use and the
 * `arg0 + i` store-address computation at the tail).
 *
 * This closes the ONE remaining genuinely-untried item named across s49-s51
 * ("outer 0..1 counter loop restructuring... this C shape is genuinely
 * untried") -- it is now tried and killed.
 *
 * Reverted immediately via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s52 chassis (fresh
 * splice of the s22-s51-banked 38/204 body + func_80053614 s32-return
 * prerequisite + header externs, loop restructured from
 * `for (i=start;i<limit;i++)` to `for (n=0;n<2;n++) { i = start+n; ... }`
 * with the `limit` local dropped, no other change, no FAKE constructs
 * present).
 */
