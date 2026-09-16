/* func_8006A564 -- src/text1b.c
 * Session 5 (enumerate). sandbox --disable all score = 45 (down from the
 * session-3/4 floor of 60), target_insns 199, build_insns 199 -- EQUAL.
 * Pure C, zero cheat constructs, zero FAKE annotations.
 *
 * Structure unchanged from session 3/4 (see below for the original
 * narrative) -- draws 3 TILE primitives whose color bytes are chosen by
 * (D_800A34F8 & 0xF == arg2), then loads a second object's field_1C->
 * field_28 into arg1's packet, halves/sets its color bytes under the same
 * condition, advances two fields, calls func_8007352C(arg1) twice, and
 * finishes with a SetDrawMode/AddPrim of *(arg0+0x1C).
 *
 * SESSION 5 METHOD (owner-mandated systematic spelling sweep, 60 -> 45):
 * `tools/spelling_enum.py` + a hand-rolled sweep loop (`sweep_variants.py`
 * itself is blocked by `worktree_contamination_guard.py` for not having a
 * `wteng.ps1`-passthrough form; a local `tmp/grind/func_8006A564/s5/
 * run_sweep.ps1` loop calling `& tools/wteng.ps1 main sandbox func_8006A564
 * --disable all` per variant was used instead -- same effect, sanctioned
 * path). Enumerated the FIRST tile-block's flat tail (the 4
 * post-if/else stores that each reused the single block-scoped `v0` for a
 * load-then-store): named each value as a distinct fresh local (valA..valE)
 * between ENUM-BEGIN/END markers, ran spelling_enum.py --no-swaps-implicit
 * (150 spellings: every subset of the 5 locals inlined x every valid
 * def-before-use declaration order x commutative-operand swaps), swept all
 * 150 against the sandbox. HISTOGRAM: 47(1) 49(7) 51(14) 52(26) 54(102) --
 * i.e. every non-trivial named-local spelling scored >= 49; only the FULLY
 * INLINED form (v149.c, every value written as an anonymous expression
 * directly at its one use site, no named locals at all in the tail) hit 47.
 *
 * Applying v149's inlining pattern to block 1's tail: 60 -> 47.
 * Manually applying the SAME "drop the reused single-scope v0, write each
 * value as an inline expression at its one use site" pattern to block 2's
 * and block 3's tails (same shape: `v0 = load; store; v0 = load2; ...;
 * v0 = v0+K; store;` all collapsed to `store = load; store = const; store =
 * const; store = (load2 + K);`): 47 -> 45.
 *
 * Register-normalized objdump diff after this change
 * (tmp/grind/func_8006A564/s5/diff2.py against
 * tmp/grind/func_8006A564/s5/build2.dis.txt) shows block 1's v0/v1
 * coloring swap (the entire residual identified in sessions 2-4) is NOW
 * FULLY RESOLVED for that block -- every `lbu`/`lw`/`sh`/`sb` in block 1
 * uses the SAME register as target (v0 throughout). Block 1's remaining
 * diff lines are pure INSTRUCTION-ORDER (scheduling), not register choice:
 * target computes+stores tile+0xA before tile+0xE/tile+0xC; our build's
 * list scheduler interleaves the independent `li v1,1; sh v1,0xE(s0)` store
 * between the tile+0xA load and its `addiu` -- same registers, different
 * cycle. This is a NEW, more specific frontier than the vague "v0/v1
 * coloring swap" sessions 2-4 chased.
 *
 * Blocks 2 and 3 (tile blocks 2 and 3) did NOT fully resolve the same way:
 * inlining flipped which of the block's TWO loaded values (the `arg1+0x18`-
 * based one and the `arg1+0x1C`-based one) gets v0 vs v1 -- before the edit
 * BOTH got v1 (both wrong); after, one gets v0 and the other v1 (still one
 * wrong, but a different one). Net still an improvement (removes one
 * register diff per block) but not a full block-level fix the way block 1
 * got. See KILLED below for the "two separate fresh locals" alternative
 * that was tried and made this WORSE (broke build_insns parity, 199->194).
 *
 * NOT yet attempted / frontier for next session:
 * 1. Enumerate block 2's and block 3's tails the SAME way spelling_enum
 *    was used on block 1's (mark ENUM-BEGIN/END around the 4-statement
 *    flat tail, sweep all inline/decl-order/swap combinations) -- only
 *    block 1 got the full systematic sweep this session; blocks 2/3 only
 *    got the ONE hand-derived "full inline" spelling transplanted from
 *    block 1's winner, not their own independent sweep. Their winning
 *    spelling may differ (they have a genuinely two-value liveness overlap
 *    block 1's did not -- block 1's tail values did not overlap in
 *    liveness the same way).
 * 2. Block 1's residual is now a SCHEDULING tie (delay-slot / list-
 *    scheduler insn order), not RA -- read the .sched dump
 *    (pwsh tools/grinder/dump.ps1 func_8006A564) for block 1's region to
 *    identify the exact rank_for_schedule tie per [[sched-rank-class-tie-
 *    wall]] before trying further statement-order variants there.
 * 3. Block 4 (obj2->field_1C->field_28 record-copy block, lines 6509-6547)
 *    still carries the ORIGINAL session-2/3 v0/v1-reuse chassis untouched
 *    this session -- it also shows a residual v0/v1 swap in the diff
 *    (lines 129-166 of the s5 diff). Same enumerate treatment (ENUM-mark
 *    its flat non-branching sub-sequences, e.g. lines 6530-6535's
 *    load/store chain which has a DEAD read at line 6531 pre-dating this
 *    session -- `v0 = *(s32 *)(arg1 + 0x1C);` immediately overwritten by
 *    line 6533 before any read -- that dead read is untouched original
 *    chassis, not introduced this session) should be tried next.
 *
 * KILLED THIS SESSION:
 * 1. Block 2 tail as TWO SEPARATE fresh named locals (`o18`, `o1c`,
 *    each written once and read once) instead of full inlining: MEASURED
 *    WORSE on both axes -- score 45 -> 42 but build_insns DROPPED to 194
 *    (target 199), i.e. combine folded something differently and broke
 *    exact instruction-count parity. Reverted; the full-inline form (which
 *    keeps 199==199 parity) is the correct chassis to build on next
 *    session. See rejected/block2-separate-locals-breaks-parity.c.
 *
 * SESSION 2-4 HISTORY (retained from the prior candidate.c; still
 * applicable to the unchanged parts of the function -- see
 * memory/grind/func_8006A564/hypotheses.md and evidence.md for full detail):
 * Session 1: first C body, floor 137. Session 2: block-local var split +
 * delay-slot fixes, floor 137 -> 68 (build_insns EXACT parity, 199==199).
 * Session 3: removed the named `v0 = mask; if (v0==arg2)` two-statement
 * form at all 4 sites, wrote the mask directly in the if-condition,
 * floor 68 -> 60. Session 4: permuter campaign, re-confirmed floor 60,
 * zero progress after ~4.4k iterations (random search did not find the
 * ENUM-derived full-inline spelling this session's SYSTEMATIC sweep found).
 */
