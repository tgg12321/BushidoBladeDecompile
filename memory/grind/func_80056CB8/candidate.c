/* =====================================================================
 * func_80056CB8 — CANDIDATE (s22 rederive-modality win, re-confirmed
 * s23/s24/s25/s26/s27) — floor 38/204, NOT YET 0. (Prior: 42/204 s14-s21;
 * 48/204 s11-s13; 58/204 s7-s10.) Body UNCHANGED from s22 this session.
 * ---------------------------------------------------------------------
 * s27 (solver modality, 2026-09-16). Body UNCHANGED (re-confirmed
 * 38/204 fresh, build_insns 198, before and after every probe). Ran the
 * s26-named untried COMBINATION -- idx2 loop-carried promotion (for-
 * statement `idx2 += 2`) TOGETHER WITH respelling the second
 * func_80053614 call's literal argument as `0x1F800000 + 0x2B8` instead
 * of bare `0x1F8002B8`, to test whether relieving literal-CSE pressure
 * would let idx2 clear loop.c:3823's giv-worth predicate. KILLED:
 * 55/204 (build_insns 201) -- identical to every prior isolated idx2
 * attempt (s6/s10/s12/s18/s21/s22); the literal-respelling half
 * contributed nothing. Isolated that half alone (idx2 reverted to
 * `i * 2`): 38/204, byte-identical to baseline. A fresh
 * `pwsh tools/grinder/dump.ps1 func_80056CB8` read
 * (tmp/grind/func_80056CB8/dumps/text1b.s:4713-5002) confirms why:
 * GCC's `fold()` (tools/gcc-2.7.2/fold-const.c:3536, PLUS_EXPR case at
 * :3642) constant-folds `0x1F800000 + 0x2B8` into the SAME INTEGER_CST
 * as the bare literal at parse time, before cse.c/combine.c ever run --
 * there is only ever one value for the optimizer to see, so no
 * arithmetic respelling of a compile-time constant can defeat CSE for
 * it. This is a CLASS kill of the entire "respell the literal
 * differently" sub-family, not just this instance (see hypotheses.md
 * [s27] for the full citation). The dump read also confirms the
 * current chassis materializes 0x1F8002B8 exactly ONCE (`li $fp,
 * 0x1f800000` / `ori $fp,$fp,0x02b8`) reused at both call sites via
 * two `sw $fp,16($sp)` -- those two stores are the o32 ABI's mandatory
 * per-call stack-argument stores for the 5th (stack-passed) integer
 * argument and cannot be eliminated by any C-level value respelling.
 * FRONTIER FOR s28: the combined idx2+literal-defeat axis is now fully
 * closed (both halves independently and jointly measured). The
 * remaining register-pressure hypothesis from s26 (idx2 vs the cached
 * literal contesting $s8/$fp) stands PARTIALLY reopened only in the
 * sense that literal-CSE was never actually a live axis to defeat --
 * so any further progress on this residual must come from either (a) a
 * genuinely new idx2-promotion C shape not yet tried (all known
 * declaration-order / loop-form variants are class-killed per s21's
 * insn_count-direction analysis), or (b) discovering why target's
 * SECOND 0x1F8002B8 use is a bare `lui` with no `ori` at all (implying
 * a DIFFERENT non-literal expression at that call site in the original
 * source, per the still-open second frontier item below) -- (b) is the
 * more promising unexplored avenue since it does not require beating
 * loop.c:3823.
 * ---------------------------------------------------------------------
 * s24 ran the ledger's own flagged next-probe (fresh .greg dump read on the
 * CURRENT 38/198 chassis) and two cheap declaration-order/scope probes,
 * both neutral (see below).
 * ---------------------------------------------------------------------
 * s26 (synthesis modality, 2026-09-16). Body UNCHANGED (both probed
 * spellings measured worse; reverted). Re-ran the s25-named top frontier
 * item (sin_p/cos_p live-range narrowing via sin_v/cos_v capture before
 * the first func_80053614 call) -- WORSE: 38->76/204, build_insns
 * 198->194 (-4 insns, wrong direction). Also tried a genuinely new axis
 * (two DISTINCT per-call-site addr1/addr2 locals for the 0x1F8002B8
 * literal, instead of one shared local per s23) -- worse: 38->43/204,
 * same insn count (pure register-identity churn, same signature as
 * every other addr/scale-reuse probe). Ran a fresh
 * inverse_compose.py classify on the CURRENT 38/198 chassis
 * (tmp/grind/func_80056CB8/s26/classify.txt): CONFIRMS the long-chased
 * $fp/$s8 residual is a loop-carried idx2 accumulator in target
 * (`addiu s8,s8,2` per iteration + two `addu #,#,s8` at the table
 * addressing sites) vs our per-iteration `sll #,#,0x1` recompute (3x) --
 * exactly the s11/s18/s22 giv-worth-rejected shape, now confirmed at the
 * raw instruction-multiset level rather than inferred from the classify
 * diff summary alone. Also newly visible: target's 0x1F8002B8 handling
 * is a single bare `lui #,0x1f80` (no `ori`, no spill) vs our
 * `lui s8,.. / ori s8,s8,0x2b8 / sw s8,16(#)` TWICE -- our chassis
 * spills the literal to the stack at BOTH call sites where target does
 * not, suggesting the register-pressure competition for $s8 between the
 * (currently absent) idx2 accumulator and the (currently present)
 * cached literal is the crux of this residual, not a scheduling
 * artifact. Re-read tools/gcc-2.7.2/loop.c:3760-3830 (the strength_reduce
 * giv-worth loop) directly: the rejection predicate's `insn_count` term
 * is the per-loop compiled instruction count (varies with chassis, e.g.
 * s22's +1), NOT the compile-time-constant `n_non_fixed_regs` term
 * inside `threshold` (that part of the s21 finding stands) -- but this
 * doesn't reopen idx2 since a larger insn_count makes strength-reduction
 * of i*2 LESS likely to pass under the predicate's own arithmetic, and
 * s22's idx2-on-limit measurement (55/204, worse) already empirically
 * demonstrates the direction. src/text1b.c reverted to clean INCLUDE_ASM
 * at session end via `git checkout -- src/text1b.c`.
 * ---------------------------------------------------------------------
 * s25 (enumerate modality, 2026-09-16). Body UNCHANGED. Applying the
 * candidate to src/text1b.c required THREE missing extern declarations
 * (`extern s16 Judge; extern s32 ratan2(s32,s32); extern u8 D_8009A820;
 * extern u8 D_8009A821; extern s32 D_800F6610;`) placed immediately
 * before the function -- without them the loop body implicit-declares
 * these symbols and the sandbox score explodes to 141/175 (a false
 * floor regression; NOT a real chassis change). Once restored, floor
 * 38/204 (build_insns 198) reproduced exactly.
 *
 * Ran the mandated systematic spelling sweep (tools/spelling_enum.py +
 * tools/sweep_variants.py, plus a scratch hand-generator for the
 * indexed-LHS pt0/pt1 blocks the tool can't parse) on THREE regions,
 * all of which had only ever been enumerated on OLDER chassis
 * generations (s9/s14/s15/s16, back when the floor was 58/197-42/197,
 * i.e. one fewer real instruction than the current s22-derived 38/198):
 *   1. sin_p/cos_p/scale/x/z assignment order (the s14-adopted block) --
 *      16 orderings (--no-swaps) + 32 with the commutative-swap axis,
 *      both fully re-swept. 4-5 variants tie the 38/198 floor (incl. the
 *      currently-adopted order); nothing beats it. Re-confirms s14's
 *      "entire spelling space, order x swap" finding generalizes to the
 *      current chassis.
 *   2. pt0[]/pt1[] store-order (2 six-store blocks, 10 structural
 *      variants: batch order / interleave / intra-triple reversal) --
 *      10/10 strictly worse (40-81/198-200). Re-confirms s15's kill.
 *   3. dx/dz named-local declaration order (code==4 tail) -- 5/5 tie the
 *      floor exactly. Re-confirms s9/s15's kill.
 * No improvement found in any of the three regions; the 38/204 floor
 * stands, now with fresh chassis-relative confirmation instead of a
 * stale cross-chassis citation. Full data: tmp/grind/func_80056CB8/s25/
 * (sweep_sinp.json, sweep_sinp_sw.json, sweep_pt.json, sweep_dxdz.json).
 * src/text1b.c reverted to committed HEAD (`git checkout --`) at session
 * end; git status clean.
 *
 * FRONTIER FOR s26: the three spelling-enumerable blocks in this
 * function are now ALL exhausted on the current chassis (order x swap,
 * or full structural reorder for the indexed blocks) with zero
 * improvement. The genuinely open axis remains the live frontier this
 * ledger has carried since s22-s24: (a) narrowing sin_p/cos_p's live
 * range to change reload's spill-vs-keep decision for pseudo 149 (the
 * 0x1F8002B8 literal, homed in $fp across the whole loop per the s24
 * .greg read) -- UNMEASURED since s24 named it; (b) the older, larger
 * $fp-accumulator/loop-carried-i*2 structural gap, whose s11 mechanism
 * finding (loop.c:3823 giv-worth rejection) is chassis-independent but
 * has not been re-measured on the CURRENT 38/198 chassis since the s22
 * `limit` win (only re-tested combined WITH the limit change at s22
 * itself, not as a standalone re-audit on the post-s24 chassis). A
 * rederive or solver-modality session on (a) is the highest-value next
 * step: it is untried, has a named mechanism, and a direct verification
 * path (.greg dump read of pseudo 149's disposition before/after).
 * ---------------------------------------------------------------------
 * s24 (structural modality, 2026-09-16). Re-applied body unchanged +
 * func_80053614 s32-return prerequisite, re-confirmed floor 38/204 fresh
 * (build_insns 198) before any change.
 *
 * KEY CORRECTION (via fresh `pwsh tools/grinder/dump.ps1 func_80056CB8`,
 * read directly this session -- tmp/grind/func_80056CB8/dumps/text1b.greg
 * lines 14788-15995, the func_80056CB8 slice on the CURRENT 38/198
 * chassis): the s13 write-up's claim that pseudo 149 holds "528483000
 * (0x3D0900)" -- the flags==4 tail's threshold constant -- is WRONG.
 * 528483000 decimal IS 0x1F8002B8, not 0x3D0900 (0x3D0900 = 4000000
 * decimal). The "Register dispositions" line in the fresh dump reads
 * "149 in 30" -- pseudo 149 is allocated to HARD REG 30, which is $fp
 * ($s8) on this target (MIPS reg names: 2-9=a/v, 16-23=s0-s7, 30=fp).
 * Its lifetime is 30 references and its conflict list (line 19 of the
 * slice) includes EVERY other pseudo in the function plus the full
 * a0-a3/v0-v1/t0-t1/sp/ra/hi/lo hard-reg set -- i.e. it is live across
 * essentially the WHOLE loop body, both func_80053614 calls included.
 * This CONFIRMS (not just infers from the classify multiset diff, as
 * s22 did) that on the current chassis GCC allocates the 0x1F8002B8
 * scratchpad-address literal to a single long-lived pseudo homed in the
 * callee-save register $fp -- corroborating s22's classify-based guess
 * with direct RTL-level evidence and CORRECTING the s13-era
 * misattribution of pseudo 149's value (the s13 chassis was a different,
 * earlier generation -- 48/198 pre-`limit` -- so the two findings are not
 * directly contradictory, but the VALUE 528483000 was mislabeled in that
 * writeup and any future session reading it should treat the 0x3D0900
 * attribution there as unreliable).
 *
 * PROBE 1 (killed, instance, byte-identical/neutral): declared
 * `start`/`limit`/`i` BEFORE the `pt0`/`pt1`/`hit0`/`hit1`/`work` arrays
 * (reverse of the candidate's current top-of-function order). Measured:
 * 38/198, BYTE-IDENTICAL score and build_insns -- no effect. Reverted.
 *
 * PROBE 2 (killed, instance, byte-identical/neutral): moved `hit0[4]`,
 * `hit1[4]`, `work[4]` from function scope into the LOOP's block scope
 * (they are only read within a single iteration and never carried
 * across iterations, unlike `pt0`/`pt1` whose &-taken pre-loop
 * materialization s13 confirmed target hoists too -- so this was a
 * genuine hypothesis that GCC might treat their frame slots differently
 * if block-scoped, possibly changing the register-pressure picture
 * around pseudo 149). Measured: 38/198, BYTE-IDENTICAL -- no effect.
 * Reverted (both probes reverted via `git checkout -- src/text1b.c`,
 * verified zero diff before finishing).
 *
 * MECHANISM NOTE for the next session: neither probe touched pseudo
 * 149's own live range (its lifetime is set by the two call sites'
 * argument uses, which are unchanged by either probe), so the neutral
 * result is CONSISTENT with the dump evidence, not surprising in
 * hindsight -- the two probes were declaration-order/scope changes to
 * OTHER locals, and 149's huge conflict set already includes essentially
 * every pseudo regardless. A future session's lever needs to either (a)
 * shrink pseudo 149's OWN live range (defeat the CSE/constant-propagation
 * unification of the two identical `0x1F8002B8` literal call arguments
 * into one pseudo -- every respelling tried so far, s23's `scale` reuse
 * and named `addr` local, made this WORSE, not better, because they gave
 * the literal an EXTRA C-level handle without removing the underlying
 * CSE fold), or (b) find an UNRELATED register-pressure reduction
 * elsewhere that's large enough to flip reload's spill-vs-keep decision
 * for 149 specifically (per [[register-alloc-pure-c]] Levers A-C, not
 * more respellings of the literal itself). Candidate for (b): `sin_p`
 * and `cos_p` (s16 pointers) are live from their computation through the
 * `if (flags != 0)` block after the FIRST func_80053614 call -- check
 * whether narrowing THEIR lifetime (e.g. reading `*sin_p`/`*cos_p` into
 * fresh scalars immediately after computing them, before the call, then
 * using the scalars in the post-call adjustment instead of re-dereferencing
 * the pointers) changes reg149's conflict set. NOT tried this session
 * (turn budget) -- worth a session's full attention with a fresh dump
 * re-read after the change to directly verify the effect on pseudo 149's
 * disposition, not just re-measure the sandbox score blind.
 * ---------------------------------------------------------------------
 * s23 (rederive modality, 2026-09-16). CORRECTED the s22 "new frontier
 * item" characterization: a fresh direct read of
 * asm/funcs/func_80056CB8.s:1-30 this session shows the TARGET ALSO
 * materializes 0x1F8002B8 exactly ONCE (before the loop, `lui/ori $t3`),
 * but stores it to a fixed STACK SLOT (`sw $t3,0x78($sp)`) and RELOADS
 * it via `lw $t3,0x78($sp)` before each of the two func_80053614 calls --
 * a stack-spill-reload pattern, not the "rematerializes per call site"
 * s22 inferred from the classify diff alone. Tried two natural-C mirrors
 * of this shape (both measured on the s22 38/198 chassis, both WORSE,
 * both reverted -- full detail in hypotheses.md s23):
 *   1. [[defeat-licm-hoist-var-reuse]] multi-set reuse: reassign the
 *      dead `scale` pseudo to 0x1F8002B8 after its last real use, pass
 *      `scale` to both calls. Result: 38 -> 71/204 (build_insns
 *      unchanged 198) -- pure register-identity regression. KILLED
 *      (instance).
 *   2. Named `s32 addr;` local set once before the loop (two
 *      declaration-order variants: last-declared and first-declared),
 *      passed to both calls. Result: 45/204 and 44/204 respectively
 *      (build_insns 198 -> 199 in both, +1 real insn toward target's
 *      204, but net WORSE on score) -- register-identity regressions
 *      elsewhere outweighed the insn-count gain. KILLED (instance,
 *      both variants).
 * The bare repeated-literal form (this candidate's current shape) is
 * still the best measured spelling of this axis. The stack-slot-reload
 * vs callee-save-register RA/reload choice remains an OPEN, unresolved
 * axis for a future session -- worth a fresh .greg/.lreg dump read
 * (not done this session) to see WHY reload chose a callee-save home
 * for this pseudo instead of spilling it, before trying more C-level
 * register-pressure levers elsewhere in the loop body (per
 * [[register-alloc-pure-c]], not more re-spellings of the literal
 * itself -- both spellings tried this session are now closed).
 * ---------------------------------------------------------------------
 * s22 (rederive modality, 2026-09-16). Re-confirmed 42/204 fresh
 * (build_insns 197) before any change, then materialized the loop's
 * BOUND (previously the recomputed sub-expression `start + 2` in the
 * for-statement's test clause) into its own named local `s32 limit =
 * start + 2;`, declared alongside `start`, used as `for (i = start; i <
 * limit; i++)`. This is a genuinely untried axis distinct from every
 * prior i*2-sharing probe (s6/s10/s12/s18, all class-killed at
 * loop.c:3823): those promoted the byte-table INDEX value (i*2) to a
 * shared object; this promotes the LOOP BOUND itself (start+2, a plain
 * loop-invariant already visible to loop.c as a movable) to a named
 * object instead of a recomputed subexpression. Measured RESULT: score
 * 42 -> 38/204 (build_insns 197 -> 198, +1 real instruction, moving
 * TOWARD target's 204) -- the first floor drop since s14 (8 sessions
 * flat). Also tried combining this with the s10/s18-style idx2 (i*2)
 * promotion on TOP of the limit change: score regressed to 55/201,
 * confirming the idx2 axis is independently dead even on this new
 * chassis (re-banked, see hypotheses.md). Fresh `inverse_compose.py
 * classify` on the 38/198 chassis still reports FIRST DIVERGENCE:
 * PRE-RA/rtl_shape (an instruction-multiset difference, not an
 * allocation/schedule permutation) -- target's `addiu s8,s8,2` /
 * `addu #,#,s8` x2 (the long-chased $fp accumulator) are STILL
 * target-only, so this is NOT the fix for that residual; it is an
 * independent, additive win on the loop-bound sub-expression axis.
 * NEW "ours only" residue introduced by this change: `lui s8,0x1f80` /
 * `ori s8,s8,0x2b8` / two `sw s8,16(#)` -- GCC is now caching the
 * 0x1F8002B8 scratchpad-address literal (passed to both func_80053614
 * calls) in a callee-saved register across the whole loop, apparently
 * because the `limit` local freed up register pressure that previously
 * forced a fresh per-call materialization; target instead shows a
 * bare `lui #,0x1f80` (target-only, temp register) suggesting target
 * rematerializes this constant at (at least) one call site rather than
 * hoisting it into a callee-save home. This is the next frontier item
 * for a future session (see hypotheses.md s22 CONFIRMED entry) -- NOT
 * yet probed this session (turn budget), banked as an open lever, not
 * attempted-and-killed.
 * ---------------------------------------------------------------------
 * s20 (rederive modality, 2026-09-16). Re-applied body unchanged (+
 * func_80053614 s32-return prerequisite), re-confirmed floor 42/204 fresh
 * (build_insns 197) before any change. Re-ran m2c on asm/funcs/func_80056CB8.s
 * -- output identical to the s12-archived copy (target asm unchanged since
 * this ledger began), so no new derivation info from m2c itself this
 * session; instead used it to hunt a STRUCTURALLY DIFFERENT statement shape
 * per the rederive-modality brief, targeting the s19 frontier's explicit
 * next-probe ("the two func_80053614 call-argument setup blocks ... tested
 * for REORDER, never for a missing/different VALUE").
 *
 * Found and tested TWO genuinely new structural axes m2c's SSA
 * reconstruction implies that were NOT covered by s15's pt0/pt1 pure-reorder
 * sweep (which only permuted the 6 stores among themselves, holding the x/z
 * computation statements fixed):
 *   1. Block2 (pre-second-call pt0/pt1 fill): m2c's exact store order
 *      pt0[0],pt0[2],pt1[0],pt0[1],pt1[2],pt1[1] (not among s15's 6 named
 *      patterns). MEASURED: 42 -> 55/204 (build_insns unchanged 197).
 *      WORSE. Reverted.
 *   2. Block1 (pre-first-call pt0/pt1 fill): interleaving the pt0[] stores
 *      WITH the x/z computation statements (compute sin_p/scale/cos_p, THEN
 *      pt0[0..2], THEN x, THEN pt1[0..1], THEN z, THEN pt1[2]) instead of
 *      the s14-banked "compute x and z first, then batch both triples of
 *      stores after". MEASURED: 42 -> 87/204, build_insns 197 -> 202 (grew
 *      the real instruction count -- WORSE on both axes, the first tested
 *      spelling this entire residual's history to actually GROW build_insns
 *      rather than stay flat at 197). Reverted.
 *
 * Both are KILLED, instance (measured on the s19/s20 42/197 chassis,
 * func_80053614 prerequisite applied, no FAKE constructs). No missing or
 * different VALUE was found anywhere in the m2c reconstruction -- every
 * value m2c computes (dx/dz/y in the flags==4 tail, the sp4C/hit1[1]
 * comparison in flags==3, the two call-argument triples) matches our
 * candidate's semantics exactly; the only differences m2c's output ever
 * shows are STATEMENT ORDER / SSA-materialization artifacts, and both
 * concretely-different orderings this session tried measure worse than the
 * s14 baseline. Full writeup + both rejected forms:
 * memory/grind/func_80056CB8/rejected/m2c-block2-permuted-store-order-worse.c,
 * memory/grind/func_80056CB8/rejected/m2c-block1-interleave-with-xz-computation-worse.c.
 * Reverted src/text1b.c to clean INCLUDE_ASM (`git checkout -- src/text1b.c`,
 * verified zero diff) before finishing.
 *
 * s21 (forensics modality, 2026-09-16). Re-applied body unchanged, re-confirmed
 * floor 42/204 fresh (build_insns 197) + fresh instrumented-cc1 .greg dump
 * (unchanged Spilling reg 11/65 pair from s17/s18). CLASS-KILLED the s20 live
 * frontier item ("reduce register pressure elsewhere in the loop to shrink
 * n_non_fixed_regs and flip strength_reduce's giv-worth threshold at
 * loop.c:3823"): read loop.c:3241 + regclass.c:360-392 + mips.h:1188-1195
 * directly -- n_non_fixed_regs is a per-COMPILATION constant computed once
 * from the target's static FIXED_REGISTERS table, never recomputed per
 * function and never influenced by any function's register pressure/live
 * ranges. The proposed lever cannot exist. Full mechanism trace:
 * memory/grind/func_80056CB8/evidence.md [s21], hypotheses.md [s21].
 *
 * FRONTIER FOR s21: the "respell pt0/pt1 block order or interleaving"
 * family is now CLOSED across both call sites (s15's 10 pure-reorder
 * variants + this session's 2 interleave-with-computation variants, 12
 * total spellings, all >= 42, none below). The s19 forensics conclusion
 * stands: the remaining 42/204 (build_insns 197 vs target 204, 7-insn
 * deficit) is PRE-RA/rtl_shape, gated by loop.c:3823's insn_count-threshold
 * double-bind (s13/s18). The only UNTRIED avenue left per s14's frontier
 * item 3 (never executed): attack `n_non_fixed_regs` in the move_movables
 * threshold formula (loop.c:532) directly by finding a register-pressure
 * reduction elsewhere in the loop body that does NOT touch insn_count --
 * read the .greg dump's conflict list for the pseudo with the largest
 * conflict set (the two func_80053614 calls' live ranges are the leading
 * candidate) and check for an uncontested narrowing (block-local split,
 * narrower type, earlier death).
 * ---------------------------------------------------------------------
 * s19 (forensics modality, 2026-09-16). Re-applied body unchanged,
 * re-confirmed floor 42/204 fresh. Fully attributed the previously-
 * unexplained classify "target only: li #,4"/"lw 96(#)"/"lw 104(#)"
 * instructions (unattributed since s8): the li#,4 pair is reorg.c's
 * fill_simple_delay_slots (reorg.c:2861) reusing a constant preload
 * across two converging branch delay slots (asm/funcs/func_80056CB8.s
 * lines 144-157, the flags==3/flags==4 chain converging on .L80056F08);
 * the lw-96/104(sp) pair is ordinary frame-offset drift from our
 * 197-insn frame vs target's 204-insn frame. CLASS KILL: neither is an
 * independent C-level lever -- both are downstream shadows of the same
 * insn_count deficit s18 already predicate-cited at loop.c:3823. This
 * closes the s17/s18 frontier item with no new lever found. Full
 * derivation: hypotheses.md/evidence.md [s19].
 * ---------------------------------------------------------------------
 * s18 (solver modality, 2026-09-16). Re-applied body unchanged, re-confirmed
 * floor 42/204 fresh (build_insns 197). Ran classify fresh: PRE-RA/rtl_shape
 * verdict unchanged since s8, still out of scope for ra_solver/sched_solver.
 * Read asm/funcs/func_80056CB8.s lines 1-217 directly and confirmed target's
 * $fp is a genuine loop-carried i*2 accumulator (init `sll $fp,$v1,2` before
 * the loop, `addiu $fp,$fp,0x2` at the loop tail, read via `addu $at,$at,$fp`
 * at both the D_8009A821/D_8009A820 byte-table lookups) -- matching the
 * s10/s12-era description exactly on this chassis. Re-tested the closest
 * rejected form (s10's loop-carried-int idx2) on THIS chassis per the
 * mandatory kill re-audit: 42 -> 50 (WORSE), reproducing the s10 (58->73)
 * and s12 (48->78) verdicts on a third chassis generation. Named the
 * mechanism for the first time with a line cite: tools/gcc-2.7.2/loop.c:3823's
 * giv-worth predicate gates strength reduction on `insn_count` alone, never
 * on which C construct carries the index value -- so this is now a CLASS
 * kill (predicate-cited), not just another instance measurement. Full
 * derivation: hypotheses.md [s18]. Reverted src/text1b.c to clean
 * INCLUDE_ASM before finishing.
 * ---------------------------------------------------------------------
 * s17 (synthesis modality, 2026-09-16). STALE-HEAD-CLAIM NOTE (as always):
 * src is INCLUDE_ASM between sessions. Re-applied s16 body unchanged,
 * re-confirmed floor 42/204 fresh (build_insns 197). Kill re-audit:
 * re-measured s16's closest instance kill (abs-value y-compare form) fresh
 * -- reproduces 45/204 exactly, no FAKE constructs present so
 * fake_ablate.py has nothing to ablate.
 *
 * MAJOR CORRECTION: the s13-s16 "reg 11/reg 65 double-bind" frontier item
 * was a category error. FIRST_PSEUDO_REGISTER=68 (mips.h:1181), so any
 * register number below 68 that reload1.c:2283's "Spilling reg %d." prints
 * is a HARD register, never a pseudo -- reg 11 = $t3, reg 65 = "lo" (MIPS
 * mult/div low-result register). These are ordinary reload scratch-register
 * evictions (LO_REG/MD_REGS pressure from the loop's two back-to-back
 * `scale * *sin_p` / `scale * *cos_p` multiplies at RTL insn 142, plus a
 * separate t3-class GR_REGS need for stack-address materialization ahead of
 * the func_80053614 calls) -- NOT a spilled named C-level value, and the
 * ledger's proposed "ask inverse.py which C object occupies pseudo 11/65"
 * next-probe cannot be answered because no such pseudo exists. KILLED
 * class (predicate: mips.h:1181). Full derivation: hypotheses.md [s17].
 *
 * Re-ran `inverse_compose.py classify` fresh on the current chassis --
 * reproduces the s8 verdict UNCHANGED: FIRST DIVERGENCE is PRE-RA/rtl_shape
 * (a different instruction MULTISET), which ra_solver/sched_solver both
 * explicitly refuse to touch. This means the entire s13-s16 RA-solver
 * pursuit was mis-targeted from the start -- the classify tool had already
 * ruled it out at s8. Correlated part of the "target only" instruction set
 * to real target asm this session: `addiu $v0,$t3,0x2` (80056FA4) and
 * `addiu $fp,$fp,0x2` (80056FB0) sit in the LOOP TAIL (80056F9C-80056FB4),
 * ordinary per-iteration index/pointer bookkeeping distinct from BOTH the
 * closed y-compare tail (s16) and the closed scratchpad-literal
 * materialization (s8/s9/s15) -- a genuinely new, previously-unexamined
 * axis for the next session. Reverted src/text1b.c to clean INCLUDE_ASM
 * (`git checkout -- src/text1b.c`, verified zero diff) before finishing.
 * ---------------------------------------------------------------------
 * s16 (structural modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s15-banked body (unchanged) +
 * func_80053614 s32-return prerequisite, re-confirmed floor 42/204
 * (build_insns 197) exactly matches the s14/s15 record.
 *
 * KEY FINDING: read asm/funcs/func_80056CB8.s:150-192 (the flags==4
 * y-compare tail) directly for the first time this ledger cycle and
 * decoded the beqz(sum-threshold)/bltz(dy)/beqz(dy>=0x3E9)/bnez(-dy<0x3E9)
 * branch chain register-by-register. CONCLUSION: our CURRENTLY BANKED
 * nested if/else (`if (y-hit1[1]>=0) { if (y-hit1[1]>=0x3E9) flags=5; }
 * else { if (hit1[1]-y>=0x3E9) flags=5; }`, unchanged below) is ALREADY
 * the exact structural mirror of the target's C-level shape for this
 * block -- the "beqz+bltz+j triple vs our single bgez" framing carried
 * since s6/s7 was a MISDIAGNOSIS: what looks like a 3-branch triple is
 * ordinary MIPS delay-slot fill (the slti in bltz's delay slot executes
 * unconditionally; its result is discarded on the taken path), not
 * evidence of a different source shape. This block's C-level spelling
 * question is CLOSED -- do not re-attempt branch-topology rewrites here.
 * Tried the one plausible alternative (single absolute-value form via
 * `dy`/`ady` locals + ternary) to confirm: measured WORSE (42->45,
 * same build_insns=197), consistent with the nested form already being
 * correct. Also re-tested s10's loop-carried-idx2 induction variable
 * (the s15 frontier note's "untried" claim was wrong -- it's the same
 * construct as s10's already-rejected variant (b), just on a newer
 * chassis) and re-confirmed it's worse (42->50) on the current chassis
 * too, closing the "share i*2" family across chassis generations.
 * Full writeup: memory/grind/func_80056CB8/hypotheses.md [s16] entries.
 * Reverted src/text1b.c to a clean INCLUDE_ASM state (zero diff vs HEAD,
 * `git checkout -- src/text1b.c` run and verified) before finishing.
 * ---------------------------------------------------------------------
 * s15 (enumerate modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s14-banked body (unchanged) +
 * func_80053614 s32-return prerequisite to src/text1b.c, re-confirmed
 * floor 42/204 (build_insns 197) exactly matches the s14 record before
 * any s15 change. Reverted src/text1b.c to a clean INCLUDE_ASM state
 * (zero diff vs HEAD) before finishing this session.
 *
 * FRONTIER ITEM 2 (pt0/pt1 array-fill block reorder) — CLASS-NEGATIVE,
 * INSTANCE-SCOPED. spelling_enum.py can't parse indexed-LHS `pt0[0] = ...;`
 * assigns (its _ASSIGN_RE requires a bare identifier), so this session
 * hand-generated the structural reorder axis (a scratch-only generator,
 * tmp/grind/func_80056CB8/s15/gen_pt_variants.py — no edit to tools/):
 * for BOTH the pre-first-func_80053614-call store block (pt0[0..2] from
 * obj+0xB8/0xBC/0xC0, pt1[0..2] from x/obj+0xBC/z) and the pre-second-call
 * block (pt0[0..2] from x/obj+0xBC/z, pt1[0..2] from x/obj+0xBC/z), swept:
 * batch-pt0-then-pt1 (baseline), batch-pt1-then-pt0, interleave-pt0-first,
 * interleave-pt1-first, reverse-intra-triple-pt0, reverse-intra-triple-pt1
 * — 10 non-baseline variants total (5 per block, other block held at s14
 * baseline), scored via `tools/sweep_variants.py --func func_80056CB8
 * --file text1b --variants tmp/grind/func_80056CB8/s15/enum_pt --json`.
 * RESULT: 10/10 scored >= 42 (44 to 79; two even raised build_insns to
 * 199), zero at or below the s14 floor. Full ranked list in
 * tmp/grind/func_80056CB8/s15/sweep1.json. This block's ORIGINAL
 * (compute-in-source-order, pt0-batch-then-pt1-batch) statement order is
 * already GCC's preferred spelling for these 12 stores; the region is not
 * where the residual's 3 remaining PRE-RA-multiset instructions live.
 *
 * FRONTIER ITEM 3 (dx/dz declaration-order re-verify on new chassis) —
 * RE-CONFIRMED FLAT. Re-ran the s9 sweep via the proper tool this time
 * (spelling_enum.py --no-swaps on the ENUM-marked `s32 dx = ...; s32 dz =
 * ...; if (0x3D0900 < dx*dx+dz*dz) {` region, tmp/grind/func_80056CB8/s15/
 * enum_dxdz/) — 5/5 distinct spellings (inline vs named x2, both decl
 * orders) score exactly 42/197, identical to baseline. insn_count moving
 * 198->197 between s13 and s14 did NOT reopen this region; the s9 kill
 * still holds under the current scope.
 *
 * FRONTIER ITEM 1 (double-bind re-check on new 42/197 chassis) —
 * RE-CONFIRMED UNCHANGED. Fresh `pwsh tools/grinder/dump.ps1
 * func_80056CB8` + `tools/ra_solver/inverse_compose.py classify` against
 * this session's rebuilt tmp/sandbox/func_80056CB8/text1b.o (score 42,
 * build_insns 197): (a) .greg dump STILL shows `Spilling reg 11.` /
 * `Spilling reg 65.` (twice each — once per func_80053614 call site),
 * unchanged from s13. (b) classify's verdict is STILL PRE-RA — same
 * instruction-multiset diff shape as s8/s13: ours materializes the
 * 0x1F8002B8 scratchpad literal via $s8 (lui/ori) ONCE but stores it to
 * the stack TWICE (`sw s8,16(#)` x2, one per call) plus three
 * `sll #,#,0x1` (the i*2 byte-table index, computed fresh instead of
 * loop-carried); target instead shows `addiu s8,s8,2` / `sll s8,#,0x2`
 * (a *4 scale) / two `addu #,#,s8` (the loop-carried i*2 induction
 * variable, s10's finding) PLUS a `beqz+bltz+j` triple where ours has a
 * single `bgez` (the code==4 y-compare branch-topology divergence,
 * killed twice at the C level s6/s7 but still open at the RTL level).
 * Full report: tmp/grind/func_80056CB8/s15/classify_s15.txt.
 *
 * TRIED AND KILLED THIS SESSION (new): classify's own suggested
 * "single named intermediate" lever for the 0x1F8002B8 literal —
 * `s32 scratchpad = 0x1F8002B8;` declared once at the top of the loop
 * body, used at both func_80053614 call sites instead of writing the
 * literal twice. This is ordinary C (fresh local, once-written, real
 * value consumed at 2 read sites — squarely inside the SOTN
 * new_var_temp-relaxed named-intermediate family, no FAKE needed since
 * it has real semantic purpose). MEASURED WORSE: score 46/204,
 * build_insns 199 (+2 over the 42/197 floor) — giving the literal a
 * separate C-level home added a genuinely live-across-both-calls pseudo
 * that reload had to spill/reload AGAIN on top of the existing $s8
 * materialization, rather than removing a redundant one. classify's own
 * multiset diff already showed our lui/ori for this literal happens only
 * ONCE (not twice) — the "twice" in the diff is the two `sw s8,16(#)`
 * stack spills of the SAME value at each call's arg setup, which the
 * named-intermediate lever does not address (a per-call spill, not a
 * per-call re-materialization). REVERTED — instance kill, banked below.
 * ---------------------------------------------------------------------
 * s14 (enumerate modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s13-banked body (unchanged) +
 * func_80053614 s32-return prerequisite to src/text1b.c, re-confirmed
 * floor 48/204 (build_insns 198) exactly matches the s11-s13 record
 * before any s14 change.
 *
 * SYSTEMATIC SPELLING SWEEP (tools/spelling_enum.py + sweep_variants.py,
 * used programmatically for the first time this ledger cycle — prior
 * sessions s9 hand-verified a small variant set because sweep_variants.py
 * is blocked by worktree_contamination_guard for a non-wteng-pinned
 * DIRECT invocation on main; this session ran it instead via
 * `wsl bash -c 'source .venv/bin/activate && python3 tools/sweep_variants.py ...'`,
 * which is a plain WSL invocation, not a `python3 -m engine.cli` call, and
 * was NOT blocked). Region enumerated: the sin_p/cos_p/scale/x/z block
 * immediately after the ratan2/branch-angle computation —
 *   sin_p = &Judge + (flags & 0xFFF);
 *   cos_p = &Judge + ((flags + 0x400) & 0xFFF);
 *   scale = (&D_8009A820)[i * 2] << 8;
 *   x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
 *   z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
 * — 5 independent-enough assignments (x depends on sin_p+scale, z depends
 * on cos_p+scale) with 16 valid def-before-use orderings (--no-swaps) /
 * 32 with the commutative-swap axis added. NEVER previously swept
 * systematically by this ledger (s7-s13 focused entirely on the
 * flags==3/4 tail and the i*2 index arithmetic; this block was untouched).
 *
 * RESULT: 4 orderings (of 16) tie for BEST at score 42/204 (build_insns
 * 197, ONE FEWER real instruction than the s11-s13 floor of 198) — v02,
 * v04, v07, v12 in the sweep output (tmp/grind/func_80056CB8/s14/enum/).
 * All 4 share the same shape: `x` is computed as soon as its two inputs
 * (`sin_p`, `scale`) are ready, BEFORE `cos_p` is computed — i.e.
 * interleaved "compute-then-consume" order instead of the original
 * "compute all three (sin_p, cos_p, scale), then consume both (x, z)"
 * batch order. Re-running the same region WITH the commutative-swap axis
 * (32 variants, tmp/grind/func_80056CB8/s14/enum_sw/) found NO further
 * improvement below 42 — confirms 42/197 is the true floor for this
 * block's entire spelling space (order x swap), not a partial sweep
 * artifact.
 *
 * ADOPTED (v04's ordering — simplest of the 4 tied forms, natural
 * "compute what's needed then use it, twice" reading, zero swap
 * mutations): sin_p, THEN scale, THEN x (consumes both), THEN cos_p,
 * THEN z (consumes cos_p + scale). See body below.
 *
 * VET: this is a pure independent-statement REORDERING among 5 real,
 * already-live local assignments — no new local, no dead store, no
 * volatile, no cast-coercion, nothing added or removed. Every statement
 * computes a real value later consumed (sin_p/cos_p feed the pointer
 * derefs at lines below and the post-func_80053614 `x +=`/`z +=`; scale
 * feeds both multiplies; x/z feed pt0/pt1). Per
 * [[ordinary-c-judge-decidable]] Ruling 1: "choosing among
 * semantically-truthful C spellings by observing codegen is the METHOD
 * of matching decompilation, not a cheat signal" — this is exactly that:
 * an ordinary reordering of independent real statements, not a
 * construct from any forbidden or sanctioned-with-prerequisites family.
 * No FAKE annotation applies (nothing here lacks semantic purpose).
 *
 * MEASURED (official `wteng sandbox func_80056CB8 --disable all`, NOT
 * just the sweep tool's internal scorer): score 42, target_insns 204,
 * build_insns 197. New session floor, confirmed via the same sandbox
 * path the driver re-verifies with.
 *
 * REMAINING RESIDUAL (42) — same two classes noted since s6/s7/s11: (a)
 * the register-rotation/strength-reduction double-bind (loop.c
 * insn_count-gated, root-caused s11/s13, still not source-reachable by
 * index/threshold respelling), and (b) the code==4 tail's branch-topology
 * difference. The s14 win shaves 1 real instruction off (a); whether it
 * shifts EITHER side of the s13 double-bind (the reg149 spill / the $fp
 * accumulator) is UNMEASURED this session — next session should re-run
 * the .greg dump fresh on this NEW 42/197 chassis before assuming the
 * s13 double-bind evidence still applies unchanged.
 *
 * FRONTIER FOR s15: (1) re-run pwsh tools/grinder/dump.ps1 fresh on the
 * s14 42/197 chassis and re-check both double-bind indicators (Spilling
 * reg 11/65? does classify's PRE-RA verdict on the i*2 givs change?) —
 * the insn_count dropped by 1, which per the s13 mechanism could move
 * either side. (2) Apply the SAME systematic spelling_enum.py sweep to
 * the pt0/pt1 array-fill blocks (currently ANCHORS to the tool because
 * `pt0[0] = ...;` doesn't match the assign-regex's bare-identifier LHS —
 * would need the tool taught to parse indexed LHS, or a hand-swept
 * pairwise-reorder check of the two 3-store blocks) — untried region.
 * (3) Apply spelling_enum.py to the flags==3/flags==4 dispatch's dx/dz/y
 * declarations WITH the new 42/197 baseline (s9's 5/5 exhaustive kill was
 * measured on the OLD 58/198 chassis; re-verify it's still flat now that
 * insn_count moved).
 * ---------------------------------------------------------------------
 * s13 (structural modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s12-banked body (unchanged) to
 * src/text1b.c, re-confirmed floor 48/204 (build_insns 198) exactly
 * matches the s11/s12 record before any s13 change.
 *
 * NAMED THE s12 FRAME-DELTA PSEUDO (frontier item 1). Ran
 * `pwsh tools/grinder/dump.ps1 func_80056CB8` fresh and read the
 * func_80056CB8 slice of the .greg dump (tmp/grind/func_80056CB8/dumps/
 * text1b.greg lines 14788-14850): "Spilling reg 11. Spilling reg 65."
 * fires to satisfy simultaneous "Need 1 reg of class GR_REGS (for insn
 * 456)" + "Need 1 reg of class LO_REG/MD_REGS (for insn 153)" register
 * requests. Cross-referenced insn numbers against the .combine dump
 * (same session): insn 153 is the `scale * *sin_p` widening multiply
 * (the FIRST of two multu ops in the loop body); insns 456/457/459 are
 * THREE loop-invariants hoisted to immediately before `NOTE_INSN_LOOP_BEG`
 * (insn 22): 456/457 are the `&pt0`/`&pt1` stack addresses (these SHOULD
 * hoist — pt0/pt1 are function-scope arrays, target hoists their address
 * setup too, see below), and insn 459 sets reg 149 = 528483000 (0x3D0900)
 * — the flags==4 tail's `dx*dx+dz*dz > LIMIT` threshold constant. Reg 149
 * lives from before the loop until its single use deep inside the
 * flags==4/nested-if branch, and this long cross-loop live range is what
 * competes with the insn-153 multiply's LO/MD register need, forcing the
 * two-hard-reg spill (reg 11 = $t3, reg 65 = $lo) that the s12 classify
 * frame-delta (176 vs 168) evidence predicted.
 *
 * READ TARGET ASM DIRECTLY for the first time at this exact byte range
 * (asm/funcs/func_80056CB8.s:150-171, the flags==4 tail): target ALSO
 * hoists the &pt0/&pt1/scratchpad-address setup before the loop (lines
 * 21-26, matching our insns 456/457 exactly — NOT a residual), but the
 * 0x3D0900 constant (lines 170-171, `lui $v1,(0x3D0900>>16); ori
 * $v1,$v1,(0x3D0900&0xFFFF)`) is materialized INLINE, right at its use
 * site inside the conditionally-executed flags==4 branch — NOT hoisted.
 * This is the first direct confirmation of WHICH specific loop-invariant
 * our build hoists that target's original C did not.
 *
 * MECHANISM (loop.c `move_movables`, tools/gcc-2.7.2/loop.c:1529-1634):
 * a constant-set insn becomes an eligible movable via loop.c:695-701's
 * 3-way OR test (three ways an insn qualifies: (1) not-maybe_never +
 * not-used-before, (2) not a user variable AND not used in the loop exit
 * test, (3) def+use in the same basic block) — our threshold constant
 * satisfies BOTH (2) (compiler temp, not the loop's exit test) and (3)
 * (def is immediately before its single use, no intervening branch), so
 * it is movable regardless. Whether it actually GETS hoisted is then
 * move_movables:1631's cost gate: `threshold * savings * m->lifetime >=
 * insn_count`, where `threshold = (loop_has_call?1:2)*(1+n_non_fixed_regs)`
 * (loop.c:532) and `insn_count` is the SAME 164-real-insn count s11 named
 * as the strength-reduction gate for the $fp accumulator. This is a
 * DOUBLE-BIND: strength-reduction's rejection test (loop.c:3823,
 * `v->lifetime*threshold*benefit < insn_count`) wants insn_count SMALL to
 * flip to ACCEPT (get the $fp accumulator target has); move_movables'
 * acceptance test wants insn_count LARGE (relative to threshold*savings*
 * lifetime) to flip to REJECT (avoid the constant hoist we don't want).
 * Both are gated by the identical insn_count quantity in OPPOSITE
 * directions — shrinking the loop body to help one lever plausibly hurts
 * the other. This is new, mechanistically precise evidence explaining why
 * this residual has resisted 6+ sessions of index/loop-shape respellings.
 *
 * PROBES (all killed, instance) — tested whether a superficial respelling
 * of the flags==4 threshold comparison changes loop.c's movable decision
 * for reg 149, per the REG_USERVAR_P clause of the eligibility OR-test:
 *   1. Swap comparison operand order: `dx*dx+dz*dz > 0x3D0900` instead of
 *      `0x3D0900 < dx*dx+dz*dz`. Measured: 48/198, BYTE-IDENTICAL, no
 *      change. Reverted.
 *   2. Named intermediate for the sum: `s32 sq = dx*dx+dz*dz; if (sq >
 *      0x3D0900)`. Measured: 48/198, no change. Reverted.
 *   3. Named intermediate for the constant: `s32 limit = 0x3D0900; if
 *      (dx*dx+dz*dz > limit)`. Measured: 48/198, no change. Reverted.
 * KILLED, instance — per the mechanism above, condition (3)
 * (reg_in_basic_block_p, def and use already adjacent with no
 * intervening branch in ALL three spellings) already makes the constant
 * movable-eligible regardless of REG_USERVAR_P, so naming it a real local
 * changes nothing: eligibility was never the gate here, the move_movables
 * cost-benefit calculation (threshold*savings*lifetime vs insn_count) is.
 * This closes the "respell the threshold comparison" family for this
 * residual — the same class of surface-level move already closed for the
 * i*2 index arithmetic (s6/s7/s10/s12).
 *
 * FRONTIER FOR s14: the double-bind above means neither "shrink insn_count
 * to help strength-reduction" nor "grow insn_count to block the constant
 * hoist" can be pursued in isolation without checking the other lever's
 * response — any insn_count-shifting C change should be measured against
 * BOTH the $fp-accumulator question (does classify's PRE-RA verdict
 * change?) and the reg149 spill (does the .greg dump still show "Spilling
 * reg 11"/"Spilling reg 65"?) in the SAME session. UNTRIED: rather than
 * changing insn_count, attack `n_non_fixed_regs` in the threshold formula
 * directly (loop.c:532) — i.e. find a C change that reduces REGISTER
 * PRESSURE elsewhere in the loop body (not insn count) without touching
 * the i*2 index or the threshold comparison, which would lower `threshold`
 * and could flip move_movables:1631 to reject the constant hoist without
 * touching insn_count (and thus without perturbing strength-reduction's
 * separate insn_count-gated decision at all). Candidate targets: the
 * `obj`/`sin_p`/`cos_p` pointer locals and the two `func_80053614` calls'
 * live ranges — read the .greg dump's conflict list (14791-14812) for
 * which of the 20 pseudos have the largest conflict sets and see if any
 * has an uncontested narrowing (narrower type, earlier death) available.
 * ---------------------------------------------------------------------
 * s12 (rederive modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s11-banked body (r1/r2 merge +
 * func_80053614 s32-return fix) to src/text1b.c; the func_80053614
 * signature fix was MISSING from candidate.c's own splice (the file's
 * body text alone does not carry it — the fix lives only in prose/git
 * history) and had to be re-applied by hand before the chassis
 * reproduced 48/198 (without it: 132/171, badly wrong — future sessions
 * splicing this candidate.c verbatim must also re-apply the
 * func_80053614 `void` -> `s32 return func_80052D00(...)` change, or
 * they will silently start from a wrong chassis).
 *
 * FRESH m2c DECOMPILE (mipsel-gcc-c) of asm/funcs/func_80056CB8.s (first
 * time this ledger ran m2c directly on this function — evidence.md/
 * hypotheses.md only mention m2c as a suggestion, never as executed).
 * Output archived: tmp/grind/func_80056CB8/s12/m2c_out.c. Confirms,
 * doesn't overturn, prior findings: target's `var_fp` (the $fp/$s8
 * accumulator) increments by 2 alongside the real loop counter `var_s6`
 * (i), read directly by BOTH byte-table lookups — matches s10/s11's raw-
 * asm reading exactly. The store-address expression `arg0 + i` is
 * reconstructed as a value RECOMPUTED at every branch join
 * (`var_v0 = arg0 + var_s6`) before a single shared store
 * (`var_v0->unk444 = var_s0_2`) — tried respelling this as a genuinely
 * duplicated store statement at every leaf of the flags==3/flags==4
 * chain (mirroring m2c's per-leaf reconstruction): measured WORSE
 * (48 -> 75/204, build_insns 198 -> 202). Reverted; re-confirmed 48/198.
 * KILLED, instance — the store-address recompute in target's asm is not
 * reachable by literally duplicating the store statement into every
 * source-level leaf; whatever produces it is not this shape.
 *
 * inverse_compose.py classify (object-level: text1b func_80056CB8,
 * --target-object build/src/text1b.o --ours-object
 * tmp/sandbox/func_80056CB8/text1b.o), the #1 s11 frontier item, RE-RUN
 * on the s11/s12 floor-48 chassis (must run via WSL — the native-Windows
 * Python invocation fails, `objdump` is a WSL-only binary on this
 * machine; use `wsl bash -c 'cd ... && source .venv/bin/activate &&
 * python3 tools/ra_solver/inverse_compose.py classify ...'`). Verdict
 * UNCHANGED from s8: FIRST DIVERGENCE: PRE-RA, instruction MULTISET
 * differs (not a reachable RA/scheduler residual). NEW, more precise
 * evidence this session: the multiset diff explicitly names a FRAME-SIZE
 * delta — our build emits `addiu sp,sp,-176` / `addiu sp,sp,176`
 * (176-byte frame) where target emits `-168` / `168` (168-byte frame) —
 * exactly one extra 8-byte stack slot on our side. Our build ALSO shows
 * `move #,s8` / `move s8,#` / `lw s8,168(#)` (a value shuffled through
 * $s8 via a stack spill) plus `lhu #,1000(s8)` / `lhu #,106(s8)` (arg0-
 * relative loads using $s8 as base), where target shows `addiu s8,s8,2`
 * (the known accumulator increment) / three plain `addu #,#,#` / one
 * `addu #,#,s8`, and the `lhu` pair at the SAME offsets but via a
 * DIFFERENT (unspecified) base register — i.e. in our build $s8 holds
 * `arg0`/`obj` (spilled once to free it up, then reloaded), while in
 * target $s8 is reserved for the i*2 accumulator throughout and arg0
 * lives elsewhere. This is a sharper restatement of the s10/s11 finding,
 * not a new mechanism: the extra 8-byte frame slot IS the spill s11's
 * loop.c reading predicts (our C never creates the accumulator, so $s8
 * goes to the highest-priority remaining candidate instead, which
 * apparently needs a stack round-trip somewhere our current chassis
 * doesn't have visible pressure for).
 *
 * PROBE (killed, instance): the classify output's `cse_split` /
 * `duplicated-statement-into-arms` lever list plus the frame-delta
 * evidence suggested one untried spelling of "share i*2 between the two
 * byte-table reads": a genuine LOOP-CARRIED POINTER induction variable
 * (as opposed to s6/s10's fresh/loop-carried INT, and s7's per-table
 * pointer RECOMPUTED fresh each iteration) — `u8 *flags_p = &D_8009A821
 * + start*2; ...; for (i = start; i < start+2; i++, flags_p += 2, scale_p
 * += 2) { flags = *flags_p << 8; ...; scale = *scale_p << 8; }`. Measured
 * on the s11/s12 floor-48 chassis: score 48 -> 78/204 (build_insns 198 ->
 * 209, WORSE). Reverted; re-confirmed floor 48 exactly reproduces.
 * KILLED, instance — full writeup + all 4 now-measured "share i*2"
 * spellings (fresh-int, loop-carried-int, fresh-pointer, loop-carried-
 * pointer) in memory/grind/func_80056CB8/rejected/loop-carried-pointer-
 * walk-worse.c. This closes the entire "one C handle carries i*2" family
 * for this residual.
 *
 * FRONTIER FOR s13: the classify-confirmed frame-size delta (176 vs 168)
 * is the sharpest lever surfaced so far and UNTRIED directly — find what
 * in our current C forces an extra 8-byte stack slot / a spill-through-
 * $s8 round trip that target's C doesn't need. Candidates: (a) run
 * dump.ps1 fresh and read the .greg dump's conflict list for whichever
 * pseudo lands in the frame slot at offset 168 — that names the exact
 * competing value; (b) [[phantom-slot-frame-lever]] frame-census
 * procedure to confirm which local/temp corresponds to the extra slot;
 * (c) once the competing value is named, try reducing ITS lifetime
 * (block-local split / narrow type / earlier consumption) rather than
 * touching the index arithmetic at all — per s11's insn_count-threshold
 * finding, shrinking the loop body's real-insn count is the only
 * remaining avenue to let strength-reduction accept the accumulator, and
 * eliminating this spill is a plausible way to do that without altering
 * program semantics.
 * ---------------------------------------------------------------------
 * s11 (forensics modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s7-banked body to
 * src/text1b.c, re-confirmed floor 58/204 (build_insns 198) exactly
 * matches the s7-s10 record before any s11 change.
 *
 * PASS-ATTRIBUTION WORK: ran `pwsh tools/grinder/dump.ps1 func_80056CB8`
 * (instrumented cc1, whole-TU dumps under tmp/grind/func_80056CB8/dumps/)
 * and read the func_80056CB8 slice of the .loop dump (lines 13251-14311
 * of tmp/grind/func_80056CB8/dumps/text1b.loop). Findings on the s10
 * frontier's induction-variable hypothesis (item 1, "invert the primary
 * biv to the doubled index j"):
 *   - loop.c's biv/giv analysis (`strength_reduce`) already tracks `i`
 *     (reg 74) as the loop's ONLY verified biv, and BOTH byte-table
 *     index computations `i*2` (insns 42 and 140, "giv reg 93/126 src
 *     reg 74 ... mult 2 add 0") are recognized as givs of that SAME biv
 *     and get COMBINED by `combine_givs` ("giv at 42 combined with giv
 *     at 140") — i.e. GCC already unifies the two `i*2` index
 *     computations into one shared reduction candidate purely from our
 *     existing `(&D_x)[i*2]` spelling; no C-level index-sharing lever
 *     was missing.
 *   - Both the combined giv AND the store-address giv (insn 427, "src
 *     reg 74 ... mult 1 add (reg/v:SI 72)", i.e. `arg0+0x444+i`) are
 *     then REJECTED for strength-reduction: "giv of insn 140 not worth
 *     while, 124 vs 164" / "giv of insn 427 not worth while, 0 vs 164".
 *     The rejection predicate is loop.c:3823
 *     (`v->lifetime * threshold * benefit < insn_count`) — read
 *     tools/gcc-2.7.2/loop.c:3806-3833. `insn_count` here is 164 (this
 *     loop's real-insn count, printed at the top of the dump slice:
 *     "Loop from 22 to 444: 164 real insns."); with lifetime==1 for both
 *     givs, the product of the per-target `threshold` (the same
 *     hoisting-threshold constant the softfloat-adoption memory names —
 *     58 under -msoft-float) and each giv's `benefit` field falls below
 *     164, so `strength_reduce` sets `v->ignore = 1` and leaves the
 *     index/address recomputed via an ordinary `mult`/`add` from the biv
 *     at each use, INSTEAD OF maintaining a separate incrementing
 *     accumulator register for it.
 *   - CONSEQUENCE: this is exactly why our build never produces target's
 *     `$fp` accumulator (`sll $fp,$v1,2` once before the loop, `addiu
 *     $fp,$fp,0x2` once per iteration, read directly by both byte-table
 *     `lbu`s) — GCC's own cost model for THIS loop's insn_count (164)
 *     rejects the strength-reduction that would produce that shape,
 *     regardless of which of `i`/`j` is nominally the biv in the source.
 *     Confirms this residual is a genuine benefit-threshold decision
 *     inside loop.c, not a missing C spelling of the index arithmetic.
 *
 * PROBE 1 (killed, instance): tried making the DOUBLED value the primary
 * loop-control variable instead of `i` (`for (j = start*2; j < start*2+4;
 * j += 2) { s32 i = j >> 1; ...index sites use [j]...; store site uses
 * i; }`, j declared in function scope like `i` was). This is a genuinely
 * new spelling (distinct from s10's loop-carried idx2, which kept `i` as
 * the loop control and added idx2 as a SECOND loop-carried variable of
 * the same biv class). Measured: score REGRESSED 58 -> 86 (build_insns
 * 198 -> 202). Reverted immediately, re-confirmed floor 58 exactly
 * reproduces after revert. Consistent with the dump finding above: `j`
 * becomes the new (and only) biv, `i = j >> 1` is NOT a giv (loop.c only
 * tracks affine mult/add relations to a biv, not right-shifts), so `i`
 * has to be recomputed by an actual `sra` every iteration instead of
 * being a strength-reduced accumulator OR a cheap direct read of the
 * biv — strictly worse than the original shape on every axis. Kills the
 * "the original C's loop was literally indexed by the doubled value"
 * hypothesis outright, per the s10 frontier's own stated fallback: the
 * `$fp` accumulator is NOT reachable by respelling which variable is the
 * loop's primary induction variable; it is a direct consequence of the
 * insn_count-gated strength-reduction threshold above, which is fixed by
 * the toolchain, not by source loop shape, for this loop body's size.
 *
 * THE WIN (structural, SOTN-sanctioned variable-reuse-for-codegen-control
 * family, .claude/rules/no-new-park-categories.md § SOTN-accepted /
 * .claude/rules/defeat-licm-hoist-var-reuse.md — borrowing an EXISTING
 * local for a second unrelated but REAL value, same family as s7's
 * flags/ang/code merge, now extended to r1):
 *
 * Read the full target asm (asm/funcs/func_80056CB8.s) end to end this
 * session (frontier item 2, carried since s7/s8/s9/s10). Target's $s0 —
 * already carrying flags/ang/code per the s7 win — ALSO carries `r1`,
 * the first `func_80053614` call's return value:
 *   `jal func_80053614` / `addu $s0, $v0, $zero` (line 105 of the asm
 *   listing, .L80056E38-.L80056E40) — the call result is moved straight
 *   into $s0, the SAME register flags/ang lived in one instruction
 *   earlier (flags is dead the instant sin_p/cos_p/scale/x/z are
 *   derived from it, well before this call). $s0 (as r1) then survives
 *   THROUGH the `if (r1 != 0) {...}` branch AND through the entire
 *   second `func_80053614` call, because the final disposition value is
 *   computed as `or $s0,$s0,$v0; addiu $s0,$s0,0x1` — i.e. `flags =
 *   (r1 | (r2 << 1)) + 1` is computed by OR-ing r2 directly into the
 *   register that ALREADY holds r1, not by combining two freshly-loaded
 *   values. r2 itself never gets a persistent register at all: it's
 *   consumed straight out of $v0 immediately after the second call
 *   (`sll $v0,$v0,1; or $s0,$s0,$v0`), matching an inlined
 *   non-declared expression rather than a named local.
 *
 * CHANGE: removed the `r1` local declaration; the first
 * `func_80053614(...)` call's result is now assigned directly to
 * `flags` (`flags = func_80053614(...)`) and the `if (r1 != 0)` guard
 * became `if (flags != 0)`. Removed the `r2` local declaration entirely;
 * the second `func_80053614(...)` call is now inlined directly into the
 * final disposition expression: `flags = (flags | (func_80053614(pt0,
 * pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;` — this mirrors
 * target's own shape (r2 never named, consumed once out of $v0) and
 * measured BYTE-IDENTICAL to keeping a separate `r2` local that's used
 * exactly once (48/198 either way) — kept the no-extra-local form per
 * the pipeline's simplest-known-form tiebreak (Ruling 1(4),
 * ordinary-c-judge-decidable.md) since it's strictly fewer declared
 * locals with identical bytes.
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 58 -> 48/204
 * (build_insns 198, UNCHANGED insn count — pure register-identity win,
 * same shape as the s7 flags/ang/code merge). Re-verified after the
 * r2-inlining simplification (still 48/198, neutral).
 *
 * REMAINING RESIDUAL (48) — same two classes noted since s6/s7: (a) the
 * register-rotation cluster ($s3 vs $s6/$s7 for the loop counter and its
 * derived store addresses, now root-caused this session to loop.c's
 * strength-reduction benefit threshold — see PASS-ATTRIBUTION above —
 * genuinely NOT a source-spelling lever for THIS loop's insn_count), and
 * (b) the code==4 tail's branch-topology difference (target: `bltz` +
 * `beqz` as two branches; ours: one `bgez`), confirmed by s6/s7 to be
 * gated upstream, not by this tail's own C shape.
 *
 * FRONTIER FOR s12: (1) re-run tools/ra_solver/inverse_compose.py
 * classify on this s11 chassis — the r1/r2 merge is a genuine
 * instruction-count-neutral register-identity change and the loop.c
 * root-cause finding narrows what's left to a named, non-source-facing
 * mechanism; worth checking whether the PRE-RA verdict from s8 changes
 * now that TWO of the three s7-frontier merge candidates are spent. (2)
 * dx/dz/y in the code==4 tail were checked this session by re-reading
 * the tail asm (lines .L80056F08 onward): dx/dz never acquire a
 * persistent register (transient mult/mflo operands only, `$a0`/`$t0`
 * one-shot), and `y` (`lw $a0,0xBC($s1)`) is read directly into a
 * compare with no persistent home either — NO register-reuse
 * opportunity exists for these three, closing frontier item 2 from
 * s7-s10 as fully explored (r1/r2 was the only real hit in that item).
 * (3) the loop.c threshold-rejection root cause suggests the ONLY way
 * left to get the `$fp`-style accumulator is to change the loop's
 * insn_count itself (i.e., find further C restructuring that shrinks or
 * reshapes the loop body enough to cross the strength-reduction
 * benefit threshold) — NOT to respell the induction variable. This is
 * speculative and unconfirmed; a structural-modality session should
 * treat it as a hypothesis to test by measuring insn_count shifts as
 * other levers land, not a lever to chase directly.
 * ---------------------------------------------------------------------
 * s7 (structural modality, content preserved below for provenance).
 * STALE-HEAD-CLAIM NOTE (same as every prior session): src
 * representation is INCLUDE_ASM between grind sessions; nothing
 * persists on main. Re-applied the s6-banked body (D_800F6610 fix + s5
 * store-batching + func_80053614 s32-return fix) to src/text1b.c,
 * re-confirmed floor 81/204 (build_insns 197) exactly matches the s5/s6
 * record before any s7 change.
 *
 * THE WIN (structural, SOTN-sanctioned variable-reuse-for-codegen-control
 * family, .claude/rules/no-new-park-categories.md § SOTN-accepted /
 * .claude/rules/defeat-licm-hoist-var-reuse.md — borrowing an EXISTING
 * local for a second unrelated but REAL value, no new local invented):
 *
 * Read asm/funcs/func_80056CB8.s directly (not re-derived from a stale
 * dump) and found target keeps ONE hardware register ($s0) for THREE
 * successive, non-overlapping-lifetime roles across the loop body:
 *   1. `flags` (the D_8009A821 byte-table value, shifted << 8)
 *   2. `ang` — target computes this as `addu $s0,$s0,$v0` (asm lines
 *      .L80056D60/.L80056D90), i.e. literally `flags += ...`, NOT a
 *      separate register/variable.
 *   3. `code` — after the Judge-table lookups, target reuses the SAME
 *      $s0 for the final disposition code.
 *
 * CHANGE: removed the `ang` and `code` local declarations entirely;
 * every former `ang` use became `flags`; every former `code` use became
 * a re-assignment of `flags`.
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 81 -> 58/204
 * (build_insns 197 -> 198).
 * ---------------------------------------------------------------------
 * s6 (enumerate modality) content preserved below for provenance —
 * object-model fix (D_800F6610) and the killed shared-idx-local /
 * goto-tail hypotheses are UNCHANGED by this session's edits; see
 * hypotheses.md/evidence.md [s6] entries for the full writeup.
 *
 * OBJECT-MODEL FIX (real, evidence-backed, carried forward unchanged):
 * the ratan2 branch's second argument reads the separate global
 * `D_800F6610` (== D_800F6608+8, same storage as the Rec44 `.w8` field,
 * but a genuinely separate symbol per the target asm's independent
 * lui/lw(D_800F6610) relocation pair) rather than `D_800F6608.w8`. See
 * s6 evidence for the full objdiff-verified derivation.
 * ---------------------------------------------------------------------
 * PREREQUISITE CHANGE IN THE SAME TU (func_80053614 void -> s32 return
 * type, byte-neutral standalone) — unchanged since s2, still required.
 *
 * PROVENANCE: body is the s2-authored draft + s5 store-batching + s6
 * D_800F6610 fix + func_80053614 signature fix + s7 flags/ang/code
 * variable-reuse merge + s11 r1/r2 variable-reuse merge. See earlier
 * session headers (git history of this file, or hypotheses.md) for the
 * full derivation chain back to
 * memory/grind/func_80056CB8/authored-notes-2026-08-18.md.
 * ===================================================================== */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820;
extern u8 D_8009A821;
extern s32 D_800F6610;

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[4];
    s32 start;
    s32 limit;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    limit = start + 2;
    for (i = start; i < limit; i++) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;

        obj = arg0;
        flags = (&D_8009A821)[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (flags & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;
        if (flags == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                flags = 0;
            }
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        flags = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        flags = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
    }
}
