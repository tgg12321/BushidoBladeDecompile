/* s9 (2026-08-30) REJECTED: carrier = the existing `s0` local. Multi-block, so the mechanism fires and the base DOES get `move v0,s1`; but s0's else-arm live range crosses func_800455AC/func_80044ED8, so the merged allocno is callee-save and both tail scratches come out $s0 instead of $v1. score 7 / 108. The carrier must be call-free. */
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
        p[3] = 0;
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
    s0 = a0 + 3;
    p[11] = s0;
    p[2] = a0;
    p[4] = a1;
    p[10] = a0;
    p[8] = a0;
    s0 = 0x8000;
    *((s32 *) (((s32) p) + 0x18)) = s0;
}
