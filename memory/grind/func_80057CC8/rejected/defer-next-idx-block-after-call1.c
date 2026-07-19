/* REJECTED s3 2026-07-19 — KILLED-HARD (score 3 -> 41, build_insns 113).
 * Moving the next_idx block (s32 tmp = arg1+1; ...) AFTER the first jal
 * shortens prev_idx's lifetime but destroys the delay-slot fill: target
 * packs the prev_idx sll into the next_idx bnez delay slot at 80057D48,
 * which requires next_idx to be computed BEFORE call1.
 * Structural bound: next_idx block MUST be before p1's addu.
 * Confirms and extends s2's swap-if-block-order kill (score 31) and
 * hoist-p1-before-nextidx-block kill (score 12).
 * Do NOT re-derive.
 */
