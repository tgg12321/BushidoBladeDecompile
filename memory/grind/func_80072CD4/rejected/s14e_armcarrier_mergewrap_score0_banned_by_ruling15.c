/* func_80072CD4 - s14e (2026-09-04, synthesis). Measured THIS session with this exact text in
 * src/text1b.c: `sandbox func_80072CD4 --disable all` = 0, build_insns 79 == target_insns 79,
 * rules_dropped 0 (tmp/grind/func_80072CD4/s14e/sandbox_v7_xblock_armreuse_mergewrap.txt).
 *
 * WHAT CHANGED vs the s14d body this replaces: that body needed THREE do-while(0) wraps - one per
 * inner arm (to stop sched1 hoisting the arm-tail blue constant to the arm top) plus one on the
 * merge-head group. The two ARM wraps are gone. They are replaced by reusing a single local `t`
 * for the arm's three colour components AND for the vertex-1 blue that the merge region consumes:
 * because GCC 2.7.2 keeps one pseudo per C variable, the four writes to `t` carry
 * REG_DEP_OUTPUT edges that sched1 may not reorder, so the final `t = 0x32/0x46` cannot be hoisted
 * above the arm's own stores and stays at the arm bottom - which is target's shape (the arms
 * measure byte-identical to target with no wrap present at all: see
 * rejected/s14e_xblock_armreuse_clean_4_79.c, 4/79).
 *
 * ONE device remains: the merge-group do-while(0). Its removal was attacked five ways this session
 * and all five measured 4/79 or worse (merge-chain variable reuse full/first/last = 12, 5, 4; the
 * base-pointer alias = 4; the red-reassign-first form = 4, its reassignment CSE-folded because
 * `red` already holds 0xFC). See hypotheses.md [s14e].
 *
 * NOTE FOR THE NEXT SESSION: this body is NOT submittable as-is. state.json banned_constructs
 * entry 9 names this exact wrap. It is filed as a ruling-request, not a candidate-ready.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;
    int t;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            t = 0xC3;
            *(u8 *)((s32)(arg1) + 5) = t;
            t = 0x1E;
            *(u8 *)((s32)(arg1) + 6) = t;
            t = 0xC8;
            *(u8 *)((s32)(arg1) + 0xD) = t;
            t = 0x32;
        } else {
            t = 0xC3;
            *(u8 *)((s32)(arg1) + 5) = t;
            t = 0x50;
            *(u8 *)((s32)(arg1) + 6) = t;
            t = 0xDC;
            *(u8 *)((s32)(arg1) + 0xD) = t;
            t = 0x46;
        }
        /* FAKE: do-while(0) wrap, mechanism: GCC 2.7.2 sched.c second-pass scheduler
         * (sched2) - it makes these three stores their own scheduling region, so the
         * ready-store/potential-hazard tiebreak (sched.c:2660-2745) that otherwise sinks
         * producer-less stores to the block tail sinks them to the tail of a region whose
         * tail IS the merge-block head; lever-exhaustion: memory/grind/func_80072CD4/
         * hypotheses.md [s9 H3, s14e H2/H3] - the register-anti-dep and memory-alias
         * deferral routes are measured dead at 12/79, 5/79, 4/79, 4/79 and 4/79. */
        do {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xE) = t;
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
