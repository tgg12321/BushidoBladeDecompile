/* REJECTED s52 (rederive modality, 2026-09-16).
 * Combination probe: the s34-established do-while loop rewrite (loop
 * syntax only, `limit` local retained, measured 46/195 at s34) COMBINED
 * WITH the s6/s11-13/s51-killed "shared `off = i * 2;` local" index family
 * (previously measured only on the FOR-loop chassis, across three chassis
 * generations, always killed -- s51's rejected/shared-offset-local-s51-
 * refresh-worse.c explicitly flagged this do-while-chassis combination as
 * untried). This session tests it for the first time:
 *
 *   i = start;
 *   do {
 *       s32 obj; ... (unchanged per-iteration locals)
 *       off = i * 2;
 *       obj = arg0;
 *       flags = D_8009A821[off] << 8;
 *       ...
 *       scale = D_8009A820[off] << 8;
 *       ...
 *       i++;
 *   } while (i < limit);
 *
 * (`off` declared as a fresh function-scope local since C89 forbids a
 * statement before block-local declarations; `limit` local retained
 * unlike the sibling dowhile-nolimit-worse.c probe.)
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 66/204,
 * build_insns 197 (baseline candidate.c: score 38/204, build_insns 198;
 * do-while-alone s34 baseline: 46/195). WORSE than both the do-while-alone
 * chassis and the for-loop+shared-idx combination (51/204 at s51) --
 * confirms the shared-idx-local family remains dead on a FOURTH chassis
 * generation (for-loop @81, @48-58, @38 three times; now do-while @46).
 * The live-range-spanning-the-ratan2-call cost (s6's original mechanism
 * finding) is chassis-independent, as expected since it is a register-
 * pressure argument about the shared local's lifetime, not about the
 * loop's control-flow shape.
 *
 * Reverted immediately via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s52 chassis
 * (s22-s51-banked body + func_80053614 s32-return prerequisite + header
 * externs, loop rewritten to do-while form with `limit` retained, `off`
 * shared-index local added replacing both `i * 2` occurrences, no FAKE
 * constructs present).
 */
