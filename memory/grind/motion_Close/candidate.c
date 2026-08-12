/*
 * motion_Close — BEST HONEST PURE-C FORM (session 2, structural).
 *
 * Honest sandbox floor with this body: 17 (target 26 insns, build 25).
 * Session 1's form scored 20; the only change here is STATEMENT ORDER —
 * `count` is assigned before `p` inside the guard (and declared first, which
 * on its own does nothing: session 1's H2 measured decl-order alone as inert).
 * GCC emits the two address materializations in source order, so assigning
 * count first puts the $s0 pair at insns 7-8 and the $s1 pair at 9-10, which
 * is the target's register ORDER at those slots (target: $s0 pair then $s1
 * pair). Worth 3 points. Nothing else in 20+ measured spellings moved it.
 *
 *   target                          this form
 *   ------                          ---------
 *   lui/lw   t0 <- D_800A2668       lui/lw   v0 <- D_800A2668
 *   addiu sp,sp,-16                 addiu sp,sp,-32          <-- FRAME (H1: dead)
 *   sw s0,4 / s1,8 / ra,12          sw ra,24 / s1,20 / s0,16 <-- SAVE OFFSETS
 *   beqz t0 ; nop                   beqz v0 ; sw s0,16(sp)   <-- delay-slot fill
 *   lui/addiu s0 (= p)              lui/addiu s0 (= count)   <-- POSITION matches
 *   lui/addiu s1 (= count)          lui/addiu s1 (= p)       <-- ROLE inverted
 *   beqz s1 ; nop                   beqz s0 ; nop
 *   lw t0,0(s0) ; addiu s0,s0,4     lw v0,0(s1) ; addiu s1,s1,4
 *   jalr ; addiu s1,s1,-1           jalr ; addiu s0,s0,-1
 *   bnez s1 ; nop                   bnez s0 ; nop
 *   lw ra/s1/s0 ; addiu sp,sp,16    lw ra/s1/s0 ; addiu sp,sp,32
 *   jr ; nop                        jr ; nop
 *
 * The 17-point residual attributes exhaustively (block-level, sums to the
 * measured score): 2 guard-load temp ($t0 vs $v0) + 1 frame size + 3 save
 * block (offsets/order) + 2 delay-slot (target wastes it, GCC fills it with a
 * pending save) + 1 inner guard register + 4 loop-body registers + 4 epilogue
 * offsets = 17.
 *
 * WHY THE FRAME CANNOT BE REACHED IN PURE C (session 1, H1): gcc-2.7.2's o32
 * backend reserves REG_PARM_STACK_SPACE = 16 bytes of outgoing-arg area for
 * every C-level call, so any pure-C body with a call has frame >= 16 + 12 = 28
 * -> 32. The target's 16-byte frame back-solves to a ZERO arg area; a census of
 * all 854 call-making functions in the oracle build found none below 16 except
 * this function and its inline-asm sibling. Axis closed.
 *
 * WHY THE $s0/$s1 ROLES CANNOT BE REACHED IN PURE C (session 2, H3 — measured
 * with the instrumented cc1, BB2_ALLOC_DEBUG=1). GCC 2.7.2's global.c sorts
 * allocnos by  floor_log2(n_refs) * n_refs / live_length  and hands out hard
 * regs in that order, so the higher-priority allocno takes $s0. For EVERY
 * spelling that emits the target's instruction sequence:
 *      count: n_refs=8, live_length=8  -> pri 30000   (takes $s0)
 *      p:     n_refs=7, live_length=7  -> pri 20000   (takes $s1)
 * count is structurally one reference richer than p (init + outer guard test +
 * in-loop decrement set/use + loop test, versus init + in-loop load + in-loop
 * bump), and the target's own instruction sequence contains all of those same
 * references. The target has p in $s0 — the inverse. Raising p to 8 refs needs
 * a reference that emits no byte, i.e. a dead coercion construct (forbidden);
 * the two spellings that DO flip the roles (an end-pointer loop, and a guard
 * laundered through p) both emit extra instructions the target does not have,
 * so they cannot match by construction. Banked in rejected/.
 *
 * Two independent hand-written-asm signals for the escalation packet (S-a: no
 * outgoing-arg area despite making a call, unique in the corpus; S-b: an
 * unfilled beqz delay slot that reorg.c demonstrably fills) — see evidence.md.
 * Session 2 adds S-c: the target's register ASSIGNMENT is likewise not
 * producible by global.c from any C with the target's instruction sequence.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
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
