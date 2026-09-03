/* REJECTED (grind s19, 2026-09-02, synthesis).  Score 4 / 78 at 78 build insns.
 *
 * The s17 WD fresh-dest chassis (`wid = idx2 + idx;`, 3/78 control) with the
 * pre-loop `D_800F10EC = 1;` store staged through `idx` (`idx = 1;
 * D_800F10EC = idx;`).  This is the FIRST form in the whole ledger that holds the
 * target's operand order at stream 20, the inner-loop head at 11/12 AND the
 * back-edge delay slot at 65 simultaneously -- the three-way tension s18 stated is
 * satisfiable.  It is banked because its residual is a NEW class, not because the
 * score improved.
 *
 * The residual: staging the constant through `idx` puts idx's allocno in the
 * function's entry block, and because idx is live across the rand() calls its
 * allocno must be callee-saved, so the pre-loop `li` lands in $s0 where the target
 * has $v0.  That in turn reorders the prologue register saves (the historical
 * `regfix.txt:2521  func_800645B0: reorder 3,1,2 @ 1-3`).  GCC 2.7.2 has no
 * live-range splitting, so the pre-loop and in-loop halves of idx cannot be given
 * different hard registers from C.
 *
 * Superseded by the session's matching form, which reaches 0/78 without extending
 * idx's live range at all.
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
    idx = 1;
    D_800F10EC = idx;
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
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
