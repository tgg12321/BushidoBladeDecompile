/* REJECTED s52 (rederive modality, 2026-09-16).
 * Combination probe: the s34-established do-while loop rewrite
 * (`i = start; do { ...; i++; } while (i < limit);`, measured 46/195 at
 * s34) COMBINED WITH the s51-established "no separately-named `limit`
 * local, inline the bound as `i < start + 2`" edit (measured 42/197 alone
 * at s51 on the for-loop chassis). Neither combination had been measured
 * together before this session:
 *
 *   i = start;
 *   do {
 *       ... unchanged body ...
 *   } while (i < start + 2);
 *
 * (no `limit` local declared at all)
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 45/204,
 * build_insns 194 (baseline candidate.c: score 38/204, build_insns 198).
 * WORSE overall despite the lowest build_insns count measured for this
 * residual so far (194, beating s34's do-while-alone 195 and every
 * for-loop variant's 197-200) -- the removed instructions are still not
 * the ones separating build from target, same qualitative finding as
 * s34's do-while-alone result.
 *
 * Reverted immediately via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s52 chassis
 * (s22-s51-banked body + func_80053614 s32-return prerequisite + header
 * externs, loop rewritten to do-while form with `limit` local removed and
 * bound inlined as `start + 2`, no other change, no FAKE constructs
 * present).
 */
