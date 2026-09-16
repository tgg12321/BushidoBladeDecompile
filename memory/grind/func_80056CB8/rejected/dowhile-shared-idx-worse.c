/* REJECTED s35 (structural modality, 2026-09-16).
 * Frontier item from s34: re-run the previously-killed shared single-idx
 * index-naming spelling on the do-while chassis (195 real insns) instead
 * of the for-loop chassis it was originally killed on.
 *
 *   i = start;
 *   do {
 *       s32 idx = i * 2;
 *       flags = (&D_8009A821)[idx] << 8;
 *       ...
 *       scale = (&D_8009A820)[idx] << 8;
 *       ...
 *       i++;
 *   } while (i < limit);
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 38 -> 66/204,
 * build_insns 195 (do-while-alone) -> 197 (+2).
 *
 * Same +2 build_insns delta as every for-loop-chassis idx-sharing spelling
 * (s6/s7/s10/s18/s21/s22/s27/s33 all measured +2..+3 on that chassis).
 * The do-while chassis's lower baseline insn count does NOT change the
 * outcome of this axis -- sharing the i*2 computation into one local still
 * costs +2 real instructions regardless of chassis. Combined with the
 * idxB variant below (identical 197 build_insns), this closes the
 * index-naming axis on BOTH chassis shapes now (8 total spellings killed:
 * 6 on for-loop + 2 on do-while).
 *
 * Reverted via `git checkout -- src/text1b.c`.
 * KILLED instance. kill_scope: instance. measured_on: s35 chassis
 * (do-while chassis body from rejected/do-while-loop-rewrite-worse.c +
 * shared `s32 idx = i * 2;` local consumed at both table reads +
 * func_80053614 s32-return fix + header externs, no FAKE constructs).
 */
