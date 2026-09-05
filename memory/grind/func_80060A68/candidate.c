/* [s26 2026-09-04 - forensics modality.  BODY UNCHANGED (still the T1 spine); the floor is still
 * 2 / build 65 / target 66, re-measured on today's HEAD chassis before any probe.  s26 CORRECTS
 * THE FRAMING s24 AND s25 WORKED UNDER, and that correction is the session's deliverable.
 *
 * THE RESIDUAL IS A READINESS CONSTRAINT, NOT A PRIORITY CONSTRAINT.  s24 stated the residual as
 * an INSN_PRIORITY predicate on uid 12 (the p10 load) and s25 exhaustively enumerated the pass-2
 * (pri, icost) board searching for a predecessor that could satisfy it.  That predicate can never
 * fire.  Read straight out of the pass-2 pick trace (extract.py text1b -> parity=True, 1764
 * blocks; this block replays BASELINE EXACT):
 *
 *     PICK insn 30  clock 44  ready [[30, 4, 21], [35, 3, 22]]
 *     PICK insn 35  clock 45  ready [[35, 3, 22]]
 *     PICK insn 12  clock 49  ready [[12, 2, 17]]
 *
 * uid 12 IS NOT ON THE READY LIST at either of the two picks where target places it.
 * rank_for_schedule (sched.c:2407) is only consulted among ready insns, so INSN_PRIORITY(12) is
 * irrelevant there - 3, 4 or 4000 would all change nothing.  The cause is
 * deps[25] = [[147,15],[9,14],[12,14],[16,14],[21,0]]: dependence kind 14 is REG_DEP_ANTI, so the
 * Z0 store is ANTI-DEPENDENT on the p10 load, and because sched.c builds the block backwards
 * uid 12 cannot join the ready list until uid 25 is scheduled - two picks too late.  Target's
 * compilation must not carry that anti-dependence at all.  This is also why the only depth-1
 * solver vector ever returned, across two sessions and 4634 atoms, was "del_dep 25 <- 12".
 *
 * THE GATE IS anti_dependence AT sched.c:845-868 (called from sched.c:1783).  It returns 0 - no
 * dependence - on four routes only:
 *   (a) RTX_UNCHANGING_P on the load.  No C spelling for a runtime-pointer deref; GCC 2.7.2 sets
 *       it only for readonly DECLs and constant-pool refs.
 *   (b) BOTH memrefs MEM_VOLATILE_P.  UNMEASURED, and a family question (the two-prong
 *       legitimate-volatile-interrupt-touched gate), not a free lever.  This is frontier item 1.
 *   (c) memrefs_conflict_p == 0.  The distinct-symbolic-base escape at sched.c:697-705 needs
 *       find_symbolic_term to succeed on BOTH sides, and the p10 load's base is a pseudo holding
 *       a gp-loaded pointer, so it returns 0 and the function reports a conflict.
 *   (d) The two MEM_IN_STRUCT_P / rtx_addr_varies_p asymmetry prongs.  CLASS-KILLED this session
 *       (predicate sched.c:862): each prong requires ONE of the two addresses to be NON-varying,
 *       and both of this body's memrefs are runtime-computed, so no arrangement of struct-typed
 *       versus scalar-typed access opens either one.
 *
 * s25 FRONTIER ITEM 1 IS CLOSED, derived and measured.  Derived: at the critical pick
 * last_scheduled_insn = uid 30, and deps[30] = [[147,0],[25,0],[28,0]] contains neither uid 12
 * nor uid 35, so rank_for_schedule's dependence-CLASS term is a tie (both class 3) and the
 * comparator falls through to LUID at sched.c:2462 - the statement-position axis s24 already
 * closed.  Measured: 12 bodies staging copy 2's (and other copies') source pointer through a
 * single-write local above or below the Z0 store score 4 to 8; nothing at or below 2.
 *
 * A NEW SPINE WORTH KNOWING ABOUT.  W1 (copy 2's source pointer hoisted into a single-write
 * local above the Z0 store) is 66 instructions and a PURE PERMUTATION of target's exact
 * instruction multiset - the first such body in 26 sessions - byte-identical from the
 * "lw v0,0(v0)" onward, raw diff 4, engine score 4.  Its extracted model is banked at
 * tmp/grind/func_80060A68/s26/W1.sched.json.  It is NOT a better solver target: the hoist
 * strands BOTH the p10 load and the copy-2 pointer load in the block-head pri-2 chain, so its
 * goal needs TWO insns lifted over the pri-3 wall instead of one.  Banked at
 * rejected/s26-W1-copy2-ptr-hoisted-above-Z0-store-...-score4-66insns.c.
 *
 * DISPOSITION.  ACTIVE.  This is a "progress" session: the floor is unchanged at 2 and the
 * ladder is not exhausted.
 */
