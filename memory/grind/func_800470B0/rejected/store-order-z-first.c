/* REJECTED — score 8 (worse than the 7 the 5,6,7 order gave at the time).
 * Mirror of store-order-z-last.c: move `arg2[6] = arg3;` to FIRST. Same outcome —
 * one extra store-order mismatch, s1/s2 swap untouched. KILLED.
 */
    arg2[6] = arg3;
    arg2[5] = var_s0[5] + (((var_s0[6] - arg3) * sp10.m[0][1]) >> 12);
    arg2[7] = var_s0[7] + (((var_s0[6] - arg3) * sp10.m[2][1]) >> 12);
