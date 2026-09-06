/* s34 (synthesis — 2026-09-06).  BODY UNCHANGED; floor still 3/175/175 (re-measured live on
 * HEAD de9606ac with the two documented byte-neutral caller-side edits; residual rows 62/63/64).
 *
 * s33's frontier item 1 is CLOSED and NEGATIVE.  e3's sixth quantity is `dp` itself (proved by
 * reading tmp/grind/func_800770B8/s34/e3_fn.lreg: insn 137 sets reg86 from the symbol_ref, 145
 * adds t0*4 into it, 151 kills it).  local-alloc.c:472 only grants a block quantity to pseudos
 * with reg_basic_block >= 0 && reg_n_deaths == 1, so giving `dp` a second live range removes it
 * from the qty table — h1/h4 do exactly that, produce EXACTLY the five-quantity table s33
 * predicted would be score 0, and measure 42 and 50, because global-alloc still seats the value
 * in $a2 across the A stores.  A correct blk=1 table is necessary, not sufficient.
 *
 * THE NEW LEAD (no FAKE construct, nothing moved, five quantities): the class-C seat does not
 * need the operand flip at all.  Naming the SECOND D_800A36A0 read in a local taken between the
 * C-group stores and the sb store —
 *      u8 *rb;  ...  rb = D_800A36A0;
 *      s16 *p_6a = (s16 *)(rb + (t0 * 10) + 0x6A);
 *      s16 *p_7e = (s16 *)(rb + (t0 * 10) + 0x7E);
 * — moves the reload's birth 48 -> 42, drops the merged dest+reload quantity's qty_compare_1
 * priority from 3.75 to 2.14 (below the chain's 2.67), so the chain sorts first and takes $v0 and
 * the dest+reload quantity takes $v1.  Target rows 62/63/64 come out BYTE-EXACT.  Score 24
 * (rejected/s34-named-reload-before-sb-classC-rows-exact-but-a0a1-tiebreak-24.c).  Its whole
 * residual is a SECOND qty_compare_1 tie: the inserted reload pushes reg87 (t0) and reg101
 * (base + base+t0) two luids later, making their priorities exactly 1.000 and 1.000, and the
 * index tiebreak swaps $a0/$a1 against the target.  Any body that holds the reload at birth <= 44
 * while leaving reg101's death at 44 (or pushing reg87's to >= 50) is a predicted score-0 body.
 *
 * SECOND LEAD: s2 (13) / s4 (12) — the D group moved ahead of the A group plus the second
 * D_800A36A0 read taken into the EXISTING multi-death `ptr` local.  That gives blk=1 only FOUR
 * quantities with the merged chain+dest born at 12 and seated in $v1 (the target's assignment)
 * and rows 54-64 byte-exact; the residual is entirely the D group's emission position.
 */
/* s33 (structural — 2026-09-06).  BODY UNCHANGED; floor still 3/175/175 (re-measured live
 * on HEAD 35950580 with the two documented byte-neutral caller-side edits).  The class-C
 * seat decision is no longer a hypothesis: it is a closed-form predicate on GCC's own
 * quantity table, read with BB2_QTY_DEBUG=1 on tools/gcc-2.7.2/cc1
 * (tmp/grind/func_800770B8/s33/qty.sh + qtydbg.py — one cc1 run per body, cheaper than a
 * scoring build and the right triage tool for any remaining seat question).
 *
 * In the loop-body block (blk=1) qty_compare_1 (local-alloc.c:1660) ranks quantities by
 * floor_log2(refs)*refs*size/(death-birth) and find_free_reg hands out ascending first-free
 * hard registers in that order:
 *
 *   THIS BODY (3):   merged dest+RELOAD  birth48 death56 refs10  pri 3.75 -> $v0
 *                    chain               birth28 death52 refs16  pri 2.67 -> $v1
 *   f2 = the flip(27): merged dest+CHAIN birth28 death56 refs22  pri 3.14 -> $v0  (WRONG)
 *                    reload              birth48 death52 refs4   pri 2.00 -> $v1
 *   TARGET NEEDS:    merged dest+CHAIN                                    -> $v1
 *                    reload                                              -> $v0
 *
 * so the flip is REQUIRED (it is what merges the dest into the chain) and the merged
 * quantity must additionally be born at <= ~12 instead of 28, which makes its priority
 * 2.00, ties it with the reload and loses it the first-free seat.
 *
 * FOUR BODIES NOW DO EXACTLY THAT at 175 insns (a2, b5, b6, e3 — all 26).  The smallest is
 * e3 = this body + the operand flip + ONE rename: the D_800A35D0 group built through its own
 * `u8 *dp` local instead of borrowing `ptr`.  No store moves, no instruction is added, and
 * its whole residual is a SIXTH quantity (reg86, birth6 death36 refs10) that takes $a2 and
 * pushes reg87/reg101 down one seat.  The lever is the TWO-statement address build
 * (`dp = (u8 *)&D_800A35D0; dp = (t0 * 4) + dp;`) — folding it to one statement scores 38.
 * Banked at rejected/s33-dp-named-two-stmt-target-classC-seats-but-extra-a2-quantity-26.c.
 *
 * Next session: kill e3's sixth quantity (identify reg86 in e3's .lreg and fold it back into
 * an existing quantity) while keeping merged.birth = 12.  That is a score-0 body.
 */
