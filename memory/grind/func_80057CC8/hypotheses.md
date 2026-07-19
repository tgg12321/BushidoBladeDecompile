# Hypothesis ledger — func_80057CC8

## [s1] Candidate.c (offset+table reassociation, s3 pin removed) replays to sandbox distance 3 on current main HEAD
- mechanism: Reassociating BOTH p-pointer adds from (table+offset) to (offset+table) lets GCC coalesce the offset temp into the pointer dest for the SECOND p (v1->v1). The first p still allocates a new pseudo (v0 sll/sra source, v1 addu dest) — the 3 remaining diffs are all from that one coalescing miss (addu v1,v0,a2 vs target addu v0,v0,a2, cascading to two lh base regs).
- probe: Applied candidate to src/text1b.c line 11837; ran `tools/wteng.ps1 main sandbox func_80057CC8 --disable all` and `canonical func_80057CC8`
- result: sandbox score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395; canonical verdict=C distance=3; objdump-confirmed the 3 diffs are addu dest v0 vs v1 and the two dependent lh base regs at 0x80057D54/0x80057D68/0x80057D6C
- verdict: CONFIRMED

## [s2] Branchless direct-bind ternary for prev_idx (no explicit sidx local) collapses prev_idx to a single-def-like SSA live range matching next_idx's shape, allowing the same coalescing decision the second p gets.
- mechanism: Frontier hypothesis: 2026-06 sidx-explicit ternary form scored 5 due to the extra local; a direct-bind form has no extra local so cse/combine would fold the duplicated (arg1-1) and prev_idx would become effectively single-def, matching next_idx.
- probe: Applied prev_idx = ((s16)(u16)(arg1-1) < 0) ? (u16)(arg0[3]-1) : (u16)(arg1-1); with cx/cy computed before. Ran sandbox --disable all.
- result: score=7, build_insns=110 vs target 111 (GCC folded the duplicated arg1-1, dropping an insn). The cmov-shape did NOT bind prev_idx into a single-def coalesceable pseudo; it introduced a NEW diff surface. Saved rejected/ternary-direct-bind-no-local.c.
- verdict: KILLED

## [s2] Moving `prev_idx = arg1 - 1;` down to be adjacent to its if-guard (after cx/cy) changes LUID/scheduling and may improve p1 coalescing.
- mechanism: Structural: relocating the def closer to the use narrows the phi's live range and may reorder pseudo priority.
- probe: Placed `prev_idx = arg1 - 1;` after cx/cy compute but before the if((s16)prev_idx<0) reload block.
- result: score=3 (unchanged); GCC produced identical codegen. Statement position of the prev_idx def is not a lever.
- verdict: KILLED

## [s2] Mirroring next_idx's block-scoped `s32 tmp` shape onto prev_idx (`{ s32 tmp = arg1-1; prev_idx = tmp; if ((s16)tmp<0) prev_idx = arg0[3]-1; }`) makes prev_idx structurally symmetric with next_idx and enables the same p1 addu coalescing that p2 enjoys.
- mechanism: Ledger's `prime suspect` — two-def phi asymmetry. If prev_idx's def shape matches next_idx's, RA should treat them identically.
- probe: Wrote the block-scoped tmp form for prev_idx (mirror of next_idx block). Sandboxed.
- result: score=3 (unchanged); objdump shows the p1 addu still emits `addu v1,v0,a2` vs target `addu v0,v0,a2`. The block-local tmp did NOT alter RA's coalescing decision for p1. THIS DISPROVES the prime-suspect two-def-phi asymmetry hypothesis: both prev_idx and next_idx are two-def yet only p1's addu misses coalescing. The discriminator lies deeper (likely lifetime of the shifted-offset pseudo across the next_idx bnez delay-slot scheduling window).
- verdict: KILLED

## [s2] Narrowing prev_idx from `unsigned short` to `s16` eliminates the explicit sign-extend cast at every use, releasing the cast pseudo and potentially freeing v0 for the p1 addu dest.
- mechanism: Type narrowing (structural modality axis). The `<<16 >>16` sign-extend chain becomes a no-op on s16-typed prev_idx and GCC's combine may fold it, changing the pseudo lifetime.
- probe: Declared `s16 prev_idx;` and dropped the explicit `(s16)` cast on the < 0 test.
- result: score=3 (unchanged); objdump identical to u16 form — GCC's combine folds the sign-extend chain the same regardless of source type. Type narrowing is NOT a lever here.
- verdict: KILLED

