/* REJECTED (s2, structural) — score 13 (from floor 8). KILLED (2 forms).
 * Region A: reorder the three inner-loop inits (j, bp, sum) to bias sum's
 * qty ahead of j. Both `bp; sum; j` (VA-2) and `sum; bp; j` (VA-3, == the
 * inherited V11) give score 13.
 * WHY DEAD: the register hand-out order for this class/range is v1 -> a0 -> a1
 * for the 1st/2nd/3rd-allocated qty. Baseline order (j;bp;sum) yields
 * bp=v1(1st), j=a0(2nd), sum=a1(3rd). Moving bp's init later RAISES bp's
 * live-range birth so bp DROPS below j in priority; j (or bp) then grabs the
 * v1 slot and bp cascades into a0. Measured qty_order flips:
 *   baseline  : 100 78(bp) 79(j) 77(sum)  -> bp=v1, j=a0, sum=a1  (score 8)
 *   bp;sum;j  : 100 79(j) 78(bp) 77(sum)  -> bp=a0, j=v1, sum=a1  (score 13)
 *   sum;bp;j  : 100 79(j) 78(bp) 99 77    -> bp=a0, j=v1, sum=a1  (score 13)
 * Source init order does NOT surgically swap sum/j; it always disturbs bp.
 * Target needs priority bp>sum>j; source reordering cannot produce it.
 */
        bp = base + offset;
        sum = 0;
        j = 0;
        /* ... loop unchanged; if (sum == chk) break; ... */
