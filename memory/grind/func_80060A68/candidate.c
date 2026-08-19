/* [s8 2026-08-19 - structural.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's HEAD.]
 *
 * s8 did not change this body, but it overturns the causal story the s6/s7 notes below tell,
 * so read this note before spending anything they say.
 *
 * 1. THE RESIDUAL IS NOT GATED ON birthing_insn_p.  s6 and s7 concluded that the only
 *    C-visible input left was sched.c:2536 `return (reg_n_sets[i] == 1);` - i.e. the
 *    multiply-written carrier the Judge banned.  That reduction is wrong.  The bump does fire,
 *    and it does pin p10's load next to its consumer IN SCHED1's ORDER - but the byte that is
 *    actually missing is decided AFTER that, by which hard register local-alloc gives p10's
 *    pseudo, and sched2 (which never bumps, reload_completed == 1) is free to undo sched1's
 *    adjacency whenever the register is dead across the gap.  Two separable gates; only the
 *    second one has to move, and it moves under ordinary statement order.
 *
 * 2. DUMP-VERIFIED MECHANISM.  text1b.greg for this function says `;; 1 regs to allocate: 83`,
 *    so p10's pseudo (reg 75, set by insn 39) is assigned by LOCAL-ALLOC, and the dispositions
 *    line reads `75 in 2` = $v0.  In sched1's order (... 51, 53, 39, 59, 56, 61, 64 ...) $v0 is
 *    killed at insn 53 (`sh $v0,0x18($v1)`) and re-born at insn 64, so it is free across reg
 *    75's entire range and local-alloc's lowest-free-hard-reg scan takes it.  Keep any value
 *    live in $v0 across insns 39..56 and reg 75 lands in $a1 - dead from slot 12 to slot 28 -
 *    and sched2 hoists the load to slot 12 as `lw $a1,0x10($v1)`, exactly as in target.
 *
 * 3. TWENTY BODIES PROVE IT, WITH p10 SINGLE-SET THROUGHOUT.  Three independent honest
 *    pressure sources reach $a1 at slot 12: hoisting the `idx` read above the p10 consumer
 *    (f3 / f4 / i3 / i4 / r1, 8 / 66); sinking copy 3's store below the +2 read (n2 10 / 67,
 *    m1 9 / 66, o4 / o5 10 / 66); and splitting copy 3 into a named `c3` local whose store
 *    becomes the p10/0x18 cse separator (u5 8 / 66, ua 8 / 66).  s10's "conservation law"
 *    (three 0x10 loads XOR an un-pinned p10 load) is falsified: f3 has BOTH, at 66
 *    instructions, with a prefix byte-identical to target THROUGH SLOT 22 - eleven slots
 *    deeper than cb and covering the whole copy triple.
 *
 * 4. WHAT IS LEFT.  f3 and u5 bracket the answer from opposite ends and their correct regions
 *    are complementary: f3 is exact in slots 1-22, u5 is exact in slots 26-34 (including
 *    `lhu $a1,0x4($a1)` at 29 and the late `lhu $a0,0x0($v1)` idx read at 30, both campaign
 *    firsts).  f3's whole 8-slot cost is that the hoisted `idx` load, which depends on nothing
 *    but $v1, is schedulable into the load-delay slot at 23 - the slot target gives to the
 *    third `lw $a0,0x10($v1)`.  The next session's job is a pressure source whose OWN emission
 *    slot is not 23.  Both banked bodies and the full 59-body sweep are in
 *    memory/grind/func_80060A68/rejected/ and tmp/grind/func_80060A68/s8/.
 *
 * The INTEGRATION HAZARD in the s10 note below (asmfix.txt:109-110 must be retired in the same
 * change as any body swap) is unchanged.  src/text1b.c was reverted to HEAD before finishing.
 */
