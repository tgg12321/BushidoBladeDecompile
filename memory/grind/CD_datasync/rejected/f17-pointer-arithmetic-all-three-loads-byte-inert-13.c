/* REJECTED (s39) - f17 chassis with all three table reads spelled as explicit
 * pointer arithmetic (`*(tbl_125c + i4)`, `*(tbl_11dc + D_800A11D5)`,
 * `*(tbl_125c + idx_1494[1])`) instead of subscripts.
 * WHY IT IS REJECTED: byte-inert - score 13 / build_insns 90, identical to
 * f17.  a[i] and *(a+i) are the same tree in this front end for a POINTER
 * base; only binding the address to a named local (see the p4 variant) changes
 * MEM_IN_STRUCT_P, and that is byte-inert too. */
/* REJECTED (s28) - `s32 i4 = idx_1494[0];` first, then value locals arg4 then
 * arg3. This form reaches target's arg4 REGISTER MAP exactly - address chain
 * in $a0 (`lbu a0,0(s1) ... sll a0,a0,2 addu a0,a0,s0`) with the value loaded
 * `lw a3,0(a0)` - which no previous session had produced.
 * WHY IT IS REJECTED: its window is 17 insns, not 18. arg4's chain completes
 * EARLY (the a0 pseudo dies at window slot 11), which is exactly the condition
 * that frees $a0 in sched1; but with arg4's load no longer last, the stack
 * store `sw ...,16($sp)` becomes the final movable insn and dbr sinks it into
 * the `jal printf` delay slot, where target has a nop.
 * This form and ya53 bracket the residual: 53 of the 465 forms swept this
 * session carry the $a0/$a3 map and ALL 53 have a 17-insn window; every
 * 18-insn form puts arg4's chain in $v0, $v1, $a2 or $a3. */
