/*
 * motion_Close — BEST MEASURED FORM (session 4, permuter). Honest sandbox
 * floor 16. target 26 insns, build 25.
 *
 * WHAT CHANGED IN SESSION 4. The floor is the same 16 that session 3 reached,
 * but this form gets there WITHOUT session 3's unresolved construct. Session 3
 * flipped the $s0/$s1 roles by making `p` carry the D_800A2668 guard value
 * through a function-pointer-pointer cast, which created an exact allocno
 * PRIORITY TIE that declaration order then broke; that construct was banked
 * with a loud "needs an owner ruling" header and was never submitted. This
 * session reaches the same roles by a strict priority WIN instead of a tie,
 * using a single-level `do { ... } while (0);` wrap — an explicitly sanctioned
 * family (see the POLICY note below). The type-punning reuse is GONE, and with
 * it the open ruling question that was blocking any use of the 16-point family.
 *
 * PROVENANCE. The lever came out of the session-4 permuter campaign on the
 * policy-clean 17-point chassis (tmp/grind/motion_Close/s4/wsA, base score 468):
 * `output-463-1` proposed `do { count = ...; p = ...; } while (0);` around the
 * two initialisers. Re-measured with the engine sandbox that proposal scores 16
 * (tmp/grind/motion_Close/s4/sweep4.log, form a1_dw0_inits), and the follow-up
 * sweep (sweep5.log) minimised it to the single-statement wrap used here
 * (form d4_dw0_p_only) — same score, smallest possible device.
 *
 * MECHANISM (measured, instrumented cc1, BB2_ALLOC_DEBUG=1). flow.c weights
 * REG_N_REFS by loop depth, and the NOTE_INSN_LOOP_BEG/END pair emitted by the
 * wrap makes its body depth 1, so the reference to `p` inside it counts twice.
 * global.c's allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length:
 *
 *   baseline (17):   count 8 refs / len 8 -> 30000 -> $s0 ;  p 7/7 -> 20000 -> $s1
 *   this form (16):  p     8 refs / len 7 -> 34285 -> $s0 ;  count 8/8 -> 30000 -> $s1
 *
 * p's live length is unchanged (the wrapped statement is where p was already
 * born) while its weighted reference count rises 7 -> 8, so p overtakes count
 * outright. No tie, no declaration-order dependence: the form scores 16 with
 * either declaration order (d0/d2 in sweep5.log).
 *
 * WHAT IT BUYS. The loop body now carries the target's registers exactly
 * (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1), the inner guard
 * branches on $s1, and — unlike session 3's form — the guard load stays in a
 * CALLER-saved temp ($v0) instead of being dragged into $s0, so the prologue is
 * not made worse to pay for the role flip. This is exactly frontier F4's probe
 * 2 ("reach parity without holding the guard"), and it is now CONFIRMED.
 *
 *   target                        this form
 *   ------                        ---------
 *   lui  t0 ; lw t0,0(t0)         lui  v0 ; lw v0,0(v0)
 *   addiu sp,sp,-16               addiu sp,sp,-32
 *   sw s0,4 / s1,8 / ra,12        sw ra,24 / sw s1,20
 *   beqz t0 ; nop                 beqz v0 ; sw s0,16   (save in the delay slot)
 *   lui/addiu s0   (= p)          lui/addiu s1  (= count)
 *   lui/addiu s1   (= count)      lui/addiu s0  (= p)
 *   beqz s1 ; nop                 beqz s1 ; nop
 *   lw t0,0(s0) ; addiu s0,s0,4   lw v0,0(s0) ; addiu s0,s0,4
 *   jalr t0 ; addiu s1,s1,-1      jalr ; addiu s1,s1,-1
 *   bnez s1 ; nop                 bnez s1 ; nop
 *   lw ra/s1/s0 ; addiu sp,16     lw ra/s1/s0 ; addiu sp,32
 *   jr ; nop                      jr ; nop
 *
 * WHAT STILL COSTS (the 16, and where the next session should look):
 *   frame size + save/restore offsets + save order  ~8   dead by H1
 *   guard temp $v0 vs $t0 (load + jalr register)     2   open, unprobed
 *   beqz delay slot (target nop vs our sw s0)        2   hand-asm signal S-b
 *   the two address materialisations emitted in the
 *     count-then-p order, target wants p-then-count  2   see the bind below
 *   inner-guard / misc positional                    2
 *
 * THE BIND IS UNCHANGED AND STILL THE FRONTIER. Assigning `p` before `count`
 * fixes the materialisation order and destroys the priority win in the same
 * move: p's live length grows 7 -> 8 (34285 -> 30000) while count's shrinks
 * 8 -> 7 (30000 -> 38571), count retakes $s0 and the score returns to 20
 * (sweep5.log d1_dw0_pfirst and d3_dw0_pdecl_pfirst; banked as
 * rejected/dw0-pfirst-order-inverts-priority.c). Nesting the wrap (d5) lifts
 * both allocnos equally and does not change the score.
 *
 * STILL DEAD: H1. gcc-2.7.2's o32 backend reserves REG_PARM_STACK_SPACE = 16
 * bytes of outgoing-arg area for every C-level call, so any pure-C body with a
 * call has frame >= 16 + 12 = 28 -> 32, while the target's frame is 16 with a
 * ZERO arg area (census of all 854 call-making functions in the oracle build:
 * none below 16). NO form in this family can reach distance 0, so this file
 * must never be submitted as `candidate-ready` on the strength of its score.
 *
 * POLICY. The single construct here is a single-level `do { ... } while (0);`
 * wrap carrying a FAKE annotation at the construct site.
 * `.claude/rules/do-while-zero-exception.md:23-24` states, verbatim:
 * "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned
 * pure-C match device for ANY codegen effect, including register allocation.**"
 * Prerequisite 1 (inline annotation naming the observed effect) is satisfied
 * below; prerequisite 2 explicitly does not gate single-level wraps; the wrap
 * is single-level so prerequisite 3 does not apply. Precedent: the same rule
 * file's confirmed application to marionation_Exec's RA-weighting wraps
 * (.claude/rules/do-while-zero-exception.md:99). The session-3 form's open
 * ruling question is therefore MOOT for reaching 16 — the policy-clean form
 * gets there too — and the session-3 variant is retained only as
 * rejected/... evidence about the tie-break mechanism.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        count = (s32)&D_00000000;
        /* FAKE: loop-note reference weighting seats p in $s0 (it overtakes
           count 34285 vs 30000) without lengthening p's live range. */
        do { p = &D_8008D070; } while (0);
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
