/* REJECTED s2 — floor 7 (candidate baseline = 3). Structural probe: branchless
 * direct-bind ternary for prev_idx, no explicit sidx local:
 *   prev_idx = ((s16)(u16)(arg1-1) < 0) ? (u16)(arg0[3]-1) : (u16)(arg1-1);
 * build_insns=110 vs target 111 — a duplicated arg1-1 folded away and the
 * cmov shape did NOT bind prev_idx into the coalesceable single-def live range
 * the second-p enjoys. Ternary shape moves the diff surface but does not
 * improve the p1-addu coalescing. Do not re-derive. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    /* ... same body except: */
    prev_idx = ((s16)(u16)(arg1 - 1) < 0) ? (u16)(arg0[3] - 1) : (u16)(arg1 - 1);
    /* ... rest unchanged from candidate.c ... */
}
