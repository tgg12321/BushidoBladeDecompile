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

## [s6] The p1 addu's dest cannot allocate to v0 because pseudo 86 (the C variable p) conflicts with hard reg 2 (v0) at global-alloc time.
- mechanism: sched1 hoists insn 115 (and $v0,$v0,0xFFF finalizing ang_prev) from position (114,117) right after call1 to position (145,147) right before call2. After the hoist, v0 remains live from call_112's set through insn 115's kill, covering insn 124 where pseudo 86 is redefined as the p2 pointer. Global.c's conflict computation registers 86-vs-2. The greg dump line ';; 86 conflicts: 72 74 75 77 84 86 87 2 4 16 17 29' includes hard reg 2 for exactly this reason.
- probe: Compared insn-115's linked-list neighbors across rtl/jump/cse/loop/cse2/flow/combine/sched dumps. Only the sched dump shows the move to (145,147); every earlier pass has it at (114,117). Confirmed sched1 is the sole reorderer.
- result: greg conflict list for pseudo 86 includes hard reg 2; conflict traced to sched1's schedule of insn 115.
- verdict: CONFIRMED

## [s6] The p1 addu's dest lands in v1 because pseudo 86 has copy-preference for hard reg 3 (v1).
- mechanism: At insn 124 (p2 addu, dest=86, src=129+130), local-alloc has already placed pseudo 129 (the shifted next_idx offset) into hard reg 3 (v1). expand_preferences in global.c propagates 129's placement (v1) into 86's hard_reg_copy_preferences via the addsi3 SET_SRC/SET_DEST relation. The greg dump line ';; 86 preferences: 3' encodes exactly this. No competing preference (86 does not conflict with 3; the two hard regs it might prefer via the p1 sources are v0 (blocked by conflict) and a2 (no propagated preference)) exists. Global-alloc thus picks v1.
- probe: Read greg header block. 86 preferences: 3. Cross-checked pseudo 129 disposition (129 in 3) and its single use at insn 124's addu source. No plain-move insn defines/uses 86, so the preference must originate from the addu operand affinity.
- result: greg copy-preference for pseudo 86 is {3}; dispositions place 129 in 3.
- verdict: CONFIRMED

## [s6] loop-rotation-two-shift opaque-one arithmetic (s5 frontier item 3) applies to func_80057CC8's shift chain.
- mechanism: The [[loop-rotation-two-shift]] rule sanctions '(Random() & 3) + 1 - 1' opaque-one only for loop-rotation two-shift contexts to defeat GCC bit-test transforms.
- probe: func_80057CC8 has NO loop. The <<16 in the p1/p2 chain is a sign-extension idiom ((s32)(x<<16)>>16); the <<2 is an element-stride scale. Neither is a loop-rotation two-shift, and there is no combine bit-test transform in scope to defeat.
- result: Rule prerequisites not met (no loop; no two-shift rotation; no bit-test target). Inapplicable.
- verdict: KILLED

## [s6] s5 frontier item 1 (greg dump names the copy-pref biasing pseudo) is dischargeable in one forensics session.
- mechanism: Instrumented cc1 -da produces greg dump; extraction of func_80057CC8's dispositions/conflicts/preferences plus per-pass insn-position tracing identifies the pseudo and the pass responsible for the conflict.
- probe: Ran cc1 -da; extracted func block from text1b.i.greg (682 lines) and func_lreg; walked insn 115 across all pass dumps.
- result: Biasing chain fully named: sched1 hoists insn 115; local-alloc places pseudo 129 in v1; expand_preferences imports v1 into 86's copy-prefs; global-alloc places 86 in v1. Both p1 and p2 addu emit v1 dest.
- verdict: CONFIRMED

## [s7] Pseudo 86 (the SI reg holding p) is /v-marked (user variable), bound at C-expand from the single C-source local declaration `s16 *p;`. Both p1 (insn 89) and p2 (insn 124) SETs target this SAME pseudo 86.
- mechanism: expand time: DECL_RTL(p) is one pseudo. All source-level `p = <expr>;` statements compile to SET reg/v:SI 86 = <expr>. Multi-def within a single BB4, dies twice (after p1 uses, after p2 uses).
- probe: Fresh cc1 -da run on candidate baseline; grep pseudo 86 across rtl/jump/cse/loop/cse2/flow/combine/sched/lreg/greg: every occurrence is (reg/v:SI 86). Two SET insns (89 = plus 112+88; 124 = plus 129+130). lreg says '86 used 6 times across 10 insns in block 4; dies in 2 places' yet greg lists 86 in 'regs to allocate' (global-alloc pool). Local-alloc skips 86 despite single-BB because it has two independent def-use chains within BB4.
- result: CONFIRMED. Pseudo 86 is one global-alloc allocno; target's separate p1-dest (v0) requires TWO RTL pseudos, which requires two C locals (rejected aliases) or inline expressions (untested).
- verdict: CONFIRMED

