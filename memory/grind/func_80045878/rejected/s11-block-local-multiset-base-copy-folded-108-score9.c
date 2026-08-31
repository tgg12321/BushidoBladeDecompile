/* DEAD (s11, rederive).  score 9 at 108 insns.  MECHANISM PROBE ONLY -- it
 * carries a fresh multi-write local and is not shippable under the
 * 2026-08-30 Judge constraint; it was measured to answer a mechanism
 * question, not as a candidate.
 * QUESTION: local-alloc.c's qty_compare priority is
 * log2(n_refs)*n_refs*size/(death-birth), so a BLOCK-LOCAL base copy (7 refs
 * over ~9 insns, pri ~15555) outranks a block-local scratch (2 refs over 2
 * insns, pri ~10000) and would take $v0 first, leaving $v1 for the scratch --
 * i.e. if the base could be block-local AND survive, the scratches would not
 * need to be non-block-local at all.
 * ANSWER: unconstructible.  A fresh base local written twice from the SAME
 * source (`p = s1; ... p = s1;`) has both copies folded by cse and the second
 * set deleted as a dead store; all six stores address through $s1 and an
 * unrelated `move v0,s2` appears.  A surviving base copy requires the two
 * sets to carry DIFFERENT values in DIFFERENT blocks -- which is exactly the
 * multi-block `v0` reuse, and multi-block means global_alloc, which is the
 * configuration that loses $v0 to the block-local scratch. */
/* P4 MECHANISM PROBE (not shippable): block-local multi-set base */
void func_80045878_set(s16 *p, s32 hi, s32 id, s32 kind, s32 flags) {
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
    s16 *p;
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
    p = s1;
    p[11] = a0 + 3;
    p = s1;
    p[2] = a0;
    p[4] = a1;
    p[10] = a0;
    p[8] = a0;
    *((s32 *) (((s32) p) + 0x18)) = 0x8000;
}
