/*
 * REJECTED (session 3) — score 20 (worse than the session-3 floor of 16).
 *
 * IDEA. In the role-flipped family (candidate.c: p owns $s0, count owns $s1),
 * the target's emission order wants the $s0 pair FIRST — target insns 8-9 are
 * `lui/addiu s0` (= p) and 10-11 are `lui/addiu s1` (= count).  Session 2's
 * count-before-p statement order was derived when count owned $s0 and is
 * therefore backwards in the flipped family.  So: keep the guard carried in p
 * (which creates the priority tie) and assign p before count in the body.
 *
 * WHY IT FAILS (measured, instrumented cc1 BB2_ALLOC_DEBUG=1):
 *      count: n_refs=8  live_length=7  -> pri 34285  -> $s0
 *      p:     n_refs=9  live_length=10 -> pri 27000  -> $s1
 * Assigning p first moves count's birth one statement later, shortening its
 * live length from 8 to 7 and RAISING its priority from 30000 to 34285, while
 * simultaneously stretching p's live length from 9 to 10 and LOWERING p's from
 * 30000 to 27000.  The exact tie that declaration order was breaking is
 * destroyed, count takes $s0 back, and the roles revert.
 *
 * This is the coupling that defines the session-4 frontier: the statement order
 * that fixes the two materialisations' POSITIONS is the same statement order
 * that destroys the priority tie fixing their ROLES.  Any form that closes the
 * remaining gap has to satisfy both at once — e.g. by giving p a reference that
 * does not extend its live range, or by lengthening count's live range without
 * adding a count reference.
 *
 * Measured twice, identically: t5_pdecl_pguard_ptrorder
 * (tmp/grind/motion_Close/s3/tiebreak.py) and w5_pfirst_count_inside
 * (tmp/grind/motion_Close/s3/ordersweep.py).
 */

void motion_Close(void) {
    void (**p)(void);
    s32 count;

    p = (void (**)(void))D_800A2668;
    if (p != 0) {
        p = &D_8008D070;
        count = (s32)&D_00000000;
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
