/* s35 ADDENDUM (solver, 2026-08-27). Re-measured at the START of s35 with this exact
 * body: STILL sandbox --disable all == 1 at 132 build / 132 target insns (HEAD = 27,
 * V15a = 15, W4 = 3, Z1 = 13). The body is UNCHANGED. It remains the floor -- but s35
 * changed what it MEANS: this body is now the proven CEILING of a bounded family, not
 * a form that is close to the answer.
 *
 * THE GOTO CHASSIS IS PERMANENTLY IMPOSSIBLE (s35 frontier note + E-s35-5/E-s35-6).
 * For out2 to take $s6 it must be allocated before pa4 (find_reg = lowest-numbered
 * non-conflicting register in descending priority order; target gives pa4 the
 * higher-numbered $s7), so pri(out2) > pri(pa4) = 1473. Target's bytes host exactly
 * THREE out2 references (asm/funcs/func_80041188.s:25, 56, 61, re-read first-hand),
 * at which pri(out2) = 30000/L and the inequality needs L < 20.4 -- against a floor
 * of 40, because out2 is defined in block 0 and live across the whole of loop1.
 * Unsatisfiable by a factor of two. So out2's fourth reference is NECESSARY on this
 * chassis, and s35 closed the last three delivery mechanisms: nothing is deleted
 * anywhere after local-alloc (E-s35-2, measured across .lreg/.greg/.jump2/.sched2/.dbr
 * plus a derivation that an in-loop1 copy of a loop-carried value is either
 * conflicting or dead); no combine cancellation can host it (E-s35-4, two exhaustive
 * horns); and the CODE_LABEL that stops the cse fold costs 134 insns even with D6's
 * absorbable nop (E-s35-3). Do not respell this body.
 *
 * WHERE THE GRIND GOES INSTEAD (E-s35-8). On the REAL-LOOP chassis loop.c's strength
 * reduction rewrites every count, and the same inequality has a seven-insn gap rather
 * than a factor of two: form W4c (alt_W4c_realloop_targetblk2_s35.c = W4 with target's
 * honest block-2 `out3 = (s32 *)(((u8 *)pa4) + 0x20);`) measures 11 at 132/132 with
 * out2 at 5 refs / live 41 / 2439 against pa4 at 9 refs / live 94 / 2872. Three
 * one-step routes close it: L(out2) <= 34; out2 at 6 refs; or pa4 down from 9 to SEVEN
 * references -- and seven is exactly what target's bytes host. See hypotheses.md's s35
 * frontier 1.
 */
/* s34 ADDENDUM (synthesis, 2026-08-27). Re-measured at the START of s34 with this
 * exact body: STILL sandbox --disable all == 1 at 132 build / 132 target insns
 * (HEAD = 27, D5 = 2 at 133, V15a = 15 at 132). The body is UNCHANGED from s33 --
 * the honest `stptr = base + 0xFC;` initialiser plus the owner-ALLOWED split
 * increment at stptr's own loop1 increment site, with its mandatory FAKE
 * annotation. It remains the floor and the shipping chassis.
 *
 * s34 did not change the body; it closed two surfaces and re-stated the residual.
 * (1) LEDGER CORRECTION (E-s34-1): the block-2 `move`/`addiu` fold is done by BOTH
 * cse passes, not just cse1. cse.c:8055 ends a cse basic block at a
 * NOTE_INSN_LOOP_END only when `after_loop == 0`, so cse1 IS breakable by a loop
 * note (measured: form L1's red.i.cse still holds `out3 = pa4 + 0x20`), but cse2
 * (-frerun-cse-after-loop, on at -O2) has after_loop == 1 and re-folds it to
 * `out3 = out2` before flow ever counts. The whole loop-note route to preserving
 * target's block-2 addiu is therefore dead, and E-s31-3's CODE_LABEL price
 * (+2 insns) is the true and only price.
 * (2) CLOSED (E-s34-2): the fifth parameter is exactly a two-element MATRIX array.
 * func_8004A348 writes its second argument only at 0x0..0x10, func_800523E0 reads
 * both matrix arguments only at 0x0..0xA, func_80044DE4 does not touch the pair --
 * so there is no hidden struct member for a block-2 or loop2 statement to consume.
 *
 * THE RESIDUAL, in its sharpest form (E-s34-3). With the owner's split-increment
 * lift available at every existing increment site, the whole of target's allocno
 * order (stptr > stptr2 > i > tbl > out2 > pa4 > a3) is spellable EXCEPT out2's
 * term: out2 has no increment site, so its reachable reference counts are 3 (714,
 * below a3), 5 (2380, the in-loop1 same-value re-store -- but that is a DEFINITION,
 * so it re-triggers the cse fold and costs its own instruction) and 7 (3255,
 * unplaceable). The entire 34-session residual is one object: a byte-free in-loop1
 * delivery of +1/+2 flow-counted references to out2 spelled as a USE rather than a
 * definition. See evidence.md s34 and hypotheses.md's s34 frontier.
 */
