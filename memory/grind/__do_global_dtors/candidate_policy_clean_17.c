/*
 * motion_Close — BEST POLICY-CLEAN FORM. Honest sandbox floor 17
 * (target 26 insns, build 25).  This was session 2's candidate.c and remains
 * the best body that contains NO construct requiring a policy ruling: two
 * ordinary locals, an ordinary guard, an ordinary function-pointer-table walk.
 *
 * Session 3 lowered the measured floor to 16, but only with a type-punned
 * variable reuse (the guard value carried in `p`) whose legality is an open
 * question — see `candidate.c` and evidence.md.  If that ruling goes against
 * the reuse, THIS is the form to fall back to, and 17 is the policy-clean
 * floor.
 *
 * The one lever that produced it (session 2): assign `count` before `p` inside
 * the guard.  GCC emits the two address materialisations in source order, so
 * count-first puts the $s0 lui/addiu pair at insns 7-8 and the $s1 pair at
 * 9-10, matching the target's register ORDER at those slots.  Worth 3 points
 * over session 1's form; 27 further spellings measured in s2 and 12 more in s3
 * were all inert or worse.
 *
 * NOTE the coupling discovered in session 3: this form's count-before-p order
 * is correct ONLY while count owns $s0.  In the role-flipped family (candidate.c)
 * the target's emission order wants p first instead.
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
