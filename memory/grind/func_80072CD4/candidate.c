/* func_80072CD4 - s16 (2026-09-04, synthesis). MEASURED THIS SESSION with this exact text in
 * src/text1b.c: `sandbox func_80072CD4 --disable all` = 0, build_insns 79 == target_insns 79,
 * rules_dropped 0. Disassembly byte-for-byte against asm/funcs/func_80072CD4.s:
 * tmp/grind/func_80072CD4/s16/A2_score0.dis.
 *
 * *** NOT SUBMITTED. *** This body writes `*(u8 *)(arg1 + 4) = red;` and
 * `*(u8 *)(arg1 + 0xC) = red;` in BOTH arms of the inner `if`, which is state.json
 * banned_constructs entry 5 and judge_constraints entry 1 (dup4_0xc_into_arms). s16 returned
 * `ruling-request`, not `candidate-ready`; the ban-free 2/79 form is kept in
 * memory/grind/func_80072CD4/fallback_banfree_2_79.c and remains the standing best if the
 * ruling refuses this one.
 *
 * WHY THIS SESSION RE-OPENED THE QUESTION (three facts measured in s16, none of them available
 * to the 2026-07-24 judge ruling that installed the ban):
 *
 * (1) THE MECHANISM IS NOT A STORE-SCHEDULE LEVER. The 2026-07-24 constraint refused the
 *     construct as a "store-SCHEDULING-order duplication" outside the duplicated-statement
 *     sanction's scope. tools/sched_solver, run for the first time on the 2/79 natural-arm-E
 *     chassis (tmp/grind/func_80072CD4/s16/perturb_naturalE_depth2.txt), reports for BOTH
 *     scheduler passes: `align honobj->tgtobj: |A|=79 |B|=79 {equal: 78, replace: 0, delete: 0,
 *     insert: 0, moved: 1}` and NOT ONE block whose scheduled order differs from target. Both
 *     scheduler passes already emit target's order everywhere. The single misplaced instruction
 *     is placed by jump.c's cross_jump, which puts its merge LABEL immediately before the common
 *     tail. That is the mechanism the sanctioned family's own scope sentence names verbatim:
 *     "incl. when cross-jump re-merges the copies to identical bytes".
 *
 * (2) THE COMPILER PROVES THE ORIGINAL SOURCE WROTE THESE STORES INSIDE THE ARMS. Target's arm 1
 *     ends `j .L80072D64 / addiu $v0,$zero,0x32` - a constant load with no in-block consumer.
 *     GCC 2.7.2's sched1 cannot leave such an insn at an arm-block tail: schedule_select
 *     (tools/gcc-2.7.2/sched.c:2706) breaks the equal-INSN_PRIORITY tie with potential_hazard
 *     (sched.c:2762), and potential_hazard is 0 for an insn with insn_unit == -1 (a `li`) and
 *     non-zero for every `sb` (mips.md:161 puts stores on the "memory" unit), so - scheduling
 *     bottom-up - every store is selected before the load and the load lands at the block HEAD.
 *     Measured on the plain-carrier chassis this session: cc1 dump insn 57 (`(reg/v:SI 75) =
 *     (const_int 50)`) sits at the arm TAIL in .combine and at the arm HEAD in .sched
 *     (tmp/grind/func_80072CD4/s16/f.combine, f.sched), and the build is 10/78 with red in $a0
 *     and blue in $v1. The ONLY thing that keeps such a load at the tail is a real in-block
 *     successor at sched1; s15 measured that every zero-byte successor (copy chains, self-assign,
 *     dead param store) is folded away before sched1. The one successor that survives sched1 and
 *     still costs no final bytes is a STORE that jump2's cross_jump later merges into the join -
 *     i.e. the arms of the original source ended with these stores. This body is that source.
 *
 * (3) THE FORM IS NOW ORDINARY, SYMMETRIC C. Each arm writes its own complete vertex-0 and
 *     vertex-1 RGB triple in canonical ascending field order (4,5,6,0xC,0xD,0xE) - textually the
 *     same shape as the outer else-arm ten lines below it, which writes exactly those six offsets
 *     in exactly that order. There is no invented local, no holder, no do-while(0), no
 *     annotation, no per-arm asymmetry and no intent-announcing name. The earlier banned spelling
 *     (rejected/dup4_0xc_into_arms.c) carried an `int fc_const` holder and injected the two
 *     stores as a bare `@4=fc_const; @0xC=fc_const;` pair after the per-arm values; this body has
 *     neither. The odd-looking-order variant (stores at 5,6,0xD then 4,0xC,0xE) also measures
 *     0/79 and is banked at tmp/grind/func_80072CD4/s16/A1_dup_arms_clean.c.
 *
 * SOTN PRECEDENT for the family, already in hand: memory/grind/func_80072CD4/census-2026-09-01.md
 * (owner's Ruling-A probe, executed s13) - full brace-aware scan of all 1224 US-PSX .c files at
 * sotn-decomp pin aa53500226ee84be763f3e8702b27de06456b3a7, 8 manually verified exhibits in 4
 * fully-matched files, e.g. src/boss/rbo5/doors.c:921 / :928.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
