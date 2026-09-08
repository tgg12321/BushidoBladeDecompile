/* [s51 REDERIVE NOTE - body unchanged, re-audited at 3 (127/127) on the HEAD
 * chassis (anchor src/ings.c:820); fake_ablate reports NO FAKE constructs in
 * this body, so every s42-s50 instance kill was measured without a FAKE mask.]
 * THE M-BRANCH MOVED FOR THE FIRST TIME: 17 -> 10.  s50's cell M2 (the first
 * form carrying both preheader copies AND the loop-1 exit-tail reload at
 * 127/127) was carrying 7 self-inflicted points.  M2 had added an explicit
 * `lnk = *(u8 **)(ctx + 0x10);` local to LOOP 2's preheader, which this body
 * never had - BASE reads that pointer inline in loop 2's body.  The extra local
 * lengthens lnk's live range, lifts its global.c priority above `sh`, and
 * rotates the a1/a2 seats in BOTH loops.  Cell M7 = M2 with that local deleted
 * measures 10 at 127/127 with the structure otherwise untouched.  That retires
 * s50's frontier item 1 ("why does sh seat in a2 and lnk in a1") outright: it
 * was the lnk local, not the added q/r allocnos.
 *
 * M7's remaining 10 is a clean TWO-WAY TRADE-OFF worth 2 points on each side,
 * and the two halves are disjoint:
 *   - WITH the pre-guard `q = p;` (M7/M19): loop-1's preheader is BYTE-EXACT -
 *     `addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3`, correct a3 seat and
 *     correct order - but reorg duplicates `addu v1,zero,zero` (i = 0) into the
 *     loop-1 guard's `bltz` delay slot where target has a nop.
 *   - WITHOUT it (M8): the `i = 0` placement is byte-exact in both loops, but
 *     both copy destinations seat in t0/t1 because `q` and `r` have no
 *     definition on their loop-skip paths, so flow makes them live-in at entry
 *     where they conflict with the incoming a0-a3 parameter registers.
 * Five statement placements were measured; every one keeping `q = p;`
 * reproduces M7's residual exactly and every one dropping it reproduces M8's,
 * so this is a liveness fact, not a statement-order artefact.
 *
 * The branch's real wall is now pinned from T.txt: the target reloads
 * *(u8 **)(ctx + 0xC) SIX times (T.txt 14/28/52/77/94/104) and every post-loop-2
 * address add reads a FRESH reload, never the a3 copy - so loop-2's
 * `addu a3,a0,zero` has NO downstream reader at all in the target.  Six C-level
 * readers for it were measured and all cost (P1 no-reader = 4 at 126 - the copy
 * is simply deleted; N1 = 18, N2 = 25, P2 = 12 at 126, P3 = 29, P4 = 34).
 * Forms banked as rejected/s51_*.c; M7 is
 * rejected/s51_M7_loop2_lnk_local_removed_seat_and_call_residual_costs_10.c.
 * Details: evidence.md/hypotheses.md s51 sections.
 */
/* [s50 REDERIVE NOTE - body unchanged, re-audited at 3 (127/127) on the HEAD
 * chassis (anchor src/ings.c:820).]  THE 3-INSN STRUCTURAL RESIDUAL IS SOLVED
 * ON A NEW CHASSIS.  The residual was pinned exactly this session (s50/T.txt vs
 * s50/B_C.txt): (a) loop-1 exit tail `lw a0,12(s2)` vs this body's `addu
 * a0,a3,zero` (from `p = q;`), (b)+(c) loop-2 preheader `addu a3,a0,zero` +
 * `addu a0,a1,a3` vs this body's `lw v0,12(s2)` + `addu a0,a1,v0`.
 * Cell F1 (this body with `p = q;` -> `p = *(u8 **)(ctx + 0xC);`) = 4 at 126:
 * it WINS residual (a) outright and costs exactly loop-1's copy, proving the
 * tail move and loop-1's copy are ONE purchase (q's only out-of-block reader).
 * Cell M1 moves that purchase to a FREE site - `q` is seeded before the loop-1
 * guard, re-read (cse -> copy) in loop-1's preheader, and consumed by loop-2's
 * BASE ADD (an add the target emits anyway) while loop-2's GUARD reads the
 * reloaded `p`; the join is a cse EBB boundary so the two same-valued pseudos
 * are never merged.  M1 = 14 at 126 with loop-1's copy AND the tail reload both
 * BYTE-MATCHING.  Cell M2 = M1 + a second copy `r = q;` in loop-2's preheader
 * bought by using `r` as math_Distance3D's first argument: 17 at 127/127, the
 * first form ever to carry BOTH preheader copies and the tail reload together.
 * M2's whole remaining residual is REGISTER SEATS (sh<->lnk rotated to a2/a1
 * instead of a1/a2; loop-2's copy sourced from q/t0 instead of p/a3), i.e. the
 * closing question on this branch is an allocation question again - and M2 IS
 * the chassis change the s47-s49 solver work said the RA axis needed.  Forms
 * banked as rejected/s50_*.c; M2 is
 * rejected/s50_STRUCTURALLY_EXACT_127of127_both_copies_seat_residual_costs_17.c.
 * Details: evidence.md/hypotheses.md s50 sections.
 */
/* [s45 OBJECT-MODEL NOTE - body unchanged, still 3 at 127/127 on the HEAD chassis
 * (anchor src/ings.c:820).]  Owner-directed object-model audit banked as
 * evidence.md "OBJECT MODEL:" (s45).  This function references no global; its
 * data model is the 52-byte object block ctx (matched sibling func_80017D84,
 * src/ings.c:824-843: s16 link count at +6, 0x40-byte record array at +0xC,
 * 0x10-byte link array at +0x10).  Typing ctx as a struct (O1) is byte-IDENTICAL
 * to this body; typing the record/link arrays costs 7 (s30 B re-audit), 37 (O2,
 * full typing + typed tail: global.c seats the guard and preheader loads both in
 * v1 and reorg deletes the preheader load), 43 (O3, symmetric U4 typed, 5 insns
 * short) and 47 (O4, natural for-loop typed, 9 insns short) because typed
 * indexing lets cse share the record pointer the target reloads in every block.
 * The raw u8 * + literal-offset idiom below is the matched sibling's idiom on
 * this object and is evidence-consistent; the residual (E-s42-1) is unchanged
 * and is not an object-model artifact.  Premises P1-P5 listed in evidence.md.
 */
/* [s44 SYNTHESIS NOTE - body unchanged, still 3 at 127/127 on the HEAD chassis
 * (anchor src/ings.c:820); U4 re-audited 6 at 125.]  Frontier items 1 and 2
 * measured dead (F1a/F1b/F2 = 6, F2b = 12; E-s44-1/2).  NEW MECHANISM, dump-
 * proven with cheat instruments that are banked in rejected/ and are NEVER
 * candidates: a loop-body read of the copy destination that combine folds to
 * a constant AFTER flow keeps the use-once preheader copy alive (combine.c:1458
 * added_sets_2 -> unrecognised PARALLEL) at zero reader bytes, and global.c
 * allocates the copy destination as live across the loop from flow's stale
 * basic_block_live_at_start.  Instrument M5 reproduces target's instruction
 * stream EXACTLY (127/127, both copies) with a pure seat permutation decided by
 * global.c:615 priority order (q allocated before sh/lnk -> a1; a3 needs q
 * after both).  Refutes E-s43-5.  The natural-C search is narrowed to loop-body
 * expressions combine proves constant/unreachable (E-s44-6).  Details:
 * evidence.md/hypotheses.md s44 sections.
 */
/* [s43 STRUCTURAL NOTE - body unchanged, still 3 at 127/127 on the HEAD chassis
 * (anchor src/ings.c:820); U4 re-audited 6 at 125.]  Six new cells, all dead:
 * W1 (loop-1 exit tail reload+sh recompute INSIDE the if, no join reload) = 12,
 * a p<->sh seat swap in both guards; W2 (`q = q;` in the loop bodies as a
 * loop-carried reader) = 6 and emits NO RTL at all (expr.c:2845 store_expr skips
 * a move whose source rtx is the target) - class kill; the sanctioned
 * do/while(0) wrap measured for the first time on this function around the
 * copy (X1), the whole then-block (X2), the preheader statements (X4) - all
 * exactly 6, objects identical to U4 - and around the inner loop only (X3) =
 * 10 (loop_depth re-weights the loop-1 pseudos, seats rotate, still no copy).
 * Reading banked: global.c:919-928 collects regs_someone_prefers from
 * LOWER-priority conflicting allocnos only and :899-910 prunes the parameter
 * pseudos' a0-a3 copy preferences as call-crossing, so the a3 seat cannot come
 * from pass-0 exclusion (third independent confirmation of s40's requirement
 * that the copy destination be live across its loop body).  Details:
 * evidence.md/hypotheses.md s43 sections.
 */
