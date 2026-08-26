/* func_80045878 - REJECTED (s7).  Reuse the existing local `v0` (the
 * func_8004574C result) as the tail base, to give the tail-base pseudo an
 * EARLY regno_first_uid so cse.c:826 keeps the join copy.
 * MEASURED: score 15, build_insns 109 vs target 108.  The tail came out as a
 * pure rename of target ($a0<->$v0, $v0<->$v1) - so the cse mechanism WORKS -
 * but the reuse costs a whole extra instruction at the top (`move a0,v0` at
 * idx 12) because the v0 pseudo now spans the call-result region AND the
 * tail, and $v0 is hard-conflicted at the call return, so global.c gives the
 * whole pseudo $a0 and has to copy the return value into it.
 * KILL REASON: any variable that is genuinely live early either crosses calls
 * (-> prune_preferences strips its $v0 preference, global.c:897) or overlaps
 * the call-return hard reg (-> hard conflict with $v0).  ra_solver
 * `inverse.py global --goal {"76": 2} --depth 2` returned FORECLOSED.
 * Superseded by rejected/chassis-p4-thenarm-p-tail-pure-rename.c, which gets
 * the same tail at 108 insns.
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
    v0 = (s32 *) s1;
    {
        s32 t = a0 + 3;
        ((s16 *) v0)[11] = t;
    }
    ((s16 *) v0)[2] = a0;
    ((s16 *) v0)[4] = a1;
    ((s16 *) v0)[10] = a0;
    ((s16 *) v0)[8] = a0;
    *((s32 *) (((s32) v0) + 0x18)) = 0x8000;
}
