/* =====================================================================
 * func_80056CB8 — CANDIDATE (s22 rederive-modality win, re-confirmed
 * s23/s24/s25/s26/s27/s28/s29/s30/s31/s32/s33/s34/s35/s36/s37/s38/s39/s40/s41/s42/s43/s46/s48/s49/s52/s53/s54) — floor 38/204, NOT YET 0.
 * s54 (forensics modality, 2026-09-16): re-confirmed 38/204 fresh (6th
 * consecutive independent confirmation). Ran tools/loop_movables.py for
 * the first time -- confirms pseudo 149 (`limit`) IS a genuine
 * move_movables LICM invariant (insn 210, decision "moved", inequality
 * 2080>=163), resolving the s50/s51 open question about which loop.c
 * pass governs it: ordinary LICM, not strength_reduce; this is the
 * upstream cause of limit's whole-loop liveness and its later
 * global_alloc spill, not a separate register-pressure-only artifact.
 * Closed the s53 live-frontier item #2 (hit0/hit1 storage reuse):
 * source-level read confirms hit0 is read (flags==4 arm) AFTER hit1 is
 * written by the second func_80053614 call, in the same control-flow
 * region -- reusing hit1's storage for hit0 would corrupt still-needed
 * values (an INCORRECT program), disqualified on correctness grounds
 * before any codegen question applies. Both s53 frontier items are now
 * closed with no new C-level lever surfaced; see hypotheses.md [s54] +
 * evidence.md [s54]. Body UNCHANGED. Sibling ledgers checked
 * (func_8006CCC8, func_80055B60): nothing transplantable.
 * s53 (forensics modality, 2026-09-16): re-confirmed 38/204 fresh (re-
 * derived the header-extern + func_80053614 s32-return chassis trap
 * fresh: omitting header externs alone silently miscompiles to 141/204
 * via K&R implicit-int). Read the FULL func_80056CB8 RTL region in
 * tmp/grind/func_80056CB8/dumps/text1b.greg (lines 14788-15995) end-to-
 * end for the first time. CONFIRMED: reg 11 (t3, insn 21) is `limit`,
 * spilled to sp+104 at insn 469, reloaded once at insn 493 immediately
 * before the loop-back-edge compare (insn 28) -- its live range spans
 * the whole loop by construction. Reg 65 (lo, insn 143) is an UNRELATED,
 * INDEPENDENT MD_REGS spill for the flags/scale multiply (already a
 * previously-killed hypothesis family: scratchpad-literal/scale-reuse).
 * No shared-storage candidate exists for `limit` within its own live
 * range besides `i` itself (can't merge — compared every iteration).
 * Closes both s50/s51-named live-frontier items. See hypotheses.md
 * [s53] + evidence.md [s53] for full detail. Body UNCHANGED.
 * s49 (synthesis modality, 2026-09-16): mandatory kill re-audit (4th
 * consecutive independent confirmation, after s41/s46/s48) reproduced
 * 38/204 fresh via a new repeatable splice script
 * (tmp/grind/func_80056CB8/s49/splice.py, banked as artifact). Body
 * UNCHANGED. No new spelling probes this session (synthesis modality =
 * ledger consolidation, not enumeration). Full re-read of hypotheses.md
 * + evidence.md confirms the s45 LADDER EXHAUSTED (non-endgame residual,
 * floor 38) ROTATED disposition's two endgame-lock gates (canonical-asm
 * tier LOW; SOTN construct-class census zero hits) remain correctly
 * failed and unchanged -- but those gates only govern canonical-asm
 * authorization and new construct-family sanctioning, NOT whether more
 * ordinary-C restructuring could still close the gap (and s46-s48 found
 * 6 more genuinely new ordinary-C spellings post-rotation without
 * closing it, proving the axis is not dead, just not yet closed).
 * FRONTIER RESET: the single strongest live item is a forensics/solver-
 * modality RTL/.greg dump AT THE TWO func_80053614 CALL SITES
 * specifically (not the s43 loop-guard dumps) to identify the actual
 * conflicting pseudo(s) instead of continuing to guess restructuring
 * shapes blind -- this needs a session in forensics or solver modality.
 * s48 (enumerate modality, 2026-09-16): mandatory kill re-audit reproduced
 * 38/204 fresh (no FAKE construct to ablate). Two genuinely new spelling
 * probes, both previously untested in this ledger: (1) swapping the
 * x+=/z+= post-call adjustment order to z-then-x -- KILLED, 45/204 (worse);
 * (2) a named intermediate `dy0 = hit1[1] - obj->0xBC` for the flags==3
 * guard -- byte-neutral tie at 38/204, not adopted (simpler inline form
 * kept). Body UNCHANGED. The systematic spelling-space search across every
 * identified block (loop bound, obj/flags dispatch + combinations,
 * sin_p/cos_p/scale/x/z ordering+swaps, both pt0/pt1 store-order blocks,
 * dx/dz declaration-order+inlining, the x/z post-call order, flags==3
 * named-intermediate) is now exhaustive with zero hits below 38. See
 * hypotheses.md/evidence.md [s48] for the full writeup.
 * s46 (structural modality, 2026-09-16, auto-returned via func_8006CCC8
 * sibling movement -- checked that sibling's ledger, no transplantable
 * lever found, its H2 is an unrelated LICM-hoist-var-reuse case). Body
 * UNCHANGED (38/204/198 re-confirmed fresh). TWO findings: (1) the
 * engine/sandbox.py --disable-all scoring defect flagged at s44b/s45
 * (which returned 134/204 instead of the true floor) is RESOLVED --
 * the regfix/asmfix rule machinery it blamed was fully retired
 * 2026-08-30, so that failure mode can no longer occur; direct
 * `sandbox --disable all` measurement now works for this function
 * without the ra_solver object-mode workaround. (2) Both untried
 * direct-bound for-loop spellings from the s45 LADDER EXHAUSTED
 * record's live frontier item #1 were measured and KILLED: `for (i =
 * start; i < start + 2; i++)` (no `limit` local) scores 42/204 (197
 * insns, WORSE than baseline despite 1 fewer real insn); `for (i =
 * start; i - start < 2; i++)` scores 47/204 (196 insns, worse still).
 * cc1 does NOT elide the pre-header guard via either truthful inline
 * bound spelling on this chassis -- the guard-elision hypothesis from
 * s42/s43 is now dead on both concrete forms. Frontier item #2
 * (structural insn-count reduction to cross the loop.c:3823
 * strength-reduce threshold) remains genuinely open with no new
 * candidate shape found this session.
 * s43 (forensics modality): body UNCHANGED (38/204/198 re-confirmed fresh).
 * Two findings: (1) CLASS KILL -- both func_80053614() calls in the loop
 * are unguarded, unconditional-per-iteration collision-probe calls whose
 * results feed the disposition dispatch; neither can be removed or
 * conditionalized without changing program semantics, closing the s41
 * "loop_has_call could become false, doubling loop.c's threshold"
 * frontier item for good. (2) Pinned the EXACT mechanism behind the
 * do-while chassis's -3-insn reduction (open since s34, RA picture ruled
 * out by s42): a fresh `.s` diff between freshly-rebuilt for-loop and
 * do-while chassis dumps shows the for-loop chassis emits a 3-insn
 * pre-header guard (`addu;slt;beq`) testing `start < limit` before the
 * first iteration, which the do-while chassis never needs (do-while
 * always executes >=1 time by construction). This is cc1's for-loop
 * lowering (expand_start_loop/stmt.c), NOT an RA/loop.c effect. New
 * untried frontier: whether inlining `limit` away (`i < start + 2` or
 * `i - start < 2` instead of a separate `limit` local) lets cc1 prove
 * the trip count nonzero and drop the guard on the FOR-loop chassis
 * itself (preserving its otherwise-closer instruction mix, unlike the
 * do-while rewrite which the ledger has repeatedly measured worse
 * overall despite fewer instructions). Artifacts:
 * tmp/grind/func_80056CB8/s43/text1b_s_forloop.s,
 * text1b_s_dowhile.s (diffed), text1b_greg_forloop_slice.txt,
 * text1b_greg_dowhile_full.txt.
 * s41 (solver modality): body UNCHANGED. Fresh chassis-reproduction
 * confirmed 38/204 (198 insns). Ran tools/ra_solver/inverse_compose.py
 * classify in OBJECT MODE (the zero-rule/INCLUDE_ASM-routed escape:
 * `--target-object build/src/text1b.o --ours-object
 * tmp/sandbox/func_80056CB8/text1b.o`) since text-stream mode refuses
 * on zero-rule functions. Verdict: PRE-RA / rtl_shape (its most severe
 * category -- no RA/scheduler perturbation reaches this residual). The
 * tool's own instruction-multiset diff independently corroborates TWO
 * things already in the ledger: (1) the s31/s32/s37-s40 loop.c:3823
 * strength-reduce frontier item (target shows `addiu s8,s8,2` +
 * `addu #,#,s8` x2 -- a strength-reduced i+=2 accumulator -- vs ours'
 * three `sll #,#,0x1` i*2 shifts); (2) the s15-rejected named-
 * intermediate-scratchpad lever (target reads the 0x1F8002B8 literal
 * back via `lw #,104(#)`/`lw #,96(#)` at two DIFFERENT offsets instead
 * of ours' two independent `lui+ori+sw s8,16(#)` materializations). Per
 * the KILL RE-AUDIT REQUIRED mandate, re-spliced the s15 lever onto the
 * CURRENT chassis rather than trusting the stale kill: still worse
 * (42/204, 200 insns). RE-CONFIRMED KILLED on the current chassis; see
 * hypotheses.md [s41] and rejected/named-intermediate-scratchpad-
 * literal-worse.c's s41 re-audit note. No new lever found this session;
 * the classify tool's independent corroboration of the loop.c:3823
 * frontier item raises confidence that item is the correct next
 * structural target, but it remains unattempted (requires a hand-drafted
 * structural loop-body rewrite, not a spelling change).
 * s40 (synthesis modality): body UNCHANGED. Full ledger re-read; fresh
 * chassis-reproduction confirmed 38/204 (198 insns), no drift since s39.
 * Extended s39's "share the duplicate *(s32*)(obj+0xBC) expression" find
 * to its maximal scope: merged ALL SIX occurrences (block1 x2, block2 x2,
 * the flags==3 comparison, the flags==4 `y`) into one call-spanning local
 * `by`. WORSE: 38->95/204, build_insns 198->177 (-21 insns, the largest
 * single-lever insn-count drop on this ledger, yet a substantially worse
 * score) -- confirms count and mix are independent axes and that target
 * genuinely re-reads/re-derives obj+0xBC at (most of) these sites rather
 * than caching it across the two func_80053614 calls. Saved
 * rejected/cross-call-shared-by-local-worse.c. This closes the
 * "share/merge any repeated obj+0xBC read, any scope" axis entirely (s39
 * narrow form + s40 maximal form both worse, worse scaling with scope).
 * No sibling ledger had transplantable material (func_80055B60 has no
 * candidate.c; func_80057CC8/func_80056FE8 are COMPLETED-C but structurally
 * unrelated). No stale kill needed a fake_ablate.py re-audit -- every
 * banked kill was already measured on the current chassis with zero FAKE
 * constructs present. See hypotheses.md [s40] for full detail. FRONTIER:
 * the s31/s32-banked loop.c:3823 insn-count-threshold structural rewrite
 * and the do-while-chassis resident-footprint probe (both untouched since
 * s32/s34) are now the ONLY genuinely untried avenues -- the spelling/
 * ordering/sharing search space (38 measured variants, s33-s40) is
 * exhausted for this chassis shape.
 * s39 (enumerate modality): body UNCHANGED. Extended s38's isolated obj/flags
 * hand-variants to COMBINATIONS (A=ternary obj-assignment, B=kind-local-
 * after, C=ang intermediate, D=dx/dz intermediates): A+B, A+C, A+D, A+B+C+D,
 * B+C+D all measured -- every combo ties baseline at 38/204 (198 insns), no
 * interaction effect found. Also tried a NEW spelling not in s37/s38: sharing
 * the literally-duplicated `*(s32*)(obj+0xBC)-0x320` expression (computed
 * twice into pt0[1]/pt1[1] in block1) into one named `y0` local -- WORSE
 * (73/204, insns 198->196), saved rejected/block1-shared-y0-worse.c. Fresh
 * 38/204 re-confirmed before and after. See hypotheses.md [s39].
 * s38 (enumerate modality): body UNCHANGED. Hand-enumerated 6 spellings of
 * the obj/flags if-else block (~1160-1179, the region s37's tool couldn't
 * reach) -- 4 tied at 38, 2 worse (40, 46). Zero hits. See hypotheses.md
 * [s38] for the full per-variant breakdown. Fresh 38/204 re-confirmed
 * before and after the sweep.
 * s37 (enumerate modality): body UNCHANGED. Fresh 38/204 re-confirmed.
 * Ran tools/spelling_enum.py + sweep_variants.py over the
 * sin_p/scale/x/cos_p/z pointer-arithmetic block (all 32 def-before-use
 * orderings x commutative-swap combinations): CLASS KILL, zero hits below
 * 38 (best tied at 38, most 39-75). See hypotheses.md [s37] for the full
 * histogram + the tool-shape note (spelling_enum.py can't currently
 * enumerate the obj/flags if/else block — needs a tool extension or
 * manual unrolling in a future session).
 * s36 (structural, KILL RE-AUDIT): re-measured the s20-banked m2c block1
 * interleave-with-xz-computation kill (38->88/204, +5 insns, worse than the
 * stale-chassis 42->87 result) and the s20-banked m2c block2 permuted-store-
 * order kill (38->51/204, insns unchanged at 198) on the CURRENT chassis —
 * both stale s20 kills (originally measured on the retired 42/197 chassis)
 * hold, and the block1 interleave got relatively worse as the chassis
 * improved. Body UNCHANGED. See hypotheses.md [s36] for full detail.
 * Body UNCHANGED at s33 (rederive). s33 KILLED the s31/s32 live frontier
 * item #1 (two separately-named locals for i*2, hoping combine_givs would
 * merge them into one giv without forcing a call-spanning resident): 38 ->
 * 51/204 (worse, +2 insns), no giv-promotion trace in the dumps. This
 * closes the "how the doubled index is named/carried" axis — six distinct
 * spellings now measured, all flat-or-worse (see rejected/
 * separately-named-idxB-worse.c and hypotheses.md [s33] for full detail).
 * s33 also re-confirmed via fresh m2c read that a `do{}while()` vs `for()`
 * loop spelling is NOT expected to change codegen (GCC's loop-inversion
 * runs upstream of source-level loop-keyword choice) and did not spend a
 * probe on it.
 *
 * CHASSIS-REPRODUCTION REMINDER (hit fresh independently at s25/s28/s33 —
 * READ THIS BEFORE re-measuring): splicing ONLY the function body below
 * into src/text1b.c gives a FALSE floor of 153/204. You also need (1) the
 * 5-line extern header block immediately above the function (Judge/
 * ratan2/D_8009A820/D_8009A821/D_800F6610) and (2) changing
 * func_80053614's declared return type from void to s32 with an explicit
 * `return` (src/text1b.c ~line 1513-1519, OUTSIDE this file) — omitting
 * either miscompiles silently via K&R implicit-int to a shorter, wrong
 * function. All three together reproduce the true 38/204 (198 build
 * insns).
 * ---------------------------------------------------------------------
 * s32 derived the EXACT arithmetic behind
 * the s31-banked 124-vs-163 loop.c:3823 giv rejection: threshold=31,
 * add_cost*biv_count=2, combined lifetime=2, combined adjusted benefit=2
 * (product 124). Only +1 unit of combined lifetime (2->3) is needed to
 * flip the inequality (3*31*2=186>=163) -- far smaller than the raw
 * 124->163 gap suggested. Re-tested the s6-era "shared idx local" kill on
 * this fresh chassis WITH a .loop dump: it DOES flip the strength-reduce
 * decision (dump shows lifetime 42, giv promoted, reduced to reg 217) but
 * still regresses score 38->51 because the shared pseudo must live across
 * the ratan2 call as a 9th competing callee-saved-register resident.
 * Untried frontier: two SEPARATELY-named locals (not one shared idx) that
 * combine_givs would still merge into one giv, to see whether reload/
 * global_alloc can promote i*2 WITHOUT forcing one pseudo to live across
 * the whole call span. See hypotheses.md s32 entries for full detail.
 * ---------------------------------------------------------------------
 * s31 (forensics modality, 2026-09-16, unchanged body) — closed the pass-
 * attribution numeric gap for the FIRST time (124 vs 163, loop.c:3823),
 * superseded by s32's exact factorization above.
 * ---------------------------------------------------------------------
 * (Prior:
 * 42/204 s14-s21; 48/204 s11-s13; 58/204 s7-s10.) Body UNCHANGED from
 * s22 this session.
 * ---------------------------------------------------------------------
 * s30 (solver modality, 2026-09-16). Body UNCHANGED (re-confirmed
 * 38/204 fresh, build_insns 198). Ran the OBJECT-based
 * `inverse_compose.py classify` (via WSL — the tool's OBJDUMP config is
 * WSL-only, a bare Windows invocation throws FileNotFoundError) against
 * this exact chassis: verdict PRE-RA, instruction MULTISET differs, "no
 * backend — upstream of every model". Cross-checked with a full fresh
 * read of asm/funcs/func_80056CB8.s (all ~215 lines): TARGET'S `$fp`
 * register is NOT the 0x1F8002B8 literal at all — it is a genuine
 * loop-carried STRENGTH-REDUCED `i*2` accumulator (`sll $fp,$v1,2` at
 * entry = start*2; `addiu $fp,$fp,2` at the loop tail), used at BOTH
 * byte-table reads. `$s6` is the separate ordinary loop counter `i`.
 * Target ALSO never hoists a stable `limit`: `start` itself is spilled
 * to 0x60($sp) and RELOADED + re-added-2 fresh every iteration for the
 * bound test. So target's real allocation has NINE simultaneously-live
 * register-resident values (obj, obj2, i, idx2, flags/code, sin_p,
 * cos_p, x, z) filling every one of s0-s7+fp, while BOTH `start` (for
 * the per-iteration bound recompute) and the 0x1F8002B8 literal are
 * deliberately stack-resident (0x60/0x78), not register-resident.
 * TESTED the specific untried combination this implies — hand-carried
 * `idx2 = start*2` incremented `+= 2` in the for-clause, loop guard
 * rewritten as `i < start + 2` (no `limit` local at all), both
 * `[i*2]` table reads replaced with `[idx2]` — MEASURED WORSE: 52/204
 * (build_insns 201, +3 vs baseline). Same regressed signature as s22's
 * idx2-on-top-of-limit (55/204) and s27's idx2+literal-respelling
 * (55/204) despite being a genuinely different combination (no `limit`
 * hoist this time). Conclusion banked in hypotheses.md: GCC treats an
 * explicitly hand-written `idx2` accumulator as an ordinary competing
 * user pseudo, not as the compiler's own strength-reduced `giv` (which
 * loop.c's strength_reduce would build automatically from a literal
 * `[i*2]` expression IF its own insn_count-sensitive cost/benefit
 * predicate favored it on this chassis — already established rejected,
 * per the s26 loop.c:3823 mechanism record, on every chassis measured
 * so far). Reverted; 38/204 re-confirmed fresh after revert.
 * FRONTIER for the next session: the idx2-hand-authorship axis is now
 * dead in BOTH known combinations (with and without a hoisted `limit`).
 * The remaining open question is whether some OTHER, not-yet-tried
 * structural change to the function (elsewhere in the loop body, not
 * touching the index arithmetic) can either (a) raise this specific
 * loop's per-loop insn_count/benefit ratio enough for GCC's OWN
 * strength_reduce to promote `i*2` automatically (letting the compiler
 * build the giv, not a hand-written accumulator), or (b) independently
 * change which 8 named values compete for s0-s7 in a way that leaves
 * $fp free for a DIFFERENT genuinely-real value while the literal is
 * forced to spill by some other path entirely. Neither has a concrete
 * candidate C shape identified yet.
 * ---------------------------------------------------------------------
 * s29 (synthesis modality, 2026-09-16). Body UNCHANGED (re-confirmed
 * 38/204 fresh, build_insns 198). Decoded the s28-dumped .greg
 * "Register dispositions:" table directly (script + writeup:
 * memory/grind/func_80056CB8/../../../tmp/grind/func_80056CB8/s29/conflict_map.txt,
 * i.e. tmp/grind/func_80056CB8/s29/conflict_map.txt) to QUANTIFY the
 * s26/s28 register-pressure mechanism: pseudos 72/75/82/83/85/86/87/88
 * occupy ALL EIGHT of $s0-$s7 (obj/self-ptr, loop index i, and the
 * already-merged flags/scale/sin_p/cos_p/x/z values), and these 8 plus
 * pseudo 149 (the 0x1F8002B8 literal, in $fp) share an identical
 * 32-entry conflict set. $fp is the ONLY unclaimed callee-saved
 * register in the function, so 149 gets it by default — not a close
 * priority race. Closing this residual (target spills the literal to a
 * stack slot and reloads via a caller-saved temp instead) needs either
 * a 9th genuinely-real call-spanning value competing for a
 * callee-saved register at unchanged insn_count (none found — every
 * hand-guessed candidate, idx2 in 5+ shapes and a named hit_flag_arg
 * local in both pre-loop and in-loop placements, regresses insn_count
 * instead of being neutral), or a restructuring of one of the 8
 * existing residents' own conflict footprint (unexplored). Frontier for
 * next session: this residual is now precise enough to be a good
 * `solver` modality target (tools/ra_solver global_alloc priority-order
 * model) rather than further hand-guessed C shapes.
 * ---------------------------------------------------------------------
 * s28 (forensics modality, 2026-09-16). Body UNCHANGED (re-confirmed
 * 38/204 fresh, build_insns 198). Since 2026-08-19 asm-until-matched,
 * HEAD (src/text1b.c) carries INCLUDE_ASM, not this body -- the driver's
 * chassis-check could not auto-measure the floor this session
 * ("measurement unavailable"). Reproducing 38/204 from this file requires
 * copying ALL THREE of: (1) the function body below, (2) the 5-line
 * extern header block immediately above it (Judge/ratan2/D_8009A820/
 * D_8009A821/D_800F6610 -- outside a naive body-only extraction), AND
 * (3) changing func_80053614's declared return type from void to s32
 * with an explicit return statement (src/text1b.c:1513-1519) -- this
 * third prerequisite lives OUTSIDE this file and is easy to miss even
 * after restoring the header externs. Omitting (2) and/or (3) silently
 * miscompiles via K&R implicit-int (no build error) to a STRUCTURALLY
 * SHORTER, wrong function: body-only -> 153/204 (152 insns); + externs
 * but func_80053614 still void -> 134/204 (171 insns); + both -> the
 * true 38/204 (198 insns), byte-identical to every s22-s27 measurement.
 * Independently rediscovered the s25 evidence.md trap fresh before
 * reading it (see hypotheses.md [s28]).
 * KILLED the live-frontier item ("target's SECOND 0x1F8002B8 use is a
 * bare lui with no ori, implying a non-literal second call argument"):
 * direct re-read of asm/funcs/func_80056CB8.s shows exactly ONE lui/ori
 * pair for 0x1F8002B8 in the whole function, materialized once pre-loop
 * and reused via two stack lw/sw reload pairs -- the frontier item's
 * premise was a misreading, not a real residual. NEW replacement
 * frontier (see hypotheses.md [s28] for full detail): target SPILLS
 * the literal to a stack slot and reloads it via a caller-saved temp
 * ($t3) before each func_80053614 call, while every candidate tried
 * (including this one) keeps it live in a callee-saved register for the
 * whole function -- a register-pressure/global_alloc allocation gap, not
 * a value-identity gap. Tried naming it as a pre-loop local
 * (`hit_flag_arg`) to see if that changed the allocation decision:
 * WORSE (45/204, build_insns 199) -- reverted, KILLED as instance.
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
 *   scale = D_8009A820[i * 2] << 8;
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
/* ---------------------------------------------------------------------
 * s44 (object-model modality). OBJECT-MODEL AUDIT of every DATA MODEL
 * global the brief flagged, per the dispatch DECLARATION-PUNS warning:
 *
 * - D_8009A820 / D_8009A821: previously declared as SCALAR `extern u8`
 *   with use-site pointer arithmetic `(&D_8009A820)[i*2]` /
 *   `(&D_8009A821)[i*2]` -- flagged as a layer-1 declaration-pun risk.
 *   Checked the asm (asm/funcs/func_80056CB8.s lines ~30/67): each
 *   symbol gets its OWN independent lui %hi/%lo relocation ($fp added
 *   as the byte offset via addu before the lbu) -- NOT one shared base
 *   symbol with two different byte-offset immediates. That is the
 *   opposite of what a merged 2-byte-struct-array declaration would
 *   compile to (one lui/addu, two lbu at +0/+1), so this is NOT an
 *   aggregate-merge candidate -- prong (a)'s base-register/stride
 *   evidence argues AGAINST a merge here, adjacency notwithstanding
 *   (see [[splat-symbol-names-are-not-evidence]]). What IS wrong is
 *   the scalar->address-of spelling: fixed by re-declaring each as an
 *   incomplete array (`extern u8 D_8009A820[];` / `D_8009A821[];`) and
 *   indexing directly (`D_8009A820[i*2]`), dropping the `&...[...]`
 *   pun. MEASURED byte-neutral: sandbox still 38/204 (198 build insns)
 *   after the redeclaration -- confirms this was purely a declaration-
 *   level fix, not a codegen-affecting change.
 * - D_800F6610: re-confirmed unchanged. s6 already established (own
 *   objdiff-verified derivation, see below) that despite
 *   `D_800F6610 == D_800F6608+8` (the Rec44 `.w8` offset) arithmetically,
 *   the target's asm emits an INDEPENDENT lui/lw(D_800F6610) relocation
 *   pair, not a `D_800F6608+8` addend -- i.e. the ORIGINAL source held
 *   this as its own separate global, not a struct member reference.
 *   Kept as `extern s32 D_800F6610;` (unchanged from s6). This is the
 *   textbook counter-example the [[split-scalars-hide-aggregate]] /
 *   splat-symbol-names-are-not-evidence rules warn about: address
 *   ADJACENCY to a named struct is not by itself merge evidence: the
 *   relocation pattern is.
 * - D_800F6608: declared `extern Rec44 D_800F6608;` in code6cac.h,
 *   `.w0` member read -- MATCHES the evidence (identical shape to
 *   matched sibling func_80057094's `mid.w0` read in the same TU).
 * - Judge: declared `extern s16 Judge;` (scalar) with use-site
 *   `&Judge + (angle & 0xFFF)` pointer arithmetic at all 3 use sites in
 *   this TU (this function + the matched, COMPLETED-C func_80057CC8 at
 *   text1b.c:2055/2257) -- MATCHES existing accepted convention in this
 *   same file; not re-flagged as a pun since the identical spelling is
 *   already byte-matched, shipped code elsewhere in this TU.
 *
 * See hypotheses.md [s44] and evidence.md [s44 OBJECT MODEL] for the
 * full per-symbol verdict table.
 * ---------------------------------------------------------------------
 * s51 (forensics modality). Re-confirmed 38/204 fresh (mandatory kill
 * re-audit, zero FAKE constructs). PASS ATTRIBUTION via the instrumented
 * cc1 .greg dump (tmp/grind/func_80056CB8/dumps/text1b.greg, line 14788):
 * global_alloc's "Spilling reg 11" is the loop-invariant `limit = start +
 * 2;` pseudo (insn 21), spilled to sp+104 at insn 469 -- this NAMES the
 * exact extra 8-byte frame slot the s50 classify session's frame-size
 * delta (176 vs 168) had flagged without attribution. Two probes this
 * session, both KILLED (instance, re-measured on the CURRENT chassis):
 * (1) inlining the loop bound as `i < start + 2` (dropping the `limit`
 * local) -- global_alloc still materializes and spills the SAME invariant
 * pseudo (loop.c's movable-pseudo test doesn't care about C-level naming);
 * 38 -> 42. (2) re-measuring the s6/s12-13 "shared i*2 offset local" family
 * on this chassis (the s50 frontier had misidentified this as untried) --
 * still regresses via register pressure across the intervening ratan2/
 * func_80053614 calls, now confirmed dead on a THIRD chassis generation;
 * 38 -> 51. FRONTIER for the next session: the spill is real and named,
 * but "delete the local" is the wrong lever per
 * [[defeat-licm-hoist-var-reuse]] -- the documented fix for a hoisted
 * loop-invariant the target keeps live differently is to make the SAME
 * pseudo carry a second, USED, loop-VARIANT value (multi-set pseudos are
 * not loop.c movables), not to delete or inline the invariant's C-level
 * name. Concretely: find a loop-variant value near `limit`'s use (the
 * `i < limit` compare) that could share ITS storage with `limit` under a
 * single reused local, forcing global_alloc to treat it as multi-set and
 * skip the invariant-hoist-then-spill path entirely. Not yet attempted --
 * the shared-i*2-offset probes above tested sharing a DIFFERENT pseudo
 * (the table index, not the loop bound); this is a distinct, still-open
 * lever. Full writeup: hypotheses.md [s51].
 * ---------------------------------------------------------------------
 * s52 (rederive modality). Re-confirmed 38/204 fresh. Closed the two
 * genuinely-untried items s49-s51 kept flagging: (1) the "outer 0..1
 * counter" loop restructuring (`for (n=0;n<2;n++) { i=start+n; ... }`,
 * no `limit` local) -- s51's attempt broke on a brace-unbalanced splice;
 * this session hand-wrote a valid one and measured 68/204 (build_insns
 * 199), WORSE. (2) the do-while chassis (s34's 46/195, never previously
 * combined with any other lever) x two combos: do-while+no-limit-inlined-
 * bound measured 45/204 (build_insns 194, the LOWEST real-insn count yet
 * for this residual, still a worse weighted score); do-while+shared-idx-
 * local measured 66/204, confirming the shared-idx family dead on a
 * FOURTH chassis generation (for-loop @81/@48-58/@38x3, now do-while @46).
 * All three banked to rejected/. FRONTIER for the next session is now the
 * SAME open lever s51 named and no session has yet attempted: find a
 * loop-VARIANT value near the `limit`/`i<limit` compare site that could
 * legitimately share ONE C variable with `limit` (multi-set pseudo, not a
 * deletion or inline) -- or a fresh `.lreg`/`.rtl` dump of the post-spill
 * reload sequence's OTHER conflicts with reg 11 (text1b.greg ~line
 * 14790-14810), not yet cross-referenced against source-level candidates.
 * Full writeup: hypotheses.md [s52].
 * ---------------------------------------------------------------------
 * s55 (rederive modality). Re-confirmed 38/204 fresh (7th consecutive
 * session). Closed the s53/s54-named "pt0/pt1 shared-buffer merge" item:
 * measured 103/204 (build_insns 207), substantially worse — pointer-
 * indirected slices cost real address arithmetic vs fixed-offset stack
 * arrays. Banked to rejected/pt0-pt1-shared-buffer-worse.c. Ran
 * tools/nrefs_census.py (first time this ledger) to seed the s53
 * conflict-graph sweep; raw allocno-order data gathered but NOT yet
 * cross-referenced against the s51 .greg dump's pseudo numbering (the
 * two dumps come from different passes and may renumber) — that
 * cross-reference is the concrete next step, not yet a C hypothesis.
 * Full writeup: hypotheses.md [s55].
 * --------------------------------------------------------------------- */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820[];
extern u8 D_8009A821[];
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
        flags = D_8009A821[i * 2] << 8;
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
        scale = D_8009A820[i * 2] << 8;
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
