/* func_80072CD4 — s14 (rederive modality, 2026-09-03): BYTE MATCH.
 * `sandbox func_80072CD4 --disable all` = 0, build_insns 79 == target_insns 79,
 * rules_dropped 0; full-build `verify-oracle` = SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * == oracle, both measured this session with this exact body in src/text1b.c.
 *
 * Structure: the CROSS-BLOCK chassis (target's own structure per s11/s12) — the inner arms set
 * @5/@6/@0xD and leave the @0xE value in `blue1`; the merge region writes @4, @0xC, @0xE and then
 * the two unconditional RGB triples. It contains NO duplication of any statement into the inner
 * arms: the banned per-arm @4/@0xC duplication is absent in every spelling.
 *
 * The three `do { ... } while (0);` wraps are the sanctioned do-while(0) match device
 * (.claude/rules/do-while-zero-exception.md:29, owner ruling 2026-07-06 — ANY codegen effect,
 * mandatory inline FAKE annotation, which each wrap carries at its site). Each is measured
 * necessary this session: arm wraps alone = 4/79, merge wrap alone = 10/78, then-arm+merge =
 * 7/79, else-arm+merge = 8/79, all three = 0/79. No wrap is nested inside another.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;
    u8 blue1;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c first-pass
             * scheduler (sched1) - without it the arm-tail blue1 constant load is
             * hoisted to the arm top, which reseats it and cross-jumps the
             * 0xD store out of the arm; lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md
             * (s2-s13, 36 banked forms, 15.8k-iteration directed permuter). */
            do { blue1 = 0x32; } while (0);
        } else {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c sched1 -
             * keeps the arm-tail blue1 constant load at the arm bottom (same effect as
             * the then-arm wrap; both arms measured necessary, 7/79 and 8/79 with
             * only one present); lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md. */
            do { blue1 = 0x46; } while (0);
        }
        /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c second-pass
         * scheduler (sched2) - it separates these three stores into their own
         * scheduling region so they keep the merge-block head instead of being
         * sunk to the block tail by the ready-store/potential-hazard tiebreak;
         * lever-exhaustion: memory/grind/func_80072CD4/hypotheses.md (this wrap
         * alone measures 10/78, the arm wraps alone 4/79). */
        do {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xE) = blue1;
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
