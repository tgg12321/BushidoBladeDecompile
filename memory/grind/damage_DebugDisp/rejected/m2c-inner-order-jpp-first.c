/* REJECTED (s9, rederive) — score 8 (from floor 6).
 * m2c fresh decompile reconstructs the inner accumulate as `j++; sum+=*bp; bp++`
 * (var_a1+=1; var_a0+=*var_v1; var_v1+=1). Applied ON the floor-6 do-while(0)
 * chassis it REGRESSES to 8: reordering the inner body changes sum's def/use
 * LUID relationship and UNDOES the do-while(0)'s sum=$a0 RA win (Region A
 * reopens). Confirms the candidate's `sum+=*bp; bp++; j++` order is load-bearing
 * for the do-while(0) win — the inner-body order is NOT a free lever on the
 * floor-6 chassis (s5 only exhausted PREHEADER order; s1 tested body order only
 * on the floor-8 plain chassis). Dead.
 */
        do { sum = 0; } while (0);
        do {
            j++;
            sum += *bp;
            bp++;
        } while (j < 0x24U);   /* -> sandbox --disable all = 8 */