/* s33 ADDENDUM (synthesis, 2026-08-27) -- THE BODY BELOW CHANGED. The floor is
 * unchanged at sandbox --disable all == 1 (132 build / 132 target insns,
 * re-measured this session; HEAD's committed body = 27), and the allocno table is
 * bit-identical to the s7..s32 candidate's. What changed is the CONSTRUCT that
 * buys stptr's two extra flow-counted references: the un-shippable F1
 * combine-foldable chain-extender `stptr = base; stptr += 0xFC;` (s11's
 * correction: an un-annotated chain-extender, a layer-1 FAIL as shipped, and the
 * only reason this body could not be submitted) is REPLACED by an owner-ALLOWED
 * split increment at stptr's own increment site, `stptr += 0x69; stptr -= 1;`
 * (owner ruling 2026-08-27, docs/grind/decisions.md:14739 -- the 2026-07-06
 * split/redundant-arithmetic class, mandatory FAKE annotation per site, which is
 * present below). The block-0 initialiser is now the honest single statement
 * `stptr = base + 0xFC;` that target emits. Measured s33: CAND (old body) = 1,
 * S1 (this body) = 1, S2 (`stptr += 0x34; stptr += 0x34;`, same class) = 1, S3
 * (both constructs at once, stptr 9 refs) = 32. ALLOCDBG for this body:
 * stptr 7/41=3414 $s3 - stptr2 6/48=2500 $s0 - i 8/97=2474 $s4 - tbl 4/47=1702
 * $s5 - out2 4/47=1702 $s6 - pa4 6/95=1263 $s7 - a3 4/99=808 $fp - out3 3/47=638
 * $s3: ALL-TARGET seats. The residual is unchanged and is the single insn at slot
 * 72, ours `move $s3,$s6` (from `out3 = out2;`) vs target `addiu $s3,$s7,0x20`.
 * See evidence.md s33.
 */
/* s32 ADDENDUM (synthesis, 2026-08-27). Re-measured this session: THIS BODY IS
 * STILL THE BEST FORM at sandbox --disable all == 1, 132 build / 132 target insns
 * (HEAD = 27). s32 foreclosed the only competing chassis family: any loop-note
 * spelling of loop1 makes loop.c strength-reduce `stptr`, so block 0 emits the giv
 * base `addiu $s3,$v0,0x134` alone (biv eliminated) or biv + giv one insn over, while
 * target emits `addiu $s3,$v0,0xFC` alone -- so the goto chassis THIS body uses is the
 * original's, confirmed from both directions (s30's REG_EQUIV argument and s32's giv
 * argument). loop2 as a real loop costs +2 insns on this same chassis. See evidence.md
 * E-s32-1/E-s32-2. The `stptr = base; stptr += 0xFC;` F1 chain-extender annotation
 * requirement noted in the s11 correction below is UNCHANGED and still blocks shipping.
 */
