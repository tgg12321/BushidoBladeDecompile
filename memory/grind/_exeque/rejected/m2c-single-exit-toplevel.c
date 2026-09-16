/* REJECTED s11 (rederive) — fresh m2c decompile reconstructs the WHOLE
 * function with a single-exit-point accumulator shape instead of the
 * early `if (guard) return 1;`:
 *
 *   s32 ret = 1;
 *   if (!(*D_8009BF54 & 0x01000000)) {
 *       ... (existing body, `ret = (...)` at the tail instead of `return`)
 *   }
 *   return ret;
 *
 * Applied to the s4-s10 floor-2/187 chassis (only the top-level guard +
 * return restructured, do-while(0) wraps and final block unchanged):
 * sandbox score 7/187 (build_insns 188, WORSE than the banked 2/187 at
 * build_insns 186). KILLED instance. The early-return two-exit-point
 * form is load-bearing.
 */
