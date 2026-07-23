/* REJECTED (s2). Splitting loads OTHER than the middle one = WORSE than the 7 floor.
 * Measured:
 *   - split load1 (block-local u=arg0[0]) ................... floor 11
 *   - split loads 1&3 (t shared for load2 only) ............. floor 11
 *   - all 3 loads as separate scoped block-locals ........... floor 11
 *   - 3 independent named temps t0/t1/t2 (s1 re-confirmed) .. floor 11
 * Why: with 2+ simultaneously-live single-use load temps, the scheduler hoists all
 * loads to the top (no anti-deps) and the temps scatter across v0/v1/a0/a1, destroying
 * the target load/store interleave. Only splitting the MIDDLE load (load2) keeps the
 * shared t=$loads1&3 and reaches floor 7. Do NOT re-run the non-middle splits.
 *
 * Representative (split loads 1&3) tail:
 *   func_80060A68();
 *   { s32 u = arg0[0]; D_800F1140 = u; }
 *   t = arg0[1]; D_800F1144 = t;
 *   mask = 0x10FFFF; D_800A3464 = mask;
 *   { s32 u = arg0[2]; D_800F1148 = u; }
 */
