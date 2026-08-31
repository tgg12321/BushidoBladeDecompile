/* DEAD (s11, rederive).  score 13 at 109 insns.
 * The frontier-3 shape: the six tail stores moved into a `static inline`
 * helper, the base still the accepted `v0 = s1;` existing-local reuse, and
 * NO carrier local anywhere -- the two tail scratch values are passed as
 * helper arguments (`a0 + 3` and `0x8000`).
 * WHAT IT PROVES (positive): integrate.c reproduces target's tail
 * INSTRUCTION SEQUENCE AND ORDER EXACTLY with no carrier at all --
 *   move a0,s1 / addiu v0,s2,3 / sh v0,22(a0) / li v0,0x8000 /
 *   sh s2,4(a0) / sh s5,8(a0) / sh s2,20(a0) / sh s2,16(a0) / sw v0,24(a0)
 * against target's identical stream in $v0/$v1.  The base copy SURVIVES cse
 * here (the helper parameter is fed from the multi-set `v0`).
 * WHY IT IS DEAD: the two scratch pseudos created by integrate.c live and die
 * inside the single tail block, so local_alloc (which runs BEFORE global_alloc)
 * hands the first of them $v0; the base allocno `v0` is multi-block, is left
 * to global_alloc, and is pushed to $a0, which also costs an extra
 * `move a0,v0` copy of the first call's return value at function top.
 * Inlining does NOT make a parameter pseudo non-block-local: the argument
 * copies are emitted in the caller's block and the branch-free inlined body
 * is spliced into that same block. */
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
    v0 = s1;
    func_80045878_set(v0, a0 + 3, a0, a1, 0x8000);
}
