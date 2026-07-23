/* func_80045878 (text1a_c.c) — best pure-C form, sandbox --disable all = 10
 * (build_insns 107 vs target 108). This IS the HEAD body; apply verbatim to
 * src/text1a_c.c to resume at the floor. Two coupled residual gaps remain:
 *
 *   Gap A (1 insn): target recomputes `addiu s3,s2,3` (=a0+3) in the else arm
 *     (0x800458F8, last insn before .L800458FC). Our build folds it away via
 *     cse.c (pre-if `s3=a0+3` dominates; a0/s3 unchanged across else calls)
 *     => 107. Arm-split (set s3 in both arms) MATERIALIZES it (107->108) but
 *     sched1 hoists it 3 slots early => score 11. See rejected/
 *     armsplit-s3-materializes-but-sched-early.c.
 *
 *   Gap B (~9 insns): target's final store block copies s1 into a caller-save
 *     base (`addu v0,s1,zero`), stores through v0, uses s2 (=a0) DIRECTLY for
 *     the a0 stores, and holds scratch (a0+3, 0x8000) in v1. Our build stores
 *     directly through s1 with an extra `move v0,s2`. Single-set `s16 *p = s1`
 *     alias is copy-propagated away (WIP-ruled). Mechanism: local-alloc gives
 *     a short-lived tail-base pseudo a caller-save; need s1 kept as the long-
 *     lived record pseudo while a separate tail base materializes.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3 = a0 + 3;
    s32 *v0;
    s16 *s1;
    s32 s0;
    v0 = func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) v0[1];
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
    s1[11] = a0 + 3;
    s1[2] = a0;
    s1[4] = a1;
    s1[10] = a0;
    s1[8] = a0;
    *((s32 *) (((s32) s1) + 0x18)) = 0x8000;
}
