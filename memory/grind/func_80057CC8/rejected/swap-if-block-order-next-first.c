/* REJECTED s2 — floor 31 (baseline candidate = 3). Structural probe: swap the
 * order of the two IF blocks (next_idx compute first, prev_idx reload second).
 * Result: build_insns=110 vs target 111 — statement-order change lets GCC fuse
 * or elide something, and the whole codegen shape shifts drastically. Prev_idx
 * reload after next_idx block is strictly worse. KILLED. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    /* body identical to candidate.c except the two if-blocks are swapped:
     * next_idx { s32 tmp = arg1+1; ... } appears BEFORE `if((s16)prev_idx<0)`
     */
}
