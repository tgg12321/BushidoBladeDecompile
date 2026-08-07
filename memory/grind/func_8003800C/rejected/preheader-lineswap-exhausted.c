/* REJECTED (s5) — Region A' preheader statement reorder is NOT the lever.
 *
 * The residual floor-6 gap has a 2-insn Region A' preheader scheduling tie:
 * target emits the three inner-loop-preheader inits in an order the build
 * (do-while(0) chassis) does not reproduce. Frontier item 1 proposed that a
 * PERM_LINESWAP over {j=0; bp=base+offset; do{sum=0}while(0);} could restore
 * the target order at score<=6.
 *
 * KILLED by exhaustive manual measurement — all 3! = 6 orderings sandboxed
 * (--disable all) on the floor-6 chassis:
 *
 *   (j,  bp, dw) = 6   <-- current candidate, UNIQUELY optimal
 *   (j,  dw, bp) = 10
 *   (bp, j,  dw) = 11
 *   (bp, dw, j ) = 13
 *   (dw, j,  bp) = 10
 *   (dw, bp, j ) = 13
 *
 * where dw = `do { sum = 0; } while (0);`
 *
 * Conclusion: the floor-6 order is a strict local optimum; every neighbor
 * regresses. The Region A' preheader tie is a sched1 whole-function LUID
 * effect, NOT reachable by preheader statement reordering. This closes the
 * PERM_LINESWAP-over-preheader probe. (The example body below shows the
 * worst-regressing (dw,bp,j)=13 form for the record.)
 */
        j = 0;
        do { sum = 0; } while (0);   /* dw moved first */
        bp = base + offset;
        /* ... = score 10; every non-(j,bp,dw) ordering regresses to 10-13 */
