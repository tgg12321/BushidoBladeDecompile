/* func_80045878 - s8 REJECTED: P4 chassis with the third-if THEN-arm store
 * written through s1 instead of p (so p's early mention is a DEAD store).
 * MEASURED sandbox --disable all = 11, build_insns 107 vs target 108.
 *
 * WHY IT IS DEAD: the copy `p = s1` in the then arm is left with no use in
 * that block, so it never keeps p canonical for cse.c:826 make_regs_eqv at
 * the join; the join copy `addu v0,s1,zero` is copy-propagated away and the
 * build comes out one instruction SHORT of target (107).
 *
 * WHAT IT PROVES (valuable, keep): with the then-arm store through s1 the
 * `beq v1,v0` delay slot at idx 50 comes out as `nop`, EXACTLY as target has
 * it -- residual R1' is CLOSED by this spelling.  On P4 (store through p)
 * reorg.c fills that slot with `move a0,s3`.  So R1' and R2 are ANTI-COUPLED
 * through one C decision: the base of the then-arm store.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s32 *v0;
    s16 *s1;
    s16 *p;
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
        p = s1;
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
    p = s1;
    {
        s32 t = a0 + 3;
        p[11] = t;
    }
    p[2] = a0;
    p[4] = a1;
    p[10] = a0;
    p[8] = a0;
    *((s32 *) (((s32) p) + 0x18)) = 0x8000;
}
