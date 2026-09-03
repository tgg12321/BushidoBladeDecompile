/* REJECTED (s17b, 2026-09-02, structural) -- INERT on all three live chassis.
 *
 * Claim tested: local DECLARATION ORDER fixes pseudo-register numbers, which
 * order local-alloc's quantities and global-alloc's allocnos on ties, and is
 * also a tie-break input in the scheduler's ready list.  If any of the three
 * live residuals is an ORDER tie rather than a priority outcome, permuting the
 * declarations moves it -- and declaration order is unimpeachable ordinary C.
 *
 * Measured, honest `sandbox func_800645B0 --disable all`, 78/78 insns each:
 *   WD chassis (control 3/78): W6 (wid,idx2,idx,mask,val,last,i,j) 3/78;
 *                              W7 (idx,wid,idx2,val,mask,last,j,i) 3/78;
 *                              W8 (last,val,mask,idx2,wid,idx,j,i) 3/78.
 *   k  chassis (control 12/78, byte offset routed through `idx`):
 *                              K1/K2/K3/K4 all 12/78.
 *   h  chassis (control 2/78, `idx = rand() & 7;` second real write):
 *                              H1/H2/H3 all 2/78, and H4 (the value staged
 *                              through `last` first) also 2/78.
 * Twelve permutations across three chassis: not one instruction moved.
 *
 * Consequence for the ledger: the k chassis' $s0/$s1 permutation between `idx`
 * and `idx2` and the h chassis' callee-saved seat for the `& 7` value are NOT
 * allocation-ORDER ties -- they are priority/conflict outcomes, exactly as s16's
 * ra_solver verdict typed them.  Declaration order is closed as a lever on this
 * function for allocation as well as (H7, s2) for operand order.
 */
s32 func_800645B0(void) {
    s32 wid;
    s32 idx2;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    s32 i;
    s32 j;
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
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
