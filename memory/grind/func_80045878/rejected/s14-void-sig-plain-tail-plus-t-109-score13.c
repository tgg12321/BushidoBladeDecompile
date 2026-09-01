/* DEAD (s14, solver).  score 13 at 109 insns.  VOID signature.
 * The s12 compliant plain tail (existing-local `v0` reused as the tail base)
 * PLUS the `t = a0 + 3;` named intermediate.  One insn better than s12's
 * 110/14 -- `t` suppresses the HImode cse temp for the three a0 half-word
 * stores -- and its tail instruction STREAM is target's, modulo exactly two
 * register names (base $a0 vs $v0, scratch $v0 vs $v1) plus the `move a0,v0`
 * copy of the first call's return value that the base's $a0 seat forces.
 * WHY DEAD: with a void signature nothing can put hard $v0 across the tail
 * block, so local_alloc gives $v0 to the block-local scratch qtys (measured:
 * blk 13 qty0 reg79 refs2 -> $v0, qty1 reg101 refs2 -> $v0) and global_alloc
 * cannot then seat the multi-block base allocno (pseudo 76, pri 25000) in
 * $v0 -- inverse.py global reported NEGATIVE at depth 2 for that goal.
 * SUPERSEDED by the return-value form (candidate.c): the copy is not a base
 * copy at all, it is the return value. */
/* s14 probe A: compliant plain tail (v0 existing-local base) + named intermediate t */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    s32 t;
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
    t = a0 + 3;
    v0[11] = t;
    v0[2] = a0;
    v0[4] = a1;
    v0[10] = a0;
    v0[8] = a0;
    *((s32 *) (((s32) v0) + 0x18)) = 0x8000;
}
