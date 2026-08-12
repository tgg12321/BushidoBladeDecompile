/* REJECTED — func_800645B0, grind session 3 (2026-08-12).
 *
 * WHY DEAD: adding a loop-TAIL `idx = i + j;` on top of the loop-TOP
 * recomputation (i.e. trying to get `reg_n_sets[idx] == 2` while keeping the
 * CA shape's `wid = idx2 + idx;` operand order) changes NOTHING: the tail
 * store is dead — the loop top recomputes `idx` on the next iteration and the
 * break path never reads it — so flow.c deletes it before reg_n_sets is taken,
 * exactly as session 2's H10 found for copies and split-inits.
 *
 * MEASURED: JA = 3 / 78, byte-identical to the CA control (3 / 78).
 * Sweep: tmp/grind/func_800645B0/s3/sweep14.py.
 *
 * This closes the last "cheap" way to satisfy constraint (b) (a real second
 * set of `idx`) without paying for it: the tail recomputation is only alive if
 * the loop top does NOT recompute, which is the maintained-index form
 * (see maintained-index-entry-copy-const-folds.c).
 */
s32 func_800645B0(void) {
    s32 i, j, idx, idx2, wid, mask, val, last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;              /* single surviving set */
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                /* ... stores ... */
                break;
            }
            idx = i + j;              /* DEAD — deleted by flow.c */
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
