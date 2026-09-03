/* REJECTED (grind s19, 2026-09-02, synthesis).  Score 23 / 78 at 80 build insns.
 *
 * The WD fresh-dest chassis with the fresh `wid` deleted and the *3 sum borrowed
 * into the existing `val` instead (`val = idx2 + idx;`, stores use `val << 2`).
 * The idea was to get WD's exact operand order at stream 20 without inventing a
 * local.  It costs two instructions: `val` is the const-1 LICM-defeat carrier, and
 * giving it a third set in the if-body changes which of its sets loop.c's
 * count_loop_regs_set sees first, so the const-1 is hoisted again (80 build insns,
 * the +2 signature of the hoist).  Companion `g2` (`val = (idx2 + idx) << 2;`)
 * measures 30/78 at 80 for the same reason.
 *
 * The lesson banked: the const-1 carrier and the *3-sum carrier must be different
 * locals.  The session's matching form respects that -- `last` carries the
 * constant, and the sum is expanded through a compiler temp by `idx = idx * 12;`.
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
                val = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (val << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (val << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (val << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
