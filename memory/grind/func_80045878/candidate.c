/* func_80045878 (src/text1a_c.c) -- BEST FORM, s9 (2026-08-30).
 * sandbox --disable all = 4, build_insns 108 == target 108.
 * This SUPERSEDES the s7 "P4" chassis (score 12) and the old floor-10 body:
 * the honest floor moved 10 -> 4 this session, the first movement since s0.
 *
 * WHAT CHANGED vs the s7 P4 chassis (two edits, both ordinary C):
 *   (1) the third if's last condition operand is read into a named local:
 *         ... && ((c = s1[3]) != (-2))
 *   (2) the tail's two scratch values are carried in that SAME local c:
 *         c = a0 + 3;  p[11] = c;   ...   c = 0x8000;  *(p+0x18) = c;
 *
 * WHY IT WORKS (measured, not guessed -- cc1 -dS trace in
 * tmp/grind/func_80045878/s9/, sched.c line numbers from tools/gcc-2.7.2):
 *   The s8 entry in docs/grind/decisions.md claimed a closed-form
 *   contradiction: "the tail base pseudo is cse-canonical iff multi-block and
 *   can win $v0 iff single-block".  That claim is now DISPROVEN.  The base can
 *   also win $v0 from global.c -- but only if NO tail-block-local pseudo has
 *   already taken $v0 in local_alloc.  local-alloc.c:472 only claims pseudos
 *   with reg_basic_block[i] >= 0, so giving the two tail scratches a mention
 *   in an EARLIER basic block (here: the condition read) makes them
 *   REG_BLOCK_GLOBAL, local_alloc claims nothing in the tail, pseudo 78 loses
 *   its hard conflict with hard reg 2, and global.c hands the base $v0 and the
 *   scratch $v1 -- target's exact assignment.
 *   The carrier must additionally NEVER BE LIVE ACROSS A CALL, or the merged
 *   allocno is callee-save: reusing `s0` (whose else-arm range crosses calls)
 *   gives $s0 and scores 7 (rejected/tail-carrier-s0-callee-save-score7.c);
 *   reusing `v0` (the call-return local) costs an extra `move v1,v0` at the
 *   top, 109 insns, score 7 (rejected/tail-carrier-v0-return-copy-109.c).
 *   The two condition temps are the only call-free multi-block anchors in the
 *   function; s1[4]'s temp is $v0 in target, so the carrier must be s1[3]'s
 *   (target: `lh v1,0x6(s1)` -- already $v1).  Using s1[4]'s scores 6
 *   (rejected/tail-carrier-cond-s1x4-wrong-seat-score6.c).
 *
 * RESIDUAL (4 words, both pure ORDERING, no register or shape diffs left):
 *   idx 29<->32  ours `addiu s3,s2,3` first, target last, in the first-if
 *                else-arm block (the long-standing "Gap A" placement).
 *   idx 89<->90  ours `addiu v1,s2,3 ; move v0,s1`, target `addu v0,s1,zero ;
 *                addiu v1,s2,0x3`.
 *   MECHANISM NAMED THIS SESSION (sched.c:2543 adjust_priority ->
 *   sched.c:2570 birthing_insn_p): the scheduler runs BACKWARD; when insn 228
 *   (`sh v1,22(v0)`) is scheduled it is temporarily given LAUNCH_PRIORITY
 *   (sched.c:187 = 0x7f000001, assigned at sched.c:4049), and adjust_priority
 *   propagates that maximum to any newly-ready predecessor that is a
 *   "birthing" insn.  birthing_insn_p returns true only when the destination
 *   is live AND `reg_n_sets[dest] == 1`.  The base copy `p = s1` is a
 *   single-set pseudo -> bumped to 0x7f000001 -> scheduled at T-8; the scratch
 *   `c = a0+3` is a multi-set pseudo (that is the price of the carrier) ->
 *   priority 1 -> scheduled at T-9 -> EMITTED FIRST.  The dump line is
 *   `;; ready list at T-8: 222 (1) 225 (7f000001), now 225 222`.
 *   To flip it, either the base must stop being single-set (measured:
 *   routing the else arm's record stores through p costs an insn and scores
 *   20 -- rejected/p-set-in-else-arm-score20.c) or the scratch must become
 *   single-set (which re-blocks it as tail-block-local).  This is the next
 *   session's target and it is a SCHEDULER question, not an RA question.
 *
 * Apply verbatim over the INCLUDE_ASM line in src/text1a_c.c.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s32 *v0;
    s16 *s1;
    s16 *p;
    s32 s0;
    s32 c;
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
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && ((c = s1[3]) != (-2))) {
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
    c = a0 + 3;
    p[11] = c;
    p[2] = a0;
    p[4] = a1;
    p[10] = a0;
    p[8] = a0;
    c = 0x8000;
    *((s32 *) (((s32) p) + 0x18)) = c;
}