/* [s7 2026-08-19 - synthesis.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's HEAD.]
 *
 * s7's job was to merge six sessions into one attack and reset the frontier.  It did not
 * change this body (still the floor at 2), but it moved the campaign forward in three ways.
 *
 * 1. A NEW AXIS, AND A BETTER SEAT THAN v2.  Every prior session moved the `p10` statement.
 *    None moved its CONSUMER (`temp_a1 = *(u16 *)(p10 + 4)`), which is what actually decides
 *    when sched1 releases p10's load.  Sweeping the seven consumer seats gives
 *    ca 8/66, cb 4/66, cc 7/66, cd 8/67, v2 5/67, ce 7/67, y1 8/68.  `cb` (consumer placed
 *    immediately after the 0x18 store, above the +2 read) is the best non-candidate body the
 *    campaign has produced: 66 instructions, ALL THREE `lw ?,0x10($v1)` loads present, and a
 *    single defect - p10's load emitted at slot 23 as `lw $v0,0x10($v1)` instead of slot 11
 *    as `lw $a1,0x10($v1)`, shifting everything from slot 11 on by one.  It is banked at
 *    rejected/s7-consumer-after-0x18-store-3-loads-p10-load-pinned-adjacent-to-consumer-score4-66insns.c
 *    and is the seat the next session should read dumps against, NOT v2.
 *
 * 2. A GEOMETRIC PROOF THAT THE TARGET'S COMPILE DID NOT BUMP INSN 39.  Under the
 *    birthing_insn_p LAUNCH_PRIORITY bump, a single-set pointer load is ALWAYS emitted
 *    adjacent to its consumer: the bump makes it outrank everything, and it becomes ready
 *    exactly when its consumer is scheduled (cb trace: `launching 39 before 59 ... at T-32`
 *    then `ready list at T-32: 53 (8) 39 (7f000001), now 39 53`).  The target puts p10's load
 *    at slot 11 and its consumer at slot 28 - SEVENTEEN slots apart.  sched1 cannot do that.
 *    Only sched2 can (reload_completed == 1, so no bump), and only if the hard register is
 *    free across the gap: target's p10 is `$a1`; cb's is `$v0`, written at slot 20 and read
 *    at slot 22, so the hoist is blocked.  This is independent of any particular body and
 *    strengthens s6's conclusion rather than replacing it.
 *
 * 3. THE LAST UNPROBED GATE IS CLOSED.  s6 left birthing_insn_p's `bb_live_regs` test
 *    (sched.c:2524-2531) unmeasured.  s7 measured it with the instrumented cc1 and
 *    BB2_SCHED_DEBUG=1: `SCHEDDBG ADJPRI insn=39 deaths=0 birth=1 maxpri=2130706433 pri=3`.
 *    birth=1.  It cannot be otherwise: backward scheduling releases insn 39 only after an
 *    insn that READS its destination is scheduled, and that read is what sets the bit.  So
 *    `reg_n_sets[i] == 1` is now the SOLE C-visible input to the bump, and that is the
 *    multiply-written carrier the Judge banned for this function on 2026-08-19.
 *
 * Also killed on the cb body: declaration order (four permutations, all 4/66 byte-inert),
 * local types (`s32 temp_a1`, `u16 *p10` with `p10[2]`, `s32 temp2`, all 4/66), and using a
 * gp store as the cse separator instead of a copy store (g1 8/68, g2 11/68 - a
 * `(mem (symbol_ref))` store is an absolute scheduling barrier).
 *
 * DISPOSITION.  The function is ACTIVE.  s7 is a `progress` outcome; src/text1b.c was
 * reverted to HEAD before finishing.  The INTEGRATION HAZARD in the s10 note below
 * (asmfix.txt:109-110 must be retired in the same change as any body swap) is unchanged.
 */
