/* REJECTED (measured 2026-08-25 [s3]): case 3 with the pre-existing volatile
 * qualifier removed —
 *     s32 raw_m1 = ptr[-1];
 * instead of
 *     s32 raw_m1 = *(volatile s32 *)&ptr[-1];
 * Measured: sandbox --disable all = 9, build_insns 245 vs target 248 — the
 * compiler folds 3 instructions the target keeps, so the plain read is NOT the
 * original spelling on this chassis. The qualifier is load-bearing. Whatever
 * ultimately justifies or replaces it must preserve those 3 insns; the fold
 * itself was not diagnosed this session (candidate closed at 0 with the
 * inherited spelling retained).
 */
