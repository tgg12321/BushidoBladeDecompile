/* REJECTED (s3, structural).  Why it is dead: converting loop1's inner
 * `goto inner` into a real loop is the obvious way to raise the reference WEIGHT
 * of everything in loop1 (loop_depth 2 -> 3), and the model says that flips tie A:
 *     with depth 2 : t1val 1+2 = 3 refs / 76  = 0.0395
 *                    t2    1+2+2 = 5 / 150    = 0.0667   -> t2 wins $t1
 *     with depth 3 : t1val 1+3 = 4 refs / 76  = 0.105
 *                    t2    1+3+3 = 7 / 150    = 0.093    -> t1val wins $t1
 * It does not work, because the conversion is not shape-neutral: cc1 manufactures
 * TWO extra pseudos for the inner loop and they take $t1 and $t2, pushing
 * t1val -> $t3(11) and t2 -> $t4(12).  Measured on the .greg dispositions:
 *     inner-dowhile : 89->10  90->9   76->11  83->12   score 10, insns 170
 *     inner-for     : 90->10  91->9   76->11  83->12   score 13, insns 170
 * Both are strictly worse than leaving the inner loop as a goto (score 5).
 *
 * This is the same failure mode s2 recorded for a FULL nested-for rewrite of
 * loop1 (t2 -> $t4, t1val -> $t3) and it now has two more data points: it is the
 * inner loop's mere existence as a recognised loop that creates the extra
 * induction pseudos, not the amount of rewriting.  loop1's inner loop must stay
 * goto-shaped.  Note the ASYMMETRY with loop2, where making the loops real is
 * exactly what was required — loop2's counters are consumed as an index, loop1's
 * are consumed as packed UV values, so cc1 builds ivs for them differently.
 */

        v1 = 1;
        do {                                /* was: inner: */
            /* ... loop1 body unchanged ... */
            a0 += 1;
            v1 += 1;
        } while (a0 < 0x10);                /* was: if (a0 < 0x10) goto inner; */
        a3 += 1;
        a0 = 0;
