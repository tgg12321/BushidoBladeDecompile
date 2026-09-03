/* REJECTED (grind s19, 2026-09-02, synthesis).  Score 12 / 78 at 78 build insns.
 *
 * The a2 multiply chassis (`idx = idx * 12;`, 3/78 control) with the function's
 * return value staged through `idx` (`idx = 1; return idx;`).  This was the one
 * second-set carrier for pseudo 74 that s18's frontier had NOT priced: s18 tried
 * the masked random (2/78), the OR result (2/78), the byte offset (12/78) and the
 * PRE-LOOP constant (16/78), but never the POST-loop constant.
 *
 * The set survives cse/flow (the loop head at stream 11/12 and the back-edge delay
 * slot at 65 are recovered, exactly as with every other surviving second set), but
 * the residual lands on the identical seat rotation the a1 byte-offset write-back
 * produces: idx = $s1 / idx2 = $s0 where the target swaps them, the sum temp taking
 * $v1, and the D_800A347C pointer displaced into $a0.  Insn-for-insn diff in
 * tmp/grind/func_800645B0/s19 (e1.o).
 *
 * Measured the same session on three other chassis: a5 3->12 (same rotation),
 * a1 12->12 (inert, idx already double-set), SB 1->1 (inert), n4 3->3 (inert),
 * and the WD fresh-dest chassis 3->3 (inert -- there the constant is propagated
 * into the return move and the set is deleted before flow counts it).
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx * 12;
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    idx = 1;
    return idx;
}