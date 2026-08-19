/* CANDIDATE - func_80017848, s9 (2026-08-18, rederive).  sandbox --disable all = 3
 * FLOOR 5 -> 3 (variant V1_sh2_both).  Residual is 3 differing instructions, all
 * of them in loop 1's EXIT TAIL and loop 2's PREHEADER.  Loop 1's preheader -
 * including target's long-unexplained dead reg-reg copy `addu a3,a0,zero` - is
 * now BYTE-EXACT for the first time in nine sessions.
 *
 * THE SESSION'S CENTRAL DISCOVERY: HOW TARGET'S DEAD PREHEADER COPY IS PRODUCED.
 * Sessions s6/s7/s8 concluded the copy had to be created by a pass AFTER
 * combine.c (local-alloc / global-alloc / reload), because "combine deletes any
 * copy it can see".  That conclusion is now REFUTED with a measurement.  combine
 * only deletes a copy it can SUBSTITUTE, and can_combine_p refuses when the
 * copy's destination is still used AFTER the insn being combined into.  So a
 * copy survives combine iff its destination pseudo has a SECOND use downstream
 * of the base add.  Concretely, loop 1's preheader is now:
 *
 *      q   = *(u8 **)(ctx + 0xC);   <- cse folds this redundant load to `q = p`,
 *                                      i.e. a plain reg-reg COPY insn
 *      lnk = *(u8 **)(ctx + 0x10);
 *      base = (u8 *)(sh + (s32)q);  <- base add reads q
 *      ... do/while ...
 *      p = q;                       <- THE SECOND USE.  Without it combine
 *                                      substitutes q:=p into the base add,
 *                                      the copy dies, and local-alloc's
 *                                      combine_regs ties base to the dying
 *                                      pointer, giving `addu a0,a1,a0`.
 *
 * Evidence that this is the mechanism and not a coincidence: the -da .lreg dump
 * for the s8 chassis (tmp/grind/func_80017848/s9/dump/F_lreg.txt, insn 85) shows
 * `(set (reg/v 79) (plus (reg/v 82) (reg/v 78)))` carrying REG_DEAD for 78 - the
 * pointer dies exactly at the base add, which is precisely the tie-to-a-dying-
 * input case.  Giving the addend a second use removes the REG_DEAD note, the
 * tie does not happen, and both `addu a3,a0,zero` and `addu a0,a1,a3` appear
 * with target's exact register numbers.
 *
 * THE THREE LEVERS STACKED THIS SESSION (each measured on its own):
 *  (1) 5 -> 6 -> ... `q` (a named local holding a redundant re-read of ctx+0xC)
 *      as loop 1's base addend, with `p = q` as the loop-1 exit tail instead of
 *      a fresh re-read (variant R2 = 6).  On its own this LOSES a point, because
 *      the tail's `p = q` materialises as `addu a0,a3,zero` where target has
 *      `lw a0,12(s2)`.  It is only worth taking together with (2).
 *  (2) 6 -> 4  an explicit `lnk = *(u8 **)(ctx + 0x10);` local for loop 1's links
 *      pointer, read BETWEEN the `q` read and the base assignment (variant S1).
 *      Order matters: reading lnk BEFORE q (S2) is 6, not 4.  This fixes the
 *      position of `lw a2,16(s2)` relative to the copy/base pair.
 *      (s7 had killed the links local outright as variants X1/X2/X3 - on the s7
 *      and s8 chassis.  Fourth confirmation that every spelling conclusion on
 *      this function is chassis-relative.)
 *  (3) 4 -> 3  a SECOND shift local `sh2 = slot_a << 6;` recomputed immediately
 *      before loop 2's guard and used by loop 2's guard AND base (variant V1).
 *      Guard-only (V2) is 5 and base-only (V3) is 7 - both sites must move
 *      together.  This restores target's `sll a1,s4,6` in loop 2's preheader,
 *      which the single shared `sh` of the s8 candidate had hoisted away.
 *      NOTE this directly contradicts s8's frontier item #2, which predicted the
 *      shift had to be shared by the two GUARDS and recomputed for loop 2's
 *      BASE; the 16-way sweep of that four-way assignment (s9 variants M0000..
 *      M1111) proved the opposite - see the rejected/ bank.
 *
 * THE REMAINING 3 (T.txt vs B.txt):
 *   loop-1 exit tail: target `lw a0,12(s2)`      ours `addu a0,a3,zero`
 *   loop-2 preheader: target `addu a3,a0,zero`   ours `lw v0,12(s2)`
 *                     target `addu a0,a1,a3`     ours `addu a0,a1,v0`
 * i.e. loop 2 still needs the copy that loop 1 now has, and loop 1's tail still
 * pays for supplying loop 1's second use.  The two are the SAME missing fact:
 * a second downstream use for loop 2's base addend.  Every use site available
 * after loop 2 was measured this session and all of them are fatal, because
 * target re-reads ctx+0xC freshly three more times in the tail (math_Distance3D,
 * rec_a, rec_b) - routing any of them through a live local costs 19 points
 * (T1/T3/T4 = 22).
 *
 * [s10 SYNTHESIS ADDENDUM - body unchanged, still 3.]  The s9 header above reads
 * target's two preheader copies as one symmetric construct.  s10 disproved that
 * from the target listing itself: loop 1's skip branch is `blez $v0, .L8001791C`
 * (asm/funcs/func_80017848.s:62), which lands on loop 2's GUARD, so loop 2's
 * guard block is a JOIN with two predecessors and cse's extended basic block
 * cannot reach loop 2's preheader.  Loop 1's preheader, by contrast, is
 * single-predecessor.  So loop 1's copy is a cse-folded redundant load that
 * survived combine (s9's H-s9-1, correct), and loop 2's copy CANNOT be - it has
 * a different, still-unidentified origin, and every C-level way to write a
 * reg-reg copy at that point is now measured dead (source copy 8-9, shared
 * local 13, duplicated-into-arms 35).
 *
 * s10 also priced the residual exactly: a preheader copy costs 2 points to BUY
 * (the loop's exit tail is spent on the second use) and returns 2.  Loop 1 only
 * nets out because its purchase instruction lands where target has one of its
 * own.  Dropping the purchase entirely (symmetric fresh reads, no second use in
 * either loop) is 4, not 1 - see rejected/s10_no_l1_second_use_symmetric_fresh
 * _reads_costs_4.c.
 */
