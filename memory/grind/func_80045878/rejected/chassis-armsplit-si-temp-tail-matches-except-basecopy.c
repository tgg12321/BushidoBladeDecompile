/* func_80045878 — s6 CHASSIS "armsplit+SItemp".  NOT a match (sandbox
 * --disable all = 11, build_insns 107 vs target 108) but the STRUCTURALLY
 * CLOSEST form found so far, and the chassis every future session should
 * start from instead of the score-10 candidate.c body.
 *
 * WHY it beats the score-10 body structurally (s6 measurement):
 *   - arm-split (drop the pre-if `s3 = a0+3`, set s3 in BOTH arms) keeps the
 *     else-arm recompute alive (cse can no longer fold it): +1 insn, and
 *     insns 0..28 come out BYTE-IDENTICAL to target, including target's
 *     `addiu s3,s2,3` in the beqz delay slot at 0x800458AC.
 *   - the SI temp `s32 t = a0 + 3; s1[11] = t;` stops the expander from
 *     truncating a0 to HImode before the add, which is what created the
 *     shared (HI)a0 pseudo that cse then reused for the three `s1[N] = a0`
 *     stores.  With the temp, the tail now stores s2 DIRECTLY — exactly like
 *     target:
 *         ours    addiu v0,s2,3 / sh v0,22(s1) / li v0,0x8000 /
 *                 sh s2,4(s1) / sh s5,8(s1) / sh s2,20(s1) / sh s2,16(s1) /
 *                 sw v0,24(s1)
 *         target  move v0,s1 / addiu v1,s2,3 / sh v1,22(v0) / li v1,0x8000 /
 *                 sh s2,4(v0) / sh s5,8(v0) / sh s2,20(v0) / sh s2,16(v0) /
 *                 sw v1,24(v0)
 *     i.e. the ENTIRE remaining tail residual is now ONE missing insn — the
 *     base copy `addu v0,s1,zero` — plus the renames that follow from it.
 *     (s2 measured the SI temp on the score-10 HEAD chassis, saw 107->106,
 *     and filed it as "wrong direction".  On the arm-split chassis it is the
 *     right direction; that mis-scoping cost sessions s3-s5.)
 *
 * Remaining residual on this chassis (2 regions, both measured s6):
 *   R1 else-arm rotation: ours `addiu s3,s2,3; li v0,-1; sh v0,8(s1);
 *      sh zero,6(s1)` vs target `li; sh; sh; addiu`.  sched1 tie-break, see
 *      evidence.md s6 for the sched.c foreclosure proof.
 *   R2 the missing `addu v0,s1,zero` base copy.  A plain `s16 *p = s1;`
 *      alias over the tail is copy-propagated on THIS chassis too (measured
 *      s6: still 107 / score 11) — banked in
 *      rejected/tail-p-alias-on-armsplit-si-chassis.c.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s32 *v0;
    s16 *s1;
    s32 s0;
    v0 = func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) v0[1];
        s3 = a0 + 3;
    } else {
        s1 = (s16 *) func_800455AC(a0);
        saSeMain_80045600(a0, 0x1A88 + ((s32) s1));
        saTan5TakeGetPos_80045230(0);
        saTan5TakeGetPos_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
        s3 = a0 - -3;
    }
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && (s1[3] != (-2))) {
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
            saTan5TakeGetPos_80045230(s0);
        }
        saSeMain_80045600(s3, s0);
        saTan5TakeGetPos_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    {
        s32 t = a0 + 3;
        s1[11] = t;
    }
    s1[2] = a0;
    s1[4] = a1;
    s1[10] = a0;
    s1[8] = a0;
    *((s32 *) (((s32) s1) + 0x18)) = 0x8000;
}
