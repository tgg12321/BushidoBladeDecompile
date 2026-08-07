/* REJECTED (s3, structural) — stage the +0x12C onto the PARTIAL side as its own
 * named insn to balance partial's dependency-chain length against the
 * reload->lookup->sum chain, hoping to win the sched1 priority tie and keep the
 * partial-add before the reload. Representative of a 7-form sweep (sweep3.py:
 * G1 staged-0x12C, G2 p+0x12C+lk, G3 double-stage, G4 lk-named-after-p,
 * G5 commuted sum, G6 0x12C-in-base, G7 named-reload).
 *
 * RESULT: ALL 7 forms remain `reload_first` in the mini proxy (which faithfully
 * reproduces the sandbox alloc). Chain-balancing does NOT flip the order.
 * MECHANISM (refined this session): sched1's list scheduler FRONT-LOADS the
 * memory reload `lw $v0,0($a0)` to the top of the join block to hide load
 * latency — this is a load-priority boost, NOT a chain-length tie, so no
 * reassociation/staging of the ADD side can prevent it. The hoist makes the
 * reload-dest ($v0) overlap live var_v0 -> var_v0 evicted from $v0 -> base
 * pushed off $a1. KILLED. */
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    {
        s32 p = base + var_v0;
        s32 p2 = p + 0x12C;
        return p2 + (*((s16 *) ((*((s32 *) arg0)) + 0x40A)));
    }
}
