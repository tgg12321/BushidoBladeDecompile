/* s66 F3 (structural) — compound dual-collapse KILLED at masked=16 (+14 vs h5 baseline of 2).
 * Hypothesis (from s65 frontier F3): simultaneously multi-set-force BOTH sides
 *   (t0-side: `t0 <<= 2` in-place → p101 multi-set → insn 111 NON-LAUNCH)
 *   (v0-side: `v0 = v0 + tbl` in-place → p107 multi-set → insn 121 NON-LAUNCH)
 * would produce symmetric priority cancellation at clock=13 — ready queue has
 * NO LAUNCH-priority candidates in the window, scheduler falls back to normal-
 * priority ordering, potentially reaching a novel low-launch basin.
 *
 * Result: +14 regression to masked=16 (worse than either single collapse at
 * +13 per s65 P1/P2). Compositional symmetry does NOT cancel; dual-non-LAUNCH
 * lands in a strictly WORSE basin than either single collapse. Confirms the
 * +13 collapse basin is single-side-driven, and the two collapses are
 * ADDITIVE in the wrong direction (dual-non-LAUNCH shifts BOTH insn slots
 * into normal-priority ordering territory, misplacing both).
 *
 * The h5 masked=2 basin genuinely requires:
 *   - insn 111 (t0*4 SLL) to LAUNCH  (mult-expander p106 fresh-single-set path)
 *   - insn 121 (arg5-addr PLUS) to LAUNCH  (v0+tbl compound p107 fresh-single-set path)
 *   - LUID tiebreak within the LAUNCH set to place 121 before 111 (residual pair swap).
 * Any structural mutation that removes LAUNCH from EITHER insn drops out of h5.
 */
    t0 <<= 2;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    v0 = v0 + (s32)tbl_125c;
    arg5 = *(s32 *)v0;