/* [s42 STRUCTURAL NOTE - body unchanged, still 3 at 127/127 on the HEAD chassis
 * (anchor src/ings.c:820).]  Fresh normalised diff: loop 1 is instruction-exact;
 * the whole residual is (a) the exit tail (target reloads p with lw a0,12(s2),
 * this body's `p = q;` is a real move), (b) loop 2's addend (target copies p
 * into a3, this body loads into v0), (c) loop 2's base add reading v0 for a3.
 * DUMP-PROVEN this session: target's tail geometry (lw/sll on the fall-through
 * only, blez landing on the guard add) is reorg.c:3442-3459/3714-3716's
 * redundant-insn thread redirect acting on an UNCONDITIONAL reload in the join
 * block, so the `p = q;` device is NOT what target does there; it is what buys
 * loop 1's copy (U5 proves that: reload after `p = q` -> dead store -> copy
 * dies, 4 at 126).  The natural symmetric chassis (U4, s42/body_U4.c: no tail,
 * unconditional reload, distinct two-step t2 guard, `q = p` copies) is 6 at 125
 * with a residual of exactly the two use-once copies combine deletes.  Reload's
 * find_equiv_reg copy producer is closed by predicate (local-alloc.c:1079).
 * Details: evidence.md/hypotheses.md s42 sections.
 */
/* [s40 REDERIVE NOTE - body unchanged, still 3 at 127/127 on the HEAD chassis
 * (anchor src/ings.c:820).]  Two new reader classes for the preheader copy were
 * measured and killed: a call-argument reader (m9a: copy+add shape reproduced,
 * but q/lnk are live at entry on the loop-skip paths and collide with the
 * incoming a2/a3 hard registers, seats t0/t1) and a return-arm reader merged by
 * cross-jump (m9c, chassis 15 at 127/129: jump.c:2525 merges only identical
 * insns, the tail keeps the read).  The a3 seat was measured from global.c's
 * own pass-0 sets (reg80: conflicts v0,v1,a0,a1,a2,s2,s3,sp; a3 first eligible):
 * target's two a3 copies are global allocnos live across their loops, so each
 * has a flow-time reader that left no bytes.  Details: evidence.md/hypotheses.md
 * s40 sections.
 */
/* [s39 FORENSICS NOTE - body unchanged, still 3 at 127/127 on the HEAD chassis
 * (anchor src/ings.c:820).]  Frontier item 1 (an interposer reading the copy
 * destination so the add loses its LOG_LINK) is CLOSED as a class kill: the
 * mechanism is real (m8a) but every same-value interposer is removed by cse2
 * before flow (cse.c:6730; m8e/m8f/m8g; J1 = 4 = P4 on the join chassis) and a
 * value-computing one is an instruction target lacks unless jump2's no-op-move
 * deletion (jump.c:441) applies, which needs a later a3 reader that does not
 * exist.  reload's find_equiv_reg copy path (reload1.c:5851) needs an
 * unallocated pseudo and none exists here.  m8b shows a combine-surviving copy
 * can still vanish at RA (Q given P's released register).  Details:
 * memory/grind/func_80017848/evidence.md and hypotheses.md, s39 sections.
 */
/* [s38 ADDENDUM 2026-09-06, forensics - body unchanged, still 3 at 127/127 on the
 * HEAD src/ings.c:820 anchor.]  Frontier item 1 (escape #8 with a vanishing label)
 * is closed on minimal geometry: a real forward branch between the copy and the
 * base add keeps the copy but emits it BEFORE the branch (m7a), a loop back-edge
 * is undone by loop.c hoisting into the copy's block (m7b), and the only
 * post-combine zero-residue label deletion is jump2's no-op-move path
 * (jump.c:449) whose C form is a dead conditional self-copy.  combine.c:985's
 * volatile_insn_p refusal does not see C volatile loads (rtlanal.c:1366).  The
 * can_combine_p refusal list is fully enumerated in evidence.md s38.  Q1 (escape
 * #9) re-measured 14 at 127/127 on this chassis.  Details: memory/grind/
 * func_80017848/evidence.md and hypotheses.md, s38 sections.
 */
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
/* [s15 FORENSICS ADDENDUM - body unchanged, still 3, re-measured on a clean tree.]
 * The long-running question "which pass makes the preheader copy, and why does
 * the base add consume it" is now ANSWERED WITH MEASUREMENTS rather than
 * inference, using the instrumented cc1 (tools/gcc-2.7.2/cc1 - NOT build/cc1),
 * re-proved codegen-identical to the frozen build/cc1 on this whole TU in every
 * dump run (tmp/grind/func_80017848/s15/idump.sh).
 *
 *  (1) THE PASS IS local-alloc.c's optimize_reg_copy_1 (local-alloc.c:700,
 *      dispatched at :1006).  Loop 1's base add is `(plus (reg 84) (reg 79))` -
 *      i.e. it reads the ORIGINAL pointer, not the copy's dest - in .cse, .loop,
 *      .cse2, .flow, .combine AND .sched, and `(plus (reg 84) (reg 80))` in
 *      .lreg, with reg79's REG_DEAD note moving from the base add up onto the
 *      copy.  optimize_reg_copy_1 rewrites SRC to DEST over the range where SRC
 *      dies "to enable DEST to be tied to SRC"; it does NOT delete the copy, and
 *      nothing after local-alloc runs DCE, so the copy is emitted.
 *
 *  (2) THE COPY SURVIVES COMBINE because flow.c:2102 creates LOG_LINKs only when
 *      the using insn is in the SAME basic block, and combine only follows
 *      LOG_LINKs.  Loop 1's copy dest (reg80) has exactly one pre-lreg use -
 *      `p = q` in the loop-1 exit tail, a different block - so combine never
 *      gets a link to it.  General rule: a preheader reg-reg copy survives
 *      combine iff nothing in its own basic block uses its destination.
 *
 *  (3) LOOP 2'S PREHEADER IS NOT C-INERT.  s13/s14's frontier said the block
 *      "has no C-level handle at all"; that is now false.  A loop-INVARIANT
 *      reg-reg copy written inside loop 2's BODY is hoisted verbatim into the
 *      preheader by loop.c's move_movables (loop.c:1690-1710) and survives
 *      combine by (2), because its uses stay in the body.  Cell C1 emits
 *      `lw $2,12($18) / lw $6,16($18) / addu $4,$5,$2 / move $5,$4` - the copy IS
 *      there, in target's block, from pure C.  It scores 6 because it copies the
 *      BASE rather than the ADDEND, so it lands one slot too late.  Hoisting the
 *      copy together with its consumer (cell D1) puts both in the same block and
 *      combine deletes the copy again: D1's preheader is byte-identical to this
 *      body's, at 3.  Both banked in rejected/.
 *
 *  (4) THE reuse-p FAMILY IS STRUCTURALLY EXCLUDED, not merely expensive.  s11/
 *      s12 recorded 8/9/19/28/41 as bare numbers.  The dump of cell A1 shows the
 *      mechanism: if loop 2's base addend is provably equal to the guard's
 *      addend, cse merges the guard-address add and the base add into ONE insn
 *      (`.L171: sll $2,$20,6 / addu $4,$2,$4 / lw $2,32($4)`), while target
 *      computes sh2+ptr TWICE (0x8001791C and 0x80017938).  A1/A2/A3/A4 all = 8.
 *      Corollary: loop 2's base addend MUST stay a fresh *(u8 **)(ctx + 0xC)
 *      read, exactly as written below.  Do not re-probe that family.
 *
 * WHAT REMAINS.  Target's loop-2 copy has a destination used exactly ONCE, at
 * the base add, in the same block, dying there (a3 appears 5 times in the whole
 * target listing).  By (2) such a copy is deleted by combine if it exists before
 * combine; by (3) a hoisted one is too; and any copy whose dest is used
 * out-of-block leaves that use visible in the asm - our loop 1 pays for it with
 * `move $4,$7` - and target has no such instruction.  So the copy is created, or
 * made unfoldable, AFTER combine.  The only remaining 2.7.2 candidate is
 * local-alloc's optimize_reg_copy_2 (local-alloc.c:874), which fires on a copy
 * whose SRC dies in it when a REVERSE copy `SRC = DEST` appears later in the same
 * block before any label/jump/LOOP_BEG/LOOP_END note, rewrites the range, and
 * LEAVES BOTH COPY INSNS IN PLACE because no DCE follows local-alloc.  That
 * predicate has never been written from C.
 */