/* s32 (structural, second run — 2026-09-06).  BODY UNCHANGED; floor still 3/175/175.
 * 46 scoring builds + 3 cc1 -da dump runs this session; src/text1b.c restored to the
 * pristine HEAD copy after every sweep.  THE RESIDUAL IS NOW LOCALISED TO A SINGLE GCC
 * DECISION, and for the first time a body exists that emits the whole class-C block
 * BYTE-EXACT.
 *
 * 1. PASS ATTRIBUTION, READ OUT OF THE DUMPS (not inferred).  `pwsh tools/grinder/dump.ps1
 *    func_800770B8` was run on this body (base) and on the operand-flip body (f2, the
 *    int-domain spelling `(s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A)`).  Their `.lreg`
 *    dumps for func_800770B8 differ in EXACTLY ONE insn out of the whole function:
 *      base: (insn 188 (set (reg 110) (plus (reg 109) (reg 108))))   <- reload first
 *      f2:   (insn 188 (set (reg 110) (plus (reg 108) (reg 109))))   <- chain first
 *    (reg 108 = the t0*10 chain, reg 109 = the second `lw %gp_rel(D_800A36A0)` reload.)
 *    Every other insn, and the ENTIRE insn ORDER, is identical.  So sched1 is NOT involved
 *    in the 24 rows of collateral f2 pays — the collateral is entirely local-alloc's
 *    response to that one swap: block_alloc's tying loop (local-alloc.c:1240-1299) now
 *    ties reg 110 to operand 1 = the CHAIN, merging the dest quantity into the chain
 *    quantity instead of the reload quantity, and the resulting qty_compare_1 priority
 *    order reseats the whole loop-body block.  f2's `(plus (reg 108) (reg 109))` IS the
 *    target's RTL; only the hard registers come out swapped ($v0<->$v1 on the chain and
 *    the reload, $a0<->$a1 on t0 and the first load).
 *    Artifacts: tmp/grind/func_800770B8/s32/{base,f2,u4}.lreg / .sched, slice.py.
 *
 * 2. THE SEATS ARE REACHABLE — u4 = 12/175/175, THE FIRST BODY IN 32 SESSIONS THAT EMITS
 *    ROWS 54 THROUGH 64 BYTE-EXACT, class C included.  u4 = f2's operand flip PLUS moving
 *    the D_800A35D0 store group (the 0x2/0x0 clears) ahead of the group-A stores.  Its
 *    entire 11-row residual is the D group's own emission position (target rows 49-53, u4
 *    rows 38-39/43-46); rows 54-64 — `addu $v0,$a0,$v1 / addu $a0,$a0,$a1 /
 *    addu $v1,$v1,$a1 / sh 0x42 / sh 0x40 / sb 0x68 / lw / sll / addu $v1,$v1,$v0 /
 *    addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E` — are the target's, register for register.
 *    Controls: w10 (the D hoist WITHOUT the flip) = 15 and still carries the 3 class-C
 *    rows, so the hoist alone does nothing for the tie; x3 (the same hoist through its own
 *    local) = 12 identically, so `ptr` reuse is not the lever.
 *
 * 3. WHY u4 IS NOT THE ANSWER, AND WHAT THE FRONTIER NOW IS.  The target's store order is
 *    A(0x10,0x8,0xC,0x14,0x3C) -> D(0x2,0x0) -> C(0x42,0x40) -> S(0x68), i.e. THIS body's
 *    order; GCC 2.7.2 will not reorder stores through an unknown pointer past stores to a
 *    known symbol, so the D-first order that wins the seats also emits the D stores first.
 *    Hoisting only the D ADDRESS and leaving its stores in place does not reproduce the
 *    effect (x1 26, x2 38, w13 29, x4 27 at 172 insns), so it is the STORE position, not
 *    the address-computation position, that moves the allocation.  The open question is
 *    therefore narrow and purely local-alloc: with insn 188 in the target's operand order
 *    and the store order left alone, restore the qty_compare_1 ordering that base already
 *    has for rows 38-61.
 *
 * 4. ALSO MEASURED AND NEGATIVE this session (all on this chassis, floor body + the class-B
 *    dead store, do-while(0) fence present).  Group-order permutations with the flip:
 *    ACDS/ADSC/ACSD/SADC 27, ASDC 36, DCAS 20, DASC 14, CDAS 14, DACS 12.  Flip variants:
 *    p_7e = p_6a + 10 (u1) 27, chain named s32 (u2) 27, constant folded into the chain (u3)
 *    14 at 176 insns, (t0*5)*2 (u10) 27, flip on p_6a only (u8) 27, flip on p_7e only (u9)
 *    3 = byte-inert, pointer-domain flip through a named `u8 *chain` local (y2) 27,
 *    declaration order of p_6a/p_7e (y4) 29.  Natural record readings: `s16 *tbl =
 *    (s16 *)D_800A36A0; &tbl[t0*5+53]` (y1) = 7 but at 176 insns — it folds the +0x6A/+0x7E
 *    constants BEFORE the base add, emitting two `addu` where the target emits one `addu`
 *    plus two `addiu`; the 0x6A-biased table (y3) 43 at 176.  Operand-order flips of the
 *    OTHER groups are byte-inert or near-inert on the floor body (z1/z2/z3 = 4, z4 = 3) and
 *    do not counteract the flip's reseat when combined with it (z5-z9 all 27).
 * Detail: evidence.md [s32b], hypotheses.md [s32b].
 */
