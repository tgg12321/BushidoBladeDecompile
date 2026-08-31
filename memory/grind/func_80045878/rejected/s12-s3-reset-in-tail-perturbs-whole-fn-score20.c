/* DEAD (s12, structural).  score 20 at 108 insns (chassis: candidate.c = 0).
 *
 * SHAPE: keep candidate.c verbatim except the tail's a0+3 scratch, which is
 * carried by RE-SETTING the existing local `s3` in the join/tail block
 * (`s3 = a0 + 3; v0[11] = s3;`) instead of by the fresh carrier `c`.  `c`
 * then carries only 0x8000 (two writes instead of three).  This is ordinary
 * C -- s3 already holds a0+3, the re-store is a real value that is read --
 * so it would have been fully Judge-compliant had it measured.
 *
 * WHY KILLED: s3's allocno crosses 3 calls (lreg dump: "Register 78 used 9
 * times across 21 insns; crosses 3 calls"), so global_alloc seats it
 * callee-save; adding a tail set does not shorten the live range (GCC 2.7.2
 * has no live-range splitting -- one allocno per pseudo per function), it
 * only lengthens it to the last insn.  The extra reference re-ranks s3
 * against a0/a1/s1 in global_alloc's priority order and the damage is not
 * local to the tail: the score goes 0 -> 20, i.e. the re-set perturbs seats
 * across the whole body, not just the two tail insns.
 *
 * GENERAL LESSON (banked in evidence.md [s12]): the tail scratch carrier
 * cannot be ANY existing local of this function.  Per the s12 lreg census
 * exactly two user pseudos are call-free -- reg 76 (`v0`) and reg 79 (the
 * fresh carrier `c`); every other user variable (a0/a1/a2/s0/s1/s3) crosses
 * 3-14 calls and is therefore seated callee-save.  v0 is already consumed by
 * the tail base copy.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    s32 c;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        func_80045600(a0, 0x1A88 + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && ((c = s1[3]) != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    v0 = s1;
    s3 = a0 + 3;
    v0[11] = s3;
    v0[2] = a0;
    v0[4] = a1;
    v0[10] = a0;
    v0[8] = a0;
    c = 0x8000;
    *((s32 *) (((s32) v0) + 0x18)) = c;
}
