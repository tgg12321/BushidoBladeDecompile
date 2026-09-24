/*
 * REJECTED (session 3) — score 22 (worse than the then-floor of 17).
 *
 * IDEA. The allocno tie-break in global.c is decided by declaration order, so
 * instead of RAISING p's priority to meet count's (which costs the prologue —
 * see candidate.c), lower count's.  flow.c weights REG_N_REFS by loop depth, so
 * count's 8 references decompose as init(1) + outer guard test(1) + in-loop
 * decrement set/use(2+2) + loop test(2).  Staging the outer guard test through
 * a separate short-lived local `n` should remove count's outer-test reference,
 * dropping it to 7 and tying p's 7 — at which point declaring p first would win
 * it $s0 with no type-punned reuse anywhere.
 *
 * WHY IT FAILS (measured, instrumented cc1 BB2_ALLOC_DEBUG=1):
 *      count (staged): n_refs=7 live_length=6 -> pri 23333   -> $s0
 *      p:              n_refs=7 live_length=9 -> pri 15555   -> $s1
 * Removing the outer test does drop count to 7 refs, but it also shortens
 * count's LIVE LENGTH from 8 to 6 (count is now born inside the inner guard),
 * and the priority is refs/live_length — so count's priority went UP, not down,
 * from 30000 to 23333 relative to p, which simultaneously grew to live_length 9
 * because `n` extends the region p spans.  The gap widened rather than closing,
 * and the extra local costs 3 more points of positional residual (22).
 *
 * Measured in all three declaration/body-order arrangements
 * (u1_staged_guard_local, u2_staged_guard_local_countfirst,
 * u4_staged_countinit_inside in tmp/grind/motion_Close/s3/rolesweep.py) — all
 * three produce the identical allocno table and the identical score 22, so the
 * declaration-order tie-break never engages.
 *
 * GENERAL LESSON for future sessions: you cannot lower an allocno's priority by
 * removing references, because references and live length shrink together and
 * the ratio moves the wrong way.  The only direction that works on this
 * function is ADDING references to the allocno you want to win.
 */

void motion_Close(void) {
    void (**p)(void);
    s32 count;
    s32 n;

    if (D_800A2668 != 0) {
        n = (s32)&D_00000000;
        p = &D_8008D070;
        if (n != 0) {
            count = n;
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
