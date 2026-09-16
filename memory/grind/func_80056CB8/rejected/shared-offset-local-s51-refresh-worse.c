/* REJECTED — func_80056CB8, s51 (2026-09-16). KILLED (instance), not a cheat —
 * re-measurement of the s6 "shared idx local" form on the CURRENT (38/204)
 * chassis, per the ledger's mandatory kill re-audit instruction. This is the
 * exact same spelling as memory/grind/func_80056CB8/rejected/shared-idx-local-worse.c
 * (s6, measured 81->90 on a since-superseded chassis) and the s12/s13
 * "fresh-int" member of the "one C handle carries i*2" family (candidate.c
 * ~line 1060, closed across s11/s12's floor-48/58 chassis). The s50 frontier
 * item had re-proposed it as "untried" -- it is not untried, but it HAD never
 * been re-measured on the current 38-floor chassis, so this session did that
 * re-measurement rather than skip it on the strength of the stale write-up.
 *
 * Measured: score REGRESSED 38 -> 51 (build_insns 198 -> 200) on the current
 * s22-s50-banked candidate.c chassis. Reverted with `git checkout -- src/text1b.c`;
 * clean revert confirmed.
 *
 * Do not re-propose this exact spelling on ANY future chassis without first
 * checking this file and shared-idx-local-worse.c/rejected/loop-carried-
 * pointer-walk-worse.c -- all four "share i*2" spellings (fresh-int,
 * loop-carried-int, fresh-pointer, loop-carried-pointer) are now measured
 * KILLED across three chassis generations (s6 @81, s11-13 @48-58, s51 @38).
 *
 * --- the only changed lines vs candidate.c, in context ---
 *
 *     s32 start;
 *     s32 limit;
 *     s32 i;
 *     s32 off;                         (new local, once-written per iter)
 *     ...
 *     for (i = start; i < limit; i++) {
 *         ...
 *         off = i * 2;
 *         flags = D_8009A821[off] << 8;
 *         ...
 *         scale = D_8009A820[off] << 8;
 *         ...
 *     }
 */
