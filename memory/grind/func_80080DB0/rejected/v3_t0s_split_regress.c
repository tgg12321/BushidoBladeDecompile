/* s3 REJECTED: V3 t0s var-split (fresh temp for t0*4).
 * masked 3 (baseline 2) — fresh t0s allocno steals priority, +1 regression.
 * Mechanism: local-alloc.c qty_compare adds a birthed pseudo above t0's
 * qty; the extra allocno participates in the pair birth order.
 */
    s32 t0s;
    /* ... */
    t0s = t0 * 4;
    t0 = (s32)((u8 *)tbl_125c + t0s);
