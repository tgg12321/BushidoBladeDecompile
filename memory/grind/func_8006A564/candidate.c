/* func_8006A564 -- src/text1b.c
 * Session 7 (structural). Chassis at session start: sandbox --disable all
 * score = 45, target_insns 199, build_insns 199 (EXACT parity), matching
 * s5/s6's recorded floor. This session's edits drop it to 29 (same exact
 * parity throughout). Pure C, zero cheat constructs, zero FAKE annotations.
 *
 * SESSION 7 METHOD: the s5/s6 frontier's item #3 -- block 4 (the
 * obj2->field_1C->field_28 record-copy block, the ONLY block that never
 * got the "drop the reused single-scope v0, write inline expressions at
 * each single-use site" transplant that closed blocks 1-3's register
 * coloring in session 5 -- was still carrying its original session-2/3
 * chassis untouched. Applied the SAME proven pattern to block 4's
 * single-use v0 occurrences (never to a value used 2+ times, since
 * duplicating a load risks breaking build_insns exact parity -- confirmed
 * by two negative measurements this session, see KILLED below):
 *
 *   1. Top-of-block: `v0 = *(tile+0x28); *(arg1+0)=v0;` (single use)
 *      -> `*(arg1+0) = *(tile+0x28);`.                          45 -> ...
 *   2. The if-arm's halving pair (v0/v1 each used once: load, shift,
 *      store) -> fully inlined `*(arg1+0x29) = (u32)(*(arg1+0x29))>>1;`
 *      style (no named locals at all).                          45 -> 39
 *   3. The else-arm's 0x28 literal broadcast (was `v0=0x28; store;
 *      store; store;`) -> each store writes the literal directly.
 *                                                                 39 -> 35
 *   4. The two `v0 = *(arg0+0x14); *(arg1+8)=v0;` single-use loads
 *      (one before each func_8007352C call) -> inlined directly.
 *                                                                 35 -> 29
 *
 * Measured via `& tools/wteng.ps1 main sandbox func_8006A564 --disable all`
 * after EACH individual edit (not as one combined diff) -- every one of
 * the 4 wins above is independently confirmed, build_insns held at
 * 199==199 (target) throughout all 4.
 *
 * KILLED THIS SESSION (both instance, both measured, both reverted):
 * 1. Inlining the FIRST group's tail (`v0=*(arg1+0)+0xC; ...; *(arg1+4)=v0;`
 *    -> `*(arg1+4) = *(arg1+0)+0xC;`) while leaving the two preceding DEAD
 *    reads (`v0=*(arg1+0);` `v0=*(arg1+0x1C);`) untouched: MEASURED WORSE,
 *    29 -> 34 (and in an earlier combined-with-other-changes form, 45 -> 47).
 *    Reverted both times. The dead reads apparently participate in the
 *    same pseudo-liveness/coloring picture as the inlined tail would if
 *    touched; this group is NOT a clean single-use case like the 4 wins
 *    above and needs a different (not yet found) lever, or is already at
 *    its local minimum. Reordering the 3 statements among themselves
 *    (dead reads before/after the `*(arg1+0x18)=0` store) measured NO
 *    CHANGE either way (still 29) -- this group's statement ORDER is not
 *    the lever, only inlining vs not, and inlining measured strictly worse.
 * 2. Fully inlining the `v0 = *(tile+0x2C); *(arg1+0)=v0; *(arg1+4)=v0+0xC;`
 *    group (v0 used TWICE) as two separate `*(tile+0x2C)` reads: MEASURED
 *    WORSE on build_insns (199 -> 201, duplicate load not commoned by
 *    CSE across the intervening store) -- confirms the "never inline a
 *    multi-use value" boundary condition that s5/s6's H8/block2/block3
 *    sweeps already established for blocks 2/3. Reverted.
 *
 * Register-normalized dump work this session: NONE (structural modality;
 * pass-attribution reads were not needed since every hypothesis this
 * session was tested by direct measurement, not RTL inspection).
 *
 * NOT yet attempted / frontier for next session:
 * 1. Block 1's remaining residual is STILL the s6-identified scheduling
 *    HAZARD tie (`insn 36 has a greater potential hazard, now 36 46` in
 *    the .sched dump, NOT the dependence-class fallback originally
 *    guessed) -- untouched this session, still needs a read of sched.c's
 *    hazard/insn_cost estimation logic to find a C-level lever (operand
 *    order / an intervening independent computation) that could change
 *    the estimate. See hypotheses.md [s6] for the full dump citation.
 * 2. Blocks 2 and 3 are EXHAUSTED at their current local-spelling minimum
 *    under the exact-parity constraint (s6's 65-variant sweeps each,
 *    confirmed) -- their residual (if any remains after this session's
 *    block-4 work closed points elsewhere) needs either a genuinely new
 *    axis (not inline-subset/decl-order/swap) or is inherent.
 * 3. Block 4's remaining residual (still ~ nonzero after this session's
 *    4 wins) is now concentrated in the KILLED-this-session group (the
 *    dead-reads + arg1+4 tail) and the necessarily-two-use tile+0x2C
 *    group -- BOTH measured NOT improvable by simple inlining/duplication.
 *    A register-normalized objdump diff of the CURRENT (floor-29) build
 *    against target (not yet run this session -- budget) would show
 *    exactly which insns remain mismatched and whether it's still a
 *    v0/v1-class coloring issue or something else (e.g. a scheduling tie
 *    like block 1's). This is the single highest-value next probe: read
 *    `pwsh tools/grinder/dump.ps1 func_8006A564` fresh against this
 *    session's floor-29 body before trying more spelling variants blind.
 * 4. Given blocks 2/3/4 are all now either exhausted or need dump-guided
 *    work rather than more blind inlining, the next session should
 *    PASS-ATTRIBUTE (read the dumps) before any further structural edit,
 *    per the session brief's PASS ATTRIBUTION contract.
 *
 * SESSION 2-6 HISTORY (retained; still applicable to the unchanged parts
 * of the function -- see memory/grind/func_8006A564/hypotheses.md and
 * evidence.md for full detail):
 * Session 1: first C body, floor 137. Session 2: block-local var split +
 * delay-slot fixes, floor 137 -> 68 (build_insns EXACT parity, 199==199).
 * Session 3: removed the named `v0 = mask; if (v0==arg2)` two-statement
 * form at all 4 sites, wrote the mask directly in the if-condition,
 * floor 68 -> 60. Session 4: permuter campaign, re-confirmed floor 60,
 * zero progress after ~4.4k iterations. Session 5: systematic
 * spelling_enum.py sweep found the "fully inline, no named locals" form
 * for block 1's tail (150 variants), transplanted to blocks 2/3,
 * floor 60 -> 45. Session 6: independent 65-variant sweeps of blocks 2
 * and 3's own tails confirmed the block-1-transplanted form is already
 * their score-minimizing spelling under exact-parity; re-dumped block 1's
 * .sched trace and refined the s5 frontier's scheduling-tie mechanism
 * from "dependence-class compare" to "hazard estimate" (floor unchanged
 * at 45, two CLASS-shaped instance kills banked).
 */
