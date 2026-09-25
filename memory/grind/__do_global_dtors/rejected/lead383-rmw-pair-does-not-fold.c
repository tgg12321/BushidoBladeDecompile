/*
 * REJECTED (session 5) — the session-4 "unmeasured lead" output-383-1 measured
 * at last, and both of its constructs are dead.
 *
 * MEASUREMENT (tmp/grind/motion_Close/s5/sweep6.log):
 *   e0_lead383_asis      score 18, build_insns 28   (the permuter's form verbatim)
 *   e2_rmw_only          score 18, build_insns 28   (the RMW pair alone)
 *   e1_stage_guard_only  score 17, build_insns 25   (the guard staging alone)
 *   (session-4 floor for comparison: 16; session-5 floor: 13)
 *
 * WHY THE RMW PAIR IS DEAD ON BOTH LEGS OF THE SESSION-4 DILEMMA.
 * `D_800A2668++; D_800A2668--;` does NOT fold: build_insns rises 25 -> 28, i.e.
 * GCC emits the load/add/store and the load/sub/store against the guard global.
 * That is the second horn session 4 predicted — the form stores to a global the
 * target never stores to, so it is a BEHAVIOURAL DIFFERENCE from the target,
 * not a match device. (Had it folded it would have been a T1/T2 coercion.) It
 * is worse than the chassis it was mutated from in every respect: +3 insns and
 * +2 score. No ruling is needed on it: it is not a candidate for one, because
 * the construct is semantically wrong, not merely policy-doubtful.
 *
 * WHY THE GUARD STAGING IS DEAD. `count = D_800A2668 != 0;` lifts count, not p:
 * count goes 8 refs / live_length 8 (pri 30000) -> 10 refs / live_length 8
 * (pri 37500), which widens count's lead over p instead of closing it. It
 * scores 17, i.e. worse than session 4's 16 and far worse than session 5's 13,
 * in every arrangement measured: alone (e1, 17), composed with the do-while(0)
 * wrap on p (e3, 17 — p reaches only 34285 against count's 37500), with p
 * declared first (e6, 17), with p assigned first (e4, 20), and with p assigned
 * first inside a wrap (e5, 20). This is the general rule session 3 already
 * extracted, re-confirmed: on this function you cannot help p by touching
 * count.
 *
 * CONCLUSION: the permuter's weighted 383 was an artifact of its scoring model
 * (it charges 100 per insertion/deletion and 60 per reordering; the RMW pair's
 * three extra instructions happened to align blocks favourably in ITS metric
 * while costing engine distance). Session 4's warning that permuter score is
 * not a proxy for engine distance on this function is confirmed a third time.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    count = D_800A2668 != 0;
    D_800A2668++;
    D_800A2668--;
    if (count) {
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
