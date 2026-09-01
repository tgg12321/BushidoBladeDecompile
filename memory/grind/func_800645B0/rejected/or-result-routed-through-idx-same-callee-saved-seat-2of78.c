/* SESSION s16 (2026-09-01, escalation).  2 / 78 at 78 insns -- ties the h form,
 * and GENERALISES its residual from two samples into a mechanism.
 *
 * WHAT IT TESTS.  Frontier item 2 of the s15 ledger: "other non-copy second
 * writes to idx exist and may seat better than `idx = last & 7;`".  This body
 * routes a DIFFERENT real value through idx -- the occupancy-bit OR result,
 * `idx = val | mask;` consumed immediately by `D_800A3444 = idx;` -- and lets
 * the halfword store take `last & 7` inline, so the masked random value gets a
 * fresh short-lived pseudo.
 *
 * RESULT: the andi/sh pair now MATCHES (the masked value does land in the
 * rand-return seat $v0 once it is not written into idx), and the residual moves
 * wholesale onto the new carrier:
 *   59 ours `or s0,v1,s2`  / tgt `or v1,v1,s2`
 *   60 ours `sw s0,0(gp)`  / tgt `sw v1,0(gp)`
 * i.e. exactly the same two-insn shape, one statement later.
 *
 * THE MECHANISM (now proven, not conjectured).  idx is live across the
 * `last = rand()` call, so its allocno (pseudo 74) has $v0 pruned from its
 * preferences by global.c:897 and carries a hard conflict with $v0; it is
 * therefore seated CALLEE-SAVED ($s0) for its whole live range.  Every value
 * routed through idx inherits that seat.  The target computes every value in
 * this arm -- the masked random, the OR result -- in a CALLER-SAVED register.
 * So no choice of WHICH real value to borrow idx for can close the residual:
 * the borrow itself is what costs the two insns.  The three remaining
 * candidates (the (rand() & 0xFF) - 0x7F store operands, idx2, mask) are all
 * consumed in the same call-straddled region and inherit the same seat.
 * Frontier item 2 is KILLED as a family.
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
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                idx = val | mask;
                D_800A3444 = idx;
                break;
            }
        }
    }
    return 1;
}