/* [s16 FORENSICS ADDENDUM - body unchanged, still 3, re-measured on a clean tree
 * as cell A_base.]  20 cells, three instrumented-cc1 dump runs (all re-proved
 * CODEGEN-IDENTICAL to the frozen build/cc1 on the whole TU).  Four hypotheses
 * killed, one confirmed, and the residual is now traced to a SINGLE cse decision.
 *
 *  (1) THE SURVIVAL TRICHOTOMY IS COMPLETE.  A preheader reg-reg copy written
 *      from C meets exactly one of three fates, decided by where its DESTINATION
 *      is used: unused -> deleted by CSE (not flow - cell I1's copy disappears
 *      between .jump and .cse, three passes before flow, and the cell is inert at
 *      3); used in the SAME basic block -> deleted by combine (cell E2, insn 83
 *      -> NOTE_INSN_DELETED); used in a DIFFERENT block -> survives, and the use
 *      costs an instruction (this body's loop 1, `move $4,$7`).  Target's loop-2
 *      copy fits none of them: its dest a3 is used once, in the same block, and
 *      is not deleted.
 *
 *  (2) THE CENTRAL NEW MEASUREMENT - THE cse CANONICALISATION FLIP.  Loop 1's
 *      preheader C is character-identical in this body and in cell E2, insn
 *      numbering is identical, and insn 83 is `(set (reg/v 80) (reg/v 79))` in
 *      both.  But .cse insn 89 (the base add) reads reg79 - the ORIGINAL - here
 *      (s15/icand/F_cse.txt:177) and reg80 - the COPY DEST - in E2
 *      (s16/iE2/F_cse.txt:178).  The only C difference is the loop-1 EXIT TAIL,
 *      which is DOWNSTREAM of insn 89.  When cse leaves the original, the copy is
 *      orphaned, gets no LOG_LINK, survives combine, and optimize_reg_copy_1
 *      re-points the base add onto it (target's bytes).  When cse substitutes the
 *      copy dest, combine deletes the copy and loop 1 comes out one instruction
 *      SHORT (E2's preheader: `lw $6,16($18) / addu $4,$5,$4 / addu $2,$4,$3`).
 *      So `p = q;` below is not merely "the second use" - it is the C handle on
 *      ONE cse canonicalisation, and that canonicalisation is the whole residual.
 *
 *  (3) FOUR KILLS.  reg_n_sets>1 does NOT protect a copy from combine (E2 has `q`
 *      set in both preheaders; combine deleted it anyway) - so target's a3 being
 *      multiply-set is an allocation coincidence, not the mechanism.
 *      optimize_reg_copy_2, s15's sole remaining producer candidate, is dead both
 *      analytically (local-alloc.c:874-935 leaves BOTH copies, so it always emits
 *      two moves where target has one) and by measurement (G1 = G2 = 13).  The
 *      hoisted-addend-copy lever is dead (F1 = 15, F2 = 16).  And loop 1's
 *      guard-clobber trick does not transplant to loop 2: K1/K2/K3/K5 = 35/32/32
 *      /36 with the CONTROL K4 (guard change alone) = 36.
 *
 *  (4) CONSEQUENCE FOR EVERY FUTURE HYPOTHESIS.  No named 2.7.2 routine creates a
 *      redundant reg-reg copy after combine.  Target's loop-2 copy is therefore a
 *      PRE-combine copy that combine DECLINED to fold.  Any future proposal must
 *      explain a combine REFUSAL on an in-block use - not a post-combine creation,
 *      and not an out-of-block use (which would show up as an instruction target
 *      does not have).
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
/* [s17 ESCALATION ADDENDUM — body unchanged, still 3, re-measured as A_base.]
 * s17 closed s16's frontier item #2 by reading tools/gcc-2.7.2/combine.c:803-970
 * end to end and enumerating EVERY `can_combine_p` refusal path for this copy
 * (E-s17-1).  Seven paths exist; two are forbidden cheat families, three are
 * structurally unreachable, one (out-of-block use) is the candidate's own loop-1
 * lever and is priced dead for loop 2, and the last — `use_crosses_set_p`, the
 * only un-tried one — was built and killed with two controls (E-s17-2): the
 * only zero-cost carrier for re-setting the copy's source is hoisting loop 2's
 * links read into a pointer local, and that hoist alone is -11 (P5 = 14) against
 * a maximum return of +2.  P7/P8 also measured the FULLY target-shaped C (fresh
 * ctx+0xC re-read as loop 1's exit tail + a loop-2 preheader copy) at 12.
 * Gate (a) scan_hand_coded = LOW 0/8.  Both endgame-lock gates fail; the
 * function was disposed under the owner's standing ruling (2026-07-27) — see
 * docs/grind/decisions.md.
 */
/* [s18 REDERIVE ADDENDUM — body unchanged, still 3, re-measured twice this
 * session on a clean tree (HEAD's committed src/ings.c body scores 16; this body
 * scores 3 at 127/127).]  s18 closed s17's frontier item #2 — "a whole-function
 * chassis derived from a different source-level decomposition" — negatively, and
 * added the first external prior art this function has ever had.
 *
 *  (1) THERE IS NO SECOND m2c READING.  Five structuring configurations
 *      (default, --no-andor, --passes 2, --gotos-only, --reg-vars) collapse to
 *      ONE decomposition: --passes 2 is byte-identical to default, --no-andor and
 *      --gotos-only differ only in how the top guard and the shared exit are
 *      spelled, and --reg-vars emits semantically broken C.  The frontier's
 *      premise was false for this function.
 *
 *  (2) THE RAW RE-DERIVATION IS 53, AND THE GAP IS NOW ATTRIBUTED.  R1 (faithful
 *      --no-andor transcription) = 53; R2 (m2c's goto-structured single-exit CFG
 *      + this body's loops) = 15; R3 (this body's CFG + m2c's walking-pointer
 *      rotated loops) = 49.  So m2c's control flow costs +12 and its loop form
 *      costs +46, additively.  The s8/s9 lineage is not a historical accident.
 *
 *  (3) THE EXPLICIT WALKING ELEMENT POINTER IS DEAD AT EVERY SITE.  Target's own
 *      preheader ends with `addu $v0,$a0,$v1` and its loop closes with the same
 *      insn, but writing that pointer in C costs 13 (loop 1, s12), 14 (loop 2,
 *      cell W2 this session, at the correct 127/127 count — all of it register
 *      identity) or 49 (both).  GCC already strength-reduces this body's
 *      index addressing into that pointer.
 *
 *  (4) PRIOR ART FOR THE RESIDUAL IDIOM EXISTS AND IT IS PURE C.  Mining the
 *      3,754-scratch decomp.me corpus for matched scratches containing the
 *      copy→add pair found 163 hits, 50 target-shaped, 28 with the exact
 *      copy-then-redefine-source pattern.  The structural twin is scratch 19TpT
 *      (func_8009C6D8, gcc2.7.2-cdk -O2, matching): its copy is an INNER loop's
 *      induction base initialised from an OUTER loop's live row-base in a plain
 *      `for (row) for (col)` nest — the consumer sits in the inner loop body, a
 *      different block, so it survives combine (s16's out-of-block leg) and is
 *      free because the outer loop keeps the source live anyway.  That is the
 *      precondition func_80017848 does not have and cannot buy: no nested loop,
 *      nothing live across loop 2, and every tail re-read routed through a live
 *      local is 19-22 (s9/s11).  Evidentiary value: the residual idiom IS
 *      compiler-producible pure C, corroborating scan_hand_coded = LOW 0/8.
 */
/* [s18 SECOND-DISPATCH ADDENDUM (rederive) - body unchanged, still 3, re-measured
 * at 127/127 at the start of the session; HEAD's committed body is 16.]  The one
 * rederive leg the first s18 dispatch had not run - SIBLING / KENGO TRANSPLANT -
 * is now closed three independent ways, and one new cell was measured.
 *
 *  (1) NO WHOLE-FUNCTION SIBLING EXISTS IN BB2.  A 5-gram opcode-sequence census
 *      of all 1,437 asm/funcs bodies in the 0.6x-1.8x size band tops out at
 *      0.120 overlap with this function.  Nothing in the game is shaped like it.
 *
 *  (2) THE RESIDUAL'S SEAM IDIOM IS UNIQUE IN THE BINARY.  Target's exact
 *      fingerprint - `addu D,S,$zero` then, within three insns and no label, a
 *      three-register `addu` consuming D and redefining S - occurs in exactly 3
 *      of 1,437 functions: this one (both loops), func_800200DC (call-return
 *      staging after `jal SquareRoot0`, and it carries 14 regfix RA rules) and
 *      func_8005E54C (still an asmfix asm body, distance 799).  Widened to any
 *      consumer over matched, rule-free, pure-C functions: 31 hits, every one
 *      call-return or shift staging, NOT ONE a loop preheader.  So the only
 *      in-tree spelling a compiler demonstrably produces needs a preceding CALL
 *      - s17's R4 refusal leg - which is not C-reachable here without changing
 *      semantics.  The census confirms E-s17-1's R4 classification; it does not
 *      open an eighth path.
 *
 *  (3) A KENGO TRANSPLANT IS STRUCTURALLY IMPOSSIBLE - FOR EVERY BB2 FUNCTION.
 *      Kengo/ ships the retail PS2 disc plus debug SYMBOLS only (name + size +
 *      source path).  There is no Kengo C source in the tree, so that channel can
 *      supply names and module attribution and never a source shape.  No symbol
 *      in the plausible modules (is_coli / hi_landhit / nm_cpu, where this
 *      function's ings.c neighbours map) reads as a pair/link registration.
 *
 *  (4) CELL X1 - target's OWN exit-edge spelling is 4.  Writing both of target's
 *      loop-1 exit-edge instructions (`lw $a0,0xC($s2)` / `sll $a1,$s4,6`) as
 *      `p = *(u8 **)(ctx + 0xC); sh = slot_a << 6;` with sh2 deleted scores 4 at
 *      126/127 - loop 1 one insn SHORT, the E-s16-2 cse-canonicalisation
 *      signature.  Equal to s12's symmetric chassis, which isolates a variable
 *      s12 could not: THE SHIFT'S PLACEMENT IS INERT, and the symmetric chassis's
 *      +1 is entirely its tail read.
 */
