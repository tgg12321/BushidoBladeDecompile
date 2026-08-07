/*
 * s59 H3 rejected form: nested-compound PERM_LINESWAP over the two poll-loop
 * callback if-blocks. First measured chassis-shape whose PERM_LINESWAP was
 * genuinely parsed (not the s50/s58 random-mode fallback) — base_score=60,
 * "Will run for 2 iterations" printed at startup, 2/2 iters completed.
 *
 * Ordering measurements (weighted permuter score):
 *   identity (status&4 first, status&2 second):  60  (= h5 baseline masked=2)
 *   swap     (status&2 first, status&4 second): 125  (+65 regression)
 *
 * The 2-combination cross-product is exhausted; the poll-dispatch mutation
 * neighborhood at this granularity is CLOSED. Non-identity ordering pushes
 * the callback dispatch scheduling away from target.
 *
 * KEY POSITIVE FINDING for future sessions: PERM_LINESWAP IS parseable inside
 *   `if (status != 0) { { PERM_LINESWAP(...) } goto poll; }`
 * — i.e. an extra explicit brace pair inside an existing compound gives the
 * pycparser AST a clean scope for the directive. The s50/s58 "no perm macros
 * found" fallback was NOT a global tool limitation on this function; it was a
 * scope-specific parse rejection. This unlocks directed permuter at any
 * function-body scope that can be wrapped in an extra `{ }`.
 */

/* Rejected mutation: */
if (status != 0)
{
  {
    if (status & 2) { if (D_800A11B4 != 0) { ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0); } }
    if (status & 4) { if (D_800A11B8 != 0) { ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8); } }
  }
  goto poll;
}