/* func_80041188 / hirahira_w_ctrl - s7 (rederive) candidate. sandbox
 * --disable all == 1, 132/132 insns, frame 72 == target 0x48.
 *
 * NOTE (s16): the header sentence below dates from s7 and is WRONG about one
 * statement -- this body carries ONE sanctioned-family F1 construct, the
 * `stptr = base; stptr += 0xFC;` chain-extender, now fully annotated at its
 * site. Everything else in the sentence still holds.
 *
 * ORDINARY C. No register pin, no inline asm, no volatile, no dead code, no
 * unused local, NO /* FAKE */ construct, no variable reuse, no do-while(0)
 * wrap. Every local is once-declared, written where a human would write it,
 * and read for its real value. This REPLACES the s3/s5/s6 "merged stptr"
 * chassis (loop2's pointer reusing the dead loop1 walker via an out2 read),
 * which scored the same 1 but only with a FAKE variable-reuse annotation.
 *
 * WHAT s7 CHANGED (the rederive). Every ledger session up to s6 assumed the
 * committed rule-era shape: ONE `out2` local, reused/re-initialised before
 * loop2. s7 read the target instead and found that target holds a4+0x20 in
 * TWO DIFFERENT callee-saved registers - $s6 across loop1, $s3 across loop2 -
 * which GCC 2.7.2 cannot do with one pseudo (it has no live-range splitting).
 * Therefore the original source has TWO SEPARATE LOCALS, and the second one
 * (`out3`) is defined in the between-loops block. Measured corollary (s7):
 * GCC does NOT hoist `a4 + 8` out of these goto-loops at all (no loop notes),
 * so both `addiu ...,$s7,0x20` in target are source-level statements, not
 * LICM output - see rejected/inline-a4plus8-no-licm-hoist.c.
 *
 * WHY `out3 = out2;` AND NOT `out3 = a4 + 8;`. This is the entire residual.
 * The GCC 2.7.2 global.c allocno priority is
 *     pri = floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000
 * and the four contested callee-saved seats need the order
 *     tbl > out2 > a4 > a3   (-> $s5, $s6, $s7, $fp).
 * Measured this session from .lreg/.greg with this exact body:
 *     73/74 a1,a2   16 refs / 99  = 6464.6   -> $s1,$s2
 *     88    stptr    7 refs / 41  = 3414.6   -> $s3 (loop1 only)
 *     91    stptr2   6 refs / 48  = 2553.2   -> $s0
 *     78    i        8 refs / 97  = 2474.2   -> $s4
 *     79    tbl      4 refs / 47  = 1702.1   -> $s5
 *     86    out2     4 refs / 47  = 1702.1   -> $s6   (EXACT TIE with tbl,
 *                                                      broken by allocno
 *                                                      number 79 < 86)
 *     77    a4       6 refs / 95  = 1263.2   -> $s7
 *     75    a3       4 refs / 99  =  808.1   -> $fp
 *     87    out3     3 refs / 47  =  638.3   -> $s3 (loop2, no conflict)
 * With `out3 = (s32 *)((u8 *)a4 + 0x20);` instead, out2 drops to 3 refs and
 * live 42 (dead at loop1's exit) -> pri 714.3, which lands it BELOW a4 and a3;
 * the whole {out2, a4, a3} triple then permutes to {$fp, $s6, $s7} and the
 * score goes 1 -> 15 (rejected/two-locals-out3-from-a4-seat-permutation.c).
 * out2's 4th reference AND its live length 47 both come from exactly one
 * thing: being read in the between-loops block, as its LAST statement.
 * Position is load-bearing - moving `out3 = out2;` earlier in that block
 * shortens out2's live range and re-breaks the tie the wrong way
 * (measured: last=1, 3rd=10, 2nd=11, 1st/0th=12).
 *
 * RESIDUAL (slot 72, the ONLY non-formatting diff in 132 insns):
 *     OURS  move  $s3, $s6            TGT  addiu $s3, $s7, 0x20
 * i.e. target re-derives loop2's pointer from a4 while STILL giving out2 a
 * 4th flow-counted reference. Since flow.c fixes reg_n_refs before combine
 * runs, the original's 4th out2 reference must be an insn in the
 * between-loops block that combine deleted. Finding a byte-free spelling of
 * that reference is the whole remaining problem - see hypotheses.md s7.
 *
 * s9 ADDENDUM (structural, 2026-08-23). This body still scores 1 and, newly
 * measured this session with a mechanical seat checker, it already holds
 * ALL-TARGET callee-saved seats - so its single residual insn is a FORM
 * difference, not an allocation difference. s9 also produced the first
 * all-target-seats form on the I2 chassis (rejected/m1-i2-symk-ilate-...c,
 * sandbox 4) whose entire residual is the preamble emission ORDER, and proved
 * that order and tbl's live length are the same variable (sched.c
 * rank_for_schedule's INSN_LUID fallback + flow.c:1685). See evidence.md s9.
 *
 * s10 ADDENDUM (structural, 2026-08-23). Still floor 1, still ALL-TARGET seats.
 * s10 closed the whole "make i outrank tbl in the i-FIRST order" axis by a
 * measured identity: with i defined before tbl, reg_live_length(i) is always
 * reg_live_length(tbl) + 49, so an 8-reference i needs tbl live >= 50, while
 * sixteen distinct block-0 mutations bound tbl's live length at 48. The escape
 * is a NINTH flow-counted reference to i: measured, it gives ALL-TARGET seats
 * on the i-first + separated-restore chassis at sandbox 3 / 133 insns, one insn
 * over target, that insn being the reference itself. A byte-free ninth
 * reference (surviving cse1 into flow, deleted by combine) is distance 0.
 * See evidence.md s10 and hypotheses.md s10 frontier 1.
 *
 * s11 ADDENDUM (escalation, 2026-08-23). Still floor 1 / 132 insns / ALL-TARGET
 * seats (re-measured this session at the start and again at the end). s11 closed
 * the block-0 side of the residual, the mirror of s8's between-block kill:
 * on the only chassis that emits target's `addiu $s3,$s7,0x20` (out3 taken from
 * pa4), out2's live length CANNOT exceed 43 - measured with its definition made
 * the first statement of block 0 - while target's seating needs out2's priority
 * inside (1458, 1702), i.e. live 47..55 at 4 references. The window is empty.
 * The F1 combine-foldable chain-extender (the last unspent sanctioned byte-free
 * reg_n_refs family) does buy out2 a 4th reference in block 0, but every spelling
 * must subtract a pa4-derived term to cancel, which lifts pa4 (8 refs / 2424)
 * above out2 (1739), AND it materialises two insns (134 vs 132), failing F1's own
 * zero-bytes prerequisite. See evidence.md E-s11-1/E-s11-2.
 *
 * s11 CORRECTION to this header's own claims. `stptr = base; stptr += 0xFC;`
 * IS a combine-foldable chain-extender: target emits one `addiu $s3,$v0,0xFC`,
 * the split is byte-neutral (132 insns either way), and its only surviving effect
 * is stptr's reg_n_refs count - un-splitting it drops stptr from 7/41=3414 to
 * 5/41=2439, below stptr2, and the floor from 1 to 15. So the "ORDINARY C ... NO
 * FAKE construct" claim above is WRONG for that one statement. A future
 * candidate must either carry a FAKE: F1 annotation on it (family:
 * .claude/rules/dead-store-fake-exception.md:32) or replace it; shipped
 * un-annotated it is a layer-1 FAIL. See evidence.md E-s11-3.
 *
 * s12 ADDENDUM (escalation, 2026-08-24). Re-measured at the start of s12 on the
 * current chassis: STILL sandbox 1 / 132 of 132 insns / ALL-TARGET seats. s12
 * retired the last carried structural frontier (a single PsyQ `MATRIX *` local
 * addressing both matrices as &m[0]/&m[1]) by measurement: `&m[1]` folds to the
 * SAME `(plus (reg) (const_int 32))` RTL as `((u8 *)pa4) + 0x20`, so cse1
 * collapses it to one pseudo and the build loses seven insns (125 vs 132,
 * sandbox 43) - it is the single-local chassis s7 already disproved, not a new
 * one. s12 also closed the out2 priority window from BELOW: defining out2 inside
 * loop1 gives 4 refs / live 21 = 3809, more than double the top of the required
 * (1263, 1702) band, and the in-loop definition position is inert across four
 * spellings (sandbox 23). With s11's ceiling this makes the enumeration complete
 * - see evidence.md E-s12-3: target's out2 priority is reachable ONLY with a
 * reference to out2 in the between-loops block, and that reference IS the
 * residual insn. The remaining unspent item is cosmetic-but-blocking: the
 * `stptr = base; stptr += 0xFC;` chain-extender above still needs either a
 * FAKE-free replacement or its annotation before this body could ship.
 *
 * s13 ADDENDUM (permuter, 2026-08-24). Floor re-measured 1 / 132 of 132 at the
 * start and again at the end of s13; this body is unchanged. s13 spent four
 * permuter campaigns (~54k iterations) and killed three chassis-level
 * hypotheses. The load-bearing one: on the floor-9 chassis (this form's s4
 * ancestor minus the banned do-while(0) wrap) the stock permuter reaches
 * distance 0 in 2,588 iterations and its ONLY delta is the banned wrap
 * re-derived verbatim, while the same chassis with `perm_ins_block = 0` in
 * settings.toml runs 22,418 iterations and finds NOTHING better than its base.
 * The wrap is therefore the unique route to zero inside the permuter's
 * transform set, not one option among several. The two-locals
 * `out3 = pa4 + 0x20` chassis was permuted for the first time and drains into
 * that same dead basin (its one find is a same-value re-store of out2, honest
 * sandbox 9). See evidence.md s13. The `stptr = base; stptr += 0xFC;`
 * chain-extender below is STILL the one unresolved artifact defect.
  *
 * s14 ADDENDUM (synthesis, 2026-08-24). Re-measured at the start of s14 on this exact
 * body: STILL sandbox 1 / 132 of 132 insns / ALL-TARGET seats. s14 decomposed the
 * residual into TWO independent allocno-priority requirements instead of one:
 *   (A) stptr > i, and (B) tbl > out2 > pa4 > a3,
 * because the six contested allocnos take $s3,$s4,$s5,$s6,$s7,$fp in DESCENDING
 * priority order (validated in both directions, E-s14-1). The `stptr = base;
 * stptr += 0xFC;` chain-extender flagged by s11 exists SOLELY to satisfy (A), and
 * s14 found an ordinary-C replacement for it: moving `i = 0x12;` to be the FIRST
 * statement of the between-loops block lengthens reg_live_length(i) 97 -> 99 and drops
 * i's priority 2474 -> 2424, below an UN-SPLIT stptr's 2439. That form
 * (memory/grind/func_80041188/alt_fakefree_floor3_s14.c) reaches ALL-TARGET seats with
 * ZERO FAKE constructs and scores 3 at 132/132 — the extra 2 over this body is a sched1
 * INSN_LUID emission-order cost (`li $s4,18` lands at slot 67 instead of 69) that three
 * decoupling attempts could not remove (E-s14-3). So the shipping choice is now explicit:
 * THIS body is floor 1 but carries the un-annotated F1 chain-extender s11 proved would
 * FAIL layer 1; alt_fakefree_floor3_s14.c is floor 3 but is honestly committable.
 * s14 also corrected s12's band arithmetic — on the only chassis that emits target's
 * slot-71 insn, pa4 is 7 refs / 95 = 1473 (not 1263), so (B) needs out2 in (1473, 1702)
 * — and named the pass that forbids the obvious escape: with out2's definition in the
 * same cse extended basic block as the between-loops block, cse1 rewrites
 * `out3 = pa4 + 0x20` into `out3 = out2` itself (E-s14-5, read out of fd.flow insn 164),
 * so target's `addiu $s3,$s7,0x20` REQUIRES out2 defined in block 0.
 *
 * s15 ADDENDUM (solver, 2026-08-24). This body was NOT re-measured this session
 * (s14 measured it at 1); the s14 honest alternative was, and still scores 3.
 * s15 ran tools/sched_solver and tools/ra_solver on this function for the first
 * time - they were structurally blocked until now because the instrumented cc1
 * SEGFAULTS on func_80040CB8 in this TU and never reaches func_80041188
 * (workaround + parity proof: evidence.md E-s15-0). Two results bear on this
 * body's one remaining defect, the un-annotated F1 chain-extender below:
 *   (1) The +2 emission-order cost of the FAKE-free s14 alternative has exactly
 *       ONE scheduler vector - move `i = 0x12;` to sit immediately before
 *       `stptr2 = ...` - and spelling it also sets reg_live_length(i) = 97, so
 *       requirement (A) breaks and the score goes 3 -> 15. Emission order and
 *       i's live length are the same variable (E-s15-1).
 *   (2) That measured form ("P1", memory/grind/func_80041188/
 *       alt_P1_honest_ipos3_s15.c) is ONE single-atom perturbation away from
 *       target's COMPLETE callee-saved disposition, and the only atoms that
 *       reach it are `stptr` live 41->40 or `stptr` refs 5->6 (E-s15-4) - i.e.
 *       precisely what the chain-extender below fakes. An honest sixth stptr
 *       reference, or one insn off stptr's live range, replaces this annotation
 *       with ordinary C at the same floor. That is s16's frontier item 1.
 *
 * s17 ADDENDUM (forensics, 2026-08-24). Re-measured at the START of s17: this body
 * still scores 1 at 132 of 132 insns. s17 NAMED the compiler decision the FAKE
 * annotation below depends on and turned it into a predictive law: a chain-extender
 * `X = Y; X += K;` lifts reg_n_refs(X) by 2 if and only if cse1's make_regs_eqv
 * (cse.c:844-857) makes X the quantity's canonical register, which requires X's last
 * use to fall after Y's last use; otherwise canon_reg (cse.c:2569) substitutes Y and
 * flow deletes the dead copy uncounted. Validated in both directions this session
 * (rejected/base-used-after-loop1-folds-stptr-chain-refs-7to5.c drops stptr 7 -> 5 refs
 * by moving `saved = base + 0x94;` alone). Two consequences for this body: (1) s16's
 * "the F1 chain-extender is not shape-portable" is superseded -- it IS portable, and
 * the law says exactly where; (2) the out2 reference-lift axis is now KILLED BY
 * DERIVATION rather than by sampling (evidence.md E-s17-3), and s16 frontier item 3
 * is killed too (reload creates exactly one insn in the loop1 region and it lands in
 * the between-loops block, evidence.md E-s17-4). The remaining defect is unchanged:
 * the stptr chain-extender below is a FAKE-annotated F1 construct, and s14's
 * alt_fakefree_floor3_s14.c remains the honest-but-floor-3 alternative.
 *
 * s19 ADDENDUM (rederive, 2026-08-25). Re-measured at the START of s19 with this exact
 * body: STILL sandbox 1 / 132 of 132 insns. s19 produced a SECOND honest alternative on
 * a new chassis -- alt_W4_realloop_giv_honest_s19.c, sandbox 3, ALL-TARGET seats, zero
 * FAKE constructs -- by writing s18's array layout literally: loop1 as a real
 * `do { } while (i < 0x12);` with the entry address inline as `ents + i * 0x68`. There
 * loop.c's strength reduction hands the walking pointer 9 refs / live 40 = 6750 for
 * free, i.e. the FAKE F1 chain-extender below has an honest structural replacement.
 * What keeps that chassis 2 insns behind THIS one is not allocation but loop.c hoisting
 * the invariant `li 2` out of the real loop (evidence.md E-s19-3, killed by derivation
 * from loop.c:686-700 + loop.c:1631), after which reload rematerialises it into $t0
 * where target has $v0. On that chassis the whole out3 lock reduces to ONE missing
 * flow-counted reference on out2 (E-s19-4/E-s19-5).
 *
 * s20 ADDENDUM (structural, 2026-08-25). Re-measured at the START of s20 with this
 * exact body: STILL sandbox 1 / 132 of 132 insns. s20 worked the GOTO chassis (this
 * one) and CLOSED it for target's block-2 spelling: out2's required 4th flow-counted
 * reference has exactly three possible sites and all three are now dead (evidence.md
 * E-s20-5). Two results bear on the FAKE annotation below. (1) The do-while(0) wrap
 * family is NOT a cheaper substitute for it here: Z3 reproduces this body's exact seat
 * table with wraps at 132 insns and still scores 8, because the LOOP notes displace
 * `tbl++` and `sw $t0,0x18($sp)` in sched1's emission order (E-s20-4). (2) The one axis
 * that got further than this body's residual is the parameter split (Y2b/Y4,
 * rejected/split-pa4-copy-*): ALL-TARGET seats WITH target's `addiu $s3,$s7,0x20`, at
 * 133 insns, the single extra insn being `addu $s7,$v0,$zero` — so a spelling that
 * separates a4's and pa4's reference sets without materialising that copy is distance 0.
 *
 * s21 ADDENDUM (structural, 2026-08-25). Re-measured at the START of s21 with this
 * exact body: STILL sandbox 1 / 132 of 132 insns. This body is UNCHANGED and remains
 * the best form. s21's result is a NEW CHASSIS, not an improvement to this one:
 * deleting the `pa4` local and writing the parameter `a4` at every use site reopens
 * E-s20-5's block-0 site, because the block-0 do-while(0) wrap's unavoidable second
 * +1 then lands on a4 (live 190, 8 refs = 1263) instead of on pa4 (live 95, 8 refs =
 * 2526). On that chassis form Q1 (memory/grind/func_80041188/alt_Q1_pa4free_alltarget_s21.c)
 * holds ALL-TARGET seats WITH target's block-2 `addiu $s3,$s7,0x20` at 132 insns and
 * scores 7, its entire residual being sched1 emission order in block 0. Two warnings
 * for anyone editing THIS body: (1) the `pa4` local here is chassis-defining, not
 * cosmetic -- deleting it drops a4 to 6/190 = 631, below a3's 808, and the two swap
 * $s7/$fp (sandbox 1 -> 13, evidence.md E-s21-2); (2) a dead store `out3 = out2;` is
 * byte-free but flow-INVISIBLE (E-s21-5), so it cannot buy out2 a reference. The
 * `stptr = base; stptr += 0xFC;` chain-extender below is still the one unresolved
 * artifact defect and still needs its FAKE annotation (present) or a replacement.
 */