/* [s19 SYNTHESIS ADDENDUM - body unchanged, still 3, re-measured at dispatch on a
 * clean tree (127/127, rules_dropped 2, cheat_asm_stripped 49).]  s19's job was to
 * merge nineteen sessions into one model and reset the frontier.  The model is a
 * TWO-CLAUSE THEOREM (evidence.md E-s19-1) that subsumes all 133 banked rejected
 * forms:
 *
 *   CLAUSE A (POSITION).  A preheader reg-reg copy is emitted BEFORE the base add
 *     only when it is a cse fold of a redundant memory read at the addend's own
 *     read point.  A copy written as a C statement, or hoisted by loop.c's
 *     move_movables, is emitted where its consumer's operand becomes available -
 *     and the only preheader value a loop body can legally consume here is `base`,
 *     because cse rewrites every addend-based body address back to `base + i`.
 *     So every body-anchored copy is a copy of BASE, one slot too late.
 *   CLAUSE B (SURVIVAL).  The copy's destination needs a consumer OUTSIDE the
 *     preheader block, and that consumer always materialises as one instruction.
 *
 * Loop 1 pays both clauses: A via the redundant `q = *(u8 **)(ctx + 0xC);` read
 * that cse folds, B via `p = q;` in the exit tail, whose instruction lands where
 * target has `lw $a0,0xC($s2)` - so loop 1 nets ONE mismatch instead of two.
 * Loop 2 fails A STRUCTURALLY (its preheader is downstream of the join
 * .L8001791C that loop 1's `blez` skip branch creates, so cse's extended basic
 * block starts there and can never fold the redundant read - that unfolded read
 * IS residual instruction #2) and has no free clause-B consumer (post-loop math
 * args 19, rec_a 32, rec_a+rec_b 52, in-body 5..16).
 *
 * s19's five new cells closed the last untried clause-A carrier - a body use of
 * the ADDEND that varies with `i`, so loop.c cannot hoist it: Z1 (loop 1) = 6,
 * Z2 (loop 2) = 6, Z3 (both) = 9, Z4 (invariant limit read through the addend)
 * = 5, Z5 (named addend alone, control) = 3.  Z2's preheader emits
 * `addu $a0,$a1,$v0 / addu $a1,$a0,$zero` - the copy is created and DOES survive
 * combine, but it is a copy of `base` and sits one slot after the base add,
 * reproducing s15's C1 from a structurally different construct.
 *
 * EXACTLY THREE EXITS REMAIN, and two are measured dead: (a) kill the join so
 * cse reaches loop 2's preheader (only known attempt: duplicating loop 2 into
 * both guard arms = 35, jump2 does not re-merge); (b) find a clause-B consumer
 * that lands on an instruction target already has - the single candidate site is
 * the post-loop-2 `lw $a1,0xC($s2)` that target uses for BOTH math_Distance3D
 * arguments, measured at 19 by s9 but on the pre-V1 chassis and never swept on
 * V1; (c) a combine refusal on an in-block single use - all seven can_combine_p
 * paths enumerated and dead (s17).
 */
/* [s20 STRUCTURAL ADDENDUM - body unchanged, still 3, re-measured at dispatch on
 * a clean tree (127/127, rules_dropped 2, cheat_asm_stripped 49).]  s20 spent 16
 * cells and one instrumented -da dump run closing BOTH of s19's remaining exits,
 * and in doing so replaced the s19 two-clause theorem with a sharper one that is
 * now observed directly in RTL rather than inferred.
 *
 *  (1) s19's CLAUSE A IS REFUTED AS STATED (E-s20-3).  s19 held that loop 2's
 *      preheader can never be cse-folded because it sits downstream of the join
 *      that loop 1's `blez` creates.  False: the join label opens a NEW extended
 *      basic block, but loop 2's guard block AND its fall-through preheader are
 *      both INSIDE it.  Cell U1 puts a `p = *(u8 **)(ctx + 0xC);` statement in the
 *      join block and the .cse dump shows loop 2's preheader read (insn 162)
 *      eliminated.  Target's own asm is this shape - `lw a0,0xC($s2)` at T.txt:52
 *      is AFTER the join label, in the guard block, not in loop 1's exit tail.
 *      s19's exit (a) needs no CFG enumeration; it is closed by construction.
 *
 *  (2) THE COPY-MATERIALISATION PREDICATE (E-s20-1, CONFIRMED from the dump).
 *      Nine sessions worked from "cse folds the redundant read to a reg-reg
 *      copy".  Half right.  U1's .jump insn 83 is `reg80 = mem(reg72+12)`; in
 *      .cse insn 83 is GONE - deleted outright, not turned into a copy - and the
 *      base add insn 89 has been rewritten to read reg79, the ORIGINAL.  cse
 *      substitutes the canonical pseudo at every use it can rewrite, which is
 *      every use inside the EBB it is processing.  A copy insn appears ONLY when
 *      the destination has a use in a LATER EBB, which cse cannot rewrite.  So
 *      clause B is not combine-survival insurance - it is the sole reason the
 *      copy exists, and it always costs exactly one instruction.
 *
 *  (3) THE GUARD'S SELF-CLOBBERING TWO-STEP IS LOAD-BEARING (E-s20-2, CONFIRMED).
 *      Loop 1's guard below is `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);` - the
 *      same local receives the address and the loaded value, so the address
 *      pseudo is dead and cse cannot reuse it for the base add.  Loop 2's guard
 *      is inline, GCC gives the load a fresh destination, the address survives,
 *      and cse MERGES the base add into the guard add (U1 .cse: insns 162 and 168
 *      both gone).  That is the mechanism behind s15's bare "A1..A4 = 8".  The
 *      two-step is free for loop 2 as well: cell W1 (this body + a loop-2 guard
 *      two-step through a fresh local `t2`) scores exactly 3 at 127/127 and is
 *      the first loop-2 guard spelling that is structurally symmetric with loop
 *      1's rather than merely tied.  Banked as rejected/s20_t2_clobber_guard_
 *      two_step_inert_3.c; a future session should build on it, not on the inline
 *      guard, because it satisfies half the predicate for free.
 *
 *  (4) EXIT (b) IS CLOSED WITH SIX MEASUREMENTS ON THE V1 CHASSIS (E-s20-5).
 *      s19 predicted the post-loop-2 `lw a1,0xC($s2)` that feeds BOTH
 *      math_Distance3D arguments would be a free clause-B consumer.  It is not:
 *      consuming it from a live local DELETES that load, so the purchase is +1
 *      move / -1 load and the move lands in loop 2's exit block where target
 *      emits nothing.  S3 (named `r2` carried to both math args, `r2 = p`
 *      pre-init) = 7 at 127/127; T1 = 11, T2 = 26, T3 = 21, W2 = 21 (the move is
 *      coalesced away entirely, 124 insns), W3 = 14.
 *
 *  (5) THE JOIN-BLOCK FAMILY IS 10-37 (E-s20-4), twelve cells, all banked.  The
 *      sharpest is U7 (join-block read + BOTH guards two-step): 125 insns -
 *      exactly two SHORT, both copies missing and nothing else wrong.  That is
 *      the cleanest existing statement that the two copies are the entire
 *      residual.
 *
 * THE s20 RESTATEMENT.  A preheader `addu DST,SRC,$zero` exists iff (i) the
 * folded read's destination has a use in a LATER EBB (materialisation, +1 insn),
 * and (ii) the guard is written self-clobbering so the base add survives (free).
 * (ii) is now available for both loops.  (i) costs one instruction wherever it is
 * paid; loop 1 pays it into a slot target also fills, loop 2 has no such slot.
 * Target satisfies (i) with no visible consumer anywhere in its listing - that
 * single fact is the whole remaining mystery, and s20 narrows it from "a pass
 * after combine creates the copy" (s15) / "a combine refusal" (s17) to "cse
 * declined to substitute reg79 at the base add", i.e. E-s16-2's canonicalisation
 * question with the cse-side predicate finally named.
 */
