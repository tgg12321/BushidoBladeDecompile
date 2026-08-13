/* func_800645B0 — REJECTED (grind session 10, escalation modality).
 *
 * WHAT WAS TESTED.  Ledger frontier item 3: the DA chassis (the 12-byte BYTE
 * offset routed through `idx`) satisfies both of the hard constraints — the *3
 * sum's destination is not `idx`, so optabs.c's commutative swap does not fire,
 * AND `idx` gets a real second set, so sched.c's birthing_insn_p lift does not
 * fire — but it measures 12/78 because local-alloc hands $s0 to the block-local
 * halfword offset `idx2` and pushes the multi-block `idx` to $s1, the mirror of
 * the target's allocation.  The lead was local-alloc.c:2205-2270, which tries
 * qty_phys_copy_sugg / qty_phys_sugg BEFORE find_free_reg's reg_alloc_order
 * scan: give the halfword offset a COPY relationship and its suggestion should
 * rank $s1, restoring the target's assignment.
 *
 * MEASURED (tmp/grind/func_800645B0/s10/sweep34.py, honest sandbox each):
 *   WA  DA control                                                12 / 78
 *   WB  copy PRODUCER   (`idx2 = idx; idx2 = idx2 << 1;`)         12 / 78
 *   WC  copy CONSUMER   (`hw = idx2;`, s16 store addresses `hw`)  12 / 78
 *   WD  both                                                      12 / 78
 *
 * All four are byte-identical: not one instruction and not one register moved.
 * A C-level copy whose source and destination are coalesced never reaches
 * local-alloc as a distinct quantity with a suggestion — the copy is folded
 * away before quantities are formed — so qty_phys_copy_sugg has nothing to
 * rank, and the DA allocation is not reachable from the C level.  This closes
 * frontier item 3 and, with it, the DA branch of the closure enumeration.
 *
 * Body below is WD (both spellings), the representative of the family.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 hw;
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
                idx2 = idx;
                idx2 = idx2 << 1;
                last = rand();
                wid = idx2 + idx;
                idx = wid << 2;
                hw = idx2;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (hw))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
