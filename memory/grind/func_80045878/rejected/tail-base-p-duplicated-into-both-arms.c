/* func_80045878 - REJECTED (s7).  `p = s1;` duplicated into BOTH arms of the
 * third if (at the end of each arm), tail through p - the first probe that
 * proved the cse.c:826 canonicality mechanism.
 * MEASURED: score 16, build_insns 108.  The base copy DOES materialise
 * (first time ever), but it lands as `move a0,s1` hoisted above the else
 * arm`s last three stores (which then also address through $a0), and the
 * then-arm gets a SECOND copy where target has a nop.  jump2 cross-jumping
 * does not merge the two copies because sched1 has already hoisted the
 * else-arm one out of the mergeable tail.
 * KILL REASON: two copies + wrong placement.  Superseded by P4, which needs
 * only ONE early mention (in the then arm) and leaves the join copy in
 * target`s exact position.
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
        s1[3] = 0;
        p = s1;
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
        p = s1;
    }
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