/* [s6 2026-08-19 - rederive.  BODY UNCHANGED from s5/s10.  Read this note first.]
 *
 * s6 re-measured this body on today's HEAD: score 2 / build 66 / target 66.  The floor is
 * confirmed again.  s6 did NOT change the body; it closed the shape question and named the
 * exact GCC gate that the whole remaining residual hangs on.
 *
 * 1. THE SHAPE IS FORCED.  s6 re-derived the target's C independently of s5 by enumerating
 *    every store that can act as a cse separator (cse.c will not disambiguate two
 *    `(plus (reg) (const_int))` addresses; sched.c will, off a common base, but never across
 *    a `(mem (symbol_ref))` gp store).  Target needs three `lw ?,0x10($v1)` loads and the +2
 *    read must sit ABOVE the gp-3478 store, which leaves exactly one legal separator
 *    assignment: the +4 pointer read first into a local, separated from the +0 read by a copy
 *    store, and the +0/+2 pair separated by the 0x18 store.  That is v2 (5 / 67).  The one
 *    partition s5 never tried -- hoisting the +0 VALUE into its own local above the copy-3
 *    store -- was measured from five seats (x1/x2/x6/x7/x8) and scores 9..13.
 *
 * 2. THE v6 FAMILY IS DEAD.  v6 (3 fresh in-line reads, 3 / 66) differs from target by
 *    exactly ONE instruction position: the third 0x10 load sits at slot 26 instead of 11.
 *    It cannot be hoisted at any priority -- `sw $v0,%gp_rel(D_800A3478)($gp)` sits at slot 25
 *    and sched.c cannot disambiguate a symbol_ref store from a (reg + const) load.  s5's
 *    frontier item 2 ("raise its INSN_PRIORITY") is therefore impossible, not merely hard.
 *
 * 3. THE RESIDUAL IS ONE GATE.  With v2 applied, tools/grinder/dump.ps1 gives sched1's trace:
 *        ;; ready list at T-31: 39 (7f000001) 53 (8), now 39 53
 *    insn 39 is p10's load, insn 53 the 0x18 store, insn 56 the +2 read's address load.
 *    0x7f000001 is LAUNCH_PRIORITY (sched.c:187): sched.c:4049 puts it on the insn being
 *    scheduled, and adjust_priority (sched.c:2541-2590) copies that max_priority onto every
 *    released insn for which birthing_insn_p is true -- and birthing_insn_p (sched.c:2505)
 *    is true for any live single-set pseudo birth, because its only real gate is
 *    `reg_n_sets[i] == 1` (the n_deaths switch above it is dead code; GCC says so itself at
 *    sched.c:2551).  So insn 39 takes T-31, insn 53 is pushed to T-33 and insn 56 to T-32;
 *    since sched.c schedules BACKWARD that emits the 0x18 store BEFORE the +2 address load,
 *    so reg91 (the +0 value, $v0) is already dead when reg92 is born and local-alloc hands
 *    reg92 $v0 -- where target, having 56 before 53, is forced onto $a0.  The "missing load"
 *    and the "wrong register" are ONE defect with ONE cause: insn 39's bump.
 *
 * 4. THE ONLY LEVER IS BANNED.  The sole C-visible input to that gate is reg_n_sets[p10] != 1,
 *    i.e. writing p10 twice with both writes live -- the multiply-assigned pointer-staging
 *    carrier the Judge banned for this function on 2026-08-19.  A dead first write does not
 *    work (flow.c deletes it; measured inert at 5 / 67).  Next session should expect the
 *    driver to move this function to `escalation`; if it does not, the remaining unmeasured
 *    gate is birthing_insn_p's `bb_live_regs` test, which no one has probed.
 *
 * The s5 and s10 notes below are unchanged and still accurate, INCLUDING the integration
 * hazard about asmfix.txt:109-110.
 */
