/* func_80072CD4 - s15 (2026-09-04, synthesis). REJECTED.
 * The 2/79 natural-arm-E body with the INNER branch sense inverted (`if (!(flag & 4))`, arms swapped).
 * 9/79 on the current chassis - re-measurement of the s6 inversion axis (11/79 on the retired chassis) on
 * the chassis where the arms are otherwise byte-exact. Still dead: the layout swap costs both arms.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (!(*(s32 *)((s32)(D_800A35C4) + 8) & 4)) {
        *(u8 *)((s32)(arg1) + 5) = 0xC3;
        *(u8 *)((s32)(arg1) + 6) = 0x50;
        *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
        *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        } else {
        *(u8 *)((s32)(arg1) + 5) = 0xC3;
        *(u8 *)((s32)(arg1) + 6) = 0x1E;
        *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
        *(u8 *)((s32)(arg1) + 0xE) = 0x32;
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
