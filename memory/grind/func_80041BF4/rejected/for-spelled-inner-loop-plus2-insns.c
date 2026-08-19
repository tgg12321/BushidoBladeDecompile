/* s8 REJECTED: score 16 at 137 insns. `for (idx = 0; tbl[0] >= 0; idx++)` instead of
 * `idx = 0; while (tbl[0] >= 0) { ... idx++; }`. The for-spelling puts the increment
 * in the loop latch, which changes where `off` is born and costs two instructions. */
    for (idx = 0; tbl[0] >= 0; idx++) { s32 off = idx << 5; ... }
