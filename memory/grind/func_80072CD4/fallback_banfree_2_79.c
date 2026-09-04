/* func_80072CD4 - BAN-FREE FALLBACK (best form carrying no banned construct).
 * This is the s14f/s15 body, re-measured 2/79 at the top of s16
 * (tmp/grind/func_80072CD4/s16/control.txt). It was memory/grind/func_80072CD4/candidate.c
 * through s15. It is preserved here because s16 replaced candidate.c with the score-0
 * per-arm body that trips state.json banned_constructs entry 5 and is pending a ruling.
 * If the ruling REFUSES that body, this 2/79 form is the standing best.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        /* FAKE: do-while(0) wrap around the two r0/r1 stores, mechanism: GCC 2.7.2
         * sched.c schedule_select() - at equal INSN_PRIORITY it picks the ready insn with
         * the largest potential_hazard (sched.c:2706-2721), so a store always outranks a
         * plain li and producer-less stores are driven to the block tail; the wrap makes
         * these two stores their own scheduling region whose tail is the merge-block head.
         * lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md [s9 H3, s14e H2/H3,
         * s14f] - register-anti-dependence, memory-alias and statement-order deferral are
         * all measured dead (12/79, 5/79, 4/79, 4/79, 4/79, 14/79). */
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
