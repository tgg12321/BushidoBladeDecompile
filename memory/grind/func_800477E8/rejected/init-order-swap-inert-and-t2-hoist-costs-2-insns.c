/* REJECTED (s1, 2026-07-30) — two ordering probes against the allocation ties.
 *
 * (a) Reordering the tied pair's initialisers is INERT for allocation:
 *       `a1 = 0; v1 = 1;`  ->  `v1 = 1; a1 = 0;`
 *     MEASURED: 17 -> 17. The re-diff showed idx 32/33 had merely traded
 *     emission slots (`li a1,1` / `move v1,zero`) with the SAME hard registers.
 *     Statement order reorders emission, it does not touch the tie.
 *
 * (b) Hoisting the `t2` constant above the last gpu_CalcClut call, to make it
 *     the earlier-numbered pseudo:
 *     MEASURED: 17 -> 26 with build_insns 172 (+2) — `t2` becomes live across
 *     the call and forces a callee-save/restore pair. Strictly worse.
 *
 * The combined probe form that was measured:
 */

    s1val = gpu_CalcClut(0x10, 0x1E0);
    t2 = 0x2C00;                        /* (b) hoisted above the call */
    t1val = gpu_CalcClut(0x10, 0x1E0);
    a3 = 0;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        v1 = 1;                         /* (a) swapped with a1 = 0 */
        a1 = 0;
        /* ... body unchanged ... */
