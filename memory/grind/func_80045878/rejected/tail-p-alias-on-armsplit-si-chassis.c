/* func_80045878 — REJECTED (s6). Tail pointer alias `s16 *p = s1;` re-measured
 * on the NEW s6 armsplit+SItemp chassis (not the score-10 HEAD chassis where
 * WIP/s2 first killed it).  Result: sandbox --disable all = 11, build_insns
 * 107 — IDENTICAL to the chassis without the alias.  cse copy-propagates the
 * single-set `p = s1` regardless of the surrounding tail shape, so no
 * `addu v0,s1,zero` base copy materialises.  The base copy is NOT reachable
 * by a C-level pointer copy on any chassis measured to date.
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
        s16 *p = s1;
        s32 t = a0 + 3;
        p[11] = t;
        p[2] = a0;
        p[4] = a1;
        p[10] = a0;
        p[8] = a0;
        *((s32 *) (((s32) p) + 0x18)) = 0x8000;
    }
}