extern s32 D_800A34F8;
void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {
    u8 *tile;
    u8 *obj2;
    s32 s4;

    tile = *(u8 **)(arg0 + 0x18);
    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            s4 = 0;
        } else {
            s4 = 0x20;
            v0 = 0x50;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        *(s16 *)(tile + 8) = (0x5F);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xF);
        *(s16 *)(tile + 0xE) = 1;
        *(s16 *)(tile + 0xC) = ((*(s32 *)(arg1 + 0x18)) + 0x19);
    }
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            tile[6] = v0;
        } else {
            v0 = 0x20;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = (*(s32 *)(arg1 + 0x18));
        *(s16 *)(tile + 0xC) = 0x78;
        *(s16 *)(tile + 0xE) = 1;
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xE);
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            v0 = (u32)v0 >> 1;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            v0 = (u32)v0 >> 1;
        } else {
            v0 = 0x10;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        *(s16 *)(tile + 8) = ((*(s32 *)(arg1 + 0x18)) + 0x40);
        *(s16 *)(tile + 0xC) = 0x38;
        *(s16 *)(tile + 0xE) = 1;
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xD);
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);

    obj2 = *(u8 **)(arg0 + 4);
    tile = tile + 0x10;
    *(u8 **)(arg0 + 0x18) = tile;
    tile = *(u8 **)(obj2 + 0x1C);
    {
        s32 v0;
        *(s32 *)(arg1 + 0) = *(s32 *)(tile + 0x28);

        if ((D_800A34F8 & 0xF) == arg2) {
            *(u8 *)(arg1 + 0x2A) = 0;
            *(u8 *)(arg1 + 0x29) = (u32)(*(u8 *)(arg1 + 0x29)) >> 1;
            *(u8 *)(arg1 + 0x2B) = (u32)(*(u8 *)(arg1 + 0x2B)) >> 1;
        } else {
            *(u8 *)(arg1 + 0x2B) = 0x28;
            *(u8 *)(arg1 + 0x2A) = 0x28;
            *(u8 *)(arg1 + 0x29) = 0x28;
        }

        v0 = *(s32 *)(arg1 + 0);
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s32 *)(arg1 + 0x18) = 0;
        v0 = *(s32 *)(arg1 + 0) + 0xC;
        *(s32 *)(arg1 + 0x1C) = *(s32 *)(arg1 + 0x1C) + 0xF;
        *(s32 *)(arg1 + 4) = v0;

        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);

        v0 = *(s32 *)(tile + 0x2C);
        *(s32 *)(arg1 + 0) = v0;
        *(s32 *)(arg1 + 4) = v0 + 0xC;
        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}
