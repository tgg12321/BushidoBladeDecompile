/* candidate for func_8007526C (src/text1b.c) -- s15b (2026-09-07, synthesis).
 *
 * THIS BODY IS THE ORDINARY, NO-CONSTRUCT BASELINE (formerly ordinary-score13-baseline.c).
 * It is candidate.c again because the s15 compound-assignment-split body that briefly held
 * this slot was ruled a CHEAT by the Judge on 2026-09-07 17:07 (decisions.md): decomposing
 * the single literal 0xA into eight synthetic addends has no semantic reading, so it is
 * neither a truthful spelling nor a member of a frozen family.  That body must never be
 * re-filed; its shape is recorded in rejected/ and in the ruling.
 *
 * RE-MEASURED THIS SESSION on the dispatch chassis (HEAD 9bf1aa2c):
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91
 *   `.loop`: "Loop from 14 to 260: 91 real insns", lim (regno 75) moved, all four switch
 *   comparison constants (regnos 124/126/127/128, life 1, savings 1) moved to the pre-header.
 *
 * THE RESIDUAL, RESTATED PRECISELY.  The 13 points are the four switch-comparison constants
 * being hoisted into $8/$9/$10/$11 where the target rematerialises them in-loop in $v0.  The
 * single gate is move_movables' desirability test at tools/gcc-2.7.2/loop.c:1631,
 * `(threshold * savings * m->lifetime) >= insn_count`, with threshold 122 decaying to 119
 * once `lim` is moved (loop.c:1904) and savings = lifetime = 1.  Two and only two mechanisms
 * flip it, and s15b measured the admissibility of both:
 *
 *   (1) RAISE loop-time insn_count to >= 120 with a payload that vanishes before the final
 *       91 words.  The sanctioned two-step split of a genuine `a + b` yields +8 (insn_count
 *       99) across every update site the function has -- 21 short (rejected/
 *       truthful-twostep-split-all-sites-insncount99-score26.c).  Dead payloads yield ZERO:
 *       cse1 ends with delete_dead_from_cse (tools/gcc-2.7.2/cse.c:8684) whose stated purpose
 *       is to keep loop from seeing dead invariants, so 32 dead stores and 11 dead invariant
 *       movables both leave insn_count at exactly 91.  jump2 cross-jump payloads reach the bar
 *       but cost 11 emitted words (s13/s14, build_insns 102).
 *
 *   (2) ARM the moved_once doubling (loop.c:1609-1611) with a second loop that also moves
 *       `lim`.  s15b found the first zero-word spelling: a trailing `while (i < 2) { lim =
 *       0xC8; i++; }` measures score 1 / build_insns 90, the target's words exactly.  It emits
 *       nothing ONLY because it is provably dead (`i` is already 2), which is precisely why it
 *       is not admissible C.  Every arming spelling that is reachable emits >= 3 words
 *       (score 4/5, build 93), and the target has zero spare words.
 *
 * So the two axes are now quantified rather than guessed, and both terminate in an
 * admissibility question rather than a measurement question.  See evidence.md [s15b].
 *
 * s16 (2026-09-07, synthesis) RE-MEASURED THIS BODY at score 13 / build_insns 93 on HEAD
 * bd38096c, and named the residual to the integer.  Compiling this exact preprocessed TU
 * twice with the production cc1 and flags, the second run adding only `-msoft-float`,
 * produces .loop dumps that agree on EVERY quantity the C controls -- "Loop from 14 to 260:
 * 91 real insns", the same five movables, regno 75 at life 63 and regnos 124/126/127/128 at
 * life 1 savings 1 -- and disagree only in the verdict: hard float moves all five, soft float
 * prints "not desirable" for the four constants.  The single differing term in loop.c:1631 is
 * `threshold`, and mips.h:524-536 fixes all 32 FP registers (mips.h:1224-1225) when
 * !TARGET_HARD_FLOAT, which drops regclass.c:380-387's n_non_fixed_regs by 32 and so
 * loop.c:532's threshold by 64: 122 -> 58.  The flag touches no float in this function; it
 * only shrinks the register file loop.c prices lifetime against.  This body is therefore
 * very probably the original C already, and the 13 points are a build-configuration
 * divergence, not a spelling divergence.  See evidence.md [s16].
 *
 * s20 (2026-09-07, rederive) RE-MEASURED THIS BODY at score 13 / build_insns 93 /
 * loop insn_count 91 on HEAD 43226623, and re-derived the dispatch from scratch.  Three
 * structurally different shapes were measured and rejected: the literal-0xC8 form without
 * the `lim` local (13, codegen-neutral), the target-exact-order if/else chain with literal
 * constants (29), and the same chain with every comparison constant written through ONE
 * reused local (26).  The last one is the important result: it removes all four constant
 * movables from loop.c -- the .loop dump prints exactly one movable, regno 75, which is the
 * target's movable set -- and it emits the target's registers exactly ($a3 holding 0xC8 in
 * the pre-header, the constants rematerialised in-loop in $v0).  It still scores 26 because
 * an if/else chain inlines its arm bodies where a switch places them out of line, costing
 * five words.  So the movable set alone controls the allocation, and the `switch` alone
 * controls the layout, and no C name reaches a switch's expand_case comparison pseudos.
 * See evidence.md [s20].
 *
 * s21 (2026-09-07, rederive) RE-MEASURED THIS BODY at score 13 / build_insns 93 / loop
 * insn_count 91 on HEAD 1930c839.  Two things about this body are now known to be
 * load-bearing beyond what earlier headers say.  (a) The `lim = 0xC8;` statement at the TOP
 * of the loop keeps scan_loop's scan_start on that insn; delete it and the loop body starts
 * with the switch's jump to expand_case's decision tree, loop.c:545 retargets scan_start to
 * the dispatch, and the four comparison constants are collected AHEAD of every body movable
 * -- which forecloses any arming construct living in an arm body.  (b) loop.c:532's
 * threshold has a third C-reachable factor nobody had named: `loop_has_call`.  One call in
 * the loop gives threshold 61 -> 58 and reproduces the target's movable set exactly (one
 * hoisted 0xC8, four in-loop constants), at a cost of eleven words.  See evidence.md [s21].
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