/* [s11 STRUCTURAL ADDENDUM - body unchanged, still 3, re-measured on a clean
 * tree as cell A_base.]  s11 swept ~120 cells across every structural axis of
 * this chassis and found no cell below 3.  What it DID establish:
 *   - s3's two surviving conclusions are retired: pointer-first association is
 *     now a strict REGRESSION at four of the five address sites (top guards 5,
 *     loop-1 guard 4, loop-1 base 4, loop-2 guard 4), and the `slots` hoist
 *     above the two >=0 top guards is INERT (inline reads in both guards = 3).
 *     The `slots` local is kept here only because it is tied, not because it
 *     earns anything.
 *   - loop 2's entire preheader is C-INERT: base addend (inline / named local /
 *     reused), links (inline / named local), a second use of the addend, the
 *     guard shape (inline / two-step through a SEPARATE t2 / count-into-local)
 *     and the association order all score identically.  47 cells, one score.
 *   - declaration order of the 13 locals is completely inert (56 permutations).
 *   - loop 2's guard MUST consume the carried `p`.  Reading ctx+0xC freshly
 *     there - which is what target's asm literally does - costs 6 (reusing `p`)
 *     to 28 (a new local), uniformly across 66 cells.
 *   - s8's loop-1 two-step guard local (3 vs 7 vs 7) and its t-reuse kill for
 *     loop 2 (36 / 18) both re-confirm on V1; s9's second shift local is +1.
 *   - THE s9 SECOND-USE LEVER IS NOT TARGET'S MECHANISM.  Target has no live
 *     carrier of ctx+0xC out of either loop: it re-reads it three times in the
 *     tail (s10/T.txt:77, :94, :104) and each addend register dies at its base
 *     add.  So both of target's preheader copies exist with NO second use, and
 *     loop 2's copy can never be bought the s9 way - there is no purchase site.
 *     The `p = q;` tail below reproduces loop 1's bytes for the wrong reason,
 *     at a price of exactly 1 (fresh-read tail = 4, no tail = 12).
 */
