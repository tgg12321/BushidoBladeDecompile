/* SESSION s16 (2026-09-01, escalation).  1 / 78 -- IDENTICAL to the SB floor,
 * same single residual at index 20 (`addu s0,s0,s1` vs target `addu s0,s1,s0`).
 *
 * WHAT IT TESTS.  The *3 word offset spelled as an honest multiply
 * (`idx = idx * 3;`) instead of the hand-expanded `idx = idx2 + idx;`.  The
 * hope was that expand_mult's synth_mult would emit `addu dst, <t=idx<<1>, idx`
 * -- target's operand order -- rather than going through expand_binop's
 * commutative swap.
 *
 * RESULT: byte-identical to SB.  GCC 2.7.2 reduces `idx * 3` to
 * (plus (ashift idx 1) idx), CSE unifies the ashift with the already-live
 * `idx2 = idx << 1`, and the PLUS is then expanded by the SAME expand_binop
 * call with target == the idx pseudo == op1, so optabs.c:400-419 swaps the
 * operands exactly as it does for the hand-written sum.  The multiply spelling
 * is not a distinct lever; H24/H58's operand-order wall is spelling-invariant
 * for every commutative-PLUS form whose destination is the idx pseudo.
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
                idx = idx * 3;
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
