/* func_80072CD4 - s15 (2026-09-04, synthesis). REJECTED.
 * Carrier chassis with the join's @4/@0xC stores spelled as the literal 0xFC instead of the pre-branch `red`
 * local, to remove red's cross-arm live range and let the hoisted carrier take $v0. 10/77 - cse merges the
 * 0xFC with the trailing @0x14 store's constant and the build loses two instructions. DEAD.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
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
        *(u8 *)((s32)(arg1) + 6) = 0x50;
        *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
        blue = 0x46;
        }
        /* FAKE: do-while(0) wrap around the merge-head stores, mechanism: GCC 2.7.2
         * sched.c schedule_select() equal-INSN_PRIORITY potential_hazard tiebreak
         * (sched.c:2706-2721) drives producer-less stores to the block tail; the wrap
         * makes these stores their own region whose tail is the merge-block head.
         * lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md [s9 H3, s14e H2/H3,
         * s14f, s15]. */
        do {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
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