/* [s12 STRUCTURAL ADDENDUM - body unchanged, still 3, re-measured this session
 * as cell T0 on a clean tree.]  s12 spent ~90 cells closing the FOUR regions the
 * s11 frontier named as the last hand-reachable structural surface, and all four
 * are now dead:
 *   - PARAMETER HOMING / PROLOGUE (s11 frontier #3).  Homing any parameter into
 *     a named local is a strict regression (slot_a 6, ctx 6, arg1 6, both slots
 *     7 in either order).  The prologue stays byte-exact in every cell, so the
 *     "allocno priority is set by pseudo creation order in the prologue" theory
 *     has no C-level handle.  Top-block spellings: fused && = 3, redundant slots
 *     re-read between the guards = 3, slots read above the equality return = 7,
 *     sh hoisted above the guards = 6, guard values through `t` = 41.
 *   - LOOP BODY.  `* 16` for `<< 4` = 3, `!=`-else = 3, byte staged into `t` = 3,
 *     links-first association = 5, `(i + 0x24)` grouping = 7, element pointer
 *     local = 13.
 *   - EXIT FORM / LOOP SHAPE.  jump.c ALREADY cross-jumps our four inline
 *     `return 0`s into target's single shared exit block, so spelling it out
 *     costs: all-goto = 15, mixed = 8, accumulator = 9.  Reversed while test = 3,
 *     separate counter `j` for loop 2 = 3, count into a local = 30, for-form = 21.
 *   - BASE DESTINATION VARIABLE IDENTITY (the sanctioned variable-reuse family
 *     applied to the RECEIVING local rather than the addend).  base1/base2 = 3,
 *     loop-2 base into `p` = 3, into `q` = 8, `slots` = 8, `lnk` = 10, rec_a = 41.
 *   And no COMPOUND win exists: eight stacked combinations of every 3-scoring
 *   lever above, including one stacking all seven at once, all score exactly 3.
 *
 * THE ONE GENUINELY NEW STRUCTURAL FACT, and the best hand-off this session has:
 * the SYMMETRIC CHASSIS.  Replace the `p = q;` loop-1 exit tail below with
 * target's own `p = *(u8 **)(ctx + 0xC);` and the residual becomes 4 but
 * PERFECTLY SYMMETRIC - both preheaders missing `addu a3,a0,zero`, both base
 * adds reading the wrong register.  Loop 1 then comes out one instruction SHORT
 * (126 vs 127) while loop 2 has the right COUNT but emits `lw v0,12(s2)` exactly
 * where target has the copy.  That is the sharpest statement of the wall to date:
 * loop 2's copy is a redundant ctx+0xC load that target's compiler folded to a
 * copy, in a block cse provably cannot reach.  On that chassis the addend MUST
 * be a fresh ctx+0xC read (any live pointer = 9) and all 12 spellings of the
 * fresh read score exactly 4 - so the symmetric chassis is a better DESCRIPTION
 * of the wall than V1 is, but not a better score.  Banked as
 * rejected/s12_symmetric_fresh_reload_tail_costs_4.c.
 *
 * Also closed: duplicating loop 2 into both arms of the loop-1 guard (to give its
 * preheader a single-predecessor block so cse folds, then let jump2 re-merge) is
 * 35 - jump2 does NOT merge them.  The join cannot be removed from C.
 */
/* [s13 PERMUTER ADDENDUM - body unchanged, still 3.]  The automated-breadth
 * modality that s10/s11/s12 all named as the highest-yield remaining lever has
 * now been spent and found nothing.  Three telemetried campaigns totalling
 * 98,604 iterations - V1 with loop 2 fenced off (25,905 iters), V1 with no
 * fences at all so the randomizer could touch loop 2's preheader jointly with
 * every other region (27,827 iters), and the s12 symmetric chassis (44,872
 * iters) - produced 16 finds and ZERO engine-scored improvements on either
 * chassis.  Both chassis floors (3 and 4) survived intact.
 *
 * The one durable fact s13 adds: on THIS function the permuter's own weighted
 * score is ANTI-CORRELATED with the engine distance, now measured a third time.
 * s13b's best find (perm 220, ninety points better than its base) re-scores to
 * engine 5, while the cells that actually tie its floor at engine 4 sit at perm
 * 310 = exactly the base score.  The residual here is 2-3 register-IDENTITY
 * differences, which the permuter's metric prices far below the instruction
 * reorderings it will happily trade them for.  Any future campaign on this
 * function must re-score every single output with `sandbox --disable all`.
 *
 * Do NOT re-run a permuter campaign on either of these two chassis.  The next
 * untried axis is the one s12's frontier named third and no session has touched:
 * the TRANSLATION-UNIT context outside the function body (the math_Distance3D
 * prototype, the position of func_80017848 within src/ings.c, the number and
 * order of the file's preceding declarations) - the only surface left that can
 * move GCC 2.7.2's allocno ordering without changing a single statement.
 */
