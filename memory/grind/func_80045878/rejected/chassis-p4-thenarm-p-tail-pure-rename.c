/* func_80045878 - s7 CHASSIS "P4 / then-arm-p".  NOT a match (sandbox
 * --disable all = 12, build_insns 108 vs target 108) but by far the CLOSEST
 * form ever produced, and the chassis every future session must start from
 * (it supersedes the s6 "armsplit+SItemp" chassis).
 *
 * Change vs the s6 armsplit+SItemp chassis: introduce `s16 *p;` and
 *   (a) in the THEN arm of the third if, write the record store through it
 *       (`p = s1; p[3] = 0;`) - this is p's EARLY mention;
 *   (b) at the join, `p = s1;` and run the whole tail through p.
 *
 * MECHANISM (cse.c:826 make_regs_eqv, read from source this session).  When
 * cse sees the join copy `p = s1`, p becomes the CANONICAL register of the
 * equivalence class only if
 *     (uid_cuid[regno_last_uid[p]]  > cse_basic_block_end
 *   || uid_cuid[regno_first_uid[p]] < cse_basic_block_start)
 *   && uid_cuid[regno_last_uid[p]]  > uid_cuid[regno_last_uid[s1]]
 * A FRESH `s16 *p = s1;` at the join fails BOTH halves of the first clause
 * (its first and last mention are inside the tail block), so s1 stays
 * canonical, every use of p is rewritten to s1 and the copy dies - which is
 * exactly why the s2 and s6 pointer-alias probes measured byte-identical
 * output.  Giving p a mention in an EARLIER basic block satisfies
 * `regno_first_uid[p] < cse_basic_block_start` (regno_first_uid comes from
 * reg_scan, which runs BEFORE cse), p becomes canonical, and the join copy
 * SURVIVES.  The then-arm copy itself is propagated away and deleted, so it
 * costs no instruction.
 *
 * RESULT (s7 2026-08-26): build_insns 108 == target 108, and the alignment is
 * index-for-index with EXACTLY two divergences left:
 *   R2 (idx 89-97) - the tail is now a PURE REGISTER RENAME of target:
 *        ours    move a0,s1 / addiu v0,s2,3 / sh v0,22(a0) / li v0,0x8000 /
 *                sh s2,4(a0) / sh s5,8(a0) / sh s2,20(a0) / sh s2,16(a0) /
 *                sw v0,24(a0)
 *        target  move v0,s1 / addiu v1,s2,3 / sh v1,22(v0) / li v1,0x8000 /
 *                sh s2,4(v0) / sh s5,8(v0) / sh s2,20(v0) / sh s2,16(v0) /
 *                sw v1,24(v0)
 *      i.e. base pseudo $a0 must become $v0 and scratch pseudo $v0 must
 *      become $v1.  Nine instructions, same order, same shapes.
 *   R1' (idx 50) - ours fills the `beq v1,v0` delay slot with `move a0,s3`
 *      (duplicated from idx 53); target leaves a `nop` there.  reorg.c.
 *
 * RA GROUND TRUTH (tools/ra_solver, model validated 8/8 dispositions on THIS
 * body; local_alloc has ZERO qty rows for this function, so every pseudo is
 * global.c's):
 *      TRACE a=78 pri=30000 calls=0 hard_conf=[2,29] someone=[] best=4  prefs=[4,5]
 *      TRACE a=76 pri=10000 calls=0 hard_conf=[29]   someone=[] best=2  prefs=[2,4,5]
 *   pseudo 78 is the tail base p, pseudo 76 the tail scratch.  p is allocated
 *   FIRST (higher priority) and cannot take $v0 because it carries a HARD
 *   conflict with hard reg 2 ($v0); it then takes $a0 off its preference list
 *   [$a0,$a1].  The scratch takes $v0 afterwards.  Both of those inputs must
 *   move for target's assignment; see hypotheses.md [s7] for the foreclosure.
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
