/* saEft01Init — best form as of grind session 2 (structural).
 *
 * Honest pure-C distance (sandbox --disable all): 18   [floor UNCHANGED from
 * sessions 1-2 baseline, which was also 18]
 * Instruction count: 92 build vs 91 target.
 *
 * WHY THIS IS THE CANDIDATE DESPITE AN UNCHANGED SCORE:
 *   CLUSTER A IS SOLVED.  This form produces target's exact callee-save map
 *       $s0 = D_800A125C (tbl_125c)   $s1 = &D_800A1494 (idx_1494)
 *       $s2 = the a0 param            $s3 = D_800A11DC  (tbl_11dc)
 *   using ONLY $s0-$s3 (no $s4/$s5), which is what the whole 3-way rename
 *   cluster in regfix.txt:97-103 + 115-119 papers over.  The previous
 *   candidate had $s0 = param and rotated the three table pointers up by one.
 *   The score is coincidentally still 18 because the residual moved wholesale
 *   into the tail block layout (see "WHAT IS LEFT" below) — but the diff is a
 *   completely different and much smaller-in-kind problem now.
 *
 * THE TWO LEVERS THAT GOT US HERE (both measured, see hypotheses.md H5/H6):
 *   1. A REAL loop (`do { ... } while (a0 == 0);`) instead of the `goto loop`
 *      back-edge.  The loop notes make flow.c weight in-loop references by
 *      loop_depth, which lifts the two 2-use table pointers over the 1-use
 *      param in global.c:allocno_compare.  (Session 1's H3 established this.)
 *   2. `k` — ONE reused scratch local holding BOTH loop-invariant compare
 *      constants (0x3C0000 then 0x1000000).  This is what makes lever 1
 *      usable: with two separate constants, loop.c:scan_loop makes each a
 *      movable and move_movables hoists both into fresh callee-saves ($s4,
 *      $s5), costing +7 insns (that is s1's rejected real-loop form, score
 *      29/98).  With ONE variable set twice, `n_times_set[k] == 2` and the
 *      sets are not consecutive, so `consec_sets_invariant_p` fails and
 *      scan_loop never builds a movable for either.  Both constants stay
 *      materialised inline inside the loop, exactly as target does.
 *      This is the [[defeat-licm-hoist-var-reuse]] family (SOTN-sanctioned
 *      "variable reuse for codegen control").  IT HAS NOT YET BEEN THROUGH
 *      cheat-reviewer — do that before any completion claim, and prefer a
 *      more natural two-set spelling if one can be found.
 *
 * WHAT IS LEFT (the whole 92-vs-91 residual, from grind_diff.py):
 *   (a) +2 in the tail: our mask exit lays out as
 *           bnez v0,<cont> / nop / j <end> / move v0,zero
 *       where target has the un-inverted, fall-through form
 *           beqz $v0,.L80081CFC / addu $v0,$zero,$zero   (delay slot)
 *       i.e. GCC put the `j end` BEFORE the `ret = 1; while (a0 == 0)` tail
 *       block instead of after it.  This is block layout, not allocation.
 *       Measured alternatives that do NOT fix it: inline `return 0;`/
 *       `return 1;` instead of break+ret (v11, 19/93), a `for (;;)` with
 *       three inline returns (v13, 19/93).
 *   (b) -1 at the 0x3C0000 compare: `k` is allocated $a0, so the scheduler
 *       drops `lui a0,0x3c` into the `bnez v1` delay slot at build idx 31
 *       where target keeps a `nop` and materialises `lui $v0,(0x3C0000>>16)`
 *       later, after the D_800F19BC store.  Target's holder is $v0 (dead
 *       after the preceding `slt`), ours is $a0.  Spelling the reuse through
 *       the existing `v0` local instead of a new `k` DOES put it in the right
 *       register class but regresses the rest (v12, 22/93).
 *
 * Do NOT re-try (banked in rejected/): inlining the three table globals at
 * their use sites (85 insns, 40); collapsing the argument staging into one
 * call expression (23); the plain real-loop form without the reused scratch
 * (29/98, both constants hoisted); v0-as-the-reused-holder (22/93).
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    do {
        v0 = sys_VSync(-1);
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        k = 0x3C0000;
        if (!(k < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 arg5, arg4;
            arg5 = tbl_125c[idx_1494[1]];
            arg4 = tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
        }
        cdrom_ClearIrq();
        v0 = -1;
        goto check;

    success:
        v0 = 0;

    check:
        if (v0 != 0) {
            ret = -1;
            break;
        }
        k = 0x1000000;
        if (!(*D_800A14C0 & k)) {
            ret = 0;
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
