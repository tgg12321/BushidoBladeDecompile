/* func_80045878 — REJECTED-AS-STANDALONE (score 11 > floor 10), but a real
 * Gap-A mechanism advance and a building block, NOT a dead end.
 *
 * Change vs HEAD: drop the pre-if `s3 = a0+3` init; set s3 in BOTH arms
 *   (taken arm: `s3 = a0 + 3;`  else arm: keeps `s3 = a0 - -3;`).
 *
 * RESULT (sandbox --disable all, s1 2026-07-23):
 *   build_insns 107 -> 108  (MATCHES target 108 — the folded else recompute
 *                            now MATERIALIZES; Gap A insn is present)
 *   score       10  -> 11   (regressed by 1)
 *
 * WHY it regressed: with no pre-if dominating def, the else `addiu s3,s2,3`
 * is a FRESH single-def whose result feeds the very next call arg
 * (`func_8004574C(s3)` -> move a0,s3). sched1 gives it high launch priority
 * and HOISTS it 3 slots EARLY — it lands at the top of the else tail
 * (right after the last call return, before `li v0,-1; sh v0,8(s1);
 * sh zero,6(s1)`), i.e. build offset 2c6c. Target places the recompute LAST,
 * after both stores (0x800458F8, just before .L800458FC). One position
 * mismatch = the +1 diff.
 *
 * In HEAD the else recompute is a REDEFINITION of an already-live s3 (defined
 * pre-if), so anti-deps anchor it — but then cse.c FOLDS it away entirely
 * (both = a0+3, a0 unchanged, s3 callee-save preserved across the else calls)
 * => 107 insns, recompute gone. The tension: HEAD folds it; arm-split keeps
 * it but sched-hoists it. The winning form must keep the recompute AND anchor
 * it last (defeat the sched hoist without a barrier, e.g. make s3 live-in so
 * the else def is a redefinition yet defeat the cse fold structurally).
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
    s1[11] = a0 + 3;
    s1[2] = a0;
    s1[4] = a1;
    s1[10] = a0;
    s1[8] = a0;
    *((s32 *) (((s32) s1) + 0x18)) = 0x8000;
}