/* [s14 PERMUTER ADDENDUM - body unchanged, still 3.]  Two things closed.
 *
 * (1) THE TRANSLATION-UNIT AXIS IS DEAD.  s13's frontier named it "the only
 * named axis in thirteen sessions that has never been probed" and the highest-
 * yield remaining item.  Five mutations, all on top of this exact body, all
 * score 3: 40 extern decls immediately above the function; 200 extern decls at
 * the top of the file; the math_Distance3D / math_Distance3D_16 DEFINITIONS
 * moved below func_80017848 and replaced by prototypes; an extra whole function
 * definition immediately above; ten file-scope static consts immediately above.
 * GCC 2.7.2 resets pseudo numbering, cse's hash tables and local-alloc's allocno
 * arrays per FUNCTION - the only TU-accumulating state is label_num and varasm
 * bookkeeping, neither of which feeds allocation or scheduling.  The
 * cc1-first-pass-scheduler-bug analogy does not transfer (that is a crash
 * pathology, not a codegen-selection channel).  Cost: six sandbox runs.
 *
 * (2) TWO MORE PERMUTER CAMPAIGNS, 81,868 iterations, nothing below 3.  s14a
 * was the first DIRECTED campaign on this function: a PERM_GENERAL cross-product
 * over the levers s11/s12 had each measured at exactly 3 (top-guard shape,
 * loop 1's element read, loop 2's block form) layered on PERM_RANDOMIZE - aimed
 * squarely at the compound space s12 could only sample 8 cells of by hand.
 * 47,221 iterations, 14 finds, nothing below 3.  s14n was seeded on a chassis
 * that did not exist before this session (N1, below), whole-function randomized:
 * 34,647 iterations, 11 finds, nothing below 3.  The perm-score / engine-distance
 * ANTI-CORRELATION reproduced on both (perm 315 -> engine 5 twice, while every
 * engine-3 output sits at perm 405 = the base score); that is five campaigns and
 * 180,472 iterations agreeing.
 *
 * THE ONE NEW STRUCTURAL FACT: cell N1, `sh` RECOMPUTED ON LOOP 1'S TAKEN PATH.
 * Take m2c's reading of the target, which puts `var_a1 = temp_s4 << 6` on loop
 * 1's exit EDGE rather than in loop 2's preheader: delete `sh2` entirely, append
 * `sh = slot_a << 6;` as the last statement of loop 1's guarded block (after
 * `p = q;`), and let loop 2's guard and base both read `sh`.  That scores 3 with
 * a residual BYTE-IDENTICAL to this body's - same three instructions, same
 * positions.  Placing the recompute BEFORE `p = q;` costs 4.  Both banked in
 * rejected/.  A faithful transcription of m2c's whole output (rotated loops,
 * walking element pointer, carried copy-source local) scores 49, which is the
 * direct measurement of an assumption eight sessions made in passing: m2c
 * reflects the SCHEDULED asm, not the source shape, so it is not a chassis seed.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    p = *(u8 **)(ctx + 0xC);
    sh = slot_a << 6;
    i = 0;
    t = sh + (s32)p;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        q = *(u8 **)(ctx + 0xC);
        lnk = *(u8 **)(ctx + 0x10);
        base = (u8 *)(sh + (s32)q);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
        p = q;
    }

    i = 0;
    sh2 = slot_a << 6;
    if (i < *(s32 *)(sh2 + (s32)p + 0x20)) {
        base = (u8 *)(sh2 + (s32) * (u8 **)(ctx + 0xC));
        do {
            if (*(s16 *)((*(u8 *)(base + i + 0x2C) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x20));
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (slot_b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | slot_b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
