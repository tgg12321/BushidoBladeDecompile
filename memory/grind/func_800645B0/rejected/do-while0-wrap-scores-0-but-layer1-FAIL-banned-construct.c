/* REJECTED -- do NOT re-propose, do NOT re-submit.
 *
 * This body measures honest sandbox distance 0 / 78 and builds to the oracle
 * SHA1.  It was submitted as candidate-ready by the previous grind session and
 * was FAILED by the layer-1 cheat-reviewer; the driver banked the FAIL
 * (docs/grind/decisions.md, commit 322fe579) and the construct is now on this
 * function's mechanically-enforced BANNED list:
 *
 *   BANNED: `do { idx = i + j; } while (0);` around the slot-index assignment (C1)
 *
 * Layer-1's reasoning, verbatim from the brief's judge_constraints: "The
 * do-while(0) wrap (C1) is the exact form this ledger already banked as
 * REJECTED in session 5 and is the fourth respelling of this function's own
 * banned scheduler-tie-steer construct; citing the do-while-zero-exception
 * family does not launder a mechanism this project has already ruled out three
 * times for this specific function."  Its next-action instruction is: revert to
 * the honest floor form and "do not resubmit ANY construct -- wrap, staged
 * variable, dead store, or otherwise -- whose purpose" is that scheduler tie.
 *
 * A candidate-ready whose self_vet re-declares this construct is discarded by
 * the driver BEFORE the Judge sees it.  The bytes are real; the form is not
 * acceptable.  Kept here only so no future session re-derives it and burns a
 * window rediscovering the same FAIL.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            /* FAKE: single-level do-while(0) wrap around the slot-index
             * assignment. Mechanism: cc1's first-pass scheduler --
             * sched.c adjust_priority / birthing_insn_p
             * (tools/gcc-2.7.2/sched.c:2505,2543). With the wrap's loop
             * notes present the const-1 `li` is DEMOTED (n_deaths > 0 =>
             * INSN_PRIORITY >>= 1 => 0) while the index `addu` keeps its
             * birthing max-priority lift, so the backward list scheduler
             * picks the `addu` last and emits it FIRST at the inner-loop
             * head -- which reorg.c then steals into the back-edge delay
             * slot, as the target does. Lever-exhaustion: 16 sessions,
             * memory/grind/func_800645B0/hypotheses.md H24/H58/H63-H72 and
             * evidence.md s9-s16 (49 banked rejected forms, floor flat at
             * 1/78; the natural-geometry close-out was searched first and
             * every spelling of it is banked dead). */
            do { idx = i + j; } while (0);
            /* FAKE: `val` carries BOTH the const-1 shift source and the
             * D_800A3444 read-modify-write. Mechanism: loop.c scan_loop /
             * move_movables -- a multi-set pseudo is not a movable
             * (the n_times_set == 1 gate, tools/gcc-2.7.2/loop.c:705), so
             * the constant 1 is recomputed inline each iteration instead of
             * being hoisted into a callee-saved seat (+2 insns). */
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