/* saEft01Init â€” best measured form as of grind session 9.   floor: 7 / 91
 * (`sandbox saEft01Init --disable all`; target is 91 instructions, so the
 * instruction COUNT is exact and the whole residual is inside the
 * debug_printf argument block, build idx 46-61).
 *
 * ===========================================================================
 * WHAT CHANGED IN SESSION 9 â€” THE CHASSIS IS NOW THE ZERO-CONSTANT-LEVER ONE
 * ===========================================================================
 * Sessions 2-5 reached 8 / 91 on a chassis that needed TWO FAKE-family levers
 * (the reused `k` holding both compare constants to defeat loop.c's LICM, plus
 * `cnt = k;` to stage the mask out of an already-live pseudo).  Session 7
 * proved that chassis is a wrong basin: this function is Sony PsyQ LIBCD
 * `CD_datasync`, whose reference source has NO `k` and NO staging.
 *
 * This file has NEITHER lever.  Both compare constants are plain literals
 * exactly as the Sony reference writes them, there is no staging of any kind,
 * and the ONLY match device is a single `do { } while (0)` wrapper around the
 * timeout/printf block.  It scores 7 where the old two-lever form scored 8.
 *
 * WHY THE WRAPPER IS THE WHOLE LEVER (session 9, H37/H38 â€” read this before
 * touching it, because the reason is a proved arithmetic dead end on both
 * sides of it):
 *
 *   * global.c:allocno_compare sorts on
 *         pri = floor_log2(nrefs) * nrefs / livelen * 10000
 *     (verified to the integer on 14 allocnos across three chassis), and
 *     find_reg then hands out $s0,$s1,$s2,$s3 in that sorted order.
 *   * Target's map is D_800A125C->$s0, D_800A1494->$s1, param->$s2,
 *     D_800A11DC->$s3.  A plain goto-loop body gives the param 2 refs over a
 *     52-insn live range => pri 384, which outranks D_800A1494's 306, so the
 *     param takes $s0 and the whole thing rotates (the inherited 18 / 91).
 *   * The param can NEVER be demoted by lengthening its live range: its live
 *     length is bounded by the function's 59 insns and it can never receive
 *     the REG_EQUIV note that DOUBLES the three pointers' live lengths
 *     (local-alloc.c:1019-1052 attaches REG_EQUIV only to a single-set pseudo
 *     whose source is CONSTANT_P or an unchanging MEM; the param's set is
 *     `(set (reg 72) (reg:SI 4 a0))`).  2*10000/59 = 338 > 306 for every
 *     spelling that exists.  So the pointers must be lifted, not the param.
 *   * A REAL loop lifts them â€” flow.c adds `loop_depth`, not 1, per reference,
 *     so the two doubly-used pointers go 3 refs -> 5 and outrank the param.
 *     But a real loop also hands loop.c the two compare constants as
 *     movables, and they are hoisted unconditionally here
 *     (loop.c:1631 moves when `threshold * savings * lifetime >= insn_count`;
 *     threshold = 1 + n_non_fixed_regs = 61, savings = lifetime = 1, and the
 *     loop is 50 real insns), costing two extra callee-saves and +5 insns.
 *     Every reference-corpus spelling pays this: sotn 35/91, xeno 27/96,
 *     tomba 27/96, `clean` 32/96.
 *   * `do { ... } while (0)` emits NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END
 *     exactly like a real loop, so flow.c's loop_depth rises inside it â€” but
 *     the function's real back edge is still a bare `goto`, so loop.c never
 *     sees a loop and never builds a movable for either constant.  It buys
 *     the ref weighting WITHOUT the hoist.  Measured: 18 -> 8 with nothing
 *     else changed, and the resulting .lreg numbers are exactly the predicted
 *     ones (D_800A125C 5/96 = 1041, D_800A1494 5/98 = 1020, param 2/52 = 384,
 *     D_800A11DC 3/100 = 300).
 *
 * The wrapper is therefore a `/ * FAKE * /`-annotated match device under the
 * owner's 2026-07-06 do-while(0) ruling (sanctioned for ANY codegen effect,
 * mandatory inline annotation, single level so no nesting justification is
 * needed).  It is NOT a semantic lie: the block executes exactly once either
 * way and the wrapper changes no program fact.
 *
 * WHAT IS LEFT (7 diffs, all in build idx 46-61, the argument block):
 *   target: lbu a0,0(s1) / lbu v0,1(s1) / lui+lw a1 / sll v0,2 / addu v0,s0 /
 *           sll a0,2 / lw v1,0(v0) / lui+lbu D_800A11D5 / addu a0,s0 /
 *           sll v0,2 / addu v0,s3 / sw v1,16(sp) / lw a2,0(v0) / lw a3,0(a0)
 *   ours:   the same two lbu IN TARGET'S ORDER, but the idx[0] address chain
 *           runs through $v0 and issues `lw a3,0(v0)` EARLY (build idx 54),
 *           where target keeps that chain alive in $a0 and issues
 *           `lw a3,0(a0)` as the very LAST memory reference of the block.
 * Session 9 swept twelve argument spellings against this chassis (t1-t6,
 * u1-u6): both-inline 13, arg5-named-only 13, index-bytes-named 13,
 * named-pointer-with-inline-deref 9-10, arg2-also-named 14 (and 90 insns),
 * arg4-named-first 7, both-named-arg4-first 7, index-staged 7.  7 is the
 * floor of that whole family.
 *
 * ===========================================================================
 * SESSION 10 (synthesis) â€” THE BODY IS UNCHANGED; THE MODEL OF THE 7 IS NOT
 * ===========================================================================
 * Re-applied and re-measured this session at exactly 7 / 91.  Nine more
 * argument spellings measured (nineteen in total) and 7 is still the floor,
 * but the instrumented cc1's real INSN_PRIORITY tables now say what the 7 IS:
 *
 *   * GCC 2.7.2's scheduler runs BACKWARD â€” `priority()` is computed over an
 *     insn's LOG_LINKS (predecessors) and `schedule_block` emits each chosen
 *     insn at the block's current TAIL.  So the printed priority is dependence
 *     DEPTH from the top of the block and HIGHER priority = emitted LATER.
 *   * The whole argument block is a three-level lattice with SEVEN insns tied
 *     at level 2, so `rank_for_schedule`'s tie-breaks decide the order (load /
 *     store dependence class against last_scheduled_insn, then INSN_LUID) â€”
 *     and INSN_LUID at sched2 is SCHED1's output order, not source order,
 *     which is why session 6 measured statement placement here as byte-inert.
 *   * The 7 DECOMPOSES into two independent sub-goals, each already solved by
 *     a DIFFERENT measured spelling:
 *       (A) `lw a3` as the last memory reference (target build idx 61) â€” the
 *           both-lookups-inline form lands it at exactly 61, but drags its
 *           whole idx[0] chain to the tail with it (13 / 91);
 *       (B) both `lbu` at the block head in target's order (idx 46/47) â€” THIS
 *           form has it, and issues `lw a3` six insns early at 54 (7 / 91).
 *     Target's idx[0] chain is SPREAD (lbu 46, sll 52, addu 56, lw a3 61);
 *     every spelling reachable from C emits it as one contiguous run.
 *   * F19's mechanism (a local-alloc copy preference for `$a0`) is KILLED: the
 *     register difference is downstream of the schedule â€” our chain is live
 *     51-54 and does not overlap arg5's value in `$v1`, target's is live 46-61
 *     and does, so the conflict graphs differ because the ORDER differs.
 *
 * ===========================================================================
 * SESSION 11 (structural) â€” BODY STILL UNCHANGED; THREE MORE AXES CLOSED
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  Thirty-one further forms on
 * three axes nobody had touched; all three are dead and the model of the 7 is
 * now positional rather than tie-break-theoretical:
 *
 *   * Target's emitted order in the argument block is EXACTLY ascending in
 *     scheduler priority (1,1,1,2,2,2,2,2,2,3,3,3,4,4 over build idx 46-61),
 *     which is what a backward list scheduler produces when it always pops the
 *     highest-priority ready insn.  So target's `lw a3` is a level-FOUR insn,
 *     a whole level ABOVE `sw 16(sp)` â€” not a level-2 insn winning a tie, as
 *     session 10's F22 assumed.  The only edge that does that is a store->load
 *     memory dependence on the 16(sp) outgoing-arg slot: in target's RTL stream
 *     the `sw` precedes both register-arg loads while both `lbu`s and the arg5
 *     value chain precede the `sw`.
 *   * `calls.c:1615-1665` is the mechanism.  The register-arg precompute loop
 *     runs FORWARD; for an INLINE array-element argument `expand_expr` emits
 *     the address chain there and returns a MEM, so the load is deferred to
 *     `load_register_parameters` (after the stack stores) â€” target's split.
 *     For a NAMED-local argument the value is already a REG and the load
 *     happened at the statement â€” ours.  And the loop-nesting half of
 *     `preserve_subexpressions_p` (stmt.c:2435) is dead code here, because
 *     `toplev.c:3387` sets `flag_expensive_optimizations` for every -O2 build:
 *     the do{}while(0) wrapper has NO influence on argument expansion.
 *   * The block has exactly THREE reachable states and they are RIGID:
 *     arg4-as-named-VALUE = 7 (this file), arg4-as-named-ADDRESS = 9 (i2),
 *     arg4-fully-inline = 13 (i1).  Six more bolt-ons (q1-q6) were byte-
 *     identical to whichever attractor they started from.
 *   * DEAD AXES measured this session: do{}while(0) wrapper EXTENT (7 forms â€”
 *     byte-inert in both directions provided the wrapper contains BOTH the
 *     tslTm2LoadImage_2 call and the arg4 store; dropping the call out costs 1,
 *     lifting the store out costs 9 and an instruction); the arg3
 *     `tbl_11dc[D_800A11D5]` spelling (8 forms, 10-15, all regressions â€”
 *     fully-inline arg3 is the family optimum); the pointers' DECLARATION order
 *     (byte-inert).  Their INITIALISATION order is live and the order written
 *     below is the unique optimum (the five permutations score 8/11/11/12/12) â€”
 *     do not "tidy" it.
 *
 * ===========================================================================
 * SESSION 12 (structural) â€” BODY STILL UNCHANGED; F25 ANSWERED, 4 AXES DEAD
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  Fifty-two further forms on
 * five axes nobody had touched:
 *
 *   * F25 is ANSWERED by reading calls.c: the register-arg precompute loop
 *     (1618-1665) runs BEFORE store_one_arg for the single stack argument
 *     (1736-1739), which runs before load_register_parameters (~1876).  So an
 *     inline arg4 genuinely does get its address chain emitted before the
 *     `sw 16(sp)` and its load after â€” target's split is reachable at expand,
 *     and the fully-inline attractor's contiguous 57-61 idx[0] chain is a
 *     SCHEDULING outcome, not an expand-order one.
 *   * The one live lever found is RTX_UNCHANGING_P on arg4's load: `const s32
 *     *tbl_125c`, or equivalently a per-access `((const s32 *)tbl_125c)[...]`
 *     cast, is a FOURTH attractor at 9 / 91 and the FIRST measured form whose
 *     `sw 16(sp)` precedes `lw a3` (target's relation).  It costs 2 â€” the two
 *     `lbu` swap and the D_800A11D5/arg3 chain moves after `lw a3` â€” and all
 *     thirteen bolt-ons on that chassis stay at >= 9.  The flag is inert on a
 *     fully inline arg4, i.e. it only moves a STATEMENT-emitted load.
 *   * DEAD AXES: arg1 / the format-string address (3 forms, all 7 â€” every one
 *     of the 51 prior forms touched only args 2-5); MEM_IN_STRUCT_P, probed by
 *     re-typing the hoisted bases as pointers-to-array so the accesses are real
 *     ARRAY_REFs (6 forms, all 7); pre-loop global-store placement among the
 *     pointer inits (4 forms, 7/7/17/7); and debug_printf's PROTOTYPE â€” varargs
 *     `(void *, ...)` and K&R `()` are byte-identical to the fixed 5-arg form
 *     against three different bodies.
 *
 * ===========================================================================
 * SESSION 13 (permuter) â€” BODY STILL UNCHANGED; THE PERMUTER MODALITY IS DEAD
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  Two fresh-seed campaigns
 * (candidate chassis, base 435, 16.5k iters; const chassis, base 455, 13.2k
 * iters) produced 9 finds; every one was re-scored in the sandbox and the only
 * permuter-score improvements are sandbox REGRESSIONS (428 -> 18/92,
 * 423 -> 17/92, 450 -> 10/91), while one 455-scored find is sandbox 10 where
 * the 455-scored base is 9.
 *
 * The reason is structural, not a budget question.  decomp-permuter charges
 * PENALTY_REORDERING = 60 and PENALTY_REGALLOC = 5 over a RE-ALIGNED
 * instruction stream (scorer.py:14-18); the engine sandbox is position-locked
 * and charges 1 for either.  This file's 435 decomposes EXACTLY as seven
 * reorderings (420) plus three register differences (15) â€” i.e. the permuter
 * is charging 60x for the very seven instructions the sandbox charges 7 for.
 * Our whole residual is ordering, so any mutation that trades ordering for
 * renaming reads as progress to the permuter and is a regression to us.  That
 * is all both campaigns found.  Random permutation cannot descend on this
 * basin; do not spend another session's modality on it without first fixing
 * the objective (the penalties are class attributes with no knob, and tools/
 * is outside a grind session's writable surface â€” an operator decision).
 *
 * One substantive new axis was measured and closed: arg5 spelled through the
 * GLOBAL, `D_800A125C[idx_1494[1]]`, instead of the hoisted base â€” the first
 * of 65 argument forms to do so.  17 / 92: the second symbol reference costs a
 * fresh lui/addu/lw chain and collapses tbl_125c's allocno priority, rotating
 * the whole callee-save map.  Banked as
 * rejected/permuter-global-arg5-rotates-callee-save-map-17-92.c.
 *
 * ===========================================================================
 * SESSION 14 (permuter) â€” BODY STILL UNCHANGED; s13's MECHANISM WAS WRONG,
 * THE CONCLUSION SURVIVES ON BETTER EVIDENCE
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.
 *
 *   * s13's stated reason for killing the permuter modality â€” "this file's 435
 *     decomposes EXACTLY as seven reorderings (420) plus three register
 *     differences (15)" â€” is FALSIFIED.  Forcing Scorer(debug_mode=True) inside
 *     the real permuter process prints its own Penalty List for base.o vs
 *     target.o: Register Differences 7 (x5) = 35, Reorderings ZERO, Insertions
 *     2 (x100) = 200, Deletions 2 (x100) = 200.  Total 435.  7*60+3*5 and
 *     2*100+2*100+7*5 both equal 435, which is how the wrong model survived.
 *     The real misalignment is 100-vs-5, not 60-vs-5: scorer.py only converts
 *     an insertion/deletion pair into a 60-point "reordering" when the two rows
 *     are IDENTICAL strings, and ours are not.
 *   * The objective was then actually re-aligned (REGALLOC 1, REORDERING 2,
 *     INSERTION/DELETION 4) via a PYTHONPATH shim in tmp/ that monkeypatches
 *     the Scorer class attributes in the permuter process â€” no write to tools/,
 *     tools/permuter_campaign.py still the launcher.  It works: base prints 23
 *     (= 7*1 + 2*4 + 2*4) for this chassis and 29 for the const chassis, i.e.
 *     the aligned objective is MONOTONE with the sandbox ACROSS chassis
 *     (23<->7, 29<->9) where the stock one was not.
 *   * It still does not descend WITHIN a basin.  60k iterations over two
 *     campaigns: on this chassis 31.7k iterations produced only ties at 23, and
 *     all four screened to sandbox exactly 7 (perfect correlation at the tie
 *     point).  On the const chassis the one genuine descent, permuter 26 from
 *     base 29, screens sandbox 10 against that base's 9, and the 29-scored ties
 *     screen 9 / 10 / 12.  Random search cannot see our residual because
 *     difflib RE-ALIGNS the stream and our whole residual is positional.
 *   * So the permuter modality stays dead for this function, now for the right
 *     reason.  Do not spend a third session on it, and do NOT retry "fix the
 *     penalties" â€” that is done, banked, and measured.  The shim itself
 *     (tmp/grind/saEft01Init/s14/sbshim/) is reusable for any OTHER function
 *     whose gap is register/field differences rather than ordering, where the
 *     stock 100-vs-5 weighting hides the signal.
 *
 * ===========================================================================
 * SESSION 15 (forensics) â€” BODY STILL UNCHANGED; F26 IS ANSWERED AND HALF OF
 * IT IS KILLED.  THE RESIDUAL IS ONE DEPENDENCE EDGE, NOT A TIE-BREAK.
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  The instrumented cc1
 * (tools/gcc-2.7.2/cc1, BB2_PRIO_DEBUG / BB2_RANK_DEBUG / BB2_ALLOC_DEBUG, via
 * tmp/grind/saEft01Init/s10/idump.sh; both dumps verified CODEGEN-IDENTICAL to
 * the frozen build/cc1) was run on THIS form and on the const chassis:
 *
 *   * `rank_for_schedule`'s load/store dependence-CLASS tie-break is TOTALLY
 *     INERT in this block.  Every single in-block RANKDBG decision returns
 *     val=0 â€” 54 of 54 on this chassis, 49 of 49 on the const chassis, zero
 *     class-resolved either way.  Both operands are always class 3.  So among
 *     equal-priority insns the order is decided ONLY by INSN_LUID, i.e. by the
 *     incoming chain order.  F26's "if it is the load/store class test" branch
 *     is KILLED; no C spelling can act through that test here.
 *   * The two chassis have BYTE-IDENTICAL final INSN_PRIORITY vectors over all
 *     19 block insns.  RTX_UNCHANGING_P changes no priority at all â€” it
 *     changes the sched1 DAG, and sched1's output order then becomes sched2's
 *     LUID order.
 *   * The single edge is visible in the .sched LOG_LINKS:
 *       ours       insn 125 (`sw 16(sp)`) ... (insn_list 89
 *                    (insn_list:REG_DEP_ANTI 100 (insn_list 123 (nil))))
 *       const      insn 125 ... (insn_list 89 (insn_list 123 (nil)))
 *     insn 100 is `arg4 = tbl_125c[idx_1494[0]]`, the `lw a3` value.  The
 *     REG_DEP_ANTI (write-after-read on the outgoing-arg stack slot, emitted
 *     by sched.c's sched_analyze_1 because the load's MEM is aliasable) FORCES
 *     `lw a3` before `sw 16(sp)`.  Target has the opposite relation.  Marking
 *     that ONE load unchanging deletes the edge and produces target's relation
 *     â€” and costs 2, because sched1 then runs the idx[1] chain ahead of the
 *     idx[0] `lbu` and the two `lbu` come out in the wrong order.
 *   * A per-access `((const s32 *)tbl_125c)[idx_1494[0]]` on arg4 ALONE is
 *     byte-identical to the whole-pointer const declaration (same sched1
 *     order, same sched2 order, 9 / 91).  RTX_UNCHANGING_P on arg5's load is
 *     inert.  The const axis is now measured to the edge â€” do not re-probe it.
 *   * The dependence edge's DIRECTION is set by the pre-sched1 chain order,
 *     which calls.c fixes: a NAMED arg4 emits the load at the statement, i.e.
 *     BEFORE store_one_arg's `sw`, hence the anti-dep; a fully INLINE arg4
 *     emits the load in load_register_parameters, i.e. AFTER the `sw`, hence a
 *     true dependence in target's direction â€” which is why the fully-inline
 *     attractor lands `lw a3` at target's idx 61 but drags its whole idx[0]
 *     chain with it (13 / 91).  Target needs the idx[0] ADDRESS chain expanded
 *     early and only the final LOAD expanded late.  The obvious C for that
 *     (named ADDRESS local + inline deref as the argument) was re-measured
 *     this session: 9 / 91, one fewer RTL insn, same attractor.
 *
 * ===========================================================================
 * SESSION 16 (forensics) â€” BODY STILL UNCHANGED (7 / 91); F28 IS KILLED AND
 * THE MODEL OF THE SCHEDULER WAS WRONG SINCE SESSION 10
 * ===========================================================================
 *   * F28 asked for C that splits the idx[0] chain across calls.c's
 *     store_one_arg boundary.  The RTL dumps say we ALREADY have it: the
 *     named-ADDRESS-pointer form (rejected/named-pointer-inline-deref-9.c)
 *     emits the address chain at the block's three earliest LUIDs and defers
 *     the load to load_register_parameters, i.e. target's expand shape exactly
 *     â€” and scores 9.  Inline arg3 has always had the same split.  The
 *     residual is 100% a SCHEDULING outcome.  Do not re-probe the boundary.
 *   * At SCHED1 the block's INSN_PRIORITYs are FLAT: sixteen of nineteen picks
 *     carry LAUNCH_PRIORITY (0x7F000001) because sched.c:3985 +
 *     birthing_insn_p re-raise every register-birthing insn while
 *     reload_completed == 0.  The dependence-depth "levels" sessions 10/11/15
 *     reasoned about are a sched2 artefact, and sched2 only re-states sched1's
 *     order.  Order at sched1 = latency-queue release + INSN_LUID, nothing
 *     else (every in-block RANKDBG on this third chassis is val=0 too).
 *   * The identical block, with the identical target order, is in TWO other
 *     queue functions â€” cpu_side_move_dir_4 (7/160) and marionation_Exec â€”
 *     and our builds emit `lw a3` early in all three, including a body that
 *     names BOTH arg4 and arg5.  This is one shared problem, not a chassis
 *     quirk, and the wrapper is not implicated in it.
 *   * Next: F29 â€” replay sched1's backward pass (it is now a deterministic,
 *     simulable function) and INVERT it to get the required LUID order,
 *     instead of sweeping more C spellings.
 *
 * ===========================================================================
 * SESSION 17 (escalation) â€” BODY UNCHANGED (7 / 91); FUNCTION DISPOSED
 * ===========================================================================
 * Re-applied this body to src/system.c and re-measured: exactly 7 / 91,
 * rules_dropped 15.  One genuinely un-tried axis was found and closed, then
 * the driver-mandated disposition was reached.
 *
 *   * NEW AXIS â€” the INDEX globals (every prior global-spelling probe, s1's H1
 *     and s13's permuter find, respelled the TABLE reference D_800A125C; the
 *     index reads had never been respelled through the two separate byte
 *     globals src/system.c already declares).  arg5's index alone via
 *     D_800A1495 = 14 / 92.  BOTH indices via globals, dropping the idx_1494
 *     base = 24 / 89 â€” and that 89 is the useful number: two instructions
 *     SHORT of target, i.e. target demonstrably DOES hoist an index base, so
 *     the original held the two indices in ONE array, not two scalars.  The
 *     three hoisted bases are structurally required by target's instruction
 *     count, not merely score-preferred.  Banked as
 *     rejected/arg5-index-via-separate-D_800A1495-global-14-92.c and
 *     rejected/both-indices-via-globals-drops-idx-base-24-89.c.
 *   * scan_hand_coded --single saEft01Init: tier=LOW, score 1/8, the lone hit
 *     is S4 (front loads @ insn 46 â€” which IS this argument block, i.e. the
 *     ordinary load_register_parameters shape).  No STRONG S1/S2/S6 signal.
 *   * DISPOSITION: both endgame-lock AND-gates FAIL (LOW scan tier; and no
 *     citable SOTN precedent is even possible because no construct CLOSES the
 *     function â€” the floor is 7, not 0).  Per the owner's standing 2026-07-27
 *     auto-ruling, filed
 *     docs/grind/decisions.md:2789 "2026-08-01 â€” saEft01Init (src/system.c) â€”
 *     OWNER-ESCALATION â€” RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
 *     OWNER-ACCEPTED INCOMPLETE" and returned owner-gated.  The function is
 *     parked terminally; NOTHING is pending on the owner.
 *   * This is NOT a claim of unmatchability.  If the function is ever
 *     un-parked, start at F29 (build and VALIDATE the sched1 replay simulator
 *     against the four banked chassis dumps, then invert it) â€” not at another
 *     argument-spelling sweep, and not at the permuter.
 *
 * NEXT (if un-parked): hypotheses.md F29.  The question is no longer "which tie-break" â€”
 * there is no live tie-break.  It is: what pure-C shape splits the idx[0]
 * chain across calls.c's store_one_arg boundary, giving the address chain a
 * statement-expanded position and the load a load_register_parameters
 * position?  Do NOT re-sweep argument spellings (sixty-seven forms banked over
 * four rigid attractors) and do NOT re-probe const / RTX_UNCHANGING_P.
 * ===========================================================================
 */