/* s32 UPDATE (2026-09-06, structural).  BODY UNCHANGED - still the honest floor.
 * Re-measured live this session: this body = `sandbox func_800770B8 --disable all` =
 * score 3, build_insns 175, target_insns 175.  36 scoring builds; the residual is still
 * exactly rows 62/63/64 and still exactly ONE local-alloc tie.
 *
 * THE TIE PREDICATE IS NOW READ OUT OF THE COMPILER SOURCE, not inferred.  block_alloc's
 * tying loop (local-alloc.c:1240-1299) walks recog_operand 1..n and calls combine_regs;
 * combine_regs (local-alloc.c:1784-1946) refuses only on three reachable grounds:
 *   (1) reg_qty[operand] < 0            -- local-alloc.c:1827
 *   (2) reg_qty[dest] == -1             -- local-alloc.c:1836
 *   (3) no REG_DEAD note for the operand at this insn -- local-alloc.c:1917
 * reg_qty is set at local-alloc.c:469-477: -2 (local-allocatable) iff
 * reg_basic_block >= 0 AND reg_n_deaths == 1, else -1.  Our sum insn is
 * `(set (reg 110) (plus (reg 109 = the D_800A36A0 reload) (reg 108 = the t0*10 chain)))`
 * and the reload satisfies all three, so operand 1 wins and the dest takes the reload's
 * seat ($v0).  The TARGET's `addu $v1,$v1,$v0` is the dest tied to the CHAIN.
 *
 * ALL THREE DENIAL GROUNDS WERE SPELLED IN C AND PRICED THIS SESSION:
 *   (3) REG_DEAD denial - give the reload a real use AFTER the sum, inside the block.
 *       The only statement available is the 0x68 sb, and CONTROL h2 (relocate the sb and
 *       change nothing else) already costs 24 rows; h1 (relocate + read the global) 27,
 *       h4 14 at 177 insns, h5 28.  Priced out before the tie is touched.
 *   (1) reg_qty denial - i1, the best form in 32 sessions: re-read D_800A36A0 into the
 *       EXISTING `base` local just before p_6a/p_7e (two sets, every reference still in
 *       the one block).  Score 14/175/175 with rows 44-53 AND rows 63/64 byte-exact and
 *       row 62 carrying the TARGET's dest register.  Its ceiling is structural: one C
 *       variable is one pseudo, and reg_qty = -1 hands it to global-alloc, which gives
 *       BOTH live ranges a single hard register ($a1), so `base` can no longer die at the
 *       S pointer (target row 55 `addu $a0,$a0,$a1`).  CONTROL i5 (the same reload named
 *       in a fresh ONCE-set local) is byte-inert at 3, isolating "two sets" as the whole
 *       lever.  Every other carrier is worse: reusing the already-multiply-set `ptr`
 *       n1/n2/n3/n5 = 27 and `p_old` n4 = 27; sharing one local with a LATER block
 *       o1/o1b 33, o2 31, o3 36, o4 39; a trailing dead store on a fresh local m1/m2/m4/m6
 *       = 3 (flow deletes it - the second set needs a real read after it, m3 176 insns).
 *   (2) dest denial - not C-reachable (the dest is a compiler temp).
 *   Operand ORDER (make the chain operand 1) - f2/h6/q7 = 27.  s32 isolated the cause:
 *       CONTROL q5 `(s16 *)((s32)D_800A36A0 + (t0 * 10) + 0x6A)` - the SAME (s32) cast
 *       with the global still first - is byte-INERT at 3.  So the 24 rows of collateral
 *       are the operand ORDER itself, not the cast: putting the chain first extends the
 *       chain quantity through row 64 and reseats rows 38-64 wholesale.
 *
 * CLASS B'S DEAD STORE IS NOT REPLACEABLE BY ORDINARY C (owner directive item 2, ledger
 * frontier item 3, executed and closed).  The target's prologue was decoded further this
 * session: func_8006E49C RETURNS A COMPUTED POINTER (`addiu $v0,$a0,0x1FB0` is its last
 * value insn, asm/funcs/func_8006E49C.s), and the target emits a REAL copy insn
 * `addu $s1,$v0,$zero` - so the raw result and the `p_old` variable really are two
 * pseudos there.  Spelling that shape directly (s1/s2/s4 = 23 at 170 insns, s5 = 25 at
 * 170, s3 = 17 at 173) always loses 2-5 insns, because a separate local for the raw
 * result lets cse rematerialise p_old's pre-call value from $s0 (= arg0) after the three
 * calls, so nothing needs a callee-saved register and the $s1 save/restore collapses.
 * The annotated dead store is the only spelling that reaches the gate at 175 insns.
 * Byte-inert equivalences banked this session (all 3/175/175): `p_7e = p_6a + 10` (r4),
 * `(t0 * 5) * 2` for the chain (r5), naming t0*10 in an s32 local (r3, s30's kill
 * re-confirmed on THIS chassis).  Declaration order of the two pointers costs 5 (r1) and
 * swapping the inner loop's two stores costs 7 (r2).
 * Detail: evidence.md [s32], hypotheses.md [s32].
 */
/* s31 UPDATE (2026-09-06, rederive).  FLOOR 5 -> 3.  CLASS B IS CLOSED.
 * Measured live on the HEAD 2fbaa47a chassis: this body =
 * `sandbox func_800770B8 --disable all` = score 3, build_insns 175, target_insns 175.
 * The previous floor body (this file without the `p_old = prev;` dead store) re-measures
 * 5/175/175 on the same chassis, so the 2-point drop is this session's, not inherited.
 * fake_ablate: TWO orthogonal FAKE units, keep-all 3 / drop-dead-store 5 /
 * drop-do-while 8 / drop-both 10 (tmp/grind/func_800770B8/ablate/).
 *
 * WHAT CLOSED CLASS B (rows 35/36).  The target issues the two prologue clear stores
 * (0x30 / 0x34) through the RAW func_8006E49C return value in $v0 while the 0x4 store
 * issues through the saved copy in $s1.  Our body issued all three through $s1 because
 * the call-result pseudo and the p_old pseudo are a pure copy pair whose used side has
 * reg_n_deaths == 1, so local-alloc.c's combine_regs coalesces them into one quantity.
 * A dead store to `p_old` placed BETWEEN the 0x4 store and the clears gives the pseudo a
 * second death, combine_regs refuses, and the two live ranges get the target's two seats.
 * Sanctioned family: dead store to a LOCAL (.claude/rules/dead-store-fake-exception.md,
 * owner ruling 2026-07-01), annotated inline.  POSITION IS LOAD-BEARING: the same dead
 * store placed AFTER both clears is byte-inert (s31 d7, 5/175), and the SAME-VALUE
 * spellings are byte-inert too - `p_old = p_old;` (d4), `p_old = (s32 *)((u8 *)p_old);`
 * (d6) and `p_old = (s32 *)D_800A36A0;` (d5) all measure 5/175, because GCC deletes the
 * self-assign and cse refolds the global re-read to the same pseudo.  Only a store of a
 * DIFFERENT value reaches the reg_n_deaths gate; `p_old = prev;` (the previous record
 * pointer, a live value, mirroring SOTN's `dest = val1; // fake`) is the spelling banked
 * here.  Nine truthful re-spellings of the whole block were measured first and are all 5
 * or worse (s31 c1-c9; c1/c4/c5/c9 collapse the callee-save at 170 insns).
 *
 * THE WHOLE RESIDUAL IS NOW CLASS C - THREE ROWS, ONE TIE.
 *   ours   lw $v0,D_800A36A0 ; sll $v1,$v1,1 ; addu $v0,$v0,$v1 ; addiu $a3,$v0,0x6A ...
 *   target lw $v0,D_800A36A0 ; sll $v1,$v1,1 ; addu $v1,$v1,$v0 ; addiu $a3,$v1,0x6A ...
 * Identical seats for the load and the shift; only the sum's DEST differs.  In the .lreg
 * dump this is insn 188 `(set (reg 110) (plus (reg 109 = the reload) (reg 108 = the
 * shift)))`; block_alloc's tying loop (local-alloc.c:1240-1299) walks operands 1..n with
 * `if (win) break;`, so the dest ties to the reload and takes its seat.
 *
 * TWO MECHANISMS THAT FLIP THAT TIE ARE NOW MEASURED (both cost more than they buy):
 *   f1/f2 - the c-typeck ptrop route (`(u8 *)(t0*10) + (s32)D_800A36A0 + 0x6A`, and the
 *     fully-integer `(t0*10) + (s32)D_800A36A0 + 0x6A`): both 27/175.  They DO flip the
 *     tie, but they seat the load and the shift the other way round ($v0<->$v1) AND they
 *     reschedule the loop head (rows 38-59).  s9 banked this at score 33 on an older
 *     chassis; re-audited here, still dead.
 *   g1 - the local-alloc reg_qty route: name the reload in a local that is SET TWICE
 *     (once for p_6a/p_7e, once for the 0x5C/0x60 stores that already reload the global),
 *     so reg_n_deaths == 2 / reg_basic_block == -1 and local-alloc.c:469-477 sets
 *     reg_qty = -1, which makes combine_regs refuse operand 1 and the tie fall through to
 *     the shift.  39/175 - but rows 60 THROUGH 78 ARE BYTE-EXACT, class C included: this
 *     is the first form in 31 sessions that emits rows 62-64 with the target's seats.
 *     Its price is entirely elsewhere: rows 38-59 (the loop head reschedules exactly as
 *     in f1) and rows 79-89 (the 0x5C/0x60 block, perturbed by the named local).
 * The open lever is therefore narrow and named: reach reg_qty[reload] == -1 WITHOUT a
 * long-lived named local.  g4 (two reads of the global into the same local inside one
 * block) is refolded by cse and byte-inert at 3; g2 (reusing `base`) 35; g3 (second set
 * in the tail block) 33.
 * Detail: evidence.md [s31], hypotheses.md [s31].
 */
