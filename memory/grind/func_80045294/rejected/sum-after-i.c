/* s29 KILLED — sum's decl position moved AFTER i=a0 assignment.
 *
 * Baseline: sum, v1, s4, i, count, s5 (position 0) -> score 2.
 * Position 3 (after i): v1, s4, i, sum, count, s5 -> score 4.
 * Position 5 (after s5, s12 result): score 4.
 *
 * Sum's assignment LUID falling AFTER i=a0's LUID demotes sum's pseudo
 * in global_alloc's reg_n_refs priority queue below i's, cascading a
 * two-diff RA rotation. Same LUID/RA coupling as walls (i)/(ii); no
 * novel lever.
 *
 * Free-axis boundary refinement (s29): sum-position is FREE for
 * {0, 1, 2} (before i=a0) and PERTURBING for {3, 4, 5} (after i=a0).
 * Sharpens s12's over-broad "sum position 0 is LOAD-BEARING" claim.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 sum = 0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* rest of body identical to candidate.c */
}
