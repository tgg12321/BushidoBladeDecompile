/* REJECTED — session 2 (structural), sweep 12 variant GA.  Score 19 / 79 insns
 * (worse than the session-2 floor of 1, and one instruction over target).
 *
 * THE IDEA: the last remaining diff at the session-2 floor is the commutative
 * operand order of the *3 sum, which expand_binop (optabs.c:399-417) fixes by
 * swapping whenever `target == op1`.  Writing the right-hand operand as a
 * RECOMPUTATION of the slot index (`i + j`) instead of the variable `idx`
 * makes op1 a fresh pseudo at expand time, so no swap happens — and CSE was
 * expected to fold the recomputation back onto `idx` at zero cost.
 *
 * WHY IT IS DEAD: CSE does NOT fold it away.  The build is 79 instructions
 * (target 78) — the recomputed `addu` survives — and the extra live value
 * cascades the allocation for a total of 19.  Three spellings measured:
 *   GA `idx = idx2 + (i + j);`        19 / 79
 *   GB `idx = (i + j) + idx2;`        19 / 79
 *   GC `idx = idx2 + i + j;`          19 / 79
 *   GD `idx2 = (i + j) << 1; idx = idx2 + (i + j);`   13 / 79
 * So the "give expand a non-`idx` op1 for free" axis is measured dead; it is
 * also the shape that would have had to answer the cheat checklist's
 * semantic-purpose test, and the measurement settles it without needing to.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx2 + (i + j);
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
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
