/* REJECTED s34 (rederive modality, 2026-09-16).
 * Empirical measurement of the do-while loop rewrite that s33 reasoned
 * about but did NOT build/measure (s33's kill was "reasoning-only", citing
 * loop.c's loop-inversion pass as a post-parse RTL transform that should
 * make a source-level do-while vs for-loop codegen-neutral). This session
 * built and measured it for real, on the s22-s33-banked 38/204 chassis:
 *
 *   start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
 *   limit = start + 2;
 *   i = start;
 *   do {
 *       ... identical loop body, unchanged from candidate.c ...
 *       i++;
 *   } while (i < limit);
 *
 * (Same body as candidate.c's `for (i = start; i < limit; i++) { ... }`,
 * only the loop's own syntactic form changed -- no other edit.)
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 38 -> 46/204,
 * build_insns 198 -> 195.
 *
 * IMPORTANT: this REFUTES s33's reasoning-only prediction of "codegen-
 * neutral" -- build_insns actually DROPPED by 3 (198 -> 195), proving the
 * do-while source form DOES change cc1's emitted RTL on this chassis, even
 * though loop-inversion is nominally a post-parse pass. (Plausible
 * mechanism, not yet dump-verified: the for-loop's guard is evaluated once
 * before the first iteration via loop-inversion's synthesized entry test,
 * while a source do-while skips that synthesis entirely since there is no
 * separate entry-guard to invert -- this changes which basic blocks/notes
 * loop.c's later passes see, altering downstream RA/scheduling decisions
 * even though the STEADY-STATE loop shape is nominally identical.) Despite
 * fewer real instructions, the overall weighted sandbox score is WORSE
 * (46 vs 38) -- the removed instructions apparently weren't the ones
 * separating build from target; some other register/ordering choice moved
 * further from target as a side effect.
 *
 * This chassis is a genuinely NEW, structurally different starting point
 * (fewer real insns: 195 vs 198) that no prior session in this ledger has
 * explored. A follow-up session should consider re-testing the OTHER
 * killed frontier items (register-pressure restructuring of the 8 named
 * residents, e.g.) on THIS do-while chassis rather than the for-loop
 * chassis, since the register/insn landscape genuinely differs now. This
 * session tried exactly one combination on top of it (idx2 hand-carried
 * accumulator, see do-while-idx2-worse.c) and it was worse (66/204) --
 * still leaves untried: any of the OTHER previously-killed index-naming
 * spellings (shared single idx local, separately-named idxB, pointer-walk)
 * re-measured on this do-while chassis instead of the for-loop chassis.
 *
 * Reverted immediately via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s34 chassis
 * (s22-s33-banked 38/204 body + func_80053614 s32-return fix + header
 * externs, loop rewritten from `for` to `do-while` with no other change,
 * no FAKE constructs present).
 */
