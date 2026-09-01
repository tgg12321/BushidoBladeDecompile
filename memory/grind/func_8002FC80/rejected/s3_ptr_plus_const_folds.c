/* REJECTED s3 (2026-08-31): pointer-plus-constant store spelling.
 * Hypothesis: expr.c:4567 sets MEM_IN_STRUCT_P=1 for INDIRECT_REF over a
 * PLUS_EXPR address, so `*((s32 *)0x1F800300 + 0x18) = v1 - v2;` would set
 * the flag on a fixed address and kill the sched.c:817 exemption like the
 * VECTOR form does. MEASURED: sandbox --disable all = 34 @ 73/74 —
 * IDENTICAL to the plain spelling. The C front end constant-folds the
 * pointer arithmetic before expand, so no PLUS_EXPR survives and the flag
 * stays 0. KILL: only genuinely aggregate-typed accesses (COMPONENT_REF /
 * real-array ARRAY_REF, expr.c:4888/4577) set the flag at a constant
 * address. Do not retry +0x19/... offset variants; the fold is uniform. */
    v1 = a1[0];
    v2 = a0[0];
    *((s32 *)0x1F800300 + 0x18) = v1 - v2;
    /* ... x6 blocks, same islands as candidate.c ... */