/* ===========================================================================
 * SESSION 18 (2026-08-25) â€” CHASSIS RE-MEASURED ON THE MIGRATED/RENAMED TU
 * ===========================================================================
 * src/system.c has been renamed since this file was written: saEft01Init ->
 * CD_datasync, sys_VSync -> VSync, tslTm2LoadImage_2 -> puts, debug_printf ->
 * printf, cdrom_ClearIrq -> CD_flush (now DEFINED in the same TU at line 403,
 * no longer extern), &D_800161B8 -> &g_str_cd_timeout.  The body below is the
 * session-9 form transcribed into those names.  Re-measured this session:
 *     sandbox CD_datasync --disable all  ->  score 7, target_insns 91,
 *     build_insns 91, rules_dropped 0
 * so the floor is chassis-invariant across the asm-until-matched migration and
 * across CD_flush becoming an intra-TU call.  HEAD itself is INCLUDE_ASM (91).
 *
 * Session 18 measured 14 further spellings (all banked in rejected/): the
 * index-naming family (naming idx_1494[0] / idx_1494[1] as locals is codegen-
 * IDENTICAL to the fully-inline attractor, 13), the pointer-ADDRESS family
 * crossed with statement order (rigid at 9 regardless of which of the two
 * arguments is named or in which order), and arg3 combinations (12-13).  None
 * beat 7.  See hypotheses.md [s18].
 * =========================================================================== */

