/* func_80073200 — s4 permuter finding, REJECTED (worse + cheat-smelling).
 * tools/perm_80073200 campaign, output-560-1 (permuter score 560, worse than
 * the 545 the same campaign found from the genuine new_var hoist below).
 *
 * Form: `if (D_800A3580 < 2)` respelled as `if ((D_800A3580 + 1) <= 2)`.
 * This is arithmetic reassociation with NO semantic purpose (adds 1 to both
 * sides of a compare with no behavioral difference) purely to perturb GCC's
 * fold/compare codegen — matches the "explicit-rejection-set-defeat" /
 * opaque-arithmetic-on-a-compare cheat smell (no real value computed, would
 * not be written by a human from the spec, justification is GCC-internal
 * fold-defeat only). Also empirically WORSE (560 vs the 545 permuter-score
 * baseline for the new_var-hoist chassis) — rejected on both grounds.
 *
 * Not applied to src/text1b.c.
 */
if ((D_800A3580 + 1) <= 2) {
    /* ... body unchanged ... */
}
