/* func_800645B0 (src/text1b.c) -- MATCHED, honest distance 0 / 78 insns,
 * measured in grind session s17 (2026-09-02, rederive modality) with this exact
 * body pasted over the `INCLUDE_ASM("asm/funcs", func_800645B0);` line:
 * `sandbox func_800645B0 --disable all` = score 0, target_insns 78,
 * build_insns 78, rules_dropped 0, zero cheat-asm in the function.  The FULL
 * BUILD was also run with this body linked in: `verify-oracle` returns
 * `"ok": true, "build_matches": true` -- SHA1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa, the oracle.
 *
 * WHAT CLOSED IT.  Two independent halves had been solved separately for
 * sixteen sessions and were believed mutually exclusive:
 *   (1) OPERAND ORDER of the *3 slot offset (`addu $s0,$s1,$s0` at index 20).
 *       expand_binop (tools/gcc-2.7.2/optabs.c:398-421) swaps a commutative
 *       operand pair whenever `target == op1`, so BOTH `idx = idx2 + idx;` and
 *       `idx = idx + idx2;` emit `addu s0,s0,s1`.  Target order requires a
 *       destination pseudo distinct from both operands -- the "WD" fresh local
 *       `wid = idx2 + idx;` (ordinary C; certified as such by s12).
 *   (2) INNER-LOOP HEAD (indices 11/12) plus the back-edge delay slot (65).
 *       With `wid` fresh, `idx` becomes single-set, so sched.c's
 *       `birthing_insn_p` (tools/gcc-2.7.2/sched.c:2505) grants the loop-top
 *       `addu` the `adjust_priority` max-priority lift (sched.c:2543); the
 *       backward list scheduler then picks it FIRST and emits it LAST, handing
 *       the loop head to the const-1 `li`.  Sixteen sessions searched for a
 *       second real write to `idx` that would deny that lift; every one costs
 *       at least two instructions or a register seat (h = 2/78, k = 12/78 --
 *       see rejected/).
 * The single-level `do { idx = i + j; } while (0);` wrap closes half (2)
 * WITHOUT a second write to `idx`, so half (1) is kept: dump-proven in
 * tmp/grind/func_800645B0/s17/{wd,dw}.scheddbg.txt (instrumented cc1,
 * BB2_SCHED_DEBUG=1).  In the unwrapped WD build the const-1 `li` (insn 41)
 * reports `ADJPRI deaths=0 birth=0 pri=1` while the index `addu` (insn 38)
 * reports `birth=1` and is lifted to maxpri, so the `li` is picked last and
 * emitted first.  In the wrapped build the `li` (insn 53) is DEMOTED instead
 * (n_deaths > 0 => `INSN_PRIORITY >>= 1` => 0, so adjust_priority never
 * reaches its birthing branch and prints no ADJPRI line), the `addu`
 * (insn 41) still takes the lift, and the scheduler picks it LAST at clock=6
 * -- emitting it first at the inner-loop head, where reorg.c then steals it
 * into the back-edge delay slot exactly as the target does.
 *
 * WHY THIS IS AN ALLOWED FORM.  `do { <any body> } while (0);` is a sanctioned
 * pure-C match device for ANY codegen effect under the owner ruling of
 * 2026-07-06 (.claude/rules/do-while-zero-exception.md:29-30), which
 * explicitly ABOLISHED the earlier reorg.c-only scoping.  This exact wrap was
 * found by the session-5 permuter campaign at score 0 on 2026-08-12 and banked
 * as REJECTED (rejected/permuter-bare-do-while0-wrapper-outside-carveout.c) on
 * the sole ground that "the carve-out ... applies ONLY to the
 * LABEL_OUTSIDE_LOOP_P / reorg.c relax_delay_slots invert-jump interaction" --
 * a scope the rule had already retired six weeks earlier.  The s17 brief's
 * CURRENT SCOPE block states the live scope, so under the driver's
 * "RULE SCOPE IS DATED" clause the form was restored and re-measured.  It is a
 * SINGLE-LEVEL wrap (no nesting justification needed) and carries the mandatory
 * inline FAKE annotation.  Self-vet: memory/grind/func_800645B0/self_vet.md.
 *
 * Full 16-session history (49 banked rejected forms, the RA-seat foreclosure
 * verdicts, the LICM/cse/combine findings): evidence.md + hypotheses.md.
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