/* s23 ADDENDUM (synthesis, 2026-08-25). Re-measured at the start and end of s23:
 * STILL sandbox 1 / 132 of 132 insns / ALL-TARGET seats. This body remains the floor.
 * s23 did not change it; s23 changed what the next session may spend measurements on.
 *
 * THE CSE-EBB LAW (evidence.md E-s23-1). cse.c builds extended basic blocks by
 * following single-predecessor successors. loop1's head has TWO predecessors, so the
 * EBB that starts in block 0 terminates there and NOTHING computed in block 0 is in
 * cse1's table at block 2; block 2 has ONE predecessor, so loop1's EBB extends
 * through it and EVERYTHING computed inside loop1 IS. Measured both ways: with out2's
 * definition moved into loop1 (D5/D6) or merely recomputed there (G1), cse1 rewrites
 * block 2's `out3 = (u8 *)pa4 + 0x20` into `out3 = out2` -- i.e. it MANUFACTURES this
 * body's residual `move $s3,$s6`. With the definition only in block 0, it does not.
 * Consequence: out2's missing 4th flow-counted reference may NOT be sited in block 2
 * (generalising E-s20-5 from enumeration to mechanism) and may NOT be an in-loop1
 * DEFINITION. It is a definition law, not a reference law, so an in-loop1 USE is
 * still open -- and that is now the single unprobed quadrant.
 *
 * TWO DIALS CLOSED. (1) a3 is rigid: 4 refs / live 99 / pri 808 in ten measured forms,
 * including two that move statements past its last use and two that give it the same
 * homing-copy local that halves a4's live length. (2) On the pa4-free chassis the
 * block-0 position of out2's definition cannot seat it above a4: out2 outranks a4 iff
 * L(a4) > 4.667*L(out2), and the whole monotone sweep tops out at 4.585.
 *
 * WHY out2 MUST HAVE FOUR REFERENCES (the closure, from both sides). At 3 references
 * out2 needs live <= 37 to clear a3's 808 and live <= 23 to clear a4 on the pa4
 * chassis; block-0 definitions floor at 41, and the only way below 41 is an in-loop1
 * definition, which is dead twice over (cse fold + the addiu lands inside loop1 where
 * target emits it in block 0). So this body's `out3 = out2;` is not one arbitrary
 * choice among many -- it is the only measured delivery of a requirement that is now
 * proven necessary. The open question is only whether a byte-free spelling of that
 * same reference exists as an in-loop1 use that combine absorbs.
 */