/* s30 UPDATE (2026-09-05, escalation).  BODY UNCHANGED - still the honest floor.
 * Re-measured live on the HEAD 6c9ca9fa chassis: sandbox func_800770B8 --disable all =
 * score 5, build_insns 175, target_insns 175.  h3 (rejected/s28-classC-paid-ADSC-...) = 7
 * and cX_c/o1 = 25 both reproduce exactly, so the s28 frontier is current, not inherited.
 *
 * THE TARGET'S STORE WINDOW IS NOW TRANSCRIBED, not inferred (asm/funcs/func_800770B8.s
 * rows 40-64; full read-out in evidence.md [s30]).  Three facts:
 *   - the store-group order is A, D, C, S with the `sb` LAST, so h3's five extra rows are
 *     purely its S-before-C order;
 *   - the three address computations are emitted as a CLUSTER (C pointer, S pointer, chain
 *     root) and only then the two `sh` and the `sb` - the S pointer is live across the C
 *     stores and reuses base's register;
 *   - the cursor is GCC's synth_mult for 10 (`t0*4 + t0`, then `<<1`) reusing the same t0*4
 *     pseudo the D and C pointers use, which the C spelling `t0 * 10` already reproduces.
 *
 * o1 (flipped cursor + C group on its own `pc` + A,D,C,S) EMITS THE TARGET'S EXACT
 * INSTRUCTION ORDER: rows 55 and 59 are byte-exact and all 25 differing rows are ONE
 * local-alloc seat (chain [28,56] r22 = 3.1428 beats pc [36,40] r6 = 3.0 at
 * local-alloc.c:1660-1683 and takes $2).
 *
 * THE CLASS-C RESIDUAL IS A TWO-HORNED DILEMMA, both horns measured in s30:
 *   HORN 1 - a short blocker in [28,48).  Only the S store's address temp qualifies (4 refs
 *     / span 2 / 4.0) and only when the S store is emitted BEFORE the C stores: h3, score 7.
 *   HORN 2 - lengthen the chain's interval so pc's 3.0 wins.  Needs the t0*4 shift floated to
 *     the top of the block ([12,56], 2.0), which happens exactly when the D group stops
 *     sharing group A's `ptr` pseudo: p1 / q1 (A or D on its own local, A,D,C,S) DO win the
 *     target's seats, and score 28 because the same freeing floats the %hi/%lo(D_800A35D0)
 *     pair; the D-first orders o2/p2/q2 win it for 14.
 * The target has the shift floated WITHOUT the symbol floated.  Separating those two sched1
 * decisions is the single open lever - it turns q1/p1 into a score-2 body.
 *
 * BYTE-INERT ON THIS CHASSIS (13 spellings, all 25/175, w1 and y2 dump-verified identical
 * quantity tables): naming the S pointer at any of five positions; spelling the C pointer
 * shift-first; swapping the C stores; naming t0*4, t0*10 or both; splitting the C pair onto
 * two once-used locals (cse refolds them); hoisting the C-pointer assignment above group A.
 * Statement POSITION of a pure address computation does not move bytes here - only which
 * LOCAL each store group uses and the ORDER of the store groups do.
 */
/* s29 UPDATE (2026-09-05, object-model).  BODY UNCHANGED - still the honest floor.
 * Re-measured live on the HEAD dcd79965 chassis: sandbox func_800770B8 --disable all =
 * score 5, build_insns 175, target_insns 175.  fake_ablate: one FAKE unit (the prologue
 * fence), keep-all 5 / drop-1 10.
 *
 * THE OBJECT MODEL IS AUDITED AND IS NOT THE RESIDUAL.  Every global this function touches
 * was checked declared-shape vs evidence (census, sibling asm addressing, this function's
 * index arithmetic).  Three declarations are wrong at the TU level (D_8009BCE4 is a u8[20]
 * table, D_8009BD20/21 is a u8[2][2] pair table, D_800A35D0 is an s16[2][2] per-player
 * pair) and correcting all three is BYTE-NEUTRAL: 5/175, whole-TU .text identical.  The
 * corrected form lives in candidate_objmodel_ALL2.c + s29-objmodel-ALL2-declaration-
 * edits.patch; this file keeps the standalone-compiling body so inherited apply scripts
 * still work.  The five residual rows (35/36 class B, 62-64 class C) contain no symbol,
 * and every symbol-bearing target row is already byte-exact.  Subscript spellings of the
 * D_800A35D0 pair are worse for a NEWLY-READ reason: a direct subscript of a global array
 * with a register index is a legitimate (plus reg (const (plus sym k))) MIPS address in GCC
 * 2.7.2 and is macro-expanded per store (lui $at/addu/sh), not LICM-hoisted - s10/s12's
 * attribution only holds for spellings that give the symbol its own pseudo.
 * Detail: evidence.md [s29] OBJECT MODEL (per-symbol verdicts + premises P1-P5),
 * hypotheses.md [s29] H1-H6.
 */