/* [s21 STRUCTURAL ADDENDUM — body unchanged, still 3, re-measured at the END of
 * the session on a clean tree (127/127, rules_dropped 2, cheat_asm_stripped 49).
 * The W1 chassis (this body + loop 2's guard written as loop 1's self-clobbering
 * two-step) was also re-measured at 3 / 127 at dispatch.]  s21 spent 19 cells
 * closing ALL THREE of s20's frontier items and, in the process, found the first
 * change in the residual's SHAPE since s9.
 *
 *  (1) s20 FRONTIER #1 IS DEAD (E-s21-1).  A later-EBB use of loop 2's named
 *      preheader ADDEND does not materialise a copy of the addend; it
 *      materialises a copy of BASE, one slot AFTER the base add — even on the W1
 *      chassis, whose whole point was that E-s20-2 keeps the base add alive for
 *      free.  A1 (i-varying element read) = 6, A2 (invariant limit read) = 5, A3
 *      (pointer-first element) = 10, A5 (pointer-first base add) = 6, all at 128
 *      insns; A4 (both body uses via the addend, so `base` dies) is inert at 3.
 *      A1/A2 emit `lw v0,12(s2) / lw a2,16(s2) / addu a0,a1,v0 / addu a1,a0,zero`
 *      against target's `addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3`.  The
 *      reason is structural: loop.c's strength reduction and cse2 re-express any
 *      body address as `base + constant` before a copy is placed, so the value
 *      that needs a later-EBB carrier is always `base`.  Body uses can never be
 *      target's clause-B consumer, on any chassis.
 *
 *  (2) s20 FRONTIER #3 IS DEAD (E-s21-5).  Cell C1 — a post-loop
 *      `end = (u8 *)((s32)q2 + sh2 + i);` whose only consumer is the math-arg base
 *      derivation, aimed at folding the clause-B purchase onto the `addu v0,a0,v1`
 *      target already emits at loop 2's exit — scores 20 at 127.  GCC rebuilds the
 *      address instead of folding it onto the induction-variable update.
 *
 *  (3) THE ONE POSITIVE RESULT, AND THE CHASSIS THE NEXT SESSION SHOULD BUILD ON
 *      (E-s21-2).  s15 (4) ordered "loop 2's base addend MUST stay a fresh
 *      *(u8 **)(ctx + 0xC) read — do not re-probe that family", on the strength of
 *      cells A1..A4 = 8 and a stated mechanism: cse merges the guard-address add
 *      into the base add when the two addends are provably equal.  E-s20-2 shows
 *      the self-clobbering guard two-step is exactly what prevents that merge, so
 *      s15's prohibition was chassis-relative and is now retracted.  On W1:
 *        D1 = loop 2's base addend reuses the carried `p`  ->  5 at **126 insns**
 *        D3 = D1 + the back-edge limit read taken via `p`  ->  4 at **127 insns**
 *        D2 (element via p) 7, D4/D5 (association variants) 4/4, D7 (named links
 *        local) 11.
 *      D1 is ONE instruction short and the missing instruction is precisely loop
 *      2's preheader copy: the fresh `lw v0,12(s2)` that THIS body emits in that
 *      slot is gone entirely.  D3's preheader is `addu a0,a1,a0 / addu a1,a0,zero`
 *      against target's `addu a3,a0,zero / addu a0,a1,a3` — the right two
 *      instruction KINDS, in the wrong order, with the wrong operands.  That is a
 *      strictly better structural description of the residual than this body's
 *      (which still has a LOAD where target has a COPY), at a cost of one point.
 *      Banked as rejected/s21_reuse_p_plus_body_limit_use_costs_4_127insns.c and
 *      rejected/s21_two_step_guard_l2_base_reuses_p_costs_5_126insns.c.
 *
 *  (4) TWO MORE CLAUSE-B FAMILIES CLOSED.  Post-loop consumers on the reuse-p
 *      chassis are as dead as they are on V1 — E1 21 / E2 31 / E3 20 / E4 15, at
 *      123 / 126 / 124 / 121 insns; every one DELETES reloads target keeps, so the
 *      purchase is net negative (E-s21-3).  And making loop 2's GUARD the free
 *      clause-B consumer for loop 1 (B1, B2) is 12 at **125 insns** — two short,
 *      BOTH copies gone — because in every spelling the guard consumes the copy's
 *      SOURCE, not its DESTINATION, so E-s20-1's predicate never fires (E-s21-4).
 *      The predicate's direction is now pinned: only a later-EBB use of the
 *      DESTINATION creates a copy.
 */
/* [s23 FORENSICS ADDENDUM — body unchanged, still 3, re-measured at dispatch on a
 * clean tree (V1 = 3 at 127/127, W1 = 3 at 127/127, D3 = 4 at 127/127;
 * rules_dropped 2, cheat_asm_stripped 49).]  s23 built the loop-2 preheader copy
 * — the thing fourteen sessions could not produce — and then killed the family
 * that produces it, at RTL, with the register allocator's own numbers.
 *
 *  (1) THE CONSTRUCT THAT WORKS (E-s23-1).  Reuse the carried pointer local to
 *      hold the loop's LINKS pointer, which sets the copy's source between the
 *      copy and the base add:
 *
 *          if (i < t) {
 *              q = p;                       // the copy
 *              p = *(u8 **)(ctx + 0x10);    // sets the copy's SOURCE
 *              base = (u8 *)(sh + (s32)q);  // combine hits use_crosses_set_p
 *              do { ... (s32)p ... } while (i < *(s32 *)(base + 0x1C));
 *              p = *(u8 **)(ctx + 0xC);     // p is clobbered -> honest re-read
 *          }
 *
 *      Applied to BOTH loops (cell Q10) this emits target's ENTIRE instruction
 *      stream at 127/127: both preheaders as `copy / links load / base add
 *      reading the copy`, and loop 1's exit tail as target's `lw a0,0xC(s2)`
 *      instead of our `move`.  There is no instruction-kind, count or ordering
 *      difference left anywhere in the function.  Score 14 — every point of it
 *      register identity (ours pointer=$a1, shift=$a0, copy=$v0, links=$a1;
 *      target $a0/$a1/$a3/$a2).
 *
 *  (2) WHY IT CANNOT REACH 0 (E-s23-2, the kill).  The mechanism REQUIRES the
 *      set of the copy's source, and the only value target computes at that
 *      point is the links pointer — so the carried pointer and links must be the
 *      same C local, hence one pseudo, hence one hard register.  Target's
 *      allocation proves they are two: the pointer is $a0 and DIES at the base
 *      add (so `base` reuses $a0), while links lives in $a2 through the body.
 *      With `p` carrying links it is live through the body, conflicts with
 *      `base`, and `base` can never inherit its register.  S1's `.greg` shows it
 *      directly (`78 in 4  80 in 5  81 in 5`).  21 tuning cells confirm the
 *      family is flat at 12-16.  Do not re-open it.
 *
 *  (3) THE TRANSFERABLE TOOL (E-s23-3).  global.c's `allocno_compare` priority is
 *      floor_log2(n_refs) * n_refs / live_length, and BOTH inputs are printed in
 *      `.lreg`'s per-register header.  Predicting from those numbers that
 *      splitting `base` into base1/base2 (10/22 -> 5/11) would drop it below `p`
 *      and hand `p` the $a0 it lacks was CORRECT on the first try (cell S1).
 *      Hard-register identity on this compiler is therefore steerable and cheap
 *      to measure — split a variable to lower its priority, fuse uses to raise
 *      it — and that is the right instrument for any register-identity residual,
 *      here or elsewhere in the queue.
 *
 *  WHAT THIS LEAVES.  V1 (this body) is still the floor at 3.  The residual's
 *  description is now sharper than at any point in the ledger: there exists a
 *  pure-C construct that emits target's exact instructions (Q10) and a different
 *  pure-C construct that emits target's exact registers for 124 of 127
 *  instructions (this body).  Neither can be both, because Q10's mechanism costs
 *  the pointer/links pseudo split and V1's mechanism costs the loop-2 copy.  The
 *  next probe should look for a THIRD producer of a surviving preheader copy
 *  whose blocking set is NOT the copy's source — i.e. a combine refusal that does
 *  not need `p` to be re-defined — since that is the single assumption both
 *  measured families share.
 */
