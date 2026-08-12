/*
 * REJECTED (session 11, modality `structural`) — MEASUREMENT-ONLY cell M1: a
 * self-assign `p = p;` is invisible to flow.c's reference counter on this
 * function, so the sanctioned dead-store family cannot substitute for the
 * do-while(0) wrap.
 *
 * WHY IT WAS MEASURED. The FROZEN SOTN-accepted list sanctions "dead stores /
 * self-assigns to LOCALS or PARAMS" (last-resort, FAKE-annotated, prerequisites
 * in .claude/rules/dead-store-fake-exception.md). The obvious question a future
 * session would ask is whether a self-assign could give p the extra weighted
 * reference the wrap currently buys, and thus reach floor 13 at wrap depth 0 —
 * which would delete the only FAKE-annotated construct in the candidate form.
 * The answer had to be a measurement, not an argument, because the competing
 * claim (.claude/rules/duplicated-statement-into-arms.md: "dead stores measured
 * INERT for this — flow deletes before counting") was established on a different
 * function.
 *
 * THE MEASUREMENT. tmp/grind/motion_Close/s11/f13sweep.py cell M1_self_assign,
 * at wrap depths 0/1/2, sandbox + instrumented cc1 (BB2_ALLOC_DEBUG=1):
 *   depth 0:  p 4 refs / live_length 8 = 10000   count 5/7 = 14285   score 20
 *   depth 1:  p 5/8 = 12500                      count 5/7 = 14285   score 20
 *   depth 2:  p 6/8 = 15000                      count 5/7 = 14285   score 13
 * That is the control ladder VALUE FOR VALUE (V0_control_d0/d1/d2), with the
 * same 25 emitted instructions. The self-assign does not appear in the
 * reference count at any depth: jump.c / cse delete the no-op set before
 * flow.c's count_reg_references ever sees it, so it changes nothing upstream of
 * the allocator either.
 *
 * CONSEQUENCE. The duplicated-statement-into-arms rule's claim transfers to
 * motion_Close: on this function a dead store is not a ref-lift device. The
 * do-while(0) wrap is not substitutable by the cheaper-looking sanctioned
 * family, and no future session should spend time on the substitution. This
 * form must NEVER be proposed as a candidate — it is here as a disproof, it
 * carries no annotation and no exhaustion record, and it is strictly dominated
 * by the banked candidate at equal score.
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        p = p;                          /* deleted before flow.c counts refs —
                                           p stays at 4 raw references */
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
