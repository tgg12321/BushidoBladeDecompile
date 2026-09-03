/* REJECTED -- s18 (structural).  `idx = (idx2 + idx) << 2;` -- the sum in a temp (correct
 * operand order) and the <<2 written back into `idx`, which gives pseudo 74 a
 * SECOND set and so denies sched.c:2526 the birthing lift.  **12 / 78, 78
 * insns, EVERY OPCODE AND EVERY POSITION EXACT** -- index 20, the inner-loop
 * head (11/12) and the back-edge delay slot (65) are all correct at the same
 * time, which no form on this function had achieved without an extra named
 * local.  The entire residual is register naming: idx=$s1/idx2=$s0 (target
 * swaps them), the sum temp gets $v1 where the target coalesces it into $s0,
 * and the D_800A347C load cascades to $a0.  This is almost certainly the
 * ORIGINAL SOURCE SHAPE -- the target's $s0 chain (idx -> sum -> byte offset)
 * is exactly one C variable written twice with a coalesced temp in between.
 * Seven declaration permutations (a1p1..a1p7) are byte-identical at 12/78, so
 * it is not a pseudo-number tie -- same verdict s17 reached on WD/h/k.
 * `idx = (idx * 3) << 2;` (a3) measures identically.
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
                idx = (idx2 + idx) << 2;
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
    return 1;
}