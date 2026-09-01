/* SESSION s16 (2026-09-01, escalation).  11 / 78 at 78 insns.
 *
 * WHAT IT TESTS.  The h chassis (rejected/both-halves-idx-second-real-nonco- * andi-regseat-2of78.c, 2 / 78) with `last = rand();` moved from BEFORE the *3
 * sum to AFTER it.  Motivation: ra_solver typed h's 2-insn residual FORECLOSED
 * because pseudo 74 (idx) crosses one call, so global.c:897 prune_preferences
 * strips the call-used $v0 from its preferences AND flow records a hard
 * conflict with $v0.  The only C lever on `calls_crossed` is to end idx's live
 * range before the call -- which is what this body does.
 *
 * RESULT: the lever WORKS on the seat and DESTROYS the schedule.  idx does drop
 * to a caller-saved seat -- but $a0, not $v0, and it drags j from $a0 to $a1.
 * More importantly the sum is now available before the call, so reorg.c fills
 * the `jal rand` delay slot with the sum (`addu s0,s1,a0`) instead of the
 * target's `sll s1,s0,0x1`, and the loop head is rewritten:
 *   10 ours `move a1,zero`      / tgt `move a0,zero`
 *   11 ours `addu a0,s3,a1`     / tgt `addu s0,s3,a0`
 *   18/19 ours `sll` then `jal` / tgt `jal` then `sll`
 *   63/65 the loop-latch pair, both register-renamed
 * The TARGET's idx is provably live across that jal (its `sll s1,s0,1` sits in
 * the delay slot and the `addu s0,s1,s0` consuming idx is AFTER the call), so
 * "make idx not cross a call" is not what the original C did.  The call-crossing
 * that forecloses the $v0 seat is a PROPERTY OF THE TARGET, not an artifact of
 * our spelling: KILLED as a direction, not merely as a spelling.
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
                wid = idx2 + idx;
                last = rand();
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
