/*
 * motion_Close — BEST MEASURED FORM (floor 13, unchanged since session 5b), and
 * as of SESSION 12 with a STRICTLY LIGHTER policy device again: a SINGLE-level
 * `do { p++; } while (0);` wrap placed INSIDE the walk, replacing the two-level
 * nested wrap on p's initialiser that sessions 9-11 carried (which in turn had
 * replaced the three-level wrap on the do-while-LOOP chassis of sessions 5b-8).
 * `sandbox motion_Close --disable all` prints score 13, target_insns 26,
 * build_insns 25 with this body in src/ings2.c (session 12, f14bsweep.py cells
 * X1_dec_before_call_d1 and X1_dec_before_call_d2 — depth 2 is no better, so
 * ONE level is the whole device).
 *
 * WHY THE PLACEMENT CHANGE IS THE POINT (session 12, modality `structural`).
 * Every wrap measurement this function had before session 12 — s4 sweep5, s5b,
 * s8, s9 gotosweep, s10 f12sweep, s11 f13sweep — placed the wrap on ONE
 * statement, p's initialiser `p = &D_8008D070;`, and varied only the NESTING
 * DEPTH around that site. The resulting 20/20/13 ladder is therefore a ladder in
 * depth-at-one-placement. But flow.c's weighting is per-REFERENCE
 * (`REG_N_REFS (regno) += loop_depth` for every reference inside the
 * NOTE_INSN_LOOP_BEG/END pair), and p's four raw references are not all at the
 * initialiser: one is the `*p` read and TWO are the `p++` set+use inside the
 * walk. A wrap around the bump therefore lifts p by 2 per level instead of 1,
 * so one level reaches the 6 weighted references that used to need two.
 *
 * MEASURED PLACEMENT TABLE (session 12, f14sweep.py, 7 placements x depths
 * 0/1/2, sandbox + instrumented cc1 BB2_ALLOC_DEBUG=1; count is pseudo 72, p is
 * pseudo 73 in this declaration order):
 *   wrap on p's initialiser  : p +1 ref/level   (4 / 5 / 6)
 *   wrap on `p++`            : p +2 refs/level  (4 / 6 / 8)
 *   wrap on `f = *p;`        : p +1 ref/level   (4 / 5 / 6)
 *   wrap on read+bump pair   : p +3 refs/level  (4 / 7 / 10)
 *   wrap on the whole body   : p +3, count +2 per level — ordering unchanged
 *   wrap on count's init     : count +1/level — the wrong allocno, 20 at every depth
 * count is unmoved by every in-walk p placement: 5 raw refs / live_length 7 =
 * 14285. p at 6/8 = 15000 clears it, and does so at ONE level via the bump.
 *
 * THE TRAP THAT COST THE NAIVE FORM TWO POINTS, AND THE FIX. Wrapping the bump
 * with the ORIGINAL statement order (`f = *p; do { p++; } while (0); f();
 * count--;`) wins the registers at depth 1 but scores 15 with 26 emitted
 * instructions instead of 13/25: the note fences `addu $16,$16,4` out of its
 * position between the load and the call, the scheduler puts it in the jal delay
 * slot instead, `lw $2,0($16)` then feeds `jal $31,$2` back-to-back and maspsx
 * inserts a load-delay nop. Moving `count--` ahead of the call gives the
 * scheduler a second delay-slot candidate; it fills the jal slot with
 * `addu $17,$17,-1` exactly as the target does, `addu $16,$16,4` stays between
 * the lw and the jal, the nop disappears and the score is 13 at 25 instructions.
 * `count--` before `f()` is independently measured FREE on this chassis (s11
 * F13 cell V7, byte-identical at every depth) and is semantically identical —
 * `count` is a local the callee cannot observe. The isolating diagnostic is s12
 * cell X6: an EMPTY `do { } while (0);` in the same in-walk position, adding no
 * reference to either allocno, still emits the 26th instruction — so the fence
 * is the NOTE, not the wrapped statement, and the statement order is what pays
 * for it.
 *
 * WRAP DEPTH 0 IS NOW PROVABLY UNREACHABLE, NOT MERELY UNREACHED. In the raw
 * count regime the priority is floor_log2(n)*n/live_length*10000 and the
 * measured constants are p 4 refs / live_length 8 = 10000 against count 5/7 =
 * 14285. With no wrap anywhere, flipping the roles needs one of: (a) a real
 * extra p reference the target does not emit; (b) p's live_length <= 5, which is
 * impossible because p's range spans the whole walk and shortening it means
 * moving p's birth after count's — the count-assigned-first form, measured at 16
 * (three residual points of materialisation order, s11 V3); or (c) count's
 * priority pushed to 10000 or below, i.e. live_length >= 10 at 5 references,
 * which needs at least three insns of intervening real computation the target
 * does not perform. Every route emits instructions the target lacks. ONE wrap
 * level is therefore the MINIMUM policy device for floor 13 on this chassis, and
 * frontier FR2 is closed at its floor rather than merely advanced.
 *
 * STREAM VERIFICATION (FR3, mandatory when candidate.c changes). This body's
 * emitted stream was re-diffed against tmp/grind/motion_Close/s10/
 * f12_G_goto_tail_d2.s — the stream session 10 verified against
 * tmp/grind/motion_Close/s5b/wsA/_base.txt, which is the build the s7 residual
 * table was written from. The two are IDENTICAL except for the internal label
 * NUMBER (.L136 here vs .L144 there; fewer loop notes are emitted, so the label
 * counter lands lower). Same 25 instructions, same registers, same
 * `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`, same filled beqz delay
 * slot, same descending save order, same $v0 temp. The s7 residual table
 * therefore describes THIS form verbatim. (Caution for any future integration:
 * the label NUMBER differs even though the label COUNT does not — see
 * [[global-label-drift-sibling-cheat]] before any sibling rule references
 * a hardcoded `.L` name.)
 *
 * REMAINING RESIDUAL AT 13 (session 7's instruction-by-instruction pairing,
 * tmp/grind/motion_Close/s7/residual_table.md, accounts for 13/13 with nothing
 * left over): ~6.5 points H1 (frame size 32 vs 16, three save offsets, three
 * restore offsets, addiu sp), 5 points F5 (guard lui/lw + body lw + jalr use
 * $v0 where the target uses $t0), ~1.5 points F7 (beqz delay slot + save
 * order). All three carry a backend-level disproof:
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
 * Sessions 8-11 closed every input to cc1 outside the walk itself: whole-TU
 * shape and signature (F10), global declaration forms (F11), loop construct
 * (F12), guard/materialisation shape and declaration scope (F13), the five
 * pointer/loop idioms (s9) and the sanctioned dead-store substitution (s11 M1,
 * deleted before flow.c counts). Session 12 closes wrap PLACEMENT (F14).
 *
 * THIS FILE MUST NOT BE SUBMITTED AS `candidate-ready`. H1 is a backend-level
 * disproof, not a plateau: no pure-C body containing a call can reach the
 * target's zero-byte outgoing-argument area, so no member of this family can
 * reach distance 0 whatever its score.
 *
 * POLICY. The single non-obvious construct is the ONE-level
 * `do { p++; } while (0);` wrap, FAKE-annotated at the construct site.
 * `.claude/rules/do-while-zero-exception.md:23-24` states, verbatim:
 * "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned
 * pure-C match device for ANY codegen effect, including register allocation.**"
 * Prerequisite 1 (an inline annotation naming the observed effect) is satisfied
 * below. Prerequisite 3 (a NESTED wrap needs a written measurement showing
 * fewer levels insufficient) no longer applies at all — the wrap is single-level
 * — and depth 0's insufficiency is measured (20) and now also derived in closed
 * form above. Neither the goto loop nor the `count--` placement is a device:
 * both are real program statements with real semantics, measured free
 * independently of any wrap (s9 five-idiom sweep, s11 F13 cell V7).
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
        again:
            {
                void (*f)(void) = *p;
                /* FAKE: loop-note reference weighting (flow.c weights REG_N_REFS
                   by loop depth; the do-while(0) emits a NOTE_INSN_LOOP_BEG/END
                   pair around the bump, whose set+use are TWO of p's four raw
                   references) lifts p to 6 weighted refs / live_length 8 = 15000,
                   so p takes $s0 ahead of count's 5/7 = 14285 while keeping the
                   target's p-then-count address-materialisation order. Depth 0
                   measured insufficient (score 20) and derived unreachable; one
                   level is the minimum — see
                   rejected/f14-in-walk-wrap-costs-load-delay-nop.c for the
                   statement order that must accompany it. */
                do { p++; } while (0);
                count--;
                f();
            }
            if (count != 0) {
                goto again;
            }
        }
    }
}
