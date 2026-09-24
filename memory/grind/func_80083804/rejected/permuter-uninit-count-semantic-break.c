/*
 * REJECTED (session 4) — the ONLY sub-base find of the session-4 permuter
 * campaign on the policy-clean chassis (tmp/grind/motion_Close/s4/wsA,
 * output-453-1, permuter score 453 vs base 468), recorded so no future session
 * mistakes it for a lead.
 *
 * It is not a candidate: the permuter deleted `count = (s32)&D_00000000;`
 * outright (replacing it with an empty statement) and moved the loop guard to
 * a test of the ADDRESS constant, so `count` enters the loop uninitialised and
 * the decrement counts down from garbage. It also invents `new_var` purely to
 * hold the loop condition. The form is semantically broken C, not a spelling
 * of motion_Close, and its lower permuter score is bought entirely by deleting
 * a real instruction. (Reminder from the s4 workspace: the permuter's weighted
 * score is NOT the engine distance; everything it proposes has to be
 * re-measured with `sandbox motion_Close --disable all` and vetted for
 * semantics before it means anything.)
 *
 * The campaign's genuinely useful output was output-463-1 (a do-while(0) wrap
 * around the two initialisers), which is semantics-preserving and became this
 * session's floor-16 policy-clean candidate.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);
    int new_var;

    if (D_800A2668 != 0) {
        ;
        p = &D_8008D070;
        if (((s32)(&D_00000000)) != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (new_var = count != 0);
        }
    }
}
