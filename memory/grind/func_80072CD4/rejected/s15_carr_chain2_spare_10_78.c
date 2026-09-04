/* func_80072CD4 - s15 (2026-09-04, synthesis). REJECTED.
 * Carrier chassis + a SECOND named intermediate `spare` copying the carrier (blue -> spare) so that the
 * arm's trailing constant load gains an in-block successor. Measured 10/78 WITH the merge-head wrap and
 * 13/78 without it (rejected/s15_carr_chain2_nowrap is the same body sans wrap) - i.e. BYTE-IDENTICAL to
 * the plain single-carrier body. The copy is folded away by cse/flow long before sched1, so the successor
 * never exists at scheduling time. DEAD: zero-byte successors do not survive to sched1.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;
    int blue;
    int spare;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
        *(u8 *)((s32)(arg1) + 5) = 0xC3;
        *(u8 *)((s32)(arg1) + 6) = 0x1E;
        *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
        blue = 0x32;
        spare = blue;
        } else {
        *(u8 *)((s32)(arg1) + 5) = 0xC3;
        *(u8 *)((s32)(arg1) + 6) = 0x50;
        *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
        blue = 0x46;
        spare = blue;
        }
        /* FAKE: merge-head do-while(0), mechanism: GCC 2.7.2 sched.c schedule_select
         * potential_hazard tiebreak (sched.c:2706-2721); lever-exhaustion:
         * memory/grind/func_80072CD4/hypotheses.md [s9 H3, s14e H2/H3, s14f, s15]. */
        do {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xE) = spare;
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
