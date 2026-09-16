/* REJECTED s34 (rederive modality, 2026-09-16).
 * idx2 hand-carried accumulator (idx2 = start*2 before the loop, idx2 += 2
 * in the do-while's increment step, both [i*2] table-index reads replaced
 * with [idx2]) applied ON TOP OF the do-while chassis from
 * do-while-loop-rewrite-worse.c (build_insns 195 baseline for this
 * chassis), instead of the for-loop chassis s30 already killed this exact
 * spelling on (build_insns 198 baseline there).
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 38 -> 66/204,
 * build_insns 198 (up from the do-while-alone chassis's 195; worse than
 * both the do-while-alone chassis (46/204) AND the original for-loop+idx2
 * combination s30 already killed (52/204)).
 *
 * Confirms the s30 conclusion generalizes: hand-authoring idx2 as an
 * explicit user pseudo costs more in register pressure than loop.c's own
 * strength-reduction would, REGARDLESS of whether the surrounding loop is
 * spelled as `for` or `do-while`. The do-while chassis's insn-count
 * advantage (195 vs 198) does not survive combination with idx2 -- adding
 * idx2 to the do-while chassis actually returns build_insns to 198 (same
 * as the for-loop+idx2 combination), suggesting the do-while chassis's
 * savings and the idx2 construct compete for the same registers/insns
 * rather than compounding.
 *
 * Reverted immediately via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s34 chassis
 * (do-while-loop-rewrite-worse.c's 46/204 do-while body + idx2 accumulator
 * substituted at both table-index reads, func_80053614 s32-return fix +
 * header externs unchanged, no FAKE constructs present).
 */
