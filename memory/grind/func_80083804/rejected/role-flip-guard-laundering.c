/* REJECTED (session 2) — flips the $s0/$s1 roles, but emits two instructions
 * the target does not contain, and the construct is a cheat by any reading.
 *
 * Measured: sandbox --disable all score 17, build_insns 27 (target 26).
 * ALLOCDBG: p -> n_refs=9 pri=30000 (takes $s0), count -> n_refs=8 pri=24000.
 * So it IS the only natural-looking spelling found that gives p the $s0 that
 * the target uses. It is still dead on two independent grounds:
 *
 *  (1) BYTES. The laundered guard compiles to `sll v0,s1,2 / addu v0,v0,s0 /
 *      beq v0,s0` — two instructions that do not exist anywhere in the target.
 *      A form that emits bytes the target lacks cannot match, whatever it does
 *      to register allocation.
 *  (2) POLICY. `p != &D_8008D070 + count` is exactly `count != 0` (p was just
 *      set to &D_8008D070 two statements earlier). The comparison has no
 *      semantic purpose a reader could defend; its only effect is to add one
 *      reference to p so global.c's floor_log2(n_refs)*n_refs/live_length
 *      priority tips p above count. That is a GCC-internals justification for
 *      a construct with no program-logic reason — cheat checklist tests T1,
 *      T2 and T3 all fail. Not proposed, not submitted; recorded so no future
 *      session re-derives it.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        count = (s32)&D_00000000;
        p = &D_8008D070;
        if (p != &D_8008D070 + count) {   /* == count != 0, laundered through p */
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