## [s2] Swapping the two IF-block order (next_idx compute first, prev_idx reload second) preserves semantics and may enable target's exact scheduling shape.
- mechanism: Target's scheduling packs the prev_idx sll into the next_idx bnez delay slot at 80057D48. If we reorder our C to put next_idx first, our compiler may pack a different insn — worth measuring.
- probe: Swapped the two IF blocks (next_idx block first).
- result: score=31 (much worse), build_insns=110. Reordering forces GCC to hoist/fuse operations and destroys the delay-slot fill entirely. Saved rejected/swap-if-block-order-next-first.c. KILLED HARD.
- verdict: KILLED

## [s2] Hoisting the p1 pointer add above the next_idx block (right after prev_idx reload) constrains scheduling to compute p1 earlier and may improve RA.
- mechanism: If p1's addu happens before next_idx's bnez delay slot fills, GCC's linear scan sees a different live range and may pick v0 as the dest.
- probe: Placed `p = (s16*)(...prev_idx offset... + table);` between the prev_idx if-reload and the next_idx block.
- result: score=12 (much worse). Target scheduling REQUIRES the prev_idx sll to fall into the next_idx bnez delay slot; hoisting the p1 add moves the addu out of that scheduling window. Saved rejected/hoist-p1-before-nextidx-block.c.
- verdict: KILLED

## [s2] Simplifying the shift chain from `((s32)(prev_idx<<16)>>16)<<2` to `((s32)(s16)prev_idx)<<2` (equivalent by definition but different AST) may nudge combine to a different pseudo lifetime.
- mechanism: Sign-extend expressed via a cast rather than the double-shift idiom may present combine.c a different substitution surface.
- probe: Applied the (s32)(s16) cast form to both p1 and p2 offset computations.
- result: score=3 (unchanged), byte-identical codegen. GCC folds both forms to the same sll+sra+sll. Alternative spelling equivalent but not a lever.
- verdict: KILLED

## [s3] Named-intermediate `s32 dx = p[0]-(s16)cx; s32 dy = p[1]-(s16)cy;` locals scoped ONLY around the first call force GCC to schedule the sign-extend/subu chain distinctly and may free v0 for the p1 addu dest.
- mechanism: Frontier item #2 partial: expose the subu chain as explicit statements so scheduling can't collapse them into the delay slot.
- probe: Applied dx/dy local block around call1 only; sandbox --disable all.
- result: score=3 (unchanged), target_insns=111, build_insns=111. GCC folded the named-intermediate back into the call-arg expression.
- verdict: KILLED

## [s3] Symmetric dx/dy named-intermediate blocks around BOTH calls (matches next_idx's block-scoping shape) may unify the two-call scheduling window.
- mechanism: Structural symmetry — if both call sites use the same block-local shape, RA may treat both p addu allocations identically.
- probe: Applied dx/dy blocks around both calls; sandbox.
- result: score=3 (unchanged). GCC folded both into identical shape as inlined args.
- verdict: KILLED

