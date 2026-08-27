/* REJECTED - reuse-dval-selection-holder-no-coalesce (s8, 2026-08-26)
 *
 * The sanctioned variable-reuse spelling of s8's mode-split lever: instead of a
 * fresh `s32 sel`, borrow the EXISTING s32 local `d_val` (dead after block_15's
 * two shifts) to hold the tail copy's 0x19/0xB selection, then assign it into
 * var_v0_2.  Intent identical to MODE_SPLIT: give the tail copy's constant
 * loads SImode so find_cross_jump PAT-MISMATCHes against the ==5 copy's HImode
 * loads and both copies survive in target's canonical order.
 *
 * MEASURED: 33 / 216 insns (base 28 / 215, MODE_SPLIT 25 / 215).  KILLED.
 * Mechanism of the failure: d_val's pseudo has real conflicts from its earlier
 * live range, so reload does NOT coalesce `var_v0_2 = d_val` onto $v0 - the
 * copy survives as a real instruction (216) and drags the surrounding
 * allocation with it.  The fresh-local spelling coalesces because the
 * intermediate has no other conflicts.
 *
 * Consequence for classification: the variable-reuse family (which would have
 * carried a sanctioned rule + FAKE annotation) CANNOT express this lever here.
 * Any working spelling needs a fresh, conflict-free intermediate.
 */
                            d_val = 0x19;
                            if (var_s1 == 0) {
                                d_val = 0xB;
                            }
                            var_v0_2 = d_val;
                            goto block_48;
