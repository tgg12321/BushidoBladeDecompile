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

 * s14b (structural, 2026-09-03) — THE @5 DUPLICATION IS BYTE-MATERIALIZING, NOT BYTE-NEUTRAL.
 * The 2026-09-03 23:36 layer-1 FAIL demanded that `*(u8 *)((s32)(arg1) + 5) = 0xC3;` be hoisted
 * out of the two inner arms "exactly as red/r0/r1 were". That remedy is measured DEAD in four
 * placements this session (tmp/grind/func_80072CD4/s14/sandbox_q{1,2,3,4}_*.txt):
 *   q1 @5 inside the merge do-while group ....... sandbox 12, build_insns 77
 *   q2 @5 pre-branch, beside `red = 0xFC` ....... sandbox  7, build_insns 77
 *   q4 @5 in the merge block, after the wrap .... sandbox  6, build_insns 77
 *   q3 @5 stored once from a holder ASSIGNED per-arm  sandbox  8, build_insns 78
 * Target is 79 insns. Deleting the second source-level @5 store deletes exactly two emitted
 * instructions, because the TARGET ITSELF ships both copies: asm/funcs/func_80072CD4.s:23-24
 * (`addiu $v0,$zero,0xC3` / `sb $v0,0x5($s1)` in the then-arm at 0x80072D28/0x80072D2C) and
 * :32-33 (the identical pair in the else-arm at 0x80072D48/0x80072D4C). Neither copy is
 * cross-jump-dead. q3 further isolates which half must be duplicated: two per-arm ASSIGNMENTS
 * with one merge STORE keeps both `addiu 0xC3` but emits one `sb 0x5` (78) — it is the STORE
 * statement that target requires twice.
 * This is the categorical difference from rejected/dup4_0xc_into_arms.c, the construct
 * banned_constructs entries 5 and 8 are derived from: there the duplicated @4/@0xC copies are
 * re-merged by jump_optimize(cross_jump=1) and vanish from the output (byte-neutral, sole effect
 * = merge-block store schedule). Here the second copy IS output. This body duplicates @4/@0xC
 * NOWHERE — they are hoisted into `red` and stored once in the merge region.
 * Session s14b returned `ruling-request`, not `candidate-ready`: the driver mechanically rejects a
 * candidate-ready whose self-vet re-declares a banned construct, and entry 8 names this store
 * verbatim. No agent may clear it.
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