## [s3] Declaring cx/cy as s16 (instead of u16 + (s16) cast at every use) narrows the type-level view of the subtract operands and may change combine's pattern for p[0]-cx and p[1]-cy.
- mechanism: Type narrowing on the OTHER half of the computation (cx/cy — parallel to s2's prev_idx type narrow attempt).
- probe: Changed `u16 cx, cy;` to `s16 cx, cy;`, loads from `*(u16*)` to `*(s16*)`, dropped (s16) casts in call args.
- result: score=3 (unchanged). Load-form differences (lhu vs lh) and cast placement are masked-identical; the subword type of cx/cy is not a lever.
- verdict: KILLED

## [s3] Hoisting `scale = arg0[2] * 40;` BEFORE p1 computation raises register pressure at the p1 addu window and may shift RA to allocate v0 for p1 dest.
- mechanism: Increase live-range pressure at the p1 allocation point to force RA to pick differently.
- probe: Moved scale computation to just before p1 pointer add.
- result: score=35, build_insns=113 (KILLED-HARD). Hoisting scale breaks the delay-slot / RA plan entirely. Structural bound documented in rejected/hoist-scale-before-p1.c.
- verdict: KILLED

## [s3] Reuse function-scope `table` local for BOTH p1 and p2 (drop the p2 re-dereference of *(s16**)(arg0+4)) — symmetric access pattern may unify allocation.
- mechanism: Single source-level pointer identity across both p adds.
- probe: Replaced `(s32)(*(s16**)(arg0+4))` at p2 with `(s32)table`.
- result: score=32, build_insns=112 (KILLED-HARD). Forces table into a callee-save across the first jal; disturbs the entire schedule. Confirms the p2 re-dereference is load-bearing. Documented in rejected/reuse-table-local-for-p2.c.
- verdict: KILLED

## [s3] Delaying `table = *(s16**)(arg0+4);` load until just before p1 pointer add narrows table's live range and may free a register at the p1 addu.
- mechanism: Point-of-definition placement — move table load to its last dominator instead of function entry.
- probe: Moved `table = ...` down to just before the p1 addu; inlined `*(s16**)(arg0+4)` at cx and cy loads.
- result: score=3 (unchanged). GCC CSE merged the three (arg0+4) loads back into a single one at the natural position. Placement of the table def is not a lever — combine.c collapses it.
- verdict: KILLED

## [s3] Moving the next_idx block AFTER the first jal shortens prev_idx's live range through the call boundary and may isolate its coalescing decision.
- mechanism: Reduce cross-call live-range interference between prev_idx and next_idx.
- probe: Placed the { s32 tmp = arg1+1; next_idx = tmp; if(...) next_idx = 0; } block between call1 and p2 setup.
- result: score=41, build_insns=113 (KILLED-HARD). Destroys the target scheduling shape: target packs prev_idx's sll into next_idx's bnez delay slot at 80057D48; that fill requires next_idx computed BEFORE call1. Confirms/extends s2's swap-if-block-order (31) and hoist-p1-before-nextidx (12) kills. Documented in rejected/defer-next-idx-block-after-call1.c.
- verdict: KILLED

## [s4] Directed permuter from the candidate-floor-3 baseline finds a legitimate (non-cheat) structural mutation that closes the p1 addu coalescing gap.
- mechanism: s1-s3 hand-derivation exhausted the low-hanging structural axes; permuter's random + PERM_* macros explore mutations that cheat-reviewer can then filter. If any closing form survives the cheat-by-any-spelling checklist, it names a legitimate lever the hand-derivation missed.
- probe: Built clean single-function permuter workspace per [[difficult-is-not-impossible]] §3 (asm/funcs/func_80057CC8.s + prelude sans .set gp=64; awk-extract of func region from cc1|prologue_fix|maspsx|fix_lwl|sed align3->2|multu_pad pipeline; hand-cp of base_full.c since strip_other_fns fails on our preprocessed file). Verified base=target=111 insns and diff signature = 3 (v1 vs v0 p1 addu + 2 cascading lh). Launched permuter_campaign.py launch --func func_80057CC8 -j 6 --stop-on-zero. Ran ~25 min, ~5780+ iterations, 1039 output-* dirs, seven distinct low-score families. Vetted every low-score form against the cheat-by-any-spelling checklist.
- result: Score-0 close found (output-0-1): `long new_var2 = ((((s32)(prev_idx<<16))>>16)<<2) + (s32)table; p = (s16*)new_var2;` — introduces a `long` local carrying the p1 pointer-address expression, shifting the pseudo lifetime/copy-pref so RA picks v0. **This is the same cheat class as rejected/block-scope-alias-p1.c** (2026-06-16 layer-1 FAIL, 2026-06-22 layer-2 FAIL): the local has no semantic purpose, its only effect is RA lifetime shaping, a human programmer would not introduce a `long` holder for an s16* address, and the justification is GCC-internals-only. Rejected per [[no-new-park-categories]] cheats-by-any-spelling. Saved as memory/grind/func_80057CC8/rejected/permuter-long-new_var2-p1-alias.c. Companion low-score families all vetted: score 15 = pure declaration reorder (no improvement); score 25/35/78 = pointer/cx/arg-alias variants (same cheat class as score-0); score 30 = `scale = (s32)table` variable-reuse (would need FAKE + layer-2 named-local-fake-exception and only masked-15); score 40 = statement-splitting on ((ang_next-ang_prev)/2)+ang_prev (no improvement); score 45 = `table++; table--;` no-op (obvious cheat).
- verdict: KILLED

## [s4] The p1 addu coalescing gap can be closed at score 0 by a non-lifetime-shaping structural axis discoverable by random+PERM_* permutation.
- mechanism: If a non-alias structural axis exists (declaration reorder, statement reorder, type refinement, expression respelling), the permuter enumerates it within the search space of PERM_LINESWAP + PERM_VAR + PERM_GENERAL.
- probe: 5780+ iterations across all mutation categories; low-score families surveyed by scanning output-N-1 for N in {15,25,30,35,40,45,78,100,105,120,125,130,145,165,220,250,265,290,300}.
- result: Only score-0 form is the `long new_var2` alias. All non-alias structural mutations plateau at score >= 15 (declaration reorder). This confirms s3's finding that 'the RA-coalescing gap CAN be closed by shaping the p1 pseudo's lexical lifetime, but that spelling is a cheat-by-any-spelling' — permuter now confirms the same statement mechanically: the search space enumerates lifetime-shaping as the sole closing axis.
- verdict: KILLED

## [s5] A directed PERM_GENERAL enumeration over structurally distinct spellings of the prev_idx reload block (3 alts) x p1 addu offset expression (4 alts) x p2 addu offset expression (2 alts) surfaces a novel legitimate structural axis the s4 undirected random chassis missed.
- mechanism: s4 was undirected (random over base); permuter search space enumerates finitely under PERM_GENERAL alone, giving deterministic coverage of the exact cross-product. If a legitimate lever exists among these 24 combinations, deterministic enumeration finds it in <=24 iters.
- probe: Chassis 1 (s5-directed): base.c with 3 PERM_GENERAL blocks around (reload if-form, p1 pointer add, p2 pointer add) yielding 3*4*2=24 combinations. Launched via tools/permuter_campaign.py, -j 6, --stop-on-zero.
- result: 24 iterations, all completed, base_score=15 permuter-scored (=sandbox floor 3); scores observed: min 15, max 25 (25 in ~6/24 iters, 15 in ~18/24). No score-0 close found in the enumerated directed space. campaign.log confirms 'Will run for 24 iterations' -> 24-iteration deterministic exhaustion. Elapsed 161s. Artifacts: tmp/grind/func_80057CC8/s5/perm/campaign.log (s5-directed section).
- verdict: KILLED

## [s5] PERM_RANDOMIZE wrapping the whole function body (combined with the directed PERM_GENERAL alternatives) explores a mutation neighborhood around each of the 24 baseline chassis that the s4 pure-random chassis missed, and surfaces a legitimate score-0 close outside the alias-holder cheat class.
- mechanism: Random mutations layered on top of directed structural choices give the permuter smaller mutation neighborhoods per iteration + fresh-seed odds within a structurally distinct basin, per [[permuter-directives]] Campaign discipline.
- probe: Chassis 2 (s5-random+directed): same PERM_GENERAL blocks wrapped in PERM_RANDOMIZE over the entire function body. Fresh launch, -j 6, --stop-on-zero. Ran 237 iterations in 134s (fast; the campaign self-terminated on score-0 find).
- result: Score-0 found at iteration 237 (57.7s in). Form: `s16 *new_var2; ...; new_var2 = (s16 *)((((s32)(prev_idx<<16)>>16)<<2) + (s32)table); p = new_var2;` -- a pointer-alias local holding the p1 pointer-address expression. Vetting per [[no-new-park-categories]] cheat catalog: (1) no semantic purpose (a human writes `p = <expr>;` directly), (2) dead in emitted output (GCC forwards the value; only source-level existence shifts the pseudo lifetime for RA copy-pref), (3) necessary only because permuter said so, (4) GCC-internals-only justification. IDENTICAL cheat class to rejected/block-scope-alias-p1.c (layer-1 FAIL 2026-06-16, layer-2 FAIL 2026-06-22) and rejected/permuter-long-new_var2-p1-alias.c (s4 2026-07-19). Rejected in-session; saved as rejected/permuter-s16-new_var2-p1-alias.c.
- verdict: KILLED
