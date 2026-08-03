/* REJECTED (grind s2, 2026-08-03) — every reordering of the Case3 arm's three
 * statements is strictly WORSE than the current order, so statement order is
 * NOT the lever on the residual `addu $a1,$zero,$zero` placement (H5).
 *
 * Baseline (the form in candidate.c), measured `sandbox --disable all` == 2:
 *
 *     Case3:
 *         stat = (s32)(&D_8009B7D0);
 *         s.p_static = (s32 *)stat;
 *         s.pad0C = mid_off;
 *         mid_off = func_80073728(&s, 0);
 *
 * Measured alternatives (all from the floor-2 base unless noted):
 *
 *   score 4 — pad0C store FIRST:
 *         s.pad0C = mid_off;
 *         stat = (s32)(&D_8009B7D0);
 *         s.p_static = (s32 *)stat;
 *         mid_off = func_80073728(&s, 0);
 *     (measured 6 from the floor-4 base as well — consistently worse)
 *
 *   score 4 — p_static store LAST:
 *         stat = (s32)(&D_8009B7D0);
 *         s.pad0C = mid_off;
 *         s.p_static = (s32 *)stat;
 *         mid_off = func_80073728(&s, 0);
 *     (measured 6 from the floor-4 base as well)
 *
 *   score 4 — the D_8009B7D0 address materialised in the PREDECESSOR block
 *     (`stat = (s32)(&D_8009B7D0);` hoisted above the `if (i == last)` test, so
 *     the arm is only [sw p_static, sw pad0C, jal]).  This was the most
 *     promising shape — it removes the la from the block entirely — and it
 *     still does not move the a1 set-up to the front.
 *
 *   score 3 — reversed equality test `if (last == i)` instead of `if (i == last)`.
 *
 *   score 39, insns 136 — Case3 storing the table address DIRECTLY
 *     (`s.p_static = &D_8009B7D0;`, i.e. reverting the s1 shared-`stat` lever
 *     for this one arm while keeping it for S7D8/S800).  Catastrophic: the
 *     shared carrier must cover ALL THREE arms or none.
 *
 * Conclusion: the a1 placement is decided inside one basic block whose
 * instruction set is fixed by the arm's semantics; no permutation of the
 * source statements in that arm reaches target's order.
 */
