/* REJECTED (s4, permuter): directed PERM chassis over BOTH residual regions.
 * PERM_LINESWAP+PERM_GENERAL exhaustively enumerate every ordering of the
 * Gap-A else tail and the 6 Gap-B tail stores (~26k combos, 21.5k iters, -j8).
 * RESULT: plateau permuter-score 210 (== plain arm-split), NO zero, NO
 * sub-plateau find. C-statement ordering/spelling is not the lever for either
 * residual; both are cc1 local-alloc/sched1 tie-breaks below permuter's reach.
 * This is the arm-split family (base already rejected as
 * armsplit-s3-materializes-but-sched-early.c) wrapped in directed macros. */
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
        PERM_LINESWAP(
        s1[4] = -1;
        s1[3] = 0;
        s3 = PERM_GENERAL(a0 - -3, a0 + 3, (a0 + 1) + 2);
        )
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
    PERM_LINESWAP(
    s1[11] = PERM_GENERAL(a0 + 3, s3);
    s1[2] = a0;
    s1[4] = a1;
    s1[10] = a0;
    s1[8] = a0;
    *((s32 *) (((s32) s1) + 0x18)) = 0x8000;
    )
}
void tslFileClose(s32 a0, s32 a1) {
    saSeMain_80045510(a0 + 3, a1);
    saTan5TakeGetPos_80045230(0);
}
extern void GetAllocPacketSize(void);
extern void func_800456F0(s32);
void func_80045A50(s32 a0) {
    s32 a0p3 = a0 + 3;
    GetAllocPacketSize();
    func_800456F0(a0p3);
    func_800456F0(a0);
    func_800453E0(a0p3);
    func_800453E0(a0);
}
extern void func_80044100(s32, s32);
extern void saFidLoad(s32, s32);
