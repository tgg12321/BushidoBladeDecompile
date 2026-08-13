/* REJECTED — func_800645B0, session 9.  Sweep29 variant VB: the const-1 split
 * into its OWN single-set local (`one`), the D_800A3444 read-modify-write left
 * in `val`.  Measured 12 / 80 against a 78-instruction target.
 *
 * Why it is dead: with only one set of `one` inside the loop, loop.c admits the
 * const-1 SET as a movable (alternative (1) at loop.c:700, reg_in_basic_block_p:
 * its only use is the in-block shift) and move_movables hoists it out of both
 * loops into a fresh callee-save — a save/restore pair, +2 instructions.  This
 * is H18's wall, reproduced on the JD chassis.
 *
 * Two more spellings that also stay at 12 / 80 and are dead for a DIFFERENT
 * reason (they never give `one` a second set that loop.c can see):
 *   VF  `one = one;`  — eliminated before loop.c ever runs.
 *   VG  `one = 1;`    — a store of the value already held; cse1 runs BEFORE
 *                       loop and folds it out.
 * The surviving form needs a DEAD store of a DIFFERENT value — see
 * ../candidate.c.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    s32 one;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            one = 1;
            mask = one << idx;
            if (!(D_800A3444 & mask)) {
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (idx << 1))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