/* s25 UPDATE (2026-09-05, synthesis).  BODY UNCHANGED - still the honest floor.
 * Re-measured live this session on the current chassis (HEAD 35957733):
 * `sandbox func_800770B8 --disable all` = score 5, build_insns 175, target_insns 175.
 * Plain flipped base X = 29/175.  fake_ablate: one FAKE unit (the prologue fence),
 * keep-all 5 / drop-1 10.  s23's fCADS re-measured live at 12/175, rows 55-64 still
 * byte-exact.
 *
 * BOTH STANDING FRONTIER ITEMS ARE EXECUTED AND NEGATIVE.
 *   - "consume the cursor sum as a MEM base like the 0x5C control site": p1n/p1f =
 *     172 insns / score 46 in both spellings, and the premise is independently false -
 *     the target's cursor sum has exactly two REGISTER uses (its rows 66/67
 *     `addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E`), the same shape this body emits.
 *   - "respell the addend as an sll of a plain register": the floor's row-61
 *     `sll $v1,$v1,1` ALREADY is an ashift of a plain register, identical in shape to
 *     the 0x5C site's row-87 sll; the s16 row-index spelling costs an insn (n5n 176/7,
 *     n5f 176/30) and does not move the tie.
 *
 * THE TIE AND THE SEATS ARE NOW READ OUT OF local-alloc.c, NOT INFERRED.
 *   TIE: block_alloc's tying loop (local-alloc.c:1240-1299) walks operands 1..n with
 *   `if (win) break;`, so the sum's dest ties to RTL operand 1 - the operand the source
 *   names first - unless combine_regs (local-alloc.c:1784-1946) refuses, which it does
 *   when reg_qty[used] < 0 (reg_basic_block < 0 or reg_n_deaths != 1, set at
 *   local-alloc.c:470-477), when the used pseudo has no REG_DEAD note here, or when the
 *   dest already has a quantity.  MEASURED: making the reload non-local flips the tie
 *   onto the shift from a BASE-FIRST source (z2n 176/42), and base-first and flipped
 *   then measure byte-identically - but global-alloc seats the ejected reload in $t0.
 *   SEATS: three fresh .lreg dumps show F and X with byte-identical pseudo tables.  The
 *   merged chain+sum quantity Q = {chain root, t0*5, t0*10, sum} always has 22 refs, so
 *   floor_log2(22)*22 = 88 is fixed and only Q's span varies against the blocking short
 *   4-refs/2-insn quantity at priority 4.0:  X chain-root span 10, Q ~14, 6.3 > 4.0, Q
 *   takes $2 (wrong seat);  fCADS chain-root span 18, Q ~23, 3.8 < 4.0, the short qty
 *   takes $2 and Q is pushed to $3 with the reload in $2 - the target's seats.
 *   So class C closes on the target's A-first store order iff Q's span exceeds ~22
 *   insns while the sum stays tied to the shift.  This supersedes s23's qty-number
 *   tie-break attribution.  Naming chain members does not move their birth: hoisting
 *   `s32 o2 = t0*2;` / `s32 o4 = t0*4;` above the D_800A36A0 read is byte-inert in all
 *   eight builds (y1f-y4f 29/175, y1n-y4n 5/175).
 * Detail: evidence.md [s25], hypotheses.md [s25].
 */
/* s23 UPDATE (2026-09-05, structural).  BODY UNCHANGED — still the honest floor.
 * Re-measured live this session on the current chassis (commit 0f03be29):
 * `sandbox func_800770B8 --disable all` = score 5, build_insns 175, target_insns 175.
 * Plain flipped base = 29/175.  fake_ablate: one FAKE unit (the prologue fence),
 * keep-all 5 / drop-1 10.
 *
 * WHAT s23 CHANGED — CLASS C IS NO LONGER FORECLOSED, AND ITS PRICE IS NOT +24 ROWS.
 * The flipped cursor base combined with a store-group source order that does not put
 * group A first emits the target's rows 55-64 BYTE-EXACT, including the class-C row
 * 62 `addu $v1,$v1,$v0` and rows 63/64's `addiu $a3,$v1,0x6A` / `addiu $a1,$v1,0x7E` —
 * correct TIE and correct SEATS together, at 175 instructions.  Best such build is
 * order C,A,D,S at 12/175 (rejected/s23-classC-SOLVED-flip-plus-C-group-first-
 * 175insn-score12.c); order D,A,C,S is 14/175.  An exhaustive 24-permutation sweep of
 * the four outer-loop store groups on the flipped base is completely regular: every
 * A-first order is 29, every C-first or D-first order reaches the target's seats.
 *
 * WHY IT IS NOT BANKED AS THE CANDIDATE.  The 12-object's residual is class B's two
 * rows plus rows 40-54, which carry exactly the target's instruction multiset in the
 * reordered sequence.  Stores cannot be scheduled across each other in either
 * scheduler pass, so source store order IS emission store order, and the target's
 * emission pins the target's source order to A-first — the one order that cannot
 * produce the seats under the flip.  Thirteen builds hoisting pointers, symbol
 * addresses, index temporaries and single stores move the score to 20-25 but never
 * reach the discriminator.
 *
 * TWO STANDING FRONTIER ITEMS ARE RETIRED AS MIS-ATTRIBUTIONS.
 *   - s22's "the merged shift+sum quantity's span must grow past ~23 insns": two fresh
 *     .lreg dumps (s23/X.lreg, s23/fCADS.lreg) show pseudos 107/108/109/110 carry
 *     IDENTICAL refs and spans in the 29-object and the 12-object and differ only in
 *     the seats, so qty_compare's ratio (local-alloc.c:1630-1657) is equal in both and
 *     cannot be the selector.  The selector is the equal-priority tie-break "sort by
 *     qty number" (qty_compare_1, local-alloc.c:1681-1683) — the insn-scan position.
 *   - the frontier's "the shift's value is freshly born": false.  The floor body's rows
 *     40-61 are already byte-exact with the target, including the shared
 *     t0*4 -> t0*5 -> t0*10 chain at rows 42/54/56/61.
 * Detail: evidence.md [s23], hypotheses.md [s23].
 */
