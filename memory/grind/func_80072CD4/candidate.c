/* func_80072CD4 - COMPLETED-C body (s14d, 2026-09-03).
 * Measured this session with this exact text in src/text1b.c:
 *   sandbox func_80072CD4 --disable all = 0, build_insns 79 == target_insns 79, rules_dropped 0
 *   verify-oracle = ok (build_matches true), build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 * Cross-block chassis: each inner arm writes its own @5/@6/@0xD and leaves its vertex-1 blue in
 * `blue1`; the shared red 0xFC is written once after the join into @4 and @0xC via `red`.
 * The three single-level do-while(0) wraps are the sanctioned match device
 * (.claude/rules/do-while-zero-exception.md:29); each carries its inline FAKE annotation.
 * Self-vet: memory/grind/func_80072CD4/self_vet.md.
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
