/* func_8006A564 -- src/text1b.c
 * Session 2 (structural). sandbox --disable all score = 68 (target_insns 199,
 * build_insns 199 -- EQUAL, was 204 vs 199 at s1). Pure C, zero cheat
 * constructs, zero FAKE annotations.
 *
 * Structure: draws 3 TILE primitives (SetTile/SetSemiTrans/AddPrim) whose
 * color bytes are chosen by (D_800A34F8 & 0xF == arg2), then loads a second
 * object's field_1C->field_28 into arg1's packet, halves/sets its color
 * bytes under the same condition, advances two fields, calls
 * func_8007352C(arg1) twice (feeding two different source structs' data
 * through the same packet), and finishes with a SetDrawMode/AddPrim of
 * *(arg0+0x1C), advancing that field by 0xC.
 *
 * SESSION 2 FIXES (137 -> 68, all confirmed via sandbox + objdump diff
 * against asm/funcs/func_8006A564.s):
 *
 * 1. BLOCK-LOCAL VAR SPLIT (137 -> 74): s1's single function-wide `s32 v0;`
 *    reused across all 3 tile-draw blocks + the final section had a live
 *    range spanning every basic block in the function, which pushed cc1's
 *    global allocator to promote it (plus block-crossing scratch values)
 *    into a 6th callee-saved register (s5), needing a 0x38-byte frame vs
 *    target's 0x30. Declaring `s32 v0;` FRESH inside a `{ }` block scoped to
 *    each tile-draw section (and the final section) confines each
 *    instance's live range to ~15-20 straight-line/branch-local
 *    instructions. Result: our build's hard-reg set dropped from
 *    {s0,s1,s2,s3,s4,s5} to exactly {s0,s1,s2,s3,s4} -- matching target's
 *    `.mask 0x801f0000` -- AND the per-variable mapping came out
 *    EXACTLY matching target's (arg0->s2, arg1->s1, arg2->s3, tile->s0,
 *    the `s4` flag local->s4) with ZERO further declaration-order
 *    massaging needed. Confirmed via `.greg` dump
 *    (tmp/grind/func_8006A564/dumps/text1b.greg, "9 regs to allocate";
 *    "Hard regs used: 2 3 4 5 6 7 16 17 18 19 20 29 31" -- no 21/s5) and
 *    objdump of tmp/sandbox/func_8006A564/text1b.o (`.frame $sp,48,$31`,
 *    `.mask 0x801f0000,-4`, byte-identical to target's prologue/epilogue
 *    mask+frame-size).
 *
 * 2. Delay-slot statement reorder in block1's else-arm (`s4 = 0x20;`
 *    moved before `v0 = 0x50; tile[4]=v0; tile[5]=v0;` -- 74 -> 73):
 *    target's `bne` delay slot fills with `addiu $s4,$zero,0x20` (the
 *    independent flag-local write); our build's delay slot was filling
 *    with the v0=0x50 constant instead. Putting the s4 assignment textually
 *    FIRST in the else arm got cc1's list scheduler to choose it for the
 *    delay slot, matching target byte-for-byte at that branch
 *    (`asm/funcs/func_8006A564.s:16-17`).
 *
 * 3. REAL BUG FOUND in the s1 candidate's block-2 field offsets (73 -> 72,
 *    then part of the drop to 68 once combined with fix 4): the s1 body
 *    had `v0 = *(arg1+0x18); v0 = v0 + 0x78; *(tile+8) = v0;` for the 2nd
 *    tile-draw block. Reading target's actual asm
 *    (`asm/funcs/func_8006A564.s:73-78`: `lw $v0,0x18($s1); ...;
 *    sh $v0,0x8($s0)` with NO intervening add, and separately
 *    `addiu $v0,$zero,0x78; sh $v0,0xC($s0)`) shows the s1 candidate
 *    double-counted the 0x78 literal: it belongs ONLY as the literal
 *    stored to `tile+0xC`, not added to the `tile+8` field. Fixed to
 *    `*(s16 *)(tile + 8) = v0;` (no add) -- this alone dropped
 *    build_insns from 202 to 201 (one spurious `addiu` removed).
 *
 * 4. Redundant re-read of `*(arg0+0x18)` replaced with `tile + 0x10`
 *    (part of the 202->199 insn-count convergence): the s1 candidate's
 *    tile-advance-to-third-object code re-derived the pointer from
 *    scratch (`tile = *(u8 **)(arg0 + 0x18) + 0x30;`) instead of reusing
 *    the ALREADY-LIVE `tile` local (which by this point already holds
 *    arg0's field_0x18 base + 0x20 from the two prior `tile += 0x10`
 *    advances). Target's asm (`asm/funcs/func_8006A564.s:124-127`) does
 *    exactly `lw $v0,0x4($s2); addiu $s0,$s0,0x10; sw $s0,0x18($s2);
 *    lw $s0,0x1C($v0)` -- i.e. `s0` (tile) is advanced IN PLACE by 0x10,
 *    never reloaded from memory. Fixed to `tile = tile + 0x10;`.
 *
 * 5. Dual-variable split for the final color-halve block (`v0`/`v1` for
 *    the two independently-halved packet bytes at arg1+0x29/+0x2B,
 *    instead of reusing one `v0` serially): target's asm
 *    (`asm/funcs/func_8006A564.s:137-144`) loads BOTH bytes first
 *    (`lbu $v0,0x29($s1); lbu $v1,0x2B($s1)`), THEN does both `srl`s,
 *    THEN both stores -- i.e. two independent live values held in two
 *    separate registers simultaneously, not one register reused
 *    load/shift/store/load/shift/store. Splitting into a fresh `s32 v1;`
 *    (block-scoped, same family as fix #1) for the second byte got cc1
 *    to interleave the two loads/shifts the same way, closing the last
 *    instruction-count gap: build_insns 199 == target_insns 199 (exact).
 *
 * RESULT: build_insns == target_insns == 199 (was 204 at s1). Sandbox
 * masked score is 68, entirely a REGISTER-COLORING residual, not an
 * instruction-count or control-flow residual (confirmed by an
 * operand-level, register-normalized diff --
 * tmp/grind/func_8006A564/s2/diff3.py -- every mismatched line is a
 * v0<->v1 swap, e.g. target's `andi $v0,$v0,0xF` (in-place AND, same
 * register as the preceding `lw $v0,...(gp)`) vs our build's
 * `andi $v1,$v0,0xf` (AND result lands in a DIFFERENT register than the
 * load) -- repeated at all 4 `D_800A34F8 & 0xF` sites and propagating
 * through most of each block's tail).
 *
 * KILLED THIS SESSION (see hypotheses.md H3): splitting the
 * `v0 = D_800A34F8 & 0xF;` combined expression into `v0 = D_800A34F8;
 * v0 &= 0xF;` (two statements, hoping cc1 would then reuse the load's
 * register for the in-place AND like target does) measured WORSE
 * (68 -> 72) and was reverted. The v0/v1 coloring choice does not
 * respond to this expression-splitting lever.
 *
 * NOT yet attempted / frontier for next session: the v0/v1 coloring
 * swap is systemic (repeats at every `D_800A34F8 & 0xF` site and
 * propagates through the block), suggesting a single upstream cause
 * (local-alloc's choice of which pseudo -- the raw global-load temp or
 * the named result -- gets the lower-numbered scratch register) rather
 * than 4 independent ties. Needs a `.lreg`/`.combine` dump read (not
 * done this session -- turn budget) to see whether cc1 is failing to
 * coalesce the load-temp and the AND-result into one pseudo (a combine.c
 * question) or whether it coalesces them but colors the merged pseudo
 * v1 instead of v0 (a local-alloc quantity-ordering question). Also
 * unresolved: one single scheduling tie in block3 (target orders
 * `sb $v0,0x6($s0)` before `addu $a0,$s0,$zero`; our build does the
 * reverse) -- an equal-priority list-scheduler tie
 * (codegen-technique-index "sched-rank-class-tie-wall"), one candidate
 * fix (moving the SetSemiTrans call earlier in source) was tried and
 * reverted (wrong -- moves the call out of its target position
 * entirely, does not just reorder the tie).
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
        v0 = D_800A34F8 & 0xF;
        if (v0 == arg2) {
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
        v0 = 0x5F;
        *(s16 *)(tile + 8) = v0;
        v0 = *(s32 *)(arg1 + 0x1C);
        v0 = v0 + 0xF;
        *(s16 *)(tile + 0xA) = v0;
        v0 = *(s32 *)(arg1 + 0x18);
        *(s16 *)(tile + 0xE) = 1;
        v0 = v0 + 0x19;
        *(s16 *)(tile + 0xC) = v0;
    }
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        v0 = D_800A34F8 & 0xF;
        if (v0 == arg2) {
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
        v0 = *(s32 *)(arg1 + 0x18);
        *(s16 *)(tile + 8) = v0;
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s16 *)(tile + 0xC) = 0x78;
        *(s16 *)(tile + 0xE) = 1;
        v0 = v0 + 0xE;
        *(s16 *)(tile + 0xA) = v0;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        v0 = D_800A34F8 & 0xF;
        if (v0 == arg2) {
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
        v0 = *(s32 *)(arg1 + 0x18);
        v0 = v0 + 0x40;
        *(s16 *)(tile + 8) = v0;
        v0 = *(s32 *)(arg1 + 0x1C);
        *(s16 *)(tile + 0xC) = 0x38;
        *(s16 *)(tile + 0xE) = 1;
        v0 = v0 + 0xD;
        *(s16 *)(tile + 0xA) = v0;
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

        v0 = D_800A34F8 & 0xF;
        if (v0 == arg2) {
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
