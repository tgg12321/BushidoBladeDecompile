/* REJECTED s2 — floor 12 (baseline candidate = 3). Structural probe: hoist the
 * p1 pointer add ABOVE the next_idx block (compute p1 immediately after the
 * prev_idx reload, before next_idx tmp compute). Result: floor 12, worse.
 * The target ORDERING is next_idx block first, THEN both p1 pointer add and
 * loads. Reordering p1 above next_idx destroys the delay-slot scheduling
 * (target packs prev_idx sll into the next_idx bnez delay slot at 80057D48).
 * KILLED. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    /* body: p = (s16*)((((s32)(prev_idx<<16)>>16)<<2) + (s32)table)
     * is placed BETWEEN the prev_idx if-reload and the next_idx block. */
}
