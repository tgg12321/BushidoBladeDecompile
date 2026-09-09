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
            a2_offset = (s32)r4 - 0xC;
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
            a2_offset = (s32)r4 - 0x19;
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
