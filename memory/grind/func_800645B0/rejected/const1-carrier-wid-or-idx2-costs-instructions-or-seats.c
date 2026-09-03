/* REJECTED (grind s19, 2026-09-02, synthesis).  Score 7 / 78 at 78 build insns.
 *
 * The const-1 LICM-defeat carrier moved from `val` to `idx2` on the WD chassis
 * (`idx2 = 1; mask = idx2 << idx;`, then `idx2 = idx << 1;` in the if-body).  The
 * defeat still works -- 78 build insns, so nothing is hoisted -- but idx2 is the
 * halfword byte offset that must stay live to the very end of the if-body, and
 * pulling its first set up to the loop top rotates the seats.  The same swap on
 * the SB chassis (`m5`) measures 8/78.
 *
 * Also banked here: `wid` as the const-1 carrier (`m1`, 21/78 at 80 build insns) --
 * that one loses the defeat outright, because `wid` then carries the constant and
 * the *3 sum, which is the same collision as the rejected val-borrowed-for-the-sum
 * form.
 *
 * The carrier that works is `last`: it is the scratch that holds the rand()
 * results, its live ranges are all short and block-local, and it is the only one of
 * the four candidates whose choice leaves both `val` and `idx2` doing exactly one
 * job.  See memory/grind/func_800645B0/candidate.c.
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
            idx2 = 1;
            mask = idx2 << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
