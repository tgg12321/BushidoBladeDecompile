/*
 * motion_Close — BEST MEASURED FORM (floor 13). Reconstructed and re-measured
 * in session 5b: `sandbox motion_Close --disable all` prints
 * score 13, target_insns 26, build_insns 25 with this body in src/ings2.c.
 *
 * PROVENANCE / WHY THIS FILE EXISTS. A prior session-5 run measured this form
 * at 13 but was DISCARDED by the driver on foreign dirt before it could update
 * candidate.c; only its rejected/*.c notes were committed, so the floor-13 form
 * itself survived only as a comment inside
 * rejected/pfirst-wrap-depth-1-and-2-insufficient.c ("f3_pfirst_wrap3 score 13
 * <-- the candidate"). Session 5b reconstructed it from that note, re-measured
 * it independently (13, confirmed), and banks it here so the floor is no longer
 * one discarded session away from being lost. The previous contents of this
 * file (session 4's single-level-wrap, count-first, floor-16 form) are
 * superseded; that form's mechanism write-up is preserved in evidence.md and in
 * rejected/dw0-pfirst-order-inverts-priority.c.
 *
 * MECHANISM (measured, instrumented cc1, BB2_ALLOC_DEBUG=1; count is pseudo 72,
 * p is pseudo 73). flow.c weights REG_N_REFS by loop depth and each
 * NOTE_INSN_LOOP_BEG/END pair from a `do { } while (0)` makes its body one level
 * deeper, so a reference nested N levels deep counts 2^N-ish in the weighting.
 * global.c's allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length:
 *
 *   p-first, wrap depth 1:  p  8 refs / len 8 = 30000 -> $s1 ; count 8/7 = 34285 -> $s0   score 20
 *   p-first, wrap depth 2:  p  9 refs / len 8 = 33750 -> $s1 ; count 8/7 = 34285 -> $s0   score 20
 *   p-first, wrap depth 3:  p 10 refs / len 8 = 37500 -> $s0 ; count 8/7 = 34285 -> $s1   score 13  <— this form
 *
 * Depth 3 is both NECESSARY (depths 1 and 2 leave count in $s0 and score 20)
 * and MINIMAL (depths 4, 5 and 6 also score 13 and buy nothing). That measured
 * demonstration is exactly prerequisite 3 of
 * `.claude/rules/do-while-zero-exception.md` for a NESTED wrap, and it is banked
 * as rejected/pfirst-wrap-depth-1-and-2-insufficient.c.
 *
 * WHY IT BEATS SESSION 4's 16. Session 4 reached the target's $s0=p / $s1=count
 * role split with a single-level wrap, but only in the COUNT-FIRST statement
 * order, so the two address materialisations came out in the wrong order (2
 * points). This form wins the same role split with p assigned FIRST, so the
 * materialisation order matches the target too.
 *
 * REMAINING RESIDUAL AT 13 (from the s5b permuter workspace,
 * tmp/grind/motion_Close/s5b/wsA/_base.txt vs _tgt_mine.txt):
 *   guard lui/lw + body lw + jalr all use $v0 where the target uses $t0   4
 *   frame size 32 vs 16 + the three save offsets + the three restore
 *     offsets + addiu sp                                                  ~7
 *   beqz delay slot (target nop, ours sw s0) + save ORDER                  2
 * The $v0-vs-$t0 bucket (frontier F5) is now the LARGEST live bucket and is a
 * local-alloc.c decision, not a global.c one.
 *
 * STILL DEAD: H1. gcc-2.7.2's o32 backend reserves REG_PARM_STACK_SPACE = 16
 * bytes of outgoing-arg area for every C-level call, so any pure-C body with a
 * call has frame >= 16 + 12 = 28 -> 32, while the target's frame is 16 with a
 * ZERO arg area (census of all 854 call-making functions in the oracle build:
 * none below 16). NO form in this family can reach distance 0, so this file
 * must never be submitted as `candidate-ready` on the strength of its score.
 *
 * POLICY. The single construct is a THREE-LEVEL `do { ... } while (0);` wrap
 * carrying a FAKE annotation at the construct site.
 * `.claude/rules/do-while-zero-exception.md:23-24` states, verbatim:
 * "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned
 * pure-C match device for ANY codegen effect, including register allocation.**"
 * Prerequisite 1 (inline annotation naming the observed effect) is satisfied
 * below; prerequisite 3 (a nested wrap needs a written measurement showing a
 * single level insufficient) is satisfied by
 * rejected/pfirst-wrap-depth-1-and-2-insufficient.c. A three-level wrap is a
 * heavier device than the single-level one and a reviewer may still reasonably
 * push back on it; that is a judgement for the layer-1/layer-2 reviewers, and
 * it is moot for completion while H1 holds.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        /* FAKE: loop-note reference weighting (flow.c weights REG_N_REFS by
           loop depth) lifts p to 10 weighted refs / live_length 8 = 37500, so
           p takes $s0 ahead of count's 34285 while keeping the target's
           p-then-count address-materialisation order. Depth 1 and 2 measured
           insufficient — see rejected/pfirst-wrap-depth-1-and-2-insufficient.c. */
        do { do { do { p = &D_8008D070; } while (0); } while (0); } while (0);
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