/* [s5 2026-08-19 - rederive.  BODY UNCHANGED from s10; read this note first.]
 *
 * s5 re-measured this body on today's HEAD: score 2 / build 66 / target 66.  The floor is
 * confirmed, not stale.  s5 kept this body as the candidate but banked TWO new same-or-near
 * floor attractors that a future session should prefer as forensic seats:
 *
 *   rejected/s5-p10-early-feeds-plus2-and-plus4-plus0-fresh-score2-66insns.c   (q5)
 *      Same floor (2 / 66) but a STRICTLY CLEANER residual: it gets target's slot 22
 *      `lhu $v0,0($a0)` right, where THIS body emits `lhu v0,0(a1)`.  Its only two wrong
 *      slots are 23 (nop vs `lw $a0,0x10($v1)`) and 25 (register).  If you are going to read
 *      .lreg/.greg for anything, read it for q5 and for v2 side by side.
 *
 *   rejected/s5-three-fresh-inline-reads-no-p10-load-lands-slot27-v0-score3-66insns.c   (v6)
 *      Score 3 / 66 with NO p10 local at all - the plainest body the campaign has produced.
 *      It emits all three `lw ?,0x10($v1)` loads at 66 instructions; it is short one HOIST,
 *      not one load.  q1 and q3 (p10 early with an early consumer) compile byte-identically
 *      to it, so "give p10 an early consumer" is not a distinct body.
 *
 * s5's main result is a DERIVATION, not a search: applying the store-separator law (cse.c does
 * not disambiguate two (plus pseudo const) addresses, so an intervening store forces a fresh
 * load; sched.c does disambiguate them, so loads may hoist across those same stores) to the
 * target's own instruction stream proves that the target's +4 pointer cannot be a third in-line
 * read - there is no store available to separate it from the +2 read, and GCC never reorders
 * two non-disambiguable stores.  The target's C is therefore the v2 body (p10 read before the
 * copy-3 store; +0 and +2 read fresh in-line; +4 read through p10), which measures 5 / 67.
 * The entire remaining gap is v2's REGISTER ALLOCATION - reload gives the +2 read's address
 * pseudo $v0 where target gives it $a0.  s5 killed the declaration-level levers on that pseudo
 * from four more seats (named address locals p0 / p2 / both, and a u16*-typed p10, all 5 / 67
 * and byte-identical to v2), on top of s10's three type-level attempts.  See evidence.md and
 * hypotheses.md, both appended this session.
 *
 * Also killed this session: the m2c-literal "no `outer` local" family (26-29 / 68-70 insns) and
 * struct-typed member access via MEM_IN_STRUCT_P (regresses 3 -> 6, 2 -> 8).
 *
 * The INTEGRATION HAZARD below is unchanged and still load-bearing.
 */