/* ===========================================================================
 * SESSION 21 (2026-09-04, rederive) - BODY UNCHANGED (7 / 91); THE ARGUMENT
 * BLOCK IS A CLOSED FOUR-ATTRACTOR SYSTEM AND THE WRAPPER HAS A TWIN
 * ===========================================================================
 * Re-applied and re-measured this session: score 7, target_insns 91,
 * build_insns 91, rules_dropped 0.
 *
 *   * The do{}while(0) wrapper is NOT the only route to the pointer ref-lift.
 *     Duplicating the timeout block into BOTH condition arms (no wrapper at
 *     all) emits assembly BYTE-IDENTICAL to this file - only .L label numbers
 *     differ - and sandbox-measures 7 / 91.  Banked as
 *     memory/grind/CD_datasync/alt_dup_into_arms_7.c.  It is a LATERAL, not an
 *     improvement: same floor, and the duplicated-statement-into-arms rule
 *     mandates its own /* FAKE * / annotation, so neither form carries fewer
 *     annotated devices.  Do not "clean up" this file by swapping them.
 *     Stacking BOTH lifts (duplicated arms each wrapped) OVERSHOOTS: 14, with
 *     the callee-save map rotated off target's $s0/$s1/$s3.
 *   * A FOURTH attractor was found - the named SCALED INDEX OFFSET,
 *     `k0 = idx_1494[0] * 4;` consumed as `*(s32 *)((u8 *)tbl_125c + k0)`.
 *     It is the first non-inline spelling to reproduce target's whole TAIL
 *     (`lw $a3` last at target's own build idx 56, `lw $a2` at 55, the
 *     D_800A11D5 chain in target's exact slots 49-53, and the idx[0] chain
 *     SPREAD rather than contiguous) - but idx[0]'s `lbu` sinks from 41 to 48
 *     and the arg5 `lw`/`sw` pair stays adjacent, costing a load-delay nop:
 *     8 / 92.  Head-correct and tail-correct remain mutually exclusive under a
 *     third independent geometry.
 *   * 86 argument spellings measured this session (a 78-form exhaustive
 *     cross-product of arg4 x arg5 x arg3 spelling x statement order, plus 8
 *     single-axis probes).  Minimum 7, reached by six byte-identical
 *     spellings.  NEW DEAD AXES: arg1 named, arg2 named / pointer-aliased /
 *     hoisted, `*idx_1494` vs `idx_1494[0]`, `*(tbl_125c + i)` vs
 *     `tbl_125c[i]`, an `i0` two-step, an arg4 same-value re-store, and
 *     `volatile u8 *idx_1494` (byte-inert here, a regression elsewhere -
 *     volatile does not pin this block's schedule).
 *   * A FRESH m2c decompile was run and contributes nothing: its arguments are
 *     the fully-inline 13-attractor and its single-accumulator tail costs an
 *     instruction (12 / 92).  This file's multiple-return tail already
 *     reproduces target's tail exactly.
 *   * Harness for any future session: tmp/grind/CD_datasync/s21/ scores a form
 *     in ~0.5 s and is calibrated - sandbox score = lev + (nop - 3).  The nop
 *     term is load-bearing; the s19 harness dropped nops and would have called
 *     the scaled-offset form a tie with this floor.
 * =========================================================================== */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    /* FAKE: do{}while(0) â€” loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 arg3;
        s32 arg4;
        s32 i4;
        puts(&g_str_cd_timeout);
        i4 = idx_1494[0];
        arg4 = *(tbl_125c + i4);
        arg3 = *(tbl_11dc + D_800A11D5);
        printf(&D_800161C8, D_800F19C0, arg3, arg4,
               *(tbl_125c + idx_1494[1]));
        CD_flush();
    } while (0);
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
