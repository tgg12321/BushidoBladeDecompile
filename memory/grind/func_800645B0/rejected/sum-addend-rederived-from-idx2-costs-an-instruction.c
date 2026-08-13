/* REJECTED — func_800645B0, grind session 9 (structural), sweep31.
 *
 * WHY IT IS DEAD: re-deriving the *3 sum's second operand ARITHMETICALLY from
 * `idx2` (so that no staging copy exists at all, and the addend is therefore a
 * pseudo other than the destination `idx`, which is what optabs.c:403-421 needs
 * in order NOT to swap the commutative pair) always pays at least one real
 * instruction for the re-derivation.  The target is 78 instructions; every
 * member of this family assembles to 79.
 *
 * MEASURED (`sandbox func_800645B0 --disable all`, SB chassis, control 1/78):
 *   idx = idx2 + (idx2 >> 1);                    ->  2 / 79
 *   idx = idx2 + (s32)(((u32)idx2) >> 1);        ->  2 / 79
 *   idx = idx2 + (idx2 / 2);                     -> 14 / 79
 *   idx = (idx << 1) + idx2 - idx2 + idx2;       -> 15 / 78  (folds back to the
 *                                                  swapped form)
 *
 * GCC 2.7.2 will not simplify `(ashiftrt (ashift x 1) 1)` back to `x` (it is
 * only equal for values whose top two bits agree, which the compiler cannot
 * know here), and the unsigned spelling costs the same single shift.  The
 * signed division spelling additionally emits the round-toward-zero bias
 * sequence.
 *
 * WHAT SURVIVED INSTEAD: the staging spelling (`val = idx; idx = idx2 + val;`,
 * sweep30 WC) reaches 0 / 78 because the copy is coalesced away.  See
 * candidate.c.  This file exists so no later session re-derives the
 * "avoid the copy by recomputing the addend" idea.
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
                idx = idx2 + (idx2 >> 1); /* 2 / 79 — the re-derivation costs a shift */
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
