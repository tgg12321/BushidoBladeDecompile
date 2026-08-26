/* REJECTED s47: score 11 (H1 basin, unchanged). Probe of the ra_solver inverse
 * vector #1 ([refs_up] pseudo 72: refs 3->4) on the H1 (i-before-v1) chassis.
 *
 * Rationale: under H1 the sched2 order is target-exact but global.c rotates the
 * callee-save allocation because cse.c substitutes the ashift operand
 * (reg 72 = a0) -> (reg 75 = i), dropping a0's reg_n_refs from 4 to 3.
 * ra_solver/inverse.py says a single atom (a0 refs 3->4) restores target's
 * allocation exactly.  This form tries to supply that 4th reference by
 * spelling the SECOND loop's index shift on a0 instead of on i
 * (`v1 = a0 << 4;` in place of `v1 = i << 4;`), on the theory that the second
 * loop's block is a different cse basic block.
 *
 * MEASURED: score stays 11.  cse.c substitutes a0 -> i in the second block as
 * well (i = a0 immediately dominates the shift there too, same BB), so the
 * reference is stolen a second time and reg_n_refs(72) stays 3.
 *
 * Conclusion banked: every a0 reference in this function lies inside the cse
 * equivalence region created by `i = a0`, so no pure-C spelling under the H1
 * order can hold a0 at 4 references.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;                 /* H1 order: i before v1 (target sched2 order) */
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... first loop identical to candidate.c ... */
    /* second loop head:  i = a0;  if (i < D_800A33AC) { v1 = a0 << 4; ... } */
}