extern s32 D_800A34F8;
extern s32 D_800A374C;
extern void SetTile(void *);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, void *);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
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
        v0 = *(s32 *)(tile + 0x28);
        *(s32 *)(arg1 + 0) = v0;

        if ((D_800A34F8 & 0xF) == arg2) {
            s32 v1;
            v0 = *(u8 *)(arg1 + 0x29);
            v1 = *(u8 *)(arg1 + 0x2B);
            *(u8 *)(arg1 + 0x2A) = 0;
            v0 = (u32)v0 >> 1;
            v1 = (u32)v1 >> 1;
            *(u8 *)(arg1 + 0x29) = v0;
            *(u8 *)(arg1 + 0x2B) = v1;
        } else {
            v0 = 0x28;
            *(u8 *)(arg1 + 0x2B) = v0;
            *(u8 *)(arg1 + 0x2A) = v0;
            *(u8 *)(arg1 + 0x29) = v0;
        }

        v0 = *(s32 *)(arg1 + 0);
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s32 *)(arg1 + 0x18) = 0;
        v0 = *(s32 *)(arg1 + 0) + 0xC;
        *(s32 *)(arg1 + 0x1C) = *(s32 *)(arg1 + 0x1C) + 0xF;
        *(s32 *)(arg1 + 4) = v0;

        v0 = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg1 + 8) = v0;
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);

        v0 = *(s32 *)(tile + 0x2C);
        *(s32 *)(arg1 + 0) = v0;
        *(s32 *)(arg1 + 4) = v0 + 0xC;
        v0 = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg1 + 8) = v0;
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}