/* [s24 ESCALATION/DISPOSITION ADDENDUM — body unchanged, still 3, re-measured
 * twice this session on a clean tree (127/127, rules_dropped 2,
 * cheat_asm_stripped 49).]  s24 closed s23's frontier and disposed the function.
 *
 *  (1) s23 FRONTIER #2 IS DEAD (E-s24-1).  Giving the links value its own local
 *      consumed inside the preheader — the proposed escape from E-s23-2's
 *      pointer/links pseudo fusion — does not work: F2 = 8 and F3 = 9, both at
 *      **126** insns, i.e. ONE SHORT, with the preheader copy gone entirely.
 *      Copy propagation folds `lnk = p` straight back into one pseudo, so the
 *      SET that `use_crosses_set_p` needs no longer sits between the copy and the
 *      base add, and combine substitutes and deletes the copy.
 *
 *  (2) s23 FRONTIER #1 IS ANSWERED, NEGATIVELY (E-s24-2).  The family admits
 *      exactly two positions and both are measured: the pointer either carries
 *      links into the loop body (one pseudo, conflicts with `base`, 12-16 across
 *      23 cells) or the value is copied out so it dies early (copy-prop refolds,
 *      the copy dies, 8-9 at 126).  There is no third position.  With s17's
 *      seven-path can_combine_p enumeration and E-s20-1's cse-side predicate
 *      (whose every clause-B consumer site for loop 2 is priced dead), no pure-C
 *      producer of a surviving loop-2 preheader copy remains on this chassis.
 *
 *  (3) s23 FRONTIER #3 IS RETIRED BY CONSTRUCTION (E-s24-3).  The allocno-priority
 *      instrument acts on hard-register identity; V1's residual is two
 *      instruction-KIND differences.  The instrument is valid and should be
 *      carried to the next queue item with a register-identity-only residual — it
 *      has no purchase here.
 *
 *  DISPOSITION.  Both endgame-lock gates fail — scan_hand_coded tier=LOW 0/8
 *  (re-run this session) and no in-hand SOTN-master precedent for the closing
 *  construct — so the owner's standing ruling (2026-07-27) applies and the
 *  function is disposed REFUSED / OWNER-ACCEPTED INCOMPLETE.  See
 *  docs/grind/decisions.md, the 2026-08-18 func_80017848 entry and its s24
 *  addendum.  This body (floor 3 at 127/127) is the preserved best pure-C form.
 */

/* [s25 ESCALATION/DISPOSITION ADDENDUM - body unchanged, still 3, re-measured on
 * the POST-MIGRATION chassis (2026-08-25).]
 *
 * CHASSIS: func_80017848 no longer carries any asmfix rule.  The 2026-08-19
 * asm-until-matched migration retired both (migration_pin.json rules_retired 2),
 * and src/ings.c:590 is now `INCLUDE_ASM("asm/funcs", func_80017848);`.  Splicing
 * the body below (candidate.c:325-403) over that line and running
 * `sandbox func_80017848 --disable all` gives score 3, 127/127, scorable,
 * rules_dropped 0.  Identical floor, honest representation, zero debt.
 *
 * COMPILER FIDELITY IS ELIMINATED (H-s25-A).  The ORIGINAL PsyQ cc1psx.exe
 * (GCC 2.7.2.SN.1, run diagnostically under dosemu2 - never a build path) was
 * given the SAME preprocessed translation unit as tools/gcc-2.7.2/cc1.  Both
 * emit 111 pre-maspsx instructions for this function and the streams are
 * identical except for label spelling and one adjacent-pair scheduling swap at
 * the math_Distance3D preamble (port `sll $16,$20,6 / lw $5,12($18)`; cc1psx the
 * reverse).  All THREE residual instructions are byte-identical between the two
 * compilers, and at the one divergent site TARGET agrees with the PORT.  So the
 * residual is a genuine C-source-structure difference; the pure-C match exists in
 * the C, and a future session must not spend a turn wondering about the compiler.
 *
 * SOLVER SUITE (H-s25-B, executes the queue item's owner directive).
 * `inverse_compose.py classify ings func_80017848` reports "FIRST DIVERGENCE:
 * IDENTICAL" - a FALSE verdict, because the function is not
 * `replace_with_asmfile`-wired so ings.tgt.s is built from this same C body.  The
 * model-side answer is s24's: the residual is two instruction KINDS plus one
 * operand, outside both solvers by construction.
 *
 * TARGET'S SYMMETRY, stated once for the record: loop 1 (0x800178B4) and loop 2
 * (0x80017914) have the SAME nine-instruction preheader shape - `lw $a0,0xC($s2)
 * / sll $a1,$s4,6 / addu $v0,$a1,$a0 / lw $v0,<0x1C|0x20>($v0) / blez / addu
 * $v1,$zero,$zero / addu $a3,$a0,$zero / lw $a2,0x10($s2) / addu $a0,$a1,$a3` -
 * and loop 1's skip branch lands at 0x8001791C, PAST loop 2's re-read/re-shift
 * pair.  Both of target's copies therefore exist with no second use anywhere,
 * while this candidate buys loop 1's copy with `p = q` and pays for it in loop
 * 1's exit tail.  That asymmetry IS the remaining 3.
 */
/* [s26 STRUCTURAL ADDENDUM - body unchanged, still 3, re-measured on a clean tree
 * (127/127, rules_dropped 0, cheat_asm_stripped 4).  Executes the owner's
 * 2026-08-30 escalation-batch ruling 10 ("ACTIVE with modality change").
 *
 *  (1) THE RESIDUAL RESTATED FROM THE TARGET LISTING.  Target's TWO preheaders
 *      are the SAME register-identical 10-instruction block (asm/funcs lines
 *      31-41 and 56-67): lw $a0,0xC($s2) / sll $a1,$s4,6 / addu $v0,$a1,$a0 /
 *      lw guard / blez / addu $v1,$zero,$zero / addu $a3,$a0,$zero /
 *      lw $a2,0x10($s2) / addu $a0,$a1,$a3 / addu $v0,$a0,$v1.  Loop 2's lw/sll
 *      pair sits on loop 1's TAKEN edge, BEFORE the join label, so on the skip
 *      path $a0/$a1 are simply carried.  On this chassis the 3-point residual is
 *      a POSITION SWAP of one `lw` and one `move` between loop 1's exit tail and
 *      loop 2's preheader - the two builds carry the SAME instruction multiset.
 *
 *  (2) CELL A - target's join shape, reproduced instruction-for-instruction, at
 *      4.  Replace the `p = q;` tail below with `p = *(u8 **)(ctx + 0xC);
 *      sh = slot_a << 6;` (that order; reversed = 6) and delete `sh2`, letting
 *      loop 2's guard and base both read the recomputed `sh`: 127 target / 126
 *      build, and the join region is byte-exact for the first time.  The whole
 *      residual on chassis A is the two missing copies - loop 1's redundant read
 *      is cse-folded to a copy and then deleted by combine (dest used in-block),
 *      loop 2's is not folded at all because cse's EBB begins at the join.  It is
 *      a better DESCRIPTION of the wall than s12's symmetric chassis and a worse
 *      score than this body, which buys loop 1's copy with the second-use lever.
 *
 *  (3) TWO KILLS.  A dedicated PRE-JOIN carrier for loop 2's addend (set on both
 *      predecessor edges so cse can never equate it with `p`, dodging the s15(4)
 *      guard/base merge) is 12 as a source copy AND as a fresh read - it costs one
 *      materialisation per predecessor path.  And an in-BODY second use of loop
 *      2's addend - the one out-of-block site never tried, loop 2's own do-while
 *      condition - is 6: loop.c's move_movables hoists the DERIVED invariant
 *      `sh + q`, not the addend, so the surviving copy is `move a1,a0`, a copy of
 *      the BASE one slot after the base add (s15's cell C1 failure mode again).
 *      CONSEQUENCE: loop 2 has NO free out-of-block use site anywhere.  Post-loop
 *      sites are 19-22 (s10/s11), the loop body is 6, a pre-join carrier is 12.
 *      The second-use lever that buys loop 1's copy is unbuyable for loop 2.
 */
/* [s27 ESCALATION ADDENDUM — body unchanged, re-measured this session at 3
 * (127 target insns / 127 build insns, scorable) on the post-migration chassis.]
 *
 * s27 executed the owner's 2026-09-01 FORECLOSED-BUCKET REVIEW Ruling-A named probe
 * verbatim — the sole ground on which this function was returned to active:
 *
 *   (1) GUARD-ONLY DUPLICATION, the last untried cse-boundary lever, is DEAD at three
 *       spellings: G1 (guard duplicated into the loop-1 taken path, shared preheader +
 *       body) = 18 at 134 insns; G2 (guard + preheader duplicated, shared do-while body)
 *       = 44 at 133; G3 (same, duplicated into the SKIP path) = 47 at 136.  jump2 does
 *       not cross-jump the duplicated tails back together, so every spelling is +6..+9
 *       instructions against a 127-insn target.
 *
 *   (2) THE .cse DUMP CONFIRMS the EBB-boundary attribution that was inference until
 *       now (dumps/ings.cse:4270-4915 on chassis A): loop 1's redundant read is folded
 *       by substitution against its own block's load; loop 2's stays a real `lw` because
 *       `code_label 145` — the join loop 1's guard branches to — ends cse's block scan
 *       (cse.c:8038).
 *
 *   (3) AND THE SAME DUMP RETIRES cse AS AN EXPLANATION OF TARGET.  Target's own CFG
 *       carries that identical join (`blez $v0, .L8001791C` at 0x800178C8, with the
 *       loop-1 exit-tail `lw $a0, 0xC($s2)` at 0x80017914 BEFORE the label), so cse's
 *       window excluded the load in the original compilation too.  cse therefore cannot
 *       have produced target's `addu $a3, $a0, $zero` at 0x80017930.  Any future
 *       proposal must name a producer that works ACROSS a join label at zero instruction
 *       cost.
 *
 *   (4) Four corollary "load inside loop 2's guard block" spellings (J/J2/L/L2 on
 *       chassis A) measured 14 / 14 / 13 / 21 at 125/125/126/125 insns — cse substitutes
 *       instead of leaving a copy, and the out-of-block second use that preserves one is
 *       priced exactly where the banked table already put it.
 *
 * Endgame-lock gates re-run: (a) scan_hand_coded = LOW 0/8; (b) no closing construct
 * exists to seek a precedent for (no C form at distance 0 in 27 sessions).  Disposed
 * under the owner's standing ruling (2026-07-27) — see docs/grind/decisions.md.
 */
