/* DEAD (s11, rederive).  score 9 at 107 insns.
 * Same inline helper as the -109- form but the base is passed directly
 * (`func_80045878_set(s1, ...)`) instead of through the reused `v0`.
 * cse folds the integrate.c parameter copy away and all six stores address
 * through $s1: the base copy insn disappears entirely (107 vs target 108).
 * Confirms that an inline helper cannot manufacture a surviving base copy. */
/* I1: static inline tail helper - integrate.c parameter pseudos as the
 * non-block-local anchor; no source-level multi-write local at all. */
static inline void func_80045878_set(s16 *p, s32 hi, s32 id, s32 kind, s32 flags) {
    p[11] = hi;
    p[2] = id;
    p[4] = kind;
    p[10] = id;
    p[8] = id;
    *((s32 *) (((s32) p) + 0x18)) = flags;
}

void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
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
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }

    func_80045878_set(s1, a0 + 3, a0, a1, 0x8000);
}