/* s28 ADDENDUM (rederive, 2026-08-27). Re-measured this session: this body is
 * STILL sandbox 1 at 132 of 132 insns (HEAD measures 27, alt_V15a measures 15),
 * so it remains the floor and the shipping chassis. s28 did not change it; s28
 * closed the last open route AROUND it.
 *
 * THE BYTE-FREE REFERENCE IS GONE AS AN IDEA (evidence.md E-s28-1/E-s28-2).
 * (1) Expression-rooted block-0 splits of out2 (`out2 = pa4 + 0x28; out2 -= 8;`
 * and two other constant spellings) are folded by cse.c's fold_rtx REASSOCIATION,
 * not by the canon_reg machinery s22's iff-law describes: red.i.cse rewrites
 * `(plus 86 -8)` into `(plus 77 32)` using reg 86's known value, the first insn
 * dies, flow deletes it, and the reference is never counted. Both horns of the F1
 * chain-extender class are now closed for out2.
 * (2) Enumerated from the red.i.flow -> red.i.combine insn-id difference, combine
 * deletes exactly two shapes here: a reg-reg copy with a single-use destination,
 * and `(set p (plus R c))` merged into its single use. Since flow fixes reg_n_refs
 * and combine is the only later pass that deletes insns, a byte-free counted
 * reference must live in one of those two shapes -- and target's bytes contain no
 * `addiu $aN,$s6,c` and no second $s6 copy to host either one. So out2's fourth
 * reference must COST an instruction (s27's D family, 133 insns at sandbox 2).
 *
 * THE HOSTING RULE, which is the useful thing to carry forward: every flow-counted
 * reference a local has must be visible in target's bytes as an emitted insn, a
 * single-use copy, or an `X + constant` argument/address expression. stptr's 7 and
 * stptr2's 6 decompose that way honestly (`addiu $a3,$s3,0x38`, `addiu $a3,$s0,0x4C`,
 * `addiu $a3,$s0,0x38`). out2's required 4-or-5 does not -- which is the strongest
 * evidence yet that the two-locals goto chassis is not the original's shape, and the
 * next session should apply the same count to a1/a2, i and tbl before assuming the
 * allocno model is right about them.
 */