/* [s10 2026-08-19 — forensics.  READ THIS FIRST: THE BODY IN THIS FILE CHANGED.]
 *
 * WHAT CHANGED.  Until s9 this file held the "staged temp_a1" body (temp_a1 written
 * twice: once with the 0x10 pointer, once with the 0x1A/0x1C halfword — the sanctioned
 * staged-value-reused-variable construct).  That body is PRESERVED VERBATIM at
 * memory/grind/func_80060A68/prior-floor2-staged-temp_a1.c together with its full s1-s9
 * header; it is NOT rejected and still measures score 2 / build 66 / target 66.  This
 * file now holds the s10 body, which measures the SAME floor (score 2, build 66 /
 * target 66, re-measured this session) but is strictly better on two axes:
 *
 *   1. It contains NO multiply-assigned local at all.  Every local here is written
 *      exactly once and read for a real job: `p10` holds the 0x10 pointer for the +4
 *      halfword read, `temp2` holds the 0x1A halfword across the D_800A3478 store,
 *      `temp_a1` holds the 0x1C halfword and the call's second argument.  There is no
 *      staged carrier, so there is no FAKE annotation and no sanctioned-family claim to
 *      defend.  (The prior body needed the staged-temp_a1 claim.)
 *   2. Its residual is in a DIFFERENT PLACE.  For the first time on this function the
 *      whole prefix through slot 20 is byte-identical to target, INCLUDING the three
 *      contested loads that every session since s1 has been fighting:
 *          slot 10  lw v0,12(v1)      slot 11  lw a0,12(v1)      slot 12  lw a1,16(v1)
 *      The sched2 rank_for_schedule LUID tie at T-45 (sched.c:2464) that s2-s9 attributed
 *      the residual to is GONE from this body.  The remaining 2-instruction gap is:
 *          target  ... lhu v0,0(a0) / lw a0,16(v1) / sh v0,24(v1) / lhu a0,2(a0) ...
 *          ours    ... lhu v0,0(a1) / nop          / sh v0,24(v1) / lhu a0,2(a0) ...
 *      i.e. the 0x18 halfword read is CSE-folded onto p10's register (a1) instead of
 *      re-loading 16(v1), and the load that target puts in the slot-23 load-delay slot is
 *      therefore missing and shows up as a nop.
 *
 * THE MECHANISM, AND WHY IT IS A CONSERVATION LAW (this is the s10 finding).
 * `p10` is single-set, so its load (insn 39) takes sched1's birthing_insn_p
 * LAUNCH_PRIORITY bump (sched.c:2504-2535) and is chained LATE, which is what finally
 * gives sched2 the LUID order it needs to emit copy 2's address load at slot 11 and the
 * staged load at slot 12.  But a bumped load is chained next to its FIRST consumer, and
 * with three independent reads of *(s32 *)(outer + 0x10) p10's only consumer is the +4
 * read at target slot 29 — sched1 then chains insn 39 between insns 56 and 58, sched2
 * reaches T-30 with insn 39 as the sole ready insn, and the load overshoots to slot 25
 * (measured: rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c and the
 * whole r2/r3/r4/t1/u1/u2/u3 family, all score 5 / 67 insns).  Moving the p10 statement
 * BELOW the copy-3 store removes the memory fence between p10's load and the 0x18 read,
 * so cse folds the 0x18 read onto p10 — which gives insn 39 an EARLY consumer (slot 22),
 * unpins it, and lands it exactly on target's slot 12.  That is the trade this body makes:
 *
 *     3 independent 16(v1) loads  <=>  p10 has no early consumer  <=>  sched2 T-30 pin
 *     p10 has an early consumer   <=>  one of the 3 loads is CSE-folded away  <=>  a nop
 *
 * Nine source positions for the p10 statement were measured this session (r1..r9) and
 * every one of them sits on one side or the other of that equivalence; so does every
 * attempt to re-separate the loads with a store (t1: split copy 3 so its store falls
 * between p10 and the 0x18 read -> 3 loads and the pin returns, score 5; t2: hoist the
 * D_800A3478 gp store above the 0x1A read -> score 8).
 *
 * WHERE THE NEXT SESSION SHOULD PUSH.  The r4-shape pin has a named and now dump-verified
 * proximate cause that is NOT scheduling at all: in the 3-load bodies reload gives insn 56
 * (the 0x1A read's address load) hard register v0, and insn 53 is `sh v0,24(v1)`, so 53
 * carries a write-after-read anti-dependence on 56.  sched2 therefore cannot release 53
 * until 56 is scheduled, T-30 comes up with only insn 39 ready, and 39 is forced into it
 * (n9b/trace_sched2.txt T-30 `;; ready list at T-30: 39 (3)`).  In the target — and in the
 * prior staged body — insn 56 gets a0 instead, 53 IS ready at T-30 (n9/trace_sched2.txt
 * `;; ready list at T-30: 39 (3) 53 (8), now 53`), 53 wins on priority 8 vs 3, and insn 39
 * survives to a much later cycle.  So the 3-load body closes the function IF insn 56's
 * destination can be steered from v0 to a0 by honest C.  Three type-level attempts failed
 * this session (s32 temp2, s32 temp_a1, a fresh single-set `q` local for the 0x1A address
 * — all score 5); the next lever is a register-allocation one, read out of .lreg/.greg
 * rather than guessed.
 *
 * INTEGRATION HAZARD (unchanged, still live and load-bearing).  This body's first body
 * instruction is `lhu $2,0($3)`, while asmfix.txt:109 (a delete_between anchored on
 * `^lhu\t\$4,0\(\$3\)$`) and asmfix.txt:110 (a 43-instruction insert_before splicing the
 * whole target body) are written against the HEAD body.  Applying this body to
 * src/text1b.c WITHOUT retiring both asmfix rules in the same change silently duplicates
 * the function in a full build.  src/text1b.c was reverted to HEAD before this session
 * finished.
 *
 * DISPOSITION.  The function is ACTIVE, not parked.  s8's terminal-disposition claim was
 * voided by the driver (wrong modality) and s9 reverted the decisions.md entry; nothing
 * this session changes that.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    p10 = *(s32 *)(outer + 0x10);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
