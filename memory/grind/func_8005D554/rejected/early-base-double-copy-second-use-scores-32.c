/* func_8005D554 -- BEST CLEAN FORM: honest sandbox distance 6 at 176/176 instructions,
 * frame 120 == target, register allocation byte-identical to the target (H12).
 * Measured s3b 2026-09-08 on HEAD main @ 4e7ad872 with `sandbox func_8005D554 --disable all`.
 *
 * This body REPLACES the previous candidate.c, which reached distance 0 but did so with the
 * fresh multi-write carriers `nv`/`nw` -- a construct the Judge FAILed on 2026-09-08
 * (docs/grind/decisions.md, "2026-09-08 22:31 -- func_8005D554 -- ruling ... FAIL").  That body
 * is banked verbatim at rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c and
 * MUST NOT be resubmitted: the driver keys review verdicts by body, so it is rejected without
 * review.  The Judge's binding constraint: no fresh (invented) local may be written more than
 * once to act as a staging carrier for the a2-site base `(s32)r4 - K`, under any name.
 *
 * The whole remaining residual is 6 = two identical 3-insn rotations, one per loop half.
 * Ours emits [addiu a2,s4,-K][addiu a0,sp,16][lw v1,gp][move a1,zero]; the target emits
 * [addiu a0,sp,16][addu a1,zero,zero][lw v1,gp][addiu a2,s4,-K]
 * (asm/funcs/func_8005D554.s:4DEB4-4DEC0).  Everything else in the function matches.
 *
 * s4 (enumerate, 2026-09-08, HEAD main @ f95f6b8a): floor RE-MEASURED at 6/176 on this body,
 * and 1,224 complete spellings were swept around it (tmp/grind/func_8005D554/enum{1,2,3},
 * histograms in tmp/grind/func_8005D554/s4/sweep{1,2,3}.json).  Nothing scores below 6.  The
 * a2-arithmetic spelling space is quantized to {6, 15}; the 800-form cross product of every
 * neutral axis (a0/a2 arithmetic forms x declaration scope x zero1C position x tail-store
 * moves) is 800/800 at exactly 6/176.  Do not re-sweep those axes: they are byte-level no-ops.
 *
 * s5 (synthesis, 2026-09-09, HEAD main @ 4f43e5cf): floor RE-MEASURED at 6/176 on this body.
 * The pass attribution is now DONE and it corrects s3 and s4.  The Judge-FAILed nv/nw body did
 * NOT win by reg_n_sets (its carrier is SINGLE-set after combine) and did NOT win by out-LUIDing
 * the argument loads (its base insn is born BEFORE the third rand call, i.e. with a LOWER LUID,
 * and sched1 SINKS it).  The real chain: a source-multi-set carrier is not a loop.c movable, so
 * the loop-invariant (s32)r4 - K is not hoisted; combine.c then erases the extra set, leaving a
 * single-set single-use pseudo that dies at the add; sched1 sinks its insn to just after the
 * D_800A3418 load; local-alloc seats it in caller-saved $a2.  Sixteen new spellings (s5/enum*)
 * show the direct-store vs accumulate shape is an exact byte-level no-op, that every early-birth
 * form costs +2 (LICM hoist for a fresh single-set carrier, a tenth callee-saved seat for an
 * existing local), and that freeing a pointer local buys the count back but leaves the base in a
 * callee-saved register -- structurally further from the target than this body.  Do not re-derive
 * the LUID framing or the reg_n_sets framing; see hypotheses.md H23-H26.
 *
 * s5b (synthesis, 2026-09-09, HEAD main @ 4f43e5cf): floor RE-MEASURED at 6/176 on this body.
 * 55 further spellings measured (tmp/grind/func_8005D554/s5b/, sweep{1..5}.json) retire all
 * three s5 frontier items: the 4-local per-half offset chassis is a byte-level no-op at the
 * floor (6/176) and strictly worse when early-birthed (47/178); a2-base carrier IDENTITY is
 * invisible in the accumulate shape (shared / fresh-shared / fresh-per-half all 6/176) and only
 * decides 176 vs 178 in the direct-store shape; the loop chassis is measured out (do-while ==
 * guard-duplicated while == for-break, all 6/176; label+goto 37/175); parameter and dead-local
 * carriers for the permuter s zero10-restage trick cost 1-3 insns (25-61); a zero constant
 * holder for the second call argument costs 2-3 insns (16-30). See hypotheses.md H27-H31 and
 * frontier R1-R3.  Do NOT re-sweep loop-chassis, carrier-identity or per-half-local axes.
 *
 * s6 (solver, 2026-09-09, HEAD main @ 07aadcc3): floor RE-MEASURED at 6/176 on this body.  The
 * residual is now TYPED: inverse_compose.py classify reports SCHED (176 vs 176, registers
 * identical, 6 slots reordered) -- there is no PRE-RA or RA component left at all.  A depth-1
 * perturb.py search over ALL 12,558 single atoms of sched1 block 6 finds only LUID moves of the
 * `addiu a2,s4,-K` insn (uid 211) to a slot at or after the func_80073728 call; and all 420
 * dependence-legal permutations of the eight half-1 source statements, replayed through the
 * exact sched1+sched2 funnel (self-checked against the real dump), miss the target order.  So
 * statement ORDER is closed with a predicate (sched.c:2464, the INSN_LUID tie-break); the next
 * lever has to change the dependence graph or the priorities, not the order.  Confirmed against
 * the compiler: the model-predicted best statement placement measures 6/176 like the control.
 * See hypotheses.md s6 and evidence.md s6.  Do NOT re-sweep statement placements of the a2 base.
 *
 * s7 (forensics, 2026-09-09, HEAD main @ 8a6f96b1): floor RE-MEASURED at 6/176 on this body.  The
 * residual was measured INSIDE the compiler with the instrumented cc1 (tools/gcc-2.7.2/cc1 -- NOT
 * build/cc1; self-checked byte-identical assembly).  rank_for_schedule has exactly three criteria
 * and s7 measured all three: (1) INSN_PRIORITY is 3/3/3/3 for the lw, the a2 base and the two
 * argument-setup insns in half 1 and 6/6/6/6 in half 2 -- a structural tie, because a call forces
 * an anti-dep from every later register set and insn_cost is 2 only for a load's consumer
 * (sched.c:1497); (2) the last-scheduled CLASS is 3 vs 3 in all 203 window comparisons
 * (sched.c:2429); (3) only INSN_LUID is left, and the a2 base's LUID (31) is bounded above by the
 * s.zero1C store's, while the argument setup sits at 42/43.  Sharing a local to manufacture an
 * anti-dep is measured inert: half 2's base insn 316 ALREADY carries those anti-deps and they are
 * dominated by the call's.  The p_b2e0 object-model axis (the last untried structural axis) is
 * also measured out: TU-local struct pointer scores 10/176 (multiset preserved) and 43/172 (full).
 * Do NOT re-derive the priority or class framing; see hypotheses.md s7 and evidence.md s7.
 *
 * s8 (rederive, 2026-09-09, HEAD main @ 4297dfd2): floor RE-MEASURED at 6/176 on this body.
 * The kill re-audit passed: fake_ablate finds NO FAKE-annotated construct in this candidate (so no
 * banked lever was ever measured behind a carrier), and the five closest banked forms all
 * reproduce their recorded scores exactly on this chassis.  A fresh m2c decompile was run and
 * measured: its shape scores 75/176, and its independent THREE-ARGUMENT reading of the callee
 * (func_80073728(&s, 0, a2)) is byte-INERT here -- 6/176 with the third argument and 6/176
 * without -- because expand_call evaluates argument expressions into pseudos before emitting
 * the hard-register moves, so an already-computed value adds no insn and no birth position.
 * The matched sibling func_8005D46C (same file, same S46C, same callee) is now SPENT: its
 * field-store order (zero1C before zero18) transplants at 28/178.  THE NEW RESULT: s4's claim
 * that the argument-setup bar is unbeatable is true only of STATEMENT ORDER.  Giving the a2
 * base insn a load producer (u32 rb[2] holding both random bases) moves `addiu a2,...,-K` from
 * BEFORE the argument setup to six slots AFTER it -- the first time in eight sessions that insn
 * has been emitted late -- at a cost of the load (+2 insns, 32/178) and a six-slot overshoot.
 * The lever is the dependence graph (sched.c:1497 load-consumer cost 2), not INSN_LUID.
 * See hypotheses.md s8 and evidence.md s8.  Do NOT re-derive the m2c shape or the sibling
 * field order; do NOT re-sweep a2-sum re-spellings (still quantized to {6,15}).
 *
 * s9 (forensics, 2026-09-09, HEAD main @ 7dcbdc8e): floor RE-MEASURED at 6/176 on this body.
 * FIRST: this file did not COMPILE as banked by s8 -- it carried a FAKE annotation with its
 * comment delimiters inside this block comment, which closed the comment early (text1b.c:2769:
 * parse error).  The sandbox reports that as "func_8005D554 not found in ...text1b.o"; if you
 * ever see that message, hand-compile the TU first.  Never write comment delimiters inside this
 * header.  SECOND, and the real result: the instrumented cc1's SCHEDDBG hooks (BB2_SCHED_DEBUG=1)
 * were captured for the first time, and they reduce the entire 6-point residual to ONE ready-list
 * pick.  GCC 2.7.2's schedule_block is a BACKWARD list scheduler (sched.c:4036-4038: selected
 * first == emitted last).  In half 1, at sched1 clock 64 the ready list is
 * [242(p=3,l=43) 240(p=3,l=42) 211(p=3,l=31)] -- the two expand_call argument moves and the a2
 * base.  Priorities tie, the last-scheduled class ties, potential_hazard ties structurally, so
 * rank_for_schedule falls to INSN_LUID (sched.c:2462) and takes the highest, 242.  If 211 were
 * taken there, the rest follows automatically (the one-cycle-blocked lw 205 re-enters with
 * LAUNCH_PRIORITY, then 242, then 240) and the emission is 240,242,205,211 == the target.  So the
 * whole residual is the single boolean INSN_LUID(a2 base) > INSN_LUID(a1 argument move).  sched2
 * is a NO-OP here (its LUIDs already equal our emitted order).  Half 2 is structurally IDENTICAL
 * (316 l=76 vs 346/348 l=88/89, all p=6) -- the halves are NOT separable, and s8's premise that
 * only half 2's window holds a load is refuted (both windows hold exactly one SELBLOCKed lw).
 * Combine creates exactly ONE insn in this whole function, (use (reg 119)) at the top, so there is
 * no combine-manufactured a2 base to place late.  And every INSN_PRIORITY lever OVERSHOOTS by
 * construction: the three struct stores also sit at priority 3, so a load producer (3+2-1=4,
 * sched.c:1497) or a single-assignment dest (birthing_insn_p, sched.c:2505, raises to
 * max_priority) lifts the a2 base above them too -- exactly s8's six-slot overshoot.  Do NOT
 * re-attempt priority levers, half-asymmetric forms, or combine-placement forms.  The one untested
 * creation site left is RELOAD, which runs after sched1 and whose insertions get fresh LUIDs for
 * sched2.  See hypotheses.md s9 and evidence.md s9.
 *
 * s10 (forensics, 2026-09-09, HEAD main @ c09c5da8): floor RE-MEASURED at 6/176 on this body;
 * fake_ablate again finds no FAKE construct here.  The clock-64 pick that IS the whole residual
 * is now closed on ALL FOUR of its deciding terms, three of them with predicates:
 * priority (s9), last-scheduled CLASS (sched.c:2429 -- insn_cost clamps to 1 for any insn with
 * no function unit, so an arith producer forces class 3 under EVERY last_scheduled_insn),
 * potential_hazard (sched.c:1338 -- MIPS type arith matches no define_function_unit in
 * mips.md:153-260, so insn_unit is -1 and the hazard is 0; this is also exactly why the three
 * struct stores, unit "memory" with maxb=3, beat the higher-LUID argument moves at clocks
 * 61-63), and INSN_LUID (calls.c:1880 -- expand_call emits the hard-register argument moves
 * after every argument expression is evaluated and after every preceding statement expands, so
 * they hold the two highest pre-call LUIDs, 42 and 43, against a measured ceiling of 41 for
 * everything else in the window).  Measured confirmation: form v1 moved both halves' a2
 * statements to the LAST possible pre-call position (after the zero10/one14/ret stores,
 * immediately before the zero1C store); the a2 base's LUID rose 31 to 34, the argument moves
 * stayed at 42/43, and the score stayed 6/176 with the rotation unchanged.  Reload is also
 * measured out: greg shows 18/18 pseudos allocated, zero reload insertions in either window.
 * Do NOT re-attempt hazard, class, or LUID levers on the a2 expression, and do NOT re-try the
 * cse-operand-order route -- cse moves no insn across statements, so it cannot beat a bound
 * that statement placement itself cannot reach.  See hypotheses.md s10 and evidence.md s10.
 *
 * s11 (rederive, 2026-09-09, HEAD main @ e4c60089): floor RE-MEASURED at 6/176 on this body;
 * fake_ablate again finds no FAKE construct in the closest banked form
 * (rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c), so the kill re-audit
 * passes for a third session.  THREE new structurally-different shapes were measured and all
 * three are dead, and the ledger's remaining structural frontier is now closed:
 *   (1) the UNSPENT in-file sibling func_8005FA98 (text1b.c:2710, matched, same S46C, same
 *       callee) contributes a genuinely different store order (c20,c24,p0,byte28,zero1C,
 *       zero18,zero10,one14,p1,ret) -- transplanted with the rand sequence preserved it costs
 *       +4 insns (60/180) because both offsets go live across a rand call at once;
 *   (2) frontier item 1's NARROW form (pointer local used ONLY as the call's first argument,
 *       field stores left direct) costs +3 insns (21/179): ps is live across both calls, so
 *       expand_call's `a0 = ps` copy is not coalescable and the preheader addiu is pure cost;
 *   (3) frontier item 3 (basic-block split) is measured for the first time: half 2's p0
 *       selection respelled as a real if/else costs only ONE insn (53/177) and does split the
 *       body into four blocks, but the emitted window in BOTH halves is the control's rotation
 *       verbatim.  The join dominates the whole a2 chain and the call, so the call's block
 *       still holds all four window insns.  Any block boundary placed between the a2 base and
 *       the call puts the base in a PREDECESSOR block, which emits it BEFORE the argument
 *       setup -- the side we already have.  schedule_insns is per-block (sched.c:4937).
 * Do NOT re-try sibling-8005FA98 order, the narrow pointer-local, or intra-body block splits.
 *
 * s12 (rederive, 2026-09-09, HEAD main @ 0c19707c): floor RE-MEASURED at 6/176 on this body; the
 * kill re-audit passes for a fourth session (fake_ablate finds no FAKE construct in the closest
 * banked form, and that form re-measures 6/176).  Three results, two of them corrections:
 *   (1) s8's "three-argument call is byte-inert" verdict has the WRONG MECHANISM.  text1b.c:2642
 *       carries a file-scope prototype extern s32 func_80073728(s32, s32); and a function-local
 *       K&R redeclaration does not override it, so GCC 2.7.2 simply DISCARDS a third argument --
 *       even a loop-variant one, which measures byte-identical to the control at 6/176.  Any
 *       future argument-position probe must bypass the prototype.
 *   (2) A GENUINE third argument (local function-pointer cast) measures 17/178.  The argument
 *       value is materialised after both argument moves, but by dependence on the accumulate, not
 *       by rank, and the four-insn window is the control's rotation with the base reseated to a3.
 *       Argument position is not a lever: the zero1C value is consumed before the call.
 *   (3) A base with no consumer but a call argument is not born late, it is DELETED -- single-set
 *       single-use (s32)r4 - K is a loop.c movable and is hoisted to the preheader (66/162,
 *       fourteen insns below target).  The accumulate is what keeps the base insn in the loop.
 * And the INSN_PRIORITY family is now closed by a PREDICATE rather than by observation:
 * schedule_select walks the ready list in maximal equal-priority groups (sched.c:2674) and only
 * advances when a group is fully queued (sched.c:2704), so a base lifted above the argument moves
 * is also lifted above the three priority-3 struct stores and, since selection order is reverse
 * emission order, is EMITTED after them -- while the target emits it before them (4DEC0 vs
 * 4DEC4/4DEC8/4DECC).  That retires frontier item 3.  Do NOT re-attempt priority levers, extra
 * call arguments, or hoisting the base out of its accumulate.  See hypotheses.md H35-H39.
 *
 * s13 (structural, 2026-09-09, HEAD main @ 5daf178d): floor RE-MEASURED at 6/176 on this body.
 * THE DECIDING COMPILER TERM IS NOW NAMED, and it is NOT the one s8-s12 were attacking.  Both
 * this body and the banked score-0 body were compiled with the instrumented cc1 under
 * BB2_SCHED_DEBUG=1 plus a new BB2_PRIO_DEBUG=1 capture, and their sched1 block-6 traces were
 * read side by side (tmp/grind/func_8005D554/s13/blk6.log, nv_blk6.log):
 *   - THIS body: the a2 base is insn 211, luid 31, INSN_PRIORITY 3, ADJPRI birth=0.  It gets no
 *     boost, so at clock 64 it loses the three-way tie [242(p3,l43) 240(p3,l42) 211(p3,l31)] on
 *     INSN_LUID and is picked LAST, at clock 67 -- which in a backward scheduler means emitted
 *     FIRST in the window.  That is the whole 6-point residual.
 *   - THE SCORE-0 body: its a2 base is insn 198, born EARLY (luid 25, before the s.zero18 store
 *     and before the third rand call), INSN_PRIORITY 1, ADJPRI birth=1 -- and adjust_priority
 *     (sched.c:2584) raises it to max_priority 2130706433, so it is picked at clock 61.
 * So the lever is `birthing_insn_p` (sched.c:2505): a SET whose REG dest is live and whose dest
 * has reg_n_sets == 1 gets LAUNCH priority.  It is NOT the priority-4 route (s8/s12), NOT the
 * INSN_LUID route (s10), and NOT the class/hazard route (s10).
 * THE BIND: loop.c:705 uses the SAME predicate (n_times_set == 1) to accept a loop-invariant as
 * a movable, and loop.c:695-700's three-way OR always considers a user-variable dest whose life
 * sits inside one basic block (this loop body is ONE basic block, block 6, 92 insns).  So a
 * single-set carrier for the invariant (s32)r4 - K is HOISTED to the preheader (every such form
 * measures 54/...), and a multi-set carrier gets no boost.  The banked score-0 body escapes only
 * because it has TWO source-level sets (loop.c sees n_times_set == 2, not a movable) of which
 * combine deletes one (`nv = ret; s.ret = nv;` folds to `s.ret = ret`), leaving reg_n_sets == 1
 * at sched1 -- i.e. exactly the fresh multi-write carrier the Judge FAILed.
 * Five new spellings measured, all dead: v0/v3 borrowed at the score-0 carrier positions with the
 * same `carrier = ret; s.ret = carrier;` second write -> 63/178; the base carried across the third
 * rand in a2_offset itself -> 31/178; the a0_offset/a2_offset ROLE SWAP (a0 value in a2_offset,
 * a2 base staged early in a0_offset), with and without the ret restage -> 30/178 both; and the
 * ret restage alone at control positions -> 6/176 BYTE-INERT.  So the `carrier = ret` second
 * write is not the lever, the EARLY base is; and every existing-loop-local carrier that spans the
 * third rand call costs exactly +2 insns.  Do NOT re-try existing-local carriers that span the
 * third rand call, and do NOT re-attempt priority-4/LUID/class levers.  See hypotheses.md s13.
 *
 * s14 (structural, 2026-09-09, HEAD main @ 46867ae6): floor RE-MEASURED at 6/176 on this body;
 * the kill re-audit passes for a fifth session (fake_ablate finds no FAKE construct in the
 * closest banked form, rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c).
 * s14 attacked s13's frontier item 2 -- the LICM escape for a SINGLE-SET base carrier -- by
 * reading loop.c's guard end to end and then measuring each of its three disjuncts.  The guard
 * at loop.c:695-700 accepts a candidate movable when ANY of
 *   A  = not maybe_never and not loop_reg_used_before_p
 *   B  = dest is neither a user variable nor a loop-test reg
 *   C  = reg_in_basic_block_p, loop.c:1062
 * holds.  B is false for every C-level local (all are user variables), so an escape needs A and
 * C false together.  Four measurements, all dead, and all four disassembled to confirm the
 * hoist by inspection rather than by score:
 *   (1) C alone falsified -- base1 set in half 1 and read again in half 2 as base2 = base1 - 0xD,
 *       with half 2's p0 selection respelled as a real if/else so a CODE_LABEL sits between
 *       base1's set and its last use -- 57/179, and the disassembly still shows both bases in the
 *       PREHEADER.  A was still true, and one true disjunct is enough.
 *   (2) The plain non-guard-duplicated while chassis does NOT buy maybe_never.  GCC rotates the
 *       top test out of the loop (guard at 0x351C, loop top at 0x3534), so no jump or label
 *       precedes the base inside the body -- 64/176 with the base hoisted AND spilled to the
 *       frame, the in-loop insn being a reload rather than the addiu.  The 176 count is a
 *       coincidence of spill-plus-reload, not a win.
 *   (3) The while chassis combined with (1) is still hoisted -- 70/177.
 *   (4) Making the base's SOURCE non-invariant is not reachable by a no-op set: both
 *       r4 = r4 and r4 = (u32)(base1 + 0xC) measure 54/178, byte-identical to the plain
 *       fresh-single-set form, because cse runs BEFORE loop and deletes the no-op move, so
 *       loop.c never sees n_times_set[r4] greater than 0.
 * CORRECTION to s12: the target window at 4DEB4-4DECC is addiu a0 / addu a1 / lw v1 / addiu a2 /
 * sw / sw / sw, so the target's sched1 picks the three struct stores FIRST (clocks 61-63) and the
 * a2 base at clock 64.  The target therefore does NOT need the base ranked above the stores, and
 * the banked score-0 body -- which boosts the base to max_priority and picks it at clock 61,
 * ahead of the stores -- reaches the target order only because a LATER pass completes the
 * rotation.  s12's overshoot argument is not a reason to abandon priority levers.
 * Do NOT re-try the plain-while chassis, no-op sets of r4, or a join placed after the base's set.
 *
 * s15 (enumerate, 2026-09-09, HEAD main @ 36291a08): floor RE-MEASURED at 6/176 on this body; the
 * kill re-audit passes for a SIXTH session (fake_ablate finds no FAKE construct in the closest
 * banked form, rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c, and that form
 * re-measures 6/176 exactly).  61 further spellings were swept
 * (tmp/grind/func_8005D554/s15/{v,enumA,enumB}, histogram in s15/sweepA.json): 8 hand-built
 * association/store-placement forms, a 48-form symmetric cross product over the a2 expression's
 * naming space (multiply named or inlined x shift named or inlined x base named or inlined x
 * base-declaration position x final-add operand order x multiply operand order, the SAME spelling
 * applied to both halves), and 5 staging/association forms.  Score histogram: 6 (2 forms), 8 (2),
 * 15 (7), 18 (1), 23 (3), 32 (1), 41 (1), 42 (44).  NOTHING scores below 6.  Three results:
 *   (1) Every spelling that gives the multiply, the shift or the base its OWN named local
 *       collapses the build to 153 instructions -- 23 BELOW the target's 176 -- at score 42.
 *       A per-half named local for any of those three sub-expressions is single-set, hence a
 *       loop.c movable (loop.c:705), and LICM hoists it and its whole chain out of the loop.
 *       44 of the 48 enumA variants land there; only the four fully-inlined spellings keep 176.
 *       This is the same predicate that killed the single-set base carrier in s13, reached from
 *       a completely different direction, and it is why a2_offset must stay MULTI-SET.
 *   (2) THE BASE-LAST ASSOCIATION IS THE FIRST FORM IN FIFTEEN SESSIONS THAT MOVES THE a2 BASE
 *       INSN OUT OF THE PRE-CALL WINDOW.  Writing the half as `a2_offset = m; a2_offset +=
 *       (s32)r4 - K;` emits the window as [addiu a0,sp,0x10][lw v1,gp][move a1,zero][sw][sw][sw]
 *       -- `addiu a0,sp,0x10` wins slot 1, which is the TARGET's slot 1 and which the control
 *       never achieves.  But combine reassociates the constant off s4 and onto the multiply
 *       result: the emitted pair is `addiu v0,v0,-12; addu v0,v0,s4` (0x35C4/0x35C8) where the
 *       target has `addiu a2,s4,-0xC; addu a2,a2,v0` (4DEC0/4DEE8).  So on this chassis, in this
 *       expression's spelling space, "emit the base after the multiply chain" and "keep the
 *       constant attached to s4" did not co-occur in any of the 61 forms measured: every
 *       base-last spelling scores 15/176 and every constant-on-s4 spelling emits the base first.
 *       Banked at rejected/a2-base-last-association-combine-moves-const-off-s4-scores-15.c.
 *   (3) Staging the multiply-shift through the DEAD existing local a0_offset (sanctioned
 *       variable reuse, no live-range extension, no extra instruction) costs 23 points, and it
 *       costs exactly the same 23 whether the base statement precedes or follows the staged
 *       value -- another independent confirmation that the base statement's POSITION is byte-inert.
 *   And one form that is byte-inert at the floor: building the base in three steps
 *   (`a2_offset = (s32)r4; a2_offset -= K; a2_offset += m;`) measures 6/176 exactly like the
 *   control, so the copy/subtract pair is folded back by combine.
 * Do NOT re-sweep the a2 expression's naming space, the base-last association, or multiply-shift
 * staging through an existing local.  See hypotheses.md s15 and evidence.md s15.
 *
 * s16 (enumerate, 2026-09-09, HEAD main @ 07f3c383): floor RE-MEASURED at 6/176 on this body; the
 * kill re-audit passes for a SEVENTH session -- fake_ablate finds no FAKE-annotated construct in
 * rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c, and that form plus
 * rejected/a2-base-built-copy-sub-acc-byte-inert-scores-6.c and
 * rejected/a2-base-last-association-combine-moves-const-off-s4-scores-15.c all reproduce their
 * banked scores exactly (6/176, 6/176, 15/176).  44 further spellings measured in two sweeps
 * (tmp/grind/func_8005D554/s16/enumA 36 forms, histogram s16/sweepA.json; s16/enumB 8 forms,
 * s16/sweepB.json).  Score histogram round A: 6 (1), 8 (2), 12 (1), 15 (3), 16 (8), 19 (12),
 * 21 (2), 22 (2), 26 (2), 33 (1), 37 (2), 43 (2).  Nothing below 6.  Three results:
 *   (1) THE CARRIER AXIS SEPARATES THE TWO PROPERTIES s15 COULD NOT CO-PRODUCE, AND STILL LOSES.
 *       s15 showed the base-last association emits the base late but lets combine move the
 *       constant off s4.  Giving the base its own MULTI-SET carrier -- the dead existing local
 *       a0_offset, reused after its s.zero18 store -- blocks that reassociation, because combine
 *       will not substitute through a pseudo with REG_N_SETS above 1: the emitted insn is
 *       addiu a2,s4,-12 with the constant on s4, exactly the target's operand form, and the build
 *       stays at 176 instructions with no LICM hoist (a0_offset has four sets, so it is not a
 *       loop.c movable).  But the base is STILL emitted in window slot 1, ahead of
 *       addiu a0,sp,0x10 / lw / move a1 -- disassembled at 0x3594 in S2_both, byte-for-byte the
 *       control rotation.  So base-last emission in s15 was never a scheduling win at all; it was
 *       pure dependence on the multiply chain, which is exactly what the reassociation created.
 *       Across all 36 round-A forms (carrier in {none, a0_offset, v0, v3, ret} x association in
 *       {base-first, base-last, sum-folded-into-the-store, carrier-accumulates} x base statement
 *       position x half-1-only / half-2-only / both) NO form emitted addiu <reg>,s4,-K after the
 *       argument setup while keeping 176 instructions.  Best of the new forms: 12/176
 *       (a0_offset carries the base and a2_offset is accumulated onto it, S11_both).
 *   (2) The v0 borrow is a pure register-pressure tax, not a codegen lever: S6_both emits the
 *       half-1 window byte-identically to the control at 0x3564-0x35bc yet scores 15, and the
 *       single-half v0/v3 borrows cost an instruction each (43/178, 37/178).
 *   (3) THE LICM ESCAPE FOR A SINGLE-SET BASE CARRIER IS REACHABLE, AND IT IS SELF-DEFEATING.
 *       loop.c:695-700 is a three-way OR and s14 could not falsify disjunct C.  Reading
 *       reg_in_basic_block_p end to end (loop.c:1062-1098) gives two falsifiers s14 did not use:
 *       an earlier reference to the carrier anywhere in the function (loop.c:1068), and a
 *       CODE_LABEL reached between the set and the carrier's last use (loop.c:1093).  Half 2's
 *       p0 selection is the loop's only real conditional; SPLITTING it into two ifs around the
 *       base's set (odd = D_800A3418 and 1; if (odd) s.p0 = ...; ... base2 = (s32)r4 - 0x19;
 *       if (!odd) s.p0 = ...; a2_offset = base2 + m;) puts one label before the set, which
 *       falsifies A through maybe_never (loop.c:930), and one label between the set and its use,
 *       which falsifies C.  IT WORKS: base2 is single-set and is NOT hoisted -- the disassembly
 *       shows addiu a0,s5,-25 at 0x365c, inside the loop, in the second branch's delay slot.
 *       But the same two labels split the loop body into separate basic blocks, and
 *       schedule_insns is per-block (sched.c:4937), so the base and the multiply chain end up in
 *       a predecessor region of the call's block: the emitted window collapses to
 *       [addu a2,a0,v0][addiu a0,sp,0x10][move a1,zero][sw][sw][sw] at 0x367c-0x3690 with the
 *       base already spent.  66/178.  The carrier IDENTITY is invisible here too -- the same
 *       shape with the multi-set a0_offset scores the identical 66/178.  Falsifying C by the
 *       other route (an earlier USE of the carrier in the loop, loop.c:1068 plus
 *       loop_reg_used_before_p) would need a single-set local to be READ before it is written on
 *       the first iteration, which is not a semantics-preserving spelling of this function.
 *       The plain conditional chassis costs exactly one instruction whether it is spelled as an
 *       if/else or as two ifs (both 53/177), reproducing s11 item 3.
 * Do NOT re-sweep the base-carrier axis (identity, association, position), the v0/v3 borrow, or
 * the split-p0-if LICM escape.  See hypotheses.md s16 and evidence.md s16.
 *
 * s17 (enumerate, 2026-09-09, HEAD main @ fa84454f): floor RE-MEASURED at 6/176 on this body; the
 * kill re-audit passes for an EIGHTH session (fake_ablate finds no FAKE-annotated construct, and
 * rejected/a2-statements-at-maximal-pre-call-birth-point-scores-6.c plus s16's best form both
 * reproduce their banked scores, 6/176 and 12/176).  THE EXISTING-LOCAL BORROW QUADRANT -- the one
 * the Judge left open when it FAILed the fresh nv/nw carrier -- IS NOW SWEPT AND FLAT.  66 further
 * spellings measured (tmp/grind/func_8005D554/s17/enumA 36 forms, enumB 30 forms; histograms in
 * s17/enumA.json and s17/enumB.json).  Round A histogram: 21 (6), 23 (3), 28 (4), 30 (3), 40 (2),
 * 43 (2), 50 (4), 55 (2), 60 (1), 61 (4), 63 (5).  Round B: 19 (2), 21 (13), 26 (2), 28 (9),
 * 40 (4).  Nothing below 6.  Three results:
 *   (1) The score-0 body's EARLY-birth slot -- the base set placed before the s.zero18 store, so it
 *       spans the third rand call -- costs at least +2 instructions for EVERY existing local:
 *       18/18 forms build 178 or 179, never 176.  arg0/arg1 pay +3, v0/v3 pay +2.  That extends
 *       s13's "+2 for every existing loop local" to the two parameters and the two preheader
 *       temporaries, so the whole carrier population of this function is now measured.
 *   (2) The combine-deletable second write is byte-inert in VALUE and in POSITION.  ret, one14,
 *       zero10, c20 and p1 restages score identically in every carrier/position cell (33 forms).
 *       It exists only to raise loop.c's n_times_set (loop.c:705) and combine erases it before
 *       sched1, so it contributes no insn and no dependence.
 *   (3) At the CONTROL slot (base set after the third rand) the consumption shape is inert too:
 *       copy+accumulate, single sum, multiply-first accumulate, direct store into s.zero1C and
 *       carrier self-accumulate all score 19-21 at 176 instructions, and the half-1 window at
 *       0x3594 disassembles to the CONTROL rotation with the base merely reseated from a2 to a3.
 *       Combine folds the carrier copy into the accumulate, so the surviving base insn's dest is
 *       a2_offset again (four sets) and birthing_insn_p's reg_n_sets == 1 precondition fails.
 * So the quadrant has exactly two cells and both are dead: early gives the birth boost and costs
 * instructions; control keeps 176 instructions and loses the carrier to combine.  Do NOT re-sweep
 * existing-local borrows, restage values/positions, or a2_offset consumption shapes.  See
 * hypotheses.md H40-H42 and frontier R1-R3.
 */
