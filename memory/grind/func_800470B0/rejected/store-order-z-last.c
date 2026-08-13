/* REJECTED — score 8 (worse than the 7 the 5,6,7 order gave at the time).
 * Probe: with the s1/s2 swap still open, move `arg2[6] = arg3;` to LAST in the
 * hope that extending arg3's source-level live range past arg2's final store
 * would equalise the two allocnos' live_length and let global.c's
 * allocno_compare tie-break on allocno number (which favours arg2).
 * RESULT: sched1 re-hoisted the `sw` into the same slot, so live_length was
 * unchanged and the swap persisted; the only effect was one extra store-order
 * mismatch. KILLED — statement order among the three destination stores is
 * load-bearing and must stay 5, 6, 7.
 */
    arg2[5] = var_s0[5] + (((var_s0[6] - arg3) * sp10.m[0][1]) >> 12);
    arg2[7] = var_s0[7] + (((var_s0[6] - arg3) * sp10.m[2][1]) >> 12);
    arg2[6] = arg3;
