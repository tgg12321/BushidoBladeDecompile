/* REJECTED-BUT-INSIGHT s2 — Variant G/I: walking-pointer (`*arg0++`)
 * for the 3 post-call loads.
 * Sandbox --disable all = 7 (LOWER than pin-free floor 9), BUT
 * build_insns=44 vs target 43 — the extra addiu is intrinsic to
 * `*arg0++` semantics and cannot be optimized away. Architecturally
 * blocked from reaching sandbox=0.
 *
 * WHY IT LOWERS THE SCORE: the walking pointer serializes the loads
 * on the pointer-dep chain, changing the RA priority surface. In the
 * emitted asm the load-temp lands in $v0 (matching target's direction)
 * and the mask in $v1 — several register-name diffs disappear vs the
 * pin-free floor. Net: 6 register/reordering diffs + 1 insn-add = 7.
 *
 * WHY IT IS NOT COMMITTABLE / CANDIDATE-WORTHY: target has 43 insns
 * with NO pointer bumps (`lw $v0, 0x0/0x4/0x8($s0)`). A form that
 * introduces an insn that has no counterpart in target cannot reach
 * sandbox 0.
 *
 * LEVER LESSON: the RA tiebreaker between the load-temp and the
 * 2-insn mask IS movable by reshaping the load side's pseudo class
 * (walking-pointer form shifts load-temps from array-indexed
 * to serialized single-def) — but the specific shape target used
 * still needs to be identified. This confirms the tiebreak is
 * NOT insurmountable; the search space for the correct target-shape
 * C is smaller than the ledger implied. */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    D_800F1140 = *arg0++;
    D_800F1144 = *arg0++;
    D_800A3464 = 0xFFFFEF;
    D_800F1148 = *arg0;
}