s32 func_8005D554(s32 arg0, s32 arg1) {
    extern s32 rand(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
    S46C s;
    s32 v3;
    s32 v0;
    s32 i;
    u32 r5;
    u32 r4;
    s32 ret;
    s32 stride;
    s32 a0_offset;
    s32 a2_offset;
    s32 c100;
    s32 c1;
    s32 *p_b388;
    s32 *p_b390;
    u8 *p_b2e0;
    u8 *p_b2ec;
    s32 *base_offset;
    s32 b1;
    s32 b2;

    v3 = D_800A326C;
    v0 = v3;
    if (v3 < 0) v0 = v3 + 3;
    D_800A326C -= (v0 >> 2) * 4;

    ret = arg0;
    if (arg1 > 0) arg1 -= 1;
    i = 0;

    D_800A3418 ^= rand();
    r5 = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    r4 = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    if (i < ((D_800A326C + 1) * 2)) {
        c100 = 0x100;
        c1 = 1;
        stride = arg1 * 0x3C;
        p_b2e0 = (u8 *)&D_8009B2E0;
        p_b2ec = p_b2e0 + 0xC;
        base_offset = (s32 *)(p_b2ec + stride);
        p_b388 = &D_8009B388;
        p_b390 = p_b388 + 2;
        do {
            s.byte28 = 0;
            s.p0 = (void *)(stride + (s32)p_b2e0);
            s.p1 = p_b388;
            D_800A3418 ^= rand();
            i += 1;
            s.c24 = c100;
            s.c20 = c100;
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x19;
            a0_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            b1 = (s32)r4 - 0xC;
            a2_offset = b1;
            s.zero1C = b1;
            a2_offset += ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);

            s.byte28 = 0;
            s.c24 = c100;
            s.c20 = c100;
            s.p1 = p_b390;
            s.p0 = (void *)((u8 *)base_offset + (D_800A3418 & 1) * 0xC);
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x32;
            a0_offset += ((u32)(D_800A3418 * 0x64) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            b2 = (s32)r4 - 0x19;
            a2_offset = b2;
            s.zero1C = b2;
            a2_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ret;
}
