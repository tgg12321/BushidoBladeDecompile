/* SESSION 16 (2026-09-01, forensics) -- 2 / 78, the BEST form the WD family has
 * ever reached, and the form that FALSIFIES H59's "closed-form KILLED".
 *
 * H59 (session 12) concluded that target's operand order at index 20 and
 * target's inner-loop-head placement are mutually exclusive, because the
 * former needs the *3 sum's destination to be neither operand (a fresh local,
 * dropping reg_n_sets[idx] to 1) and the latter needs reg_n_sets[idx] >= 2.
 * It then asserted the function computes no second real value that can land in
 * idx.  THAT LAST STEP IS WRONG.  `idx = last & 7;` -- borrowing the (dead
 * after the sum) slot index to hold the masked random value that the halfword
 * store consumes -- is a second, semantically REAL write, and critically it is
 * NOT A COPY, so no pass deletes it (contrast the s15 cse.c kill and this
 * session's combine.c kill of `idx = rand();`).
 *
 * RESULT, honest `sandbox func_800645B0 --disable all`: score 2, 78 insns.
 *   - index 20 operand order:      EXACT  (addu s0,s1,s0)
 *   - inner-loop head 11/12:       EXACT
 *   - back-edge delay slot 65:     EXACT (the target's addu is stolen)
 *   - RESIDUAL, both diffs:  55 OURS `andi s0,v0,7` / TGT `andi v0,v0,7`
 *                            58 OURS `sh   s0,X(at)` / TGT `sh   v0,X(at)`
 * i.e. what remains is a pure REGISTER-SEAT question: the masked value must
 * stay in the rand-return seat ($v0) instead of being computed into idx's
 * callee-saved seat ($s0).  That is ra_solver territory (and the owner's
 * 2026-09-01 Ruling C `--target-object` escape now makes inverse_compose
 * classify usable on this INCLUDE_ASM function).
 *
 * NOT banked as the candidate: the standing floor is still the SB chassis at
 * 1 / 78.  Banked here so the next session inherits the both-halves chassis.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
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
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                idx = last & 7;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = idx;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
