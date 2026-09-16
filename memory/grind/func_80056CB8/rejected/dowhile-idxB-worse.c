/* REJECTED s35 (structural modality, 2026-09-16).
 * Frontier item from s34: re-run the previously-killed separately-named
 * idxB spelling (idxB = i * 2, assigned right after the flags-table i*2
 * read, consumed ONLY at the scale-table read) on the do-while chassis
 * (195 real insns) instead of the for-loop chassis it was originally
 * killed on (s33).
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 38 -> 66/204,
 * build_insns 195 (do-while-alone) -> 197 (+2). Byte-identical build_insns
 * to the shared-idx variant above (dowhile-shared-idx-worse.c) -- both
 * index-naming spellings compile to the SAME insn count on this chassis,
 * matching the pattern already observed on the for-loop chassis where
 * different idx-naming spellings converged on similar deltas.
 *
 * Reverted via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s35 chassis
 * (do-while chassis body + separately-named `s32 idxB;` declared at loop-
 * body top, assigned right after the flags read, consumed only at the
 * scale read + func_80053614 s32-return fix + header externs, no FAKE
 * constructs).
 */