/* [s28 FORENSICS ADDENDUM - body unchanged, still 3, re-measured 127/127 this
 * session with this file applied over the src/ings.c:719 INCLUDE_ASM anchor.]
 * s27 left a toolchain question rather than a C probe: which GCC 2.7.2 pass can
 * emit a (set (reg) (reg)) that is not in the incoming RTL, across a join label?
 * s28 answered it with a complete emit_move_insn/gen_move_insn call-site census.
 * regmove.c and optimize_reg_copy_3 DO NOT EXIST in this compiler; local-alloc.c,
 * combine.c, global.c, reload.c, reorg.c, sched.c and caller-save.c have ZERO
 * move-emitting sites. The one new producer found - cse_set_around_loop, emitter
 * at cse.c:7969 - plants its copy in the PREHEADER and is not foreclosed by a
 * join, but its REG_LOOP_TEST_P gate (cse.c:7933) is opened only by jump.c:2253
 * inside duplicate_loop_exit_test, which jump.c:626 runs only when a
 * NOTE_INSN_LOOP_BEG is immediately followed by a simplejump - i.e. only for a
 * source-level `while`/`for` loop, never for the pre-rotated `if + do/while`
 * shape this body uses. Rewriting the loops as `while` opens that gate (the
 * .jump dump shows NOTE_INSN_LOOP_VTOP for both loops) but emits no copy and
 * lets LICM hoist the loop bound out, which target does not do: 29 / 19 / 21 at
 * 120 / 123 / 123 insns. Do NOT re-derive the while-shape. See evidence.md
 * E-s28-1..E-s28-5.
 */
/* [s30 REDERIVE ADDENDUM — body unchanged, still 3, re-measured 127/127 this
 * session as cell BASE.]  s30 closed the rederive modality's three legs.
 * (1) INLINE HELPERS: integrate.c — the last of the seven reg-reg-copy producers
 * in the s28 census never exercised by this function — was built three ways
 * (static inline helper carrying the loop body / the base computation / the guard
 * too) and scores 42/45/30 at 136-137 build insns.  GCC 2.7.2 -O2 really does
 * inline them (no `jal` in the stream), but no arg copy survives into the
 * preheader, and each callee costs ~4 insns of return-value materialisation
 * (`j <inline-return>` + `addiu v0,zero,1` + a caller test) where target has one
 * `beq $v0,$s3` into the shared return-0 block.
 * (2) STRUCT TYPING: the real 0x40-byte record / 0x10-byte link types priced at
 * 34/7/14/7 — a uniform +4 over the equivalent u8* spelling; the fully indexed
 * form loses 4 instructions to cse folding reads the target keeps.
 * (3) SIBLING TRANSPLANT: the (index<<4)+links+field idiom exists nowhere else in
 * ings.c/ings2.c; obj_CalcOffset (src/ings.c:693) has the same geometry, is
 * matched, and is NOT called from here — it does confirm the author's idiom is
 * explicit shift arithmetic over a byte base, which this body already uses.
 * Two further inputs were proved inert on BOTH chassis: declaration block-scope
 * (cell I = 3, cell J = 14) and the source order of the two preheader definitions
 * (cell K = 14).  The cell-E four-seat permutation therefore does not move for
 * declaration scope or write order, which sharpens the standing frontier: the
 * perturbation that reaches target's seats has to come from inside the modelled
 * local-alloc/global-alloc inputs, not from source presentation.
 */
/* [s31 REDERIVE ADDENDUM - body unchanged, re-measured 3 at 127/127.]
 * The residual is now named instruction-for-instruction: (1) loop-1 exit tail,
 * target `lw a0,12(s2)` vs ours `addu a0,a3,zero`; (2) loop-2 preheader, target
 * `addu a3,a0,zero` vs ours `lw v0,12(s2)`; (3) loop-2 base add, target
 * `addu a0,a1,a3` vs ours `addu a0,a1,v0`.
 *
 * TWO CORRECTIONS TO THE s9 HEADER ABOVE, both measured this session.
 *  (a) The "cse leaves an ORPHANED copy that combine never sees" story is not
 *      sufficient on its own.  s31 cell N1 keeps `q = *(u8 **)(ctx + 0xC);` in
 *      loop 1 (so the cse fold still happens) but drops the downstream `p = q;`
 *      and the copy is DELETED.  Loop 1's copy here is bought by escape 1 - a
 *      live second use of q - and by nothing else.
 *  (b) Target cannot be using escape 1: its $a3 is written once and read once in
 *      each loop and nowhere else, and its loop-1 exit tail RELOADS the pointer
 *      rather than copying $a3.  So this body matches loop 1 for the wrong
 *      reason, and the open question is what makes a USE-ONCE copy survive
 *      combine.
 *
 * Escape 2 is closed as a route to the target seats: the ra_solver goal
 * derivation on the exact-instruction-stream form (cell E) returns an EMPTY goal
 * because that form merges the record pointer and the link pointer into one
 * variable (one pseudo, one hard register) while target seats them at $a0 and
 * $a2.  See tmp/grind/func_80017848/s31/goal_E_report.txt.
 */
/* [s32 STRUCTURAL ADDENDUM - body unchanged, still 3, re-measured this session
 * as cell BASE on a clean tree (127/127).]  s32 answered the s31 frontier's
 * first question with dumps instead of inference and found two previously
 * unused combine escapes; neither beats this body, but the wall is now
 * described mechanically rather than statistically.
 *   - PASS ATTRIBUTION SETTLED.  A use-once reg-reg copy is deleted by COMBINE,
 *     not by jump.c / flow.c / reload.  Cell R's copy is insn 162 in ings.cse2
 *     and absent from ings.combine; loop 1's copy (insn 83) survives there only
 *     because this body's `p = q;` tail gives it a second use.
 *   - ESCAPE #8 (new): flow.c:2102 builds a LOG_LINK only when the next use is
 *     in the SAME basic block, so a copy defined in the guard block and used in
 *     the preheader block is invisible to combine and survives with ONE use at
 *     zero cost.  First use-once surviving copy ever measured here (cell S).
 *     Dead for position: the copy must sit before the `blez`, and with the
 *     guard's `sh + p` still available cse eats the preheader's base add.
 *   - ESCAPE #9: combine.c:914's `use_crosses_set_p` refusal, bought for free by
 *     writing the links read as `p = *(u8 **)(ctx + 0x10);` (variable reuse), so
 *     the insn between the copy and the base add sets the copy's source.  This
 *     is s23's construct re-derived from the combine source.  Cell AK
 *     (rejected/s32_both_loops_p_reused_as_links_inline_l2_guard_costs_14.c)
 *     builds 127/127 INSTRUCTION-FOR-INSTRUCTION IDENTICAL to target across the
 *     whole function - both preheader copies, both exit tails - with nothing
 *     left but the register permutation a0<->a1 (p/sh), a3->v0 (q), a2->a1
 *     (links).  It stays dead for s31's reason: the escape merges the record
 *     and links pointers into one pseudo, so target's a0/a2 split is not an
 *     allocation this RTL admits.
 *   - cse.c:826 make_regs_eqv is the C-level handle on WHICH register a folded
 *     copy leaves behind (evidence.md E-s32-3); it is what promotes a copy
 *     destination first mentioned before the cse block start to canonical, and
 *     outside cells S/Z it is still unexploited.
 */
/* [s34 SYNTHESIS ADDENDUM - body unchanged, re-measured 3 at 127/127.]
 * s34 re-localized the residual.  Normalising the target listing shows BOTH loops
 * open with the SAME nine instructions (lw a0,0xC(s2) / sll a1,s4,6 / addu v0,a1,a0 /
 * lw v0,GUARD(v0) / blez / addu v1,zero,zero / addu a3,a0,zero / lw a2,0x10(s2) /
 * addu a0,a1,a3), so what this ledger has called "loop 1's exit tail" is really loop
 * 2's record-pointer read AT THE JOIN, outside loop 1's if-block.  Moving it there
 * (the JOIN-SHAPE chassis, banked as candidate_alt_join_shape_6.c, score 6 at 127/125)
 * makes the C block-for-block identical to target and, per the fresh -da dumps,
 * makes cse produce BOTH of target's preheader copies: cse.c:826 make_regs_eqv
 * promotes the copy destination to canonical, the base add reads it (ings.cse2
 * insns 83/89), and the copy is no longer trivially dead.  COMBINE then deletes
 * both (ings.combine insn 89 reads reg79 with REG_DEAD).  So this candidate's
 * header account above - "cse folds the redundant load into a copy combine never
 * sees" - is superseded twice over: combine always sees it, and this body's loop-1
 * copy is bought solely by the `p = q;` second use (escape 1), which is exactly the
 * instruction target spends on `lw a0,0xC(s2)` instead.  The whole remaining gap is
 * one question asked twice: how a use-once reg-reg copy survives combine at zero
 * instruction cost.  s34 read can_combine_p (combine.c:880-928) against target's
 * three-insn preheader and found use_crosses_set_p the only zero-cost refusal it
 * admits; firing it merges the record pointer and the links pointer into one pseudo,
 * which target seats in two different hard registers (a0/v1 vs a2), measured at
 * 14 (Q1) and 15 (Q2) on the join chassis and 10 (K1) / 14 (AK) elsewhere.
 */
