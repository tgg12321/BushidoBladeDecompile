/*
 * motion_Close — BEST MEASURED FORM (session 3, structural). Honest sandbox
 * floor 16 (was 17). target 26 insns, build 25.
 *
 * ############################################################################
 * # THIS FORM IS NOT POLICY-VETTED AND MUST NOT BE SUBMITTED AS-IS.          #
 * # It contains ONE construct that needs an owner ruling before any Judge     #
 * # cycle: the guard value of D_800A2668 is carried in `p` (a function-       #
 * # pointer-pointer) via a type-punning cast, purely so that global.c's       #
 * # allocno tie-break hands `p` the $s0 register.  See the RULING QUESTION    #
 * # block at the bottom of this comment.  The policy-clean form that carries  #
 * # no such construct is banked beside this file as                           #
 * # `candidate_policy_clean_17.c` (floor 17) and is what a session should     #
 * # fall back to if the ruling goes against the reuse.                        #
 * ############################################################################
 *
 * WHAT SESSION 3 FOUND (the headline): session 2's H3 — "the target's
 * $s0 = p / $s1 = count role split is NOT reachable from pure C" — is FALSE.
 * The role split IS reachable, and this form reaches it.
 *
 * Session 2 measured, correctly, that for every spelling emitting the target's
 * instruction sequence global.c ranks
 *      count: n_refs=8, live_length=8 -> pri 30000  (takes $s0)
 *      p:     n_refs=7, live_length=7 -> pri 20000  (takes $s1)
 * and concluded the margin was too wide to nudge.  What it did not do is read
 * the rest of `allocno_compare` (tools/gcc-2.7.2/global.c:633-654):
 *
 *      if (pri2 - pri1) return pri2 - pri1;
 *      // If regs are equally good, sort by allocno,
 *      // so that the results of qsort leave nothing to chance.
 *      return *v1 - *v2;
 *
 * On an EXACT priority TIE the order is decided by allocno number, which
 * follows pseudo-register creation order, which for block locals follows
 * DECLARATION order.  Session 1's H2 measured declaration order as inert —
 * true, but only because there was no tie to break at the time (30000 vs
 * 20000).  Create the tie and declaration order becomes decisive.
 *
 * The tie is created by having `p` carry the D_800A2668 guard value before it
 * is re-loaded with the table address.  That lifts p to n_refs=9,
 * live_length=9 -> floor_log2(9)*9/9*10000 = 30000, exactly count's priority,
 * and with `p` declared FIRST p gets the lower allocno and wins $s0.
 * Measured with the instrumented cc1 (BB2_ALLOC_DEBUG=1):
 *      p     (pseudo 72, declared first): n_refs=9 live_length=9 pri=30000 -> $16
 *      count (pseudo 73):                 n_refs=8 live_length=8 pri=30000 -> $17
 * Both halves are load-bearing and were measured independently:
 *      count declared first + guard in p  -> tie, count wins   -> score 17
 *      p declared first + no guard in p   -> no tie, count wins -> score 17
 *      p declared first + guard in p      -> tie, p wins        -> score 16
 *
 * WHAT THAT BUYS, AND WHAT IT COSTS. The loop body now carries the target's
 * registers exactly (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1),
 * and the inner guard branches on $s1 as the target does.  But `p` is
 * callee-saved, so making it hold the guard drags the guard LOAD into $s0,
 * which must therefore be saved before the load — the target loads the guard
 * into $t0 (caller-saved) before the frame exists.  The prologue consequently
 * gets worse by roughly what the loop body gained, and the net is 17 -> 16:
 *
 *   target                        this form
 *   ------                        ---------
 *   lui  t0                       addiu sp,sp,-32
 *   lw   t0,0(t0)                 sw   s0,16(sp)
 *   addiu sp,sp,-16               lui  s0
 *   sw   s0,4 / s1,8 / ra,12      lw   s0,0(s0)
 *   beqz t0 ; nop                 sw   ra,24(sp)
 *   lui/addiu s0   (= p)          beqz s0 ; sw s1,20(sp)
 *   lui/addiu s1   (= count)      lui/addiu s1  (= count)
 *   beqz s1 ; nop                 lui/addiu s0  (= p)
 *   lw   t0,0(s0) ; addiu s0,s0,4 beqz s1 ; nop
 *   jalr t0 ; addiu s1,s1,-1      lw v0,0(s0) ; addiu s0,s0,4
 *   bnez s1 ; nop                 jalr ; addiu s1,s1,-1
 *   lw ra/s1/s0 ; addiu sp,sp,16  bnez s1 ; nop
 *   jr ; nop                      lw ra/s1/s0 ; addiu sp,sp,32 ; jr ; nop
 *
 * Note the two address materialisations are now emitted count-pair-then-p-pair
 * while the target emits p-pair-then-count-pair.  Moving `p = &D_8008D070;`
 * above `count = ...` fixes the order but shortens count's live length to 7
 * (pri 34285), which takes $s0 straight back and returns the score to 20
 * (measured: w5_pfirst_count_inside).  That coupling — order vs priority — is
 * the live frontier for session 4.
 *
 * STILL DEAD: H1.  gcc-2.7.2's o32 backend reserves REG_PARM_STACK_SPACE = 16
 * bytes of outgoing-arg area for every C-level call, so any pure-C body with a
 * call has frame >= 16 + 12 = 28 -> 32, while the target's frame is 16 with a
 * ZERO arg area (census of all 854 call-making functions in the oracle build:
 * none below 16).  So NO form in this family can reach distance 0, and this
 * file must never be submitted as `candidate-ready` on the strength of its
 * score alone.
 *
 * RULING QUESTION (raised by session 3, unanswered):
 *   `p = (void (**)(void))D_800A2668; if (p != 0) { ... p = &D_8008D070; ... }`
 *   reuses one C variable for two unrelated values.  "Variable reuse for
 *   codegen control" is on the FROZEN SOTN-sanctioned list
 *   (.claude/rules/no-new-park-categories.md, "SOTN-accepted techniques"),
 *   and the guard test itself is a real, semantically required test that the
 *   target performs.  Against that: the *choice* to hold an s32 flag in a
 *   function-pointer-pointer has no observable effect (checklist T1), a human
 *   would not write the cast (T2), and the mechanism is stated purely in terms
 *   of global.c's allocno tie-break (T3).  It carries no /* FAKE *​/ annotation
 *   and no lever-exhaustion citation here.  Until an owner rules, treat this
 *   form as EVIDENCE ABOUT THE COMPILER, not as a submittable body.
 */

void motion_Close(void) {
    void (**p)(void);
    s32 count;

    p = (void (**)(void))D_800A2668;
    if (p != 0) {
        count = (s32)&D_00000000;
        p = &D_8008D070;
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
