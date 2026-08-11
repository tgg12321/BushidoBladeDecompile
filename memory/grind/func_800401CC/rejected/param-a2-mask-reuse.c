/*
 * REJECTED — session 2 probes P1/P2: reusing the dead a2 PARAM as the tail
 * mask holder. Both directions measured broken. Root cause (both): a2's pseudo
 * spans head (parity value, needs $6 = its param preference) AND tail (mask);
 * global-alloc must pick ONE hard reg for both ranges; local-alloc runs FIRST
 * and a tail-local mask qty always grabs $6 before a2's preference is
 * considered, so a2 lands $7 and the HEAD breaks (andi/beq/sw parity cluster
 * renames $6 -> $7 = 4 diffs).
 *
 * P1  a2 = 0xFFFFFF;   score 12 (head 4 + tail masks still swapped ~7)
 *     QTYDBG: FF000000 local qty birth=22 refs=3 got=$6; a2 global -> $7.
 * P2  a2 = 0xFF000000; score 7  (tail ANDs MATCHED — FFFFFF local qty got $6,
 *     a2 -> $7 which is target for FF000000 — but head 4 diffs + li-order 3).
 *     P2's partial success identified the winning mechanism: the FF000000
 *     holder must be a global-alloc'd multi-set pseudo with a $7 preference
 *     and NO head-register commitment -> that variable is `v` (the SetDrawMove
 *     4th arg), which closed the function (see candidate.c).
 */
void func_800401CC(s32 a0, s32 a1, s32 a2) {
    /* ... identical to floor-7 form except the tail: ... */
    /* P1: */ /* a2 = 0xFFFFFF;   *pkt = (*pkt & 0xFF000000) | (otw & a2); ... */
    /* P2: */ /* a2 = 0xFF000000; *pkt = (*pkt & a2) | (otw & 0xFFFFFF); ... */
}