/* [s37 SOLVER ADDENDUM - body unchanged, still 3 at 127/127, re-measured on the
 * drifted chassis (main landed in ings.c; anchor now src/ings.c:820).]
 * The owner's 2026-09-06 directive was executed: the preheader copy geometry
 * (set b a)/(set c mem)/(set d (plus e b)) was built as standalone scratch TUs
 * under the exact project flags (tmp/grind/func_80017848/s37/mini/). With the
 * copy destination promoted by cse.c:826 the geometry appears verbatim in
 * .cse2 and combine deletes the copy in isolation exactly as it does here
 * (combine.c:1458, i2dest dead in i3). There is no protecting context in this
 * chassis; frontier item 3 is closed as a class kill. The object-level
 * classify reports RA for the residual, but the diff is positional (BASE has
 * the move in loop 1's exit tail and the load in loop 2's preheader, target the
 * reverse), so the residual is not a seat permutation and inverse.py has no
 * goal - agreeing with s31. m2/m3 also show that the `t = sh + p; t = *(t+0x1C)`
 * guard reassignment is load-bearing: without it cse2 folds the preheader add
 * into the guard's add pseudo entirely.
 */
/* [s41 ADDENDUM - body unchanged, re-measured 3 at 127/127 on the HEAD chassis.]
 * s41 (rederive) measured four un-banked shapes, all dead: a same-value `p = q;`
 * re-store after the base add (M1 = 12, M2 = 14; cse deletes it before flow so it is
 * not a reader), for(;;)+break-at-top loops (F1 = 22, no rotation) and goto-into-loop
 * invalid loops (G1 = 43, the guard block vanishes). Reading checks in evidence.md
 * close the global.c preference route to the a3 seat: set_preference only sees hard
 * registers and local-alloc seats, none of which can be a3 here. The residual is
 * still exactly one byte-free flow-time reader of each preheader copy destination.
 */
/* [s46 ESCALATION ADDENDUM - body unchanged, re-measured 3 at 127/127 on the
 * HEAD chassis (tmp/grind/func_80017848/s46/sandbox_base.txt).]  Disposition
 * session: the owner's 2026-09-06 directive (minimal scratch TU of the copy
 * geometry, .cse2/.combine read) was executed at s37 and is closed as a class
 * kill (combine.c:1458; no protecting context exists).  Kill re-audit: the s15
 * hoisted-copy form carries no FAKE construct and re-measures 3 on HEAD.
 * Frontier item 3 measured on scratch TUs: the s32 byte-holder compare reaches
 * RTL but materialises as slt/nor (not byte-free); the u8 holder is front-end
 * folded.  Gate (a) scan LOW 0/8; gate (b) SOTN census negative.  Disposed
 * FORECLOSED under the 2026-07-27 standing ruling - see docs/grind/decisions.md
 * 2026-09-06 func_80017848 entry.
 */
/* [s47 SOLVER ADDENDUM - body unchanged, re-measured 3 at 127/127 on the HEAD
 * chassis (tmp/grind/func_80017848/s47/sb_BASE.txt).]  The owner's sibling-
 * transplant directive is EXECUTED: func_80017D84 confirms the object model and
 * adds that the link base (+0x10) equals the record base (+0xC) plus
 * (count << 6), but spelling loop 2's links that way costs sll+lh+addu (D3 = 33
 * at 128); func_80016E60's do-while(0) wrap on loop 2's preheader is 15 at 128
 * (D1); its pointer-alias named local is byte-inert (D2 = 3), as s11 had it.
 * SOLVER RESULT (new, mechanical): the residual's one seat divergence
 * (`addu a0,a1,v0` vs `addu a0,a1,a3`) belongs to reg 113, a LOCAL allocno
 * (blk 13 qty 0, birth 2 death 6, refs 2) that global.c never models - which is
 * why goal_from_tgt derives an EMPTY goal on this chassis for a reason unrelated
 * to s31's.  inverse.py's LOCAL backend returns a validated NEGATIVE at depth 3
 * and depth 4 over 24 atoms in 6 classes: no perturbation of refs, live span,
 * birth order, conflicts, preferences or calls-crossed reaches $a3.  The only
 * route to $a3 is promoting reg 113 to a GLOBAL allocno live across loop 2 -
 * exactly the precondition the missing `addu a3,a0,zero` copy needs
 * (combine.c:1458).  All three residual instructions are ONE fact, and it is
 * the E-s44-3 byte-free-flow-time-reader wall.  Q1 re-audited at 14; candidate
 * carries no FAKE construct.  Details: evidence.md/hypotheses.md s47 sections.
 */
/* [s48 FORENSICS ADDENDUM - body unchanged, re-measured 3 at 127/127 on the HEAD
 * chassis (anchor src/ings.c:820).]  Two frontier items closed with the
 * instrumented cc1 and a source read of local-alloc.c.
 *   (1) s47 frontier item 3 is answered: reg 113 (blk 13 qty 0) carries NO
 *       suggestion at all (ncopysugg=0, nsugg=0), so local-alloc's
 *       SUGGESTED-REGISTER pass never runs for it, and its single find_free_reg
 *       call scans used=first_used={0,1,26..67} - every GPR 2..25 free.  $a3 is
 *       in the scan order and is not excluded by anything.  It loses because the
 *       MIPS back end defines no REG_ALLOC_ORDER, so local-alloc.c:2249-2255
 *       walks regno ASCENDING from 0 and takes the first free one, $v0.  A
 *       LOCAL-pass $a3 seat therefore needs regnos 2..6 ($v0,$v1,$a0,$a1,$a2)
 *       all live over [2,6) - five hard registers across loop 2's preheader -
 *       and blk 13 holds one quantity and contains no call, so neither
 *       post_mark_life nor an RTL hard-reg reference can supply them.
 *   (2) s47 frontier item 2 is measured: a real second use of the loop-2 addend
 *       (cell A, math_Distance3D's first argument, read unconditionally before
 *       loop 2's guard) DOES promote reg 113 out of local-alloc - blk 13
 *       vanishes from the quantity table - but the promoted allocno takes $v1,
 *       not $a3, and cse forwards it into every later ctx+0xC consumer, deleting
 *       one of target's tail reloads: 17 at 126 vs 127.  Every C-level second
 *       use of that load is SUBSTITUTIVE, not additive, which is why E-s44-3's
 *       byte-free reader is still the requirement - now sharpened to "promote
 *       the value without giving cse an equivalence it can forward to the tail".
 */
/* [s49 FORENSICS ADDENDUM - body unchanged, re-audited at 3 (127/127) on the HEAD
 * chassis (anchor src/ings.c:820).]  Two of s48's three frontier items are closed
 * with dumps and the third is measured:
 *  - Reload NEVER revisits an allocation here: BB2_RELOAD_DEBUG shows zero
 *    new_spill_reg / spill_hard_reg / kickout / retry_global_alloc events for
 *    func_80017848 and `needs pass=1 changed=0` (E-s49-2).  With E-s48-2 that
 *    eliminates all three surfaces s47's inverse.py negative called unmodelled,
 *    so the solver negative for reg 113 -> $a3 is COMPLETE on this chassis.
 *  - The ascending-scan seat model is calibrated inside this function (blk 16:
 *    0/1/1/2 earlier-allocated overlapping qtys -> $v0/$v1/$v1/$a0).  A LOCAL-pass
 *    $a3 seat for reg 113 therefore needs five extra block-local definitions in a
 *    preheader target builds from two instructions: +5 insns, priced dead (E-s49-3).
 *  - Cell C (loop-2 exit test re-reading the count through the named base addend)
 *    = 5 at 127/127 - the FIRST promoting form that is not an instruction short,
 *    and the first with a real surviving preheader copy.  The copy-survival device
 *    is an OUT-OF-BLOCK use of the copy's destination (the same can_combine_p
 *    escape by which this body already buys loop 1's copy via `p = q;`), but it
 *    copies whichever pseudo carries that use - cse folds in-loop uses of the
 *    addend back to `base`, so cell C copies base after the add where target
 *    copies the record pointer before it (E-s49-4).
 */
