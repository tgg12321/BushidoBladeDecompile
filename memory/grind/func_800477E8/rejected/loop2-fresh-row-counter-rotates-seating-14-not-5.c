/* REJECTED (s3, structural).  Why it is dead: loop2's row counter must be the
 * SAME C variable as loop1's row counter `a3`.  A fresh local for it scores 14
 * where sharing scores 5.
 *
 * Mechanism (cc1 -da `.greg`, model priority = floor_log2(n_refs)*n_refs/live_length):
 *   fresh local r : 7 refs / 12 insns = 1.17  -> allocated 3rd of the loop2 group
 *   shared a3     : 18 refs / 86      = 0.84  -> allocated LAST, which is what
 *                                               makes it take $a3, as target has
 * The loop2 group must be allocated in the order p, val, a0, rowptr, w, counter
 * so that greedy lowest-free assignment hands out 2, 3, 4, 5, 6, 7.  With a fresh
 * counter at 1.17 it lands third and takes $a0's register, rotating every later
 * member of the group by one.  No spelling of a fresh counter reaches a priority
 * below w's 1.00: with 7 refs it needs live_length > 14, and its range is the
 * loop2 body (12); with 5 refs it needs > 14 as well.
 *
 * This is the reverse of the s2 lesson.  s2 SPLIT loop2's row counter out of a3
 * to free loop1, and that was right for s2's goto-shaped loop2 (no loop notes, no
 * reference weighting).  Once loop2 is a real loop nest the weighting appears and
 * the sharing becomes mandatory again.
 *
 * Also dead for the same reason, measured this session: giving loop2 its OWN
 * inner counter instead of sharing `a0` (the compiler-made walking pointer then
 * loses $v0), and splitting loop3's index off `a0` (score 32).
 */

    /* ... loop1 exactly as in candidate.c ... */

    r = 0;                                  /* <- fresh local, NOT a3: score 14 */
    w = 0;
    do {
        val = w;
        p = &D_800EF59C[r * 0x11];
        for (a0 = 0x10; a0 >= 0; a0--) {
            p[a0] = val;
        }
        w += 0x7D0;
        r += 1;
    } while (r < 9);
