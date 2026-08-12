/*
 * motion_Close — BEST MEASURED FORM (floor 13, unchanged since session 5b),
 * but as of session 9 with a STRICTLY LIGHTER policy device: a TWO-level
 * `do { } while (0);` wrap on a goto-loop chassis, replacing the three-level
 * wrap on the do-while-loop chassis that sessions 5b-8 carried.
 * `sandbox motion_Close --disable all` prints score 13, target_insns 26,
 * build_insns 25 with this body in src/ings2.c (session 9, gotosweep.py
 * d2_pdecl_pfirst_tail and d2_cdecl_pfirst_tail — both 13, so unlike the
 * session-3 family this form does not depend on the allocno tie-break at all).
 *
 * WHY THE CHASSIS CHANGE IS THE WHOLE POINT (session 9, modality `rederive`).
 * The device that wins $s0 for p is loop-depth reference weighting: flow.c
 * weights REG_N_REFS by loop depth, and every `do { } while (0);` emits a
 * NOTE_INSN_LOOP_BEG/END pair that makes its body one level deeper. On the
 * do-while-LOOP chassis the walk's own references are ALSO weighted by that
 * loop, so both allocnos start large (p 10 refs, count 8) and p needs three
 * wrap levels to clear count. A `goto` loop emits NO loop notes for the walk,
 * so every reference in the function is counted raw: p 4 refs, count 5. The
 * margin p must close shrinks from "10 vs 8 weighted" to "5 vs 5 raw", and two
 * wrap levels suffice. This is a structural change to the function's C shape,
 * not a respelling of the old one — it changes what flow.c counts, not how the
 * same counts are spelled.
 *
 * MEASURED LADDER ON THIS CHASSIS (session 9, sandbox + instrumented cc1
 * BB2_ALLOC_DEBUG=1; p and count are pseudos 72/73 in declaration order):
 *   depth 0:  p 4 refs / live_length 8 = 10000 -> $s1 ; count 5/7 = 14285 -> $s0   score 20
 *   depth 1:  p 5/8 = 12500              -> $s1 ; count 5/7 = 14285 -> $s0   score 20
 *   depth 2:  p 6/8 = 15000              -> $s0 ; count 5/7 = 14285 -> $s1   score 13  <— this form
 * Depth 2 is NECESSARY (0 and 1 both score 20, banked as
 * rejected/goto-chassis-wrap-depth-0-and-1-insufficient.c) and it is necessary
 * across FIVE pointer/loop idioms (`*p++`, `p[0]`, `--count`, decrement-before-
 * call, and call-through-`p[-1]`), i.e. it is a property of the reference
 * structure and not of this body's spelling (s9 idiomsweep.py). Declaration
 * order of the two locals is inert here (13 in both). That is exactly
 * prerequisite 3 of `.claude/rules/do-while-zero-exception.md` for a nested
 * wrap.
 *
 * FRONTIER F4b IS THEREFORE ADVANCED, NOT CLOSED. Its success criterion is
 * score 13 with wrap depth < 3; that is now met at depth 2. Depth <= 1 remains
 * unreached and the s9 measurements say why: with p assigned FIRST (which the
 * target's address-materialisation order requires) count sits at 5 raw refs /
 * live_length 7 = 14285 while p sits at 5/8 = 12500, so p must reach 6 weighted
 * references, and no honest extra reference to p exists that the target does
 * not also emit. The one chassis that drops count to 4 references — a top-
 * tested goto loop entered by `goto test;`, which deletes the outer
 * `if (count != 0)` guard — lets p win at depth 1 but scores 14, because the
 * target HAS that outer guard (s9 gotosweep.py d1_*_top). Trading one residual
 * point for one wrap level is not an improvement.
 *
 * MECHANISM RECORD FOR THE SUPERSEDED FORM (kept because the ledger's floor
 * history references it): the three-level wrap on the do-while-loop chassis
 * measured p 10 refs / live_length 8 = 37500 -> $s0 against count 8/7 = 34285
 * -> $s1, with depths 1 and 2 insufficient (rejected/pfirst-wrap-depth-1-and-2-
 * insufficient.c) and depths 4-6 worthless. Session 8 re-measured that same
 * 20/20/20/13 ladder value-for-value on two inlined chassis. That form is not
 * disproven — it is simply dominated by this one at equal score.
 *
 * REMAINING RESIDUAL AT 13 (session 7's instruction-by-instruction pairing,
 * tmp/grind/motion_Close/s7/residual_table.md, accounts for 13/13 with nothing
 * left over): ~6.5 points H1 (frame size 32 vs 16, three save offsets, three
 * restore offsets, addiu sp), 5 points F5 (guard lui/lw + body lw + jalr use
 * $v0 where the target uses $t0), ~1.5 points F7 (beqz delay slot + save
 * order). All three now carry a backend-level disproof:
 *   H1  — REG_PARM_STACK_SPACE is the compile-time constant 16 on every
 *         call-expansion path (mips.h:1822, calls.c:1245/1400, mips.c:4466);
 *         the only cfoas-free site is __builtin_apply, which emits a frame
 *         pointer, a 72-byte frame and 34 instructions (s7).
 *   F5  — local-alloc/global.c scan hard regs ascending with no MIPS
 *         REG_ALLOC_ORDER, so the first free caller-saved temp is always $v0,
 *         never $t0 (s5b).
 *   F7a — mips.c:4680 emits GP saves GP_REG_LAST -> GP_REG_FIRST
 *         unconditionally; giving the function real incoming parameters
 *         produces the WAR anti-dependence and the saves still come out
 *         descending (s8).
 *   F7b — an empty beqz delay slot needs the branch's block to hold no
 *         eligible single insn; the target's holds four (s6).
 * Session 8 additionally closed the whole-TU axis (signature, inlining, TU
 * order, helper parameterisation all inert), and session 9 closed the
 * declaration axis (F11): ten declaration forms of the three touched globals —
 * array-typed table, array-typed count, `extern char` count, `void *` count,
 * const-qualified table, u32 guard, and reordered externs — every one emits the
 * identical allocno table (p 10/8 = 37500, count 8/7 = 34285) and the identical
 * score 13.
 *
 * SESSION 10 (synthesis) VERIFICATION — this body's EMITTED STREAM was diffed
 * against tmp/grind/motion_Close/s5b/wsA/_base.txt, the build stream the s7
 * residual table was written from on the superseded three-level do-while
 * chassis. They are the same 25 instructions in the same order with the same
 * registers, the same `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`, the
 * same filled beqz delay slot and the same descending save order
 * (tmp/grind/motion_Close/s10/f12_G_goto_tail_d2.s). The s7 table therefore
 * describes THIS form, not just its predecessor. Session 10 also closed F12:
 * `for(;;)`+break, `while(1)`+break, `for(;;)`+continue and a middle-clause
 * `for` all reproduce the do-while-LOOP family's allocno table exactly and all
 * score 20 at wrap depths 0-2, so the goto chassis' depth-2 minimum is now
 * minimal across the whole loop-construct surface as well
 * (rejected/f12-loop-construct-surface-inert.c).
 *
 * THIS FILE MUST NOT BE SUBMITTED AS `candidate-ready`. H1 is a backend-level
 * disproof, not a plateau: no pure-C body containing a call can reach the
 * target's zero-byte outgoing-argument area, so no member of this family can
 * reach distance 0 whatever its score.
 *
 * POLICY. The single non-obvious construct is the two-level
 * `do { ... } while (0);` wrap, FAKE-annotated at the construct site.
 * `.claude/rules/do-while-zero-exception.md:23-24` states, verbatim:
 * "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned
 * pure-C match device for ANY codegen effect, including register allocation.**"
 * Prerequisite 1 (an inline annotation naming the observed effect) is satisfied
 * below; prerequisite 3 (a nested wrap needs a written measurement showing
 * fewer levels insufficient) is satisfied by
 * rejected/goto-chassis-wrap-depth-0-and-1-insufficient.c. The goto loop itself
 * is NOT a device — it is a real loop with real semantics, and the five-idiom
 * sweep shows the loop's spelling is a free variable rather than a coercion.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        /* FAKE: loop-note reference weighting (flow.c weights REG_N_REFS by
           loop depth; each do-while(0) emits a NOTE_INSN_LOOP_BEG/END pair)
           lifts p to 6 weighted refs / live_length 8 = 15000, so p takes $s0
           ahead of count's 5/7 = 14285 while keeping the target's p-then-count
           address-materialisation order. Depth 0 and 1 measured insufficient —
           see rejected/goto-chassis-wrap-depth-0-and-1-insufficient.c. */
        do { do { p = &D_8008D070; } while (0); } while (0);
        count = (s32)&D_00000000;
        if (count != 0) {
        again:
            {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            }
            if (count != 0) {
                goto again;
            }
        }
    }
}