/* [s25 2026-09-04 - forensics modality.  BODY UNCHANGED (still the T1 spine); the floor is
 * still 2 / build 65 / target 66, re-measured on today's HEAD before any probe.  What s25 adds
 * is the CLOSED ENUMERATION behind s24's predicate, read out of the extracted scheduler model
 * rather than reasoned about:
 *
 *   pri(12) = max over LOG_LINKS of (pri(pred) + insn_cost(pred) - 1)   [sched.c:1495],
 *   and ADJUST_COST zeroes anti/output costs, so only a TRUE dep can contribute more than
 *   pri(pred) - 1.  Adding a dep to uid 12 changes no other insn's priority, so the pass-2
 *   table is a fixed scoring board:
 *     pri(12)=4 needs a (pri 3, icost 2) true-dep pred -> only uid 28 / uid 35, both
 *       `lw ?,12($3)`.  Both are LOADS; a load cannot be a true-dep pred of another load and
 *       the p10 address is not a function of *(s32 *)(outer + 0xC).  NO C SPELLING.
 *     pri(12)=3 needs (pri 3, icost 1) or (pri 2, icost 2) -> uid 21 (sll), uid 16 (lhu), or
 *       uid 25 (the Z0 store).  21 and 16 are semantically unreachable; uid 25 is reachable as
 *       a true MEMORY dep by putting the p10 read below the store - the six-position family
 *       s24 already killed (one normalised cc1 stream, 67 insns, score 5).
 *     uid 30 (4,2) and uid 32 (5,1) both give 5, which ranks 12 ABOVE uid 30 - wrong side.
 *
 * s25 also read the pass-1 `adjpri` stream: the p10 load IS a birthing insn (birth=1) and IS
 * bumped to max_priority 0x7F000001, so the pass-1 order among the head insns is LUID-descending
 * over the raw source order.  That means the OTHER half of the predicate (pass-2 luid(12) > 22)
 * reduces to the SAME statement-position axis s24 closed.  Both halves now sit on one lever.
 *
 * THE NEW STANDING CONSTRAINT.  Target's stream is this body's stream plus exactly one load-delay
 * NOP, so the residual must be bought with ZERO NET INSTRUCTIONS.  s25 measured the one spelling
 * that synthesises the required (pri 3, icost 2) predecessor - a SECOND source read of the gp
 * global, `p10 = *(s32 *)(D_800A3468 + 0x10);` below the Z0 store.  The cse prediction held
 * exactly (the store to D_800F10D0($idx) is a varying gp address, so the re-read is not merged
 * and a real second `lw $4,D_800A3468` appears), but it costs an instruction: 67 insns, score 9.
 * Banked at rejected/gp-global-reread-below-Z0-store-survives-cse-adds-an-lw-score9-67insns.c.
 *
 * DISPOSITION.  ACTIVE.  `progress`; the ladder is not exhausted.
 */