/* s21 UPDATE (2026-09-05, rederive).  BODY UNCHANGED.  Re-measured on the MOVED
 * chassis (func_80060A68 landed as matched C in this same TU, commit 27441fa5):
 * `sandbox func_800770B8 --disable all` = score 5, build_insns 175, target_insns 175.
 * The TU-mate landing is byte-neutral for this function.
 *
 * WHAT s21 CLOSED.  s18-forensics left the instruction "start from THIS body (which
 * already has the target's class-C seats, $2 = reload, $3 = shift) and attack the tie,
 * not from the flip and attack the collateral".  That instruction is now answered
 * NEGATIVE: the tie and the seats are the SAME bit of C, read twice.  Eleven fresh
 * builds staging the D_800A36A0 reload and/or the t0*10 shift into their own preceding
 * statements, at several hoist positions, in both the pointer and the integer domain,
 * with p_7e derived three different ways, land on exactly TWO objects - 29/175 for every
 * spelling whose tree names the SHIFT first, 5/175 (byte-identical to this body) for
 * every spelling whose tree names the RELOAD first.  The .lreg dumps say why: expand
 * numbers the two pseudos in the REVERSE of the source tree's operand order (floor body
 * reg 108 = the sll / reg 109 = the lw; flipped body the spans swap), and find_free_reg
 * hands $2 to the LATER-numbered quantity in both.  So the operand a body names first is
 * always born second and always lands in $2, and block_alloc's ordered combine_regs loop
 * (local-alloc.c:1240-1298) then ties the sum to that same first-named operand.  The
 * target needs the tie on the shift AND the shift in $3 - one bit set both ways at once.
 * Banked rejected/s21-classC-staged-reload-flip-*, s21-classC-staged-shift-flip-*,
 * s21-classC-int-domain-pointer-first-*.  Detail: evidence.md [s21].
 *
 * ALSO SPENT: the CD_ready (marionation_Exec) sibling pairing.  It shares NO code window
 * with this function; every mention of it in this ledger is the do-while-zero rule's
 * "Confirmed application" policy citation.  Nothing to transplant.
 */
