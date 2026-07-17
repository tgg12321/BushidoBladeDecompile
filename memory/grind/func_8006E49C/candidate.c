/* func_8006E49C — s2 candidate: single reused-t accumulator (sandbox 28).
 *
 * STRUCTURALLY EXACT: all 38 insns in target order, target opcodes, target
 * immediates. The ONLY residual is a clean 2-register swap: our build puts
 * the cse 0x9C40 constant in $v0 and t in $v1; target has const=$v1, t=$v0.
 * A diagnostic pin-probe (t asm("$2"), step asm("$3") — NEVER committable)
 * over this exact body compiles to a 38/38 EXACT match (s2 artifacts:
 * pinprobe.c/.s), proving the residual is allocation-only.
 *
 * Mechanism (named, local-alloc.c:472 + find_free_reg ascending scan):
 * the cse constant pseudo (1 death) is local-allocated first and takes $v0;
 * t is multi-death (a0-based redefs forced by target bytes) -> global-alloc
 * -> finds $v0 occupied -> $v1. Next lever must evict the const from $v0
 * WITHOUT splitting the t-chain (every split un-pins that segment's store
 * in sched1 — measured dead across all 6 group-1 segments, s2 ledger).
 *
 * The accumulator (arg0 += K) + single reused t are REAL semantics; no dead
 * stores, no banned constructs. (Alternative all-expression-temps spelling
 * scores 25 by raw count but with large reorderings — this form is the
 * correct permuter/lever base.)
 */
s32 func_8006E49C(s32 arg0, s32 *arg1) {
    s32 t;
    arg1[0] = arg0;
    arg0 += 0x9C40;
    t = arg0 + 0x5DC0;
    arg1[2] = t;
    t = arg0 + 0x61F8;
    arg1[1] = arg0;
    arg0 += 0x6838;
    arg1[3] = t;
    t = arg0 + 0x1B58;
    arg1[5] = t;
    t = arg0 + 0x1DB0;
    arg1[6] = t;
    t = arg0 + 0x1E28;
    arg1[7] = t;
    t = arg0 + 0x1EA0;
    arg1[4] = arg0;
    arg0 += 0x1FB0;
    arg1[0xB] = arg0;
    arg0 += 0x9C40;
    arg1[8] = t;
    t = arg0 + 0x5DC0;
    arg1[0xD] = t;
    t = arg0 + 0x61F8;
    arg1[0xC] = arg0;
    arg0 += 0x6838;
    arg1[0xE] = t;
    t = arg0 + 0x1B58;
    arg1[0x10] = t;
    t = arg0 + 0x1DB0;
    arg1[0x11] = t;
    t = arg0 + 0x1E28;
    arg1[0x12] = t;
    t = arg0 + 0x1EA0;
    arg1[0x13] = t;
    t = arg0 + 0x1FB0;
    arg1[0xF] = arg0;
    return t;
}