## [s7] Pseudo 86's copy-preference {3} arises from ONLY one of the four operand sources across its two SET insns — specifically pseudo 129's v1 at insn 124 — not from pseudo 112 (v0) or 88 (a2) at insn 89, and not from pseudo 130 (a0) at insn 124.
- mechanism: global.c expand_preferences propagation for PLUS-based SETs is asymmetric: only one operand's hard reg biases the dest's copy-pref set. Empirically the choice landed on 129 (the shift-derived operand at insn 124), not 130 (the load-derived operand) and not either p1 operand.
- probe: Read greg header ';; 86 preferences: 3'. Cross-checked dispositions: 112 in 2, 88 in 6, 129 in 3, 130 in 4. Only {3} appears in 86's preferences.
- result: CONFIRMED. Suggests a targeted probe: does swapping PLUS operand order in source or in RTL toggle which operand propagates its hard reg into 86's copy-prefs? Deferred to next-session (source reading of global.c + directed test).
- verdict: CONFIRMED

## [s7] Suppressing sched1's hoist of insn 115 (removing the 86-vs-2 conflict) would BY ITSELF re-enable target's p1-in-v0 allocation.
- mechanism: If v0 no longer conflicted with pseudo 86, global-alloc could pick v0 for 86. However, 86's positive copy-pref is {3}, biasing it toward v1 regardless of what regs are conflict-free.
- probe: Analytical: greg preferences for 86 are {3}, unconditionally. Conflict removal would open v0 as a legal choice, but the preference bias would still steer selection to v1. Two separate levers would be required to reach v0.
- result: KILLED (downgraded). The consumer-of-ang_prev-early frontier axis is a NECESSARY-but-NOT-sufficient lever. Even with a legitimate ang_prev consumer eliminating the hoist, pseudo 86 (a single allocno bound to C-source `p`) would still land in v1 by copy-pref. Any pure-C form that keeps `p` as ONE C variable cannot reach target's p1=v0 allocation.
- verdict: KILLED

## [s7] loop-rotation-two-shift opaque-one arithmetic applies to func_80057CC8's shift chain (kept in ledger from s6; retested for completeness).
- mechanism: The rule sanctions '(Random() & 3) + 1 - 1' opaque-one ONLY for loop-rotation two-shift contexts.
- probe: func_80057CC8 has no loop; the <<16>>16<<2 chain is sign-extend + element-stride, not loop rotation.
- result: KILLED — inapplicable (unchanged from s6).
- verdict: KILLED

## [s8] s7 frontier axis #1: fully inline both call sites' `p` expression (no `p` local declared) generates two distinct RTL pseudos and lands p1's dest in v0 legitimately.
- mechanism: The C-frontend allocates a fresh RTL pseudo per source expression rather than DECL_RTL of a user local. With no `p` bound anywhere, each call-arg site's (s16*)(...) address expression becomes its own pseudo; CSE collapses the [0]/[1] pair inside each arg-list to one temp, giving two distinct pseudos (one per call site) with independent global-alloc decisions.
- probe: Applied inline-expression form to both call sites (address expr written 4 times: [0]/[1] for prev + [0]/[1] for next, no `p` local); sandbox --disable all.
- result: score=0, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395. Closes cleanly. BUT the form is character-for-character duplication of a 30+ character compound address expression, 4 times. Rejected in-session per [[no-new-park-categories]] cheats-by-any-spelling checklist: (1) no semantic purpose — programmer would write `p = <expr>; use p[0]; use p[1];` (candidate.c form); (2) dead in emitted output (CSE collapses); (3) GCC-internals-only justification (RTL pseudo lifetime shaping); (4) coercion signal (character-for-character expression duplication is not idiomatic C). IDENTICAL cheat class to rejected/duplicate-address-expr-pseudo-inline.c (2026-06-15, which inlined only the FIRST call and was rejected on identical grounds). This form is a MORE egregious spelling (both calls) of the SAME cheat, not a distinct axis. Saved as rejected/inline-both-call-sites-no-p-local.c.
- verdict: KILLED

## [s8] s7 frontier axis #2: swapping p2's PLUS operand order in the C source (write `table + offset` instead of `offset + table`) shifts expand_preferences propagation so pseudo 86's copy-preference draws from pseudo 130 (a0) instead of pseudo 129 (v1), redirecting global-alloc away from v1.
- mechanism: Per s7 evidence, expand_preferences propagation for the p2 addu SET is asymmetric — only pseudo 129 (offset, v1) propagates its hard-reg into 86's copy-prefs; pseudo 130 (table load, a0) does not. Reversing the source-level operand order might rearrange RTL's src1/src2 assignment so 130 becomes the propagating source instead.
- probe: Applied p2 form `p = (s16*)((s32)(*(s16 **)(arg0 + 4)) + (((s32)(next_idx << 16) >> 16) << 2))` (table-first) with p1 kept at candidate's offset+table form; sandbox --disable all. Then also tested p1 swap alone (p1 = table+offset, p2 = offset+table).
- result: p2 swap alone: score=3 -> 9 (regression, matches HEAD-form's un-reassociated distance). p1 swap alone (p1 table+offset, p2 offset+table): score=3 (byte-neutral). This measurement shows the operand-order in the C source has one effect only: it controls whether GCC's late reassociation reaches the coalesce-friendly form for the SECOND p. Swapping p2 back to table-first does not redirect propagation — it just breaks p2's own coalescing (from `v1,v1,X` to `v1,X,v1`-shape or similar). The pseudo-129-source-propagation asymmetry is a fixed property of the addsi3 SET's SRC1/SRC2 slot layout that operand-order rewriting in the C source does not toggle.
- verdict: KILLED