/* [s24 2026-09-04 - solver modality.  SPINE CHANGE: candidate.c is now the T1 body
 * (the p10 read above the Z0 store), not the E2 body.  Both measure engine score 2 on
 * today's HEAD chassis; T1 is 65 build insns against target's 66, E2 is 66.  The E2 body
 * is preserved verbatim, unchanged and NOT disproven, at
 * memory/grind/func_80060A68/e2-spine-floor2-cse-merged.c.
 *
 * WHY THE SPINE MOVED.  s24 measured both against the REAL target bytes (an
 * engine.score.normalized_insns diff of the sandbox object against build/src/text1b.o,
 * which is the canonical INCLUDE_ASM object and therefore literally asm/funcs/
 * func_80060A68.s assembled):
 *
 *     E2 : 66 insns, raw diff 4, TWO `lw ?,16(v1)` at slots 11 ($a1) and 19 ($a0)
 *          - target's exact first two slots and seats - and NO THIRD LOAD.
 *     T1 : 65 insns, raw diff 3, THREE `lw ?,16(v1)` at slots 4, 18, 21
 *          against target's 11, 19, 22.
 *
 * `tools/ra_solver/goal_from_tgt.py classify text1b func_80060A68` on E2 returns
 * FIRST DIVERGENCE: PRE-RA, "next tool: none - the residual is upstream of every model"
 * (ours only: nop x1; target only: lw #,16(#) x1).  E2's missing load is the s20 cse
 * merge, closed as a class at cse.c:1701; no RA or scheduler model can express it.
 *
 * T1's residual, by contrast, is ONE INSTRUCTION MOVED and nothing else:
 *
 *     ours   : lhu v0,0(v1) | lw a1,16(v1) | sll | lui at | addu at | sw zero
 *              | lw v0,12(v1) | lw a0,12(v1) |                | lw v0,0(v0) ...
 *     target : lhu v0,0(v1) | nop          | sll | lui at | addu at | sw zero
 *              | lw v0,12(v1) | lw a0,12(v1) | lw a1,16(v1) | lw v0,0(v0) ...
 *
 * Every one of the other 64 instructions, every nop included, is identical.  That is a
 * pure permutation of an identical instruction multiset - exactly what tools/sched_solver
 * models exactly - so T1 is the spine a solver session can actually work on.
 *
 * THE RESIDUAL, STATED AS A PREDICATE (s24's main deliverable; do not re-derive it).
 * The scheduler model extracted clean (`extract.py text1b` -> parity=True, 1764 blocks;
 * this function's pass1 block 0 of 42 insns and pass2 block 0 of 44 insns both replay
 * BASELINE EXACT).  The cc1-index -> RTL-uid table is in evidence.md; the insns that
 * matter are uid 9 `lw $3,D_800A3468`, uid 16 `lhu $2,0($3)`, uid 12 `lw $5,16($3)`
 * (THE P10 LOAD), uid 21 `sll`, uid 25 `sw $0,D_800F10D0($2)` (the Z0 store),
 * uid 28 `lw $2,12($3)`, uid 35 `lw $4,12($3)`, uid 30 `lw $2,0($2)`, uid 32
 * `sw $2,32($3)`.  sched.c builds the block BACKWARDS, so the pick list is the reverse
 * of the emission order.  Our pass-2 pick tail is
 *     ... 32, 30, 35, 28, 25, 21, 12, 16, 147, 145, 9
 * and target's must be
 *     ... 32, 30, 12, 35, 28, 25, 21, 16, 147, 145, 9
 * i.e. UID 12 MUST BE PICKED BETWEEN UID 30 AND UID 35.  `rank_for_schedule` is priority
 * descending, then dependence class, then LUID descending, and the harvested pass-2
 * values are pri(30)=4, pri(35)=pri(28)=pri(25)=pri(21)=3, pri(12)=pri(16)=2, with
 * luid(12)=17, luid(30)=21, luid(35)=22.  So the requirement is exactly:
 *
 *     INSN_PRIORITY(12) = 3 AND pass-2 LUID(12) > 22,   or   INSN_PRIORITY(12) = 4.
 *
 * `priority()` is the longest dependence path FROM THE BLOCK HEAD, and with the p10 read
 * written first uid 12's only LOG_LINKS predecessor is uid 9 (pri 1, load cost 2), so
 * pri(12) = 1 + 2 - 1 = 2 and CANNOT be raised without giving the load a predecessor -
 * which means moving it later in source.  pass-2 LUID is the sched1 emission position, so
 * the whole question collapses to: sched1 must emit the p10 load after uid 35 and before
 * uid 32, with the Z0 store as the only store above it in source.
 *
 * WHAT s24 MEASURED AGAINST THAT PREDICATE (8 bodies, zero FAKE constructs in any).
 *
 * (1) DEPTH-1 SOLVER SEARCH, FULL ATOM VOCABULARY - CLEAN NEGATIVE.
 *     `perturb.py --pass 2 --block 0 --goal-before 30:12 --goal-before 12:35 --depth 1`
 *     searched all 4634 single atoms (add_dep / del_dep / luid / luid_move / cost) and
 *     reports NO perturbation reaches the goal.  The pass-1 sub-goal (`--goal-before
 *     12:35`, 4400 atoms) returns exactly ONE vector: `del_dep 25 <- 12`, i.e. drop the
 *     Z0 store's ANTI-dependence on the p10 load.  That vector is the right shape and has
 *     NO C SPELLING under GCC 2.7.2: both memrefs are `(plus reg X)` with a varying index
 *     so `memrefs_conflict_p` cannot disambiguate them, and C offers no `restrict`, no
 *     type-based aliasing, and no `RTX_UNCHANGING_P` on an ordinary load.
 *
 * (2) THE SIX-POSITION COLLAPSE - the statement-move axis below the Z0 store is CLOSED.
 *     Six bodies put the p10 read directly after the Z0 store, after copy 1, after copy 2,
 *     and - three of them - INSIDE a copy statement, between the copy's load and its store
 *     (`c1 = *(s32 *)(*(s32 *)(outer + 0xC) + 0); p10 = ...; *(s32 *)(outer + 0x20) = c1;`
 *     and the copy-2 equivalent), which was the only shape that keeps the Z0 store as the
 *     read's sole preceding store.  ALL SIX score 5 / build 67 with their three 0x10 loads
 *     at 18 ($a0) / 23 ($v0) / 24 ($a1) - the p10 load LAST - and their raw cc1 streams are
 *     BYTE-IDENTICAL to each other (empty diffs).  The six positions are normalised to one
 *     stream upstream of the scheduler, so splitting a copy buys no LUID control.  Only two
 *     regimes are reachable by moving this statement: p10 load FIRST (this body, 65 insns,
 *     score 2) or p10 load LAST (67 insns, score 5).  Target's middle seat is in neither.
 *
 * TOOL NOTE for the next solver session.  Upstream `tools/sched_solver/goalmap.py`
 * ABORTS object-mode goal derivation for this function: `_macro_expand_counts` counts
 * `sw $0,SYM($idx)` as one insn, but GNU as assembles it to lui/addu/sw, so the honest
 * text stream is estimated at 63 against the object's 65.  A read-only fork with that
 * third case added is at tmp/grind/func_80060A68/s24/goalmap.py, driven by
 * tmp/grind/func_80060A68/s24/run_perturb.py.  tools/ is not an editable surface for a
 * grind session; if the operator wants it upstream it is a three-line change.
 *
 * The dispatch call keeps the s23 `()` spelling (func_80060B70 at src/text1b.c:3174 and
 * the callee prototype `u8 func_80063AF0(void)` at src/text1b.c:4042); s23 proved it
 * byte-identical to the old `(idx, temp_a1)` spelling on E2.  This body carries ZERO FAKE
 * constructs and re-declares none of the five banned constructs.
 *
 * DISPOSITION.  The function is ACTIVE.  This is a `progress` session: the floor is
 * unchanged at 2 and the ladder is not exhausted.
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
    p10 = *(s32 *)(outer + 0x10);
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))();
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