/* candidate.c - func_800770B8 (src/text1b.c) - s15 synthesis, 2026-09-01
 * Measured THIS form on today's chassis (s15, first action of the session):
 *     sandbox func_800770B8 --disable all = 5   (175 build insns / 175 target insns)
 *
 * THIS BODY IS THE HONEST FLOOR AND IT IS CHEAT-FREE.  It carries exactly one
 * non-semantic construct - the single-level annotated `do { } while (0);`
 * prologue fence - and s15 resolved that construct's four-session-old
 * classification doubt by READING the rule chain rather than by asking again:
 *   - .claude/rules/no-new-park-categories.md:256-271 still prints the stale
 *     2026-06-04 scoping ("applies only to the LABEL_OUTSIDE_LOOP_P / reorg.c
 *     interaction"), BUT that same paragraph designates the dedicated rule as
 *     the authority on the prerequisites ("The dedicated rule
 *     [[do-while-zero-exception]] enumerates the strict prerequisites");
 *   - .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06, the
 *     LATER document) states in its scope sentence that the wrap is "an allowed
 *     pure-C match device for ANY codegen effect incl. register allocation, with
 *     mandatory inline FAKE annotation", and in its body that "The former
 *     scoping to the reorg.c label-note mechanism is abolished";
 *   - its prerequisite 2 explicitly removes exhaustion as a hard gate for
 *     SINGLE-LEVEL wraps, and this body uses one level, annotated inline.
 * So the sched2 mechanism of this wrap is NOT a scoping problem, and the honest
 * floor of func_800770B8 is 5 - not 9.  (s11/s12/s13/s14 all carried this as an
 * open ruling question; it is closed by citation, and no Judge cycle is owed.)
 *
 * RESIDUAL AT FLOOR 5 - two classes, both measured dead
 * ----------------------------------------------------
 *   class B, rows 35-36: ours `sw $0,0x30($17) / sh $0,0x34($17)`, target
 *     `sw $zero,0x30($v0) / sh $zero,0x34($v0)` - the two stores go through the
 *     p_old copy instead of the raw func_8006E49C result pseudo.  FORECLOSED
 *     three times over (s7/s8/s9 dumps on the floor-9 chassis, s14 on the
 *     floor-4 chassis): every spelling that actually reaches the raw pseudo
 *     collapses the function to 170 instructions, FIVE FEWER than the target's
 *     175, because flow.c then deletes the copy and its four dependents.
 *   class C, rows 62-64: ours `addu $2,$2,$3`, target `addu $v1,$v1,$v0`.
 *     Reachable ONLY by naming the addend first in p_6a's address (the "flip"),
 *     and the flip costs +24 rows of loop-head collateral (score 29).  The ONLY
 *     construct ever measured to repair that collateral is the arithmetic
 *     identity `(t0 * 4) >> 1` for group A's index, which the Judge FAILED on
 *     2026-09-01 (docs/grind/decisions.md, 07:23 entry) as a byte-materializing
 *     chain-extender.  Banked at rejected/s14-classD-identity-detour-JUDGE-
 *     FAILED-2026-09-01.c; do not respell it in any form.
 *
 * s15 NEGATIVE RESULTS (38 fresh builds; full detail in evidence.md [s15]):
 *   - The Judge's own suggested replacement - "group A addressed through a cursor
 *     genuinely derived from group C's t0*4 pointer" - is MEASURED DEAD (8
 *     builds).  Every pointer-cursor spelling either materialises the pointer
 *     subtraction (`>>1` form 36/176, `- (s16 *)base` form 36/176: +1 insn the
 *     target does not have) or is byte-worse (`/2` 54/178, `rowC - t0*2` 51/175).
 *     GCC 2.7.2 does not fold the difference back to a shared shift, so a
 *     truthfully-derived cursor cannot supply the dependence edge for free.
 *   - Type-forced derivation (frontier item 1c) is DEAD (20 builds): array-typing
 *     group A (`*((s16 *)(base + 0x10) + t0)`) is byte-neutral under the flip
 *     (29) and worse without it (31); array-typing group C as
 *     `((s16 (*)[2])(base + 0x40))[t0]` costs an insn (15-18 at 176); both
 *     together 33-37 at 176.  Neither creates the t0*4 -> t0*2 edge.
 *   - The operand-order flip is MECHANISM-INDEPENDENT: casting the ADDEND to a
 *     pointer (`(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A)`), which reaches
 *     pointer_int_sum's ptrop slot by a completely different route than the
 *     `(s32)` cast, is BYTE-IDENTICAL to the (s32) flip (29/175 on both the
 *     unflipped and flipped bases).  Array-typing the 5-element rows
 *     (`((s16 (*)[5])(D_800A36A0 + 0x6A))[t0]`) is the flip plus one insn
 *     (9/176).  Eleven flip spellings across s6/s13/s15 now collapse onto exactly
 *     one build.
 *
 * s16 NEGATIVE RESULTS (128 fresh builds + a whole-corpus census; detail in
 * evidence.md [s16]):
 *   - The class-B "2+2 split" is UNIQUE in the executable.  An exhaustive census
 *     of all 1,435 asm/funcs/*.s (tmp/grind/func_800770B8/s16/census.py and
 *     census_broad.py) finds exactly ONE call-result copy whose copy register AND
 *     raw $v0 are both used as store bases afterwards: this function.  All four
 *     sibling call sites of func_8006E49C store through the raw $v0 with no copy
 *     retained.  The s15 frontier's only reserved re-opening path for class B -
 *     "find a sibling that names the source shape" - is measured non-existent.
 *   - Procedural factoring is byte-transparent (8 builds).  `static inline`
 *     helpers for the post-call header init that take the ALREADY-ASSIGNED pointer
 *     are byte-identical to this body (5/175); ones that take the CALL RESULT as a
 *     parameter collapse to 170 insns exactly as s7/s8's two-local forms did, since
 *     an inline parameter is the same pseudo-to-pseudo copy make_regs_eqv kills.
 *   - Local declaration ORDER is inert: all 120 permutations of the five top-level
 *     locals measure 5/175, one single distinct build.  GCC 2.7.2 numbers pseudos
 *     at first RTL emission, not at declaration, so the pseudo-number tie-break in
 *     cse.c make_regs_eqv / local-alloc allocno ordering is not C-controllable here.
 *
 * s17 NEGATIVE RESULTS (12 fresh builds + a compiler-source enumeration; detail in
 *   evidence.md [s17]):
 *   - CLASS B'S MECHANISM IS NOW DEMONSTRATED AND PRICED, not merely unfound.  Under
 *     cse pass 2 (`after_loop = 1`) an extended basic block is terminated by exactly
 *     two things - a CODE_LABEL or a NOTE_INSN_SETJMP (tools/gcc-2.7.2/cse.c:8038-
 *     8063), so no note/scope/wrap/inline fence can ever break it.  With a surviving
 *     CODE_LABEL placed between the copy-based stores and the raw-result stores, plus
 *     the s8 make_regs_eqv canonical promotion, the target's exact 2+2 split IS
 *     emitted (rejected/s17-classB-split-DEMONSTRATED-join-label-177insn-score30.c:
 *     `move $17,$2 / sw $17,0($28) / sw $18,4($17) ... sw $0,48($2) / sh $0,52($2)`).
 *     It costs 177 insns.  A label with no live reference is demoted to
 *     NOTE_INSN_DELETED_LABEL by jump.c pass 1 (dump-proven, A3 byte-identical to A2),
 *     so a free label does not exist; a label with a live reference costs its branch
 *     (176 s9, 177 B1, 177 C1).  This function has ZERO insn slack (175 = 175), so
 *     class B is foreclosed BY PRICE.
 *
 * s18 NEGATIVE RESULTS (5 fresh builds + the full solver suite re-run with
 *   FULL-DISPOSITION goals; detail in evidence.md [s18]):
 *   - THE FLOOR-5 BUILD IS THE TARGET MODULO FIVE REGISTER NAMES.  goal_from_tgt.py
 *     classify reports 5 renamed pairs and ZERO skeleton-differing pairs, and
 *     sched_solver perturb (object-level goal, --target-object build/src/text1b.o)
 *     reports NO differing block in sched1 AND sched2, with align honobj->tgtobj =
 *     {equal 170, replace 5, delete 0, insert 0, moved 0}.  s9 had only ever checked
 *     sched2; "emission order" is retired as a description of this residual.
 *   - GLOBAL ALLOCATION IS FORECLOSED FOR BOTH CLASSES, not just class B.
 *     inverse.py global with the FULL goal {"75": 2, "110": 3} is NEGATIVE at depth 2
 *     and depth 3, and the class-C-only goal {"110": 3} is NEGATIVE at depth 3.  s5's
 *     subset goal {"75": 2} is superseded under solver rule (3).
 *   - LOCAL ALLOCATION offers exactly one family and it is dead in C.  inverse.py
 *     local on the BASE model (--block 1 --swap 3,4, 392 atoms) is REACHABLE with 55
 *     single-atom vectors, all `live_shrink qty3 born later (28 -> 35..46)` (qty3 =
 *     r108, the t0*4 shift chain).  Five in-place spellings that delay that birth
 *     measure 37/177, 6/175, 15/177, 37/177, 15/177; the only 175-insn one leaves
 *     rows 62-64 byte-unchanged and breaks row 54 instead.  Statement-level
 *     relocation of the same quantity was already exhausted by s12 (24/24) and s13
 *     (120 orders).  Banked rejected/s18-qty3-birth-delay-*.c.
 *   - TOOLING TRAP: tools/sched_solver/mkasm.sh ignores --target and copies hon.s to
 *     tgt.s, so the solver playbook's "--target <stem>.tgt.head.s" compares this
 *     function against ITSELF and prints "GOAL == OURS (identity)" for every block.
 *     cmp the two files before trusting any sched goal here.
 *
 * s18-forensics NEGATIVE RESULTS (3 fresh builds + two instrumented-cc1 dumps;
 *   detail in evidence.md [s18-forensics]):
 *   - CLASS C IS ATTRIBUTED TO A NAMED DECISION AT LAST.  The divergent insn is
 *     RTL 185 `(set (reg 110) (plus (reg 109) (reg 108)))` (109 = the second
 *     D_800A36A0 reload, 108 = the t0*10 shift).  Its hard register is chosen by
 *     local-alloc.c block_alloc's operand-tying loop (tools/gcc-2.7.2/
 *     local-alloc.c:1240-1298), which walks operands 1..n IN ORDER, calls
 *     combine_regs(operand_i, operand_0) and BREAKS AT THE FIRST SUCCESS.
 *     Operand 1 dies here, so 110 merges into the reload's quantity and inherits
 *     its seat (`;; Register 109 in 2.` / `;; Register 110 in 2.`) -> our
 *     `addu $2,$2,$3`.  The target merges 110 with operand 2 -> `addu
 *     $v1,$v1,$v0`.  This is NOT the find_free_reg seat-priority story s17's
 *     local-alloc solver run modelled.
 *   - ALL TEN combine_regs GATES ARE TYPED (local-alloc.c:1784-1946).  Six are
 *     structurally impossible for two SImode pseudos in a plain addsi3, one is
 *     unreachable on MIPS (all GR_REGS), one detaches the dest from BOTH
 *     operands.  The only two C-reachable gates - reg_qty[ureg] < 0 (not
 *     block-local / multi-death) and "no REG_DEAD note here" - BOTH require the
 *     reload to stay live past the add, i.e. an extra use.  The target reloads
 *     D_800A36A0 fresh at every later use and rows 38-64 are one basic block, so
 *     any such use deletes or adds an instruction, and this body has zero slack
 *     (175 == 175).
 *   - GATE 1 DEMONSTRATED AND PRICED: hoisting the reload into a local reused
 *     after the inner loop reaches the target's tie topology WITHOUT the flip
 *     (`addu $2,$8,$2`, dump-proven at insn 186 with the reload's pseudo lacking
 *     a block marker) but deletes the third reload: 174 insns, score 49.
 *     rejected/s18fx-classC-blocklocal-reload-defeats-op1-tie-174insn-score49.c.
 *   - THE FLIP IS NOT "CLASS C PLUS COLLATERAL".  Re-measured plain flip = 29/175
 *     and prints `lw $3 / sll $2 / addu $2,$2,$3`: right tie, WRONG SEATS - the
 *     reload and the shift swap hard registers.  THIS body already has the
 *     target's seats ($2 = reload, $3 = shift) and differs only in the tie.  Any
 *     future class-C attack should start here and attack the tie, not start from
 *     the flip and attack the collateral.  rejected/s18fx-classC-flip-fixes-tie-
 *     but-swaps-seats-175insn-score29.c.
 *
 * INHERITED, STILL BINDING (do not re-derive): s6 (19 address spellings),
 *   s9 (3234-atom sched_solver sweep, 0 hits), s10 (full struct rewrite 178 insns),
 *   s11 (63-position single-wrap sweep + 18 nested), s12 (24/24 store-group orders,
 *   8 group spellings, 12 hoists, 79+79 second-wrap), s13 (315 loop-shape builds,
 *   17 inner-address spellings, 240 five-element orders), s14 (33 demand-order,
 *   13 dependence-direction, 8 reference-count, 79-position second-wrap).
 *
 * Applying this body also requires the two caller-side edits (see
 * tmp/grind/func_800770B8/s3/try.py): the prototype becomes
 * `s32 func_800770B8(s32, s32, s32);` and the call site passes
 * `(s32)&D_8009BD24`. Byte-neutral for the caller.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap. Effect: it anchors a
       NOTE_INSN_LOOP_BEG/END pair at this statement position, which stops sched2
       interleaving the five reload-emitted frame-save stores with the first body
       insns; without it the prologue emits sw $s1 / addiu $s1,$s0,0x58 / lw
       D_800A374C / li 0x1008 / sw $ra where the target emits sw $ra / sw $s1 /
       li 0x1008 / lw D_800A374C / addiu $s1 (residual class A, 4 rows).
       mechanism: GCC 2.7.2 sched.c list scheduler, second pass (sched2, post-reload);
       the notes bound the scheduling region so the save stores cannot be hoisted
       across them. See evidence.md [s9] for the insn-level read-out of the
       unfenced order and [s11] for the measurement.
       lever-exhaustion: hypotheses.md classes A/B/C; s3 (12 statement orderings),
       s5 (honest-loop fence hunt, +11 insns), s9 (exhaustive 3234-atom sched_solver
       depth-1 sweep against the target emission order: 0 hits; the only reachable
       sub-goal needs atoms not expressible in C), s10 (struct-typed rederive 178
       insns), s11 (63-position single-wrap sweep + 18 nested-wrap variants). */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    p_old = (s32 *)(arg0 + 0x58);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        /* FAKE: dead store to the local `p_old` (its stored value is never read;
           GCC DCEs the store - insn count is 175 with and without it).  Without
           it the func_8006E49C result pseudo and the p_old pseudo are a single
           copy pair whose used side has reg_n_deaths == 1, so local-alloc.c's
           combine_regs (local-alloc.c:1784-1946, gated by the reg_qty init at
           local-alloc.c:469-477) coalesces them into ONE quantity and both the
           0x4 store and the two 0x30/0x34 clears issue through $s1; the target
           issues the clears through the raw return value in $v0 (rows 35/36).
           The extra set gives the pseudo a second death, combine_regs refuses,
           and the two live ranges get the target's two seats.
           mechanism: GCC 2.7.2 local-alloc.c combine_regs / reg_n_deaths gate.
           lever-exhaustion: hypotheses.md class B, s7/s8/s20/s24 (raw-vs-copy
           store-base spellings), s31 c1-c9 (nine further truthful spellings of
           the same block, all 5 or worse) and s31 d4-d7 (self-assign, cast
           self-assign, same-value re-store from the global, and the same dead
           store placed after the clears - all four byte-inert at 5). */
        p_old = prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    t0 = 0;
    do {
        u8 *base = D_800A36A0;
        u8 *ptr;
        a2 = 0;
        ptr = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)&D_800A35D0;
        ptr = (t0 * 4) + ptr;
        *(s16 *)(ptr + 2) = 0;
        *(s16 *)(ptr + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            s16 *p_6a = (s16 *)(D_800A36A0 + (t0 * 10) + 0x6A);
            s16 *p_7e = (s16 *)(D_800A36A0 + (t0 * 10) + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}
