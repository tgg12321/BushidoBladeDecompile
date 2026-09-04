/* func_80072CD4 - s15 (2026-09-04, synthesis). REJECTED at 12/79 - but EVIDENTIALLY IMPORTANT.
 * This is the FIRST body in the whole ledger whose JOIN BLOCK is in the target's order:
 * `sb ?,4 / sb ?,0xC / sb ?,0xE` (tmp/grind/func_80072CD4/s15/../s14f/s15N2.dis). It is the carrier
 * chassis (arms leave the vertex-1 blue in a local, the join stores it) with arm 2's store order
 * permuted to @5,@0xD,@6 so the post-hoist arm tails are not a common tail and jump2 cannot lift
 * `sb v0,0xD` out - which restores the 79th instruction.
 * The entire residual 12 is INSIDE THE ARMS and is caused by sched1's hoist of the carrier's constant
 * load to the arm head: the hoist gives the arm three simultaneously live values (red, blue, the arm
 * scratch), so red is pushed to $a0 and blue to $v1 where target uses $v1 and $v0.
 * Read together with memory/grind/func_80072CD4/candidate.c (2/79, arms byte-exact and join order
 * wrong) this body proves the two halves of the 2-instruction residual are each individually
 * reachable, on mutually exclusive chassis, and that the pivot between them is exactly the sched1
 * hoist. See hypotheses.md [s15].
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;
    int blue;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
        *(u8 *)((s32)(arg1) + 5) = 0xC3;
        *(u8 *)((s32)(arg1) + 6) = 0x1E;
        *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
        blue = 0x32;
        } else {
        *(u8 *)((s32)(arg1) + 5) = 0xC3;
        *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
        *(u8 *)((s32)(arg1) + 6) = 0x50;
        blue = 0x46;
        }
        /* FAKE: do-while(0) wrap around the merge-head stores, mechanism: GCC 2.7.2
         * sched.c schedule_select() equal-INSN_PRIORITY potential_hazard tiebreak
         * (sched.c:2706-2721) drives producer-less stores to the block tail; the wrap
         * makes these stores their own region whose tail is the merge-block head.
         * lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md [s9 H3, s14e H2/H3,
         * s14f, s15]. */
        do {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xE) = blue;
        } while (0);
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