/* s29 ADDENDUM (rederive, 2026-08-27). Re-measured at the START of s29: this body is
 * STILL sandbox 1 at 132 of 132 insns, and it remains the floor. s29 changed nothing in it.
 * What s29 changed is the STATUS of the chassis it sits on. The target-hosted reference
 * census (evidence.md E-s29-1) decomposed every one of target's callee-saved registers into
 * flow-countable references straight out of asm/funcs/func_80041188.s, and the model is exact
 * everywhere: a1 16, a2 16, stptr 5, i 8, tbl 4, stptr2 6, a3 4, pa4 7, out2 3, out3 3. So
 * s28's frontier item 1 -- "some allocno is mis-counted" -- is KILLED, and this body's residual
 * gets a sharper name: it is a TRANSFER, not a shortfall. Target spends its 7th pa4 reference on
 * `out3 = pa4 + 0x20` (asm line 74); we spend it on `out3 = out2`, which moves the reference to
 * out2 and emits `move $s3,$s6`.
 *
 * The load-bearing consequence is E-s29-2. Feed target's OWN hosted counts into the validated
 * priority model on the only chassis whose pseudo set matches target's (V15a): pa4 7/95 = 1473,
 * a3 4/99 = 808, out2 3/42 = 714 -- which seats pa4 in $s6, a3 in $s7 and out2 in $fp, the
 * opposite of target's bytes. Since E-s28-2 already excluded both combine-deletable shapes that
 * could host a hidden fourth out2 reference, the original's source is NOT this pseudo set. The
 * two-locals goto chassis is therefore bounded above at floor 1 by derivation, and the next
 * sessions should spend their measurements on a different pseudo set (hypotheses.md s29 item 2)
 * or on the one untested byte-free live-length class (item 1), not on respelling this one.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base + 0xFC;
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x69;
    stptr -= 1;
    /* FAKE: `stptr += 0x68;` written as the two live arithmetic statements
       `stptr += 0x69; stptr -= 1;` (owner-ALLOWED split/redundant-arithmetic
       class, ruling 2026-08-27, docs/grind/decisions.md:14739 -- semantically
       true C on a live loop-carried variable, net effect identical). Its only
       effect is lifting stptr's reg_n_refs 5 -> 7, i.e. allocno priority
       2439 -> 3414, which seats stptr in $s3 ABOVE `i` (2474); target emits the
       single `addiu $s3,$s3,0x68` in the loop-back delay slot either way
       (measured s33: 132 build insns, sandbox 1, allocno table identical to the
       superseded F1 spelling).
       mechanism: flow.c:2081 fixes reg_n_refs during life analysis, BEFORE
       combine runs; combine.c then folds the `+0x69 / -1` pair back into the one
       addiu (E-s28-2 shape B, the same deletion that makes the tbl and i split
       increments byte-free), so the extra reference is counted by flow, spent by
       global.c's allocno_compare (global.c:635-656), and costs no byte. sched1's
       post-combine live-length recount (sched.c:5106) leaves stptr at 41.
       lever-exhaustion: memory/grind/func_80041188/hypotheses.md s7..s32 --
       stptr live 41 -> 40 killed in s16 (E-s16-2); every non-chain reference-lift
       spelling for stptr measured and folded (E-s16-4); s14 Y1/Y2/Y3 killed every
       statement-order route to stptr > i from the other side; s29's i live-length
       ladder (E-s29-3) shows the only honest alternative costs two sched1
       emission-order diffs (the FAKE-free H3 form, sandbox 3). */
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = out2;
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, pa4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(pa4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
