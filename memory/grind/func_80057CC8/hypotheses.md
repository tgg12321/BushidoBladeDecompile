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

## [s9] Rewriting the two p computations as a walking-pointer shape (`p = table; p += (s16)prev_idx*2;` then `p = *(s16**)(arg0+4); p += (s16)next_idx*2;`) reinterprets the address-add as a semantic table walk, giving GCC a two-SET-per-p pattern that may bias RA to allocate separate pseudos for the intermediate `p = base` and `p += off` stages.
- mechanism: Fresh rederive angle: instead of one address expression per slot, split each p into (base-init) then (compound-assign advance). Multiple SETs per p might change GCC's RA priority or copy-pref propagation vs one SET per compound expression.
- probe: Applied form to src/text1b.c line 11869-11874 (candidate baseline replaced with `p=table; p+=(s16)prev_idx*2;` for slot 1 and `p=*(s16**)(arg0+4); p+=(s16)next_idx*2;` for slot 2). Sandbox `--disable all`.
- result: score=3 -> 7, target_insns=111, build_insns=111. GCC does NOT fold `p = base; p += off*2;` back to `p = base + off*4;` cleanly — the two-SET form emits extra insns and regresses coalescing. Saved memory/grind/func_80057CC8/rejected/rederive-p-plus-equals-walk.c.
- verdict: KILLED

## [s9] Collapsing both p-compute + call blocks into a two-iteration `for (i=0; i<2; i++)` loop over `unsigned short idx[2]` / `s32 ang[2]` arrays is the natural human-C shape a fresh reader would derive (both blocks are semantically identical). The loop RA may allocate a fresh pseudo per iteration through PHI structure, giving target's asymmetric p1=v0/p2=v1 allocation.
- mechanism: Fresh rederive angle: the two call sites compute the same thing (angle from current to arena[idx[i]]); the human C would express this as a loop. GCC 2.7.2 might unroll a 2-iter constant-bound loop, producing structurally distinct RTL from the linear form.
- probe: Applied form to src/text1b.c line 11837 (function rewritten with idx[2], ang[2], and a for i<2 loop). Sandbox `--disable all`.
- result: score=3 -> 77, target_insns=111, build_insns=118. GCC 2.7.2 at -O2 does NOT unroll a 2-iter for-loop (needs -funroll-loops, which is NOT in project's canonical flag set per [[compiler-flags-canonical]]). The loop compiles as an actual loop with branch + induction, adding ~7 insns and destroying the scheduling window target uses. Saved memory/grind/func_80057CC8/rejected/rederive-for-loop-idx-array.c.
- verdict: KILLED

## [s9] Baseline candidate.c (offset+table reassociation, s3 pin removed) replays to sandbox distance 3 on current main HEAD.
- mechanism: Baseline re-verification: apply memory/grind/func_80057CC8/candidate.c to src/text1b.c line 11837 and re-measure.
- probe: Applied candidate.c to src/text1b.c; ran `tools/wteng.ps1 main sandbox func_80057CC8 --disable all`.
- result: score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395. Replays s1-s8 baseline cleanly.
- verdict: CONFIRMED

## [s10] SYNTHESIS: the single-pseudo impossibility + Judge constraint together leave a 3-item frontier (F1 duplicated-statement-into-arms, F2 arg0 struct header-type correction, F3 Kengo transplant); no other sanctioned axes remain unmeasured.
- mechanism: Merged s1-s9 findings. Single-pseudo (s7 CONFIRMED) + Judge's binding no-two-locals sentence + s8's dead-axis measurements narrow the sanctioned surface to exactly these 3 axes. F1 is structural (highest plausibility); F2 requires caller evidence; F3 requires corpus accessibility. Full analysis: tmp/grind/func_80057CC8/s10/synthesis.md.
- probe: Analytical only; no sandbox this session (synthesis modality). Re-read evidence.md + hypotheses.md + rejected/; cross-checked Judge constraint vs the 17-form rejected bank; verified no unlisted sanctioned axis exists.
- result: Frontier reset to F1/F2/F3 (structural/rederive/rederive). Recommended kill-order: s11 F1 -> s12 F2 -> s13 F3. If all KILLED with measurements, function has exhausted sanctioned pure-C axes and OWNER-ESCALATION becomes correct next step (not eligible this session -- no pre-filed docs/grind/decisions.md entry for func_80057CC8).
- verdict: CONFIRMED (synthesis; not a measurement kill)

## [s10] The s1-s9 ledger, merged, defines a proven impossibility surface: single-C-variable `p` cannot reach target's p1=v0 (s7 CONFIRMED via forensics on pseudo 86's {3} pref origin and v0 conflict origin), the Judge's binding sentence closes ALL two-C-local spellings across 17 rejected forms, and s8 measured BOTH Judge-suggested named axes (inline-both-calls, p2-operand-swap) dead. The frontier is therefore exactly: F1 duplicated-statement-into-arms (structural, SOTN-sanctioned per [[duplicated-statement-into-arms]], not yet directly measured), F2 arg0 header-type correction (rederive, requires caller struct evidence), F3 Kengo transplant (rederive, requires corpus accessibility).
- mechanism: Merge of s1-s9 evidence: (i) pseudo 86's copy-pref {3} originates ONLY from pseudo 129's v1 propagation via expand_preferences at insn 124's addsi3 (s7 CONFIRMED); (ii) 86's v0 conflict originates from sched1 hoisting insn 115 across insn 124's redefinition of 86 (s6 CONFIRMED); (iii) both properties are inherent to a single-pseudo binding of C-source `p`; (iv) Judge binding constraint forbids all two-local splits; (v) s8 measured inline-both-calls at 0-but-cheat-class, p2 swap regressed to 9. Only F1/F2/F3 remain uneliminated within sanctioned bounds.
- probe: Analytical synthesis only (session modality=synthesis). Re-read evidence.md + hypotheses.md + rejected/ (17 forms). Cross-checked Judge binding sentence vs rejected bank; verified F1 has SOTN-sanction basis, F2/F3 require external evidence.
- result: Frontier reset to F1 (primary structural), F2 (secondary rederive), F3 (tertiary rederive). Synthesis document saved to tmp/grind/func_80057CC8/s10/synthesis.md. Recommended kill-order: s11 F1 -> s12 F2 -> s13 F3. If all measured dead, owner-escalation becomes the correct disposition (not eligible this session -- no pre-filed decisions.md entry).
- verdict: CONFIRMED

## [s10] F1 (duplicated-statement-into-arms) is a WEAK lever a priori: reg_n_refs on pseudo 86 enters allocno priority ordering, but expand_preferences computes 86's copy-pref set from STATIC SET-operand analysis before realized placements, so a priority-order bump alone does NOT alter 86's {3} pref. F1 will only move if it also shifts the sched1 hoist of insn 115 or changes the operand structure at insn 124.
- mechanism: Per s6/s7 forensics: expand_preferences runs before global-alloc realizes hard-reg placements; it walks SET insns and propagates operand hard-reg hints statically. Bumping reg_n_refs on pseudo 86 changes its allocno priority (allocated earlier), but the {3} pref set itself is built from the p2 addsi3 SET structure, which duplicated-arms does not touch. Therefore F1's plausible mechanism must be indirect: the duplicated statement changes the RTL insn count in the prev_idx-if window, which may perturb sched1's hoist decision for insn 115 (removing the v0 conflict).
- probe: s11 (structural): draft duplicated-arms form, objdump vs candidate.c to verify cross-jump merged arms byte-identically (SOTN sanction prerequisite), sandbox --disable all. If floor drops AND merge is byte-neutral, invoke layer-1 + layer-2 cheat-reviewer for [[duplicated-statement-into-arms]] policy compliance (mandatory FAKE annotation, s1-s9 ledger as exhaustion evidence).
- result: Not measured this session. Reasoning-only kill would be inappropriate given SOTN sanction; must be sandbox-measured.
- verdict: CONFIRMED

## [s11] F1 variant A: duplicate p1 assignment into both arms of the prev_idx if (implicitly moves p1 before next_idx block, cross-jump expected to merge byte-neutrally).
- mechanism: Per [[duplicated-statement-into-arms]] + s10 F1 mechanism note: extra RTL SETs of pseudo 86 bump reg_n_refs, potentially shifting global-alloc priority order or perturbing sched1's hoist of insn 115. Cross-jump find_cross_jump merges the identical p= tails byte-neutrally.
- probe: Applied F1 variant A to src/text1b.c line 11857 (if((s16)prev_idx<0){prev_idx=...; p=<expr>;} else {p=<expr>;}). Sandbox --disable all.
- result: score=12, target_insns=111, build_insns=111. Matches s3's rejected hoist-p1-before-nextidx-block score-12 kill exactly. Cross-jump did NOT merge to byte-neutrality — p1 assignment out of its scheduling window kills delay-slot fill regardless of duplication. Saved rejected/dup-arms-prev_idx-if.c.
- verdict: KILLED

## [s11] F1 variant B: split the next_idx `default; if(...) override;` block into a proper if/else and duplicate p1 into both arms of THAT (keeps p1 in its natural post-next_idx scheduling window position).
- mechanism: Attempt to preserve the scheduling window while still injecting extra RTL SETs of pseudo 86. If cross-jump merges the tails, only the reg_n_refs bump survives.
- probe: Applied F1 variant B: {s32 tmp=arg1+1; if((s16)tmp>=(s32)arg0[3]){next_idx=0; p=<expr>;} else {next_idx=tmp; p=<expr>;}} then ang_prev use. Sandbox --disable all.
- result: score=8, target_insns=111, build_insns=111. Regression from candidate baseline 3. Rewriting the next_idx `default; override;` shape into if/else disturbs sched1's delay-slot fill (target's shape packs prev_idx sll into the next_idx bnez delay slot at 80057D48); the duplicated p1 does NOT compensate. Cross-jump did not merge. Saved rejected/dup-arms-next_idx-ifelse.c.
- verdict: KILLED

## [s11] F1 variant C: duplicate `table = *(s16**)(arg0+4);` (non-p statement, sibling [[split-read-defeats-hoist]] pattern) into both arms of the prev_idx if, keeping natural next_idx block and single post-block p= assignment.
- mechanism: Avoids moving p= out of its scheduling window. If GCC preserves the two loads (as split-read-defeats-hoist expects), the table pseudo's reg_n_refs bumps and may perturb schedule around insn 124 or shift RA of pseudos 88/130 (p1/p2 table operands).
- probe: Applied F1 variant C: added redundant `table = *(s16**)(arg0+4);` inside both prev_idx arms; kept single p= after next_idx block. Sandbox --disable all.
- result: score=3 (byte-neutral, no change from baseline). GCC's cse1 pass merges the two redundant reloads back into ONE before global-alloc sees them; reg_n_refs on table's pseudo is NOT bumped, and no priority lift reaches pseudo 86. The duplicated statement disappears at CSE, so no downstream effect on p1 coalescing. Saved rejected/dup-arms-table-reload.c.
- verdict: KILLED

## [s11] Baseline replay: candidate.c (offset+table reassoc, s3 pin removed) still measures at sandbox distance 3 on current main HEAD.
- mechanism: Session-entry precondition check.
- probe: Applied candidate.c to src/text1b.c line 11837 (removed `register asm("s3")` from next_idx; swapped table+offset -> offset+table for both p adds). Sandbox --disable all.
- result: score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 — replays s1-s10 baseline cleanly.
- verdict: CONFIRMED

## [s12] Candidate.c (offset+table reassociation, s3 pin removed) still measures at sandbox distance 3 on current main HEAD.
- mechanism: Session-entry precondition check.
- probe: Applied candidate.c form to src/text1b.c line 11837 (u8 *arg0, offset+table reassociation for both p adds, no s3 pin); ran `tools/wteng.ps1 main sandbox func_80057CC8 --disable all`.
- result: score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 — replays s1-s11 baseline cleanly on current main HEAD.
- verdict: CONFIRMED

## [s12] F2 axis: correcting arg0's type from `u8 *` to a struct pointer (ArenaHdr_57CC8 { u8 flags; u8 _p1; u8 scale_units; u8 count; s16 *table; }) presents combine.c a different SET_SRC/SET_DEST substitution surface for the (offset + table) expression at insn 124, altering pseudo 86's copy-preference propagation and enabling target's p1=v0 allocation.
- mechanism: SOTN-sanctioned four-prong [[header-type-correction-from-use-sites]] carve-out (2026-07-13). A struct-member access might present combine a different addsi3 pattern, potentially shifting which operand's hard reg propagates into pseudo 86's copy-preference set (per s7 forensics, currently only pseudo 129's v1 propagates via insn 124's addsi3; pseudo 130's a0 does not).
- probe: Rewrote func_80057CC8 signature to `void func_80057CC8(ArenaHdr_57CC8 *arg0, ...)`; replaced `*(s16**)(arg0+4)` with `arg0->table` at both p1 (via cached `table` local) and p2 (fresh member access); replaced `arg0[3]` with `arg0->count` at both if-boundary tests; replaced `arg0[2]` with `arg0->scale_units` at scale compute. Sandbox --disable all.
- result: score=3 (unchanged from candidate baseline), target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395. Byte-neutral: the type-level change is fold-equivalent under GCC 2.7.2 combine.c. Struct member access `arg0->table` compiles to identical `lw $rD, 0x4(arg0)` as `*(s16**)(arg0+4)`; `arg0->count` compiles to identical `lbu $rD, 0x3(arg0)` as `arg0[3]`. combine.c sees the same addsi3 SET_SRC/SET_DEST structure at insn 124 regardless of arg0's source-level type. Pseudo 86's copy-pref {3} origin (pseudo 129's v1 propagation) is unchanged. Saved memory/grind/func_80057CC8/rejected/struct-typed-arg0-header-correction.c.
- verdict: KILLED

## [s12] The [[header-type-correction-from-use-sites]] four-prong sanction is applicable to func_80057CC8's arg0 signature (arg0 usage pattern: bytes 2/3 + s16* table at offset 4 = a plausible 8-byte struct).
- mechanism: Prong (a) 'grep-consistent use sites, at least one signed-specific'; prong (b) 'OLD type required functionally necessary compensating casts'; prong (c) 'one extern edit, never alias-rename/pointer-pun'; prong (d) 'casts eliminated at every use site'.
- probe: Grep across src/ and include/ for `func_80057CC8`: single C use site (the definition itself). Grep across asm/ for callers: only asm caller is func_80057E84 (three call sites, all passing $s1 = struct-table-base + 8*index). No other C caller exists; there is no shared header extern.
- result: Prongs (a)/(c) fail: 'grep-consistent use sites' is trivially unfalsifiable with one C use site and no shared extern to correct — the four-prong bar was designed for globals with a canonical header declaration, not for parameter types on a function with only asm callers. Even setting the sanction bar aside, the codegen measurement is byte-neutral (0 gradient), so no closure opportunity exists on this axis regardless of policy fit.
- verdict: KILLED

## [s13] A permuter chassis targeting the downstream ang_mid ternary rearrangement (with the p1/p2 alternatives from s5 removed) surfaces a novel legitimate lever.
- mechanism: Re-arranging the `if(ang_next<ang_prev){base=...;half=...;ang_mid=base-half;}else{...}` block (4 alternatives incl. inlined base-half, distributed subtraction, and ternary-form) could shift sched1's hoist window for insn 115 (and $v0,0xFFF finalizing ang_prev) and break pseudo 86's {3} pref origin (per s6/s7 forensics: 86's copy-pref = {3} propagates ONLY via pseudo 129 v1 through expand_preferences at insn 124). Also permuted *arg2 assignment ordering (cx-first vs shift-first).
- probe: Fresh workspace s13/perm cloned from s5/perm; base.c PERM_GENERAL over ternary (4 variants) + *arg2 write order (2 variants), PERM_RANDOMIZE overlay; -j 6 --stop-on-zero via tools/permuter_campaign.py; base_score reported 15.
- result: 199 iterations in ~150s wall; ONE score-0 find (output-0-1, iter ~55s). The close-form: `s16 *new_var2 = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4))); p = new_var2;` — an alias-holder for the p2 SET, no downstream-ternary mutation. Rejected in-session per [[no-new-park-categories]] cheat-by-any-spelling + s10 Judge binding constraint (`no shared-pointer split into two source-level locals under any spelling`). Saved as rejected/permuter-s16-new_var2-p2-alias.c.
- verdict: KILLED

## [s14] A permuter chassis that mutates p1/p2 SHIFT-EXPRESSION spelling (not previously permuted) plus p2 TABLE-SOURCE spelling (cached `table` local vs reload of *(arg0+4)) will surface a legitimate score-0 lever by shifting pseudo 129's copy-pref propagation into pseudo 86 at insn 124.
- mechanism: s6/s7 forensics traced 86's {3} preference to pseudo 129 (v1, shifted next_idx offset) propagating via expand_preferences at insn 124's addsi3 SET. Alternative shift spellings ((s16)*4, (s32)(s16)<<2, <<16>>14, <<16>>16<<2) route through different quantity/subreg chains, potentially altering pseudo 129's placement or a2/a0 selection at p2. Table-source spelling (fresh reload vs cached `table` local) affects pseudo 130 (a0), the OTHER p2 operand.
- probe: Cloned s13/perm -> s14/perm. Replaced s13's PERM_GENERAL over (prev_idx if-reload, ang_mid ternary, *arg2 write-order) with 4-alt PERM_GENERAL on p1 shift, 4-alt PERM_GENERAL on p2 shift, 3-alt PERM_GENERAL on p2 table source. Launched `permuter_campaign.py launch --func func_80057CC8 --label s14-p1p2-shift-and-table-source -j 6 --stop-on-zero`, pid 410, base_score=15.
- result: score-0 found at iter 19 / 40.1s. Form: `s16 *new_var2 = (s16 *)((((s32)((s16)prev_idx))<<2) + (s32)table); p = new_var2;` for the p1 SET. This is a p1-side pointer-alias holder — same cheat class as s5 rejected/permuter-s16-new_var2-p1-alias.c (character-for-character equivalent under Judge s10 binding constraint's `no shared-pointer split into two source-level locals under any spelling, incl. numeric suffix`). Saved to memory/grind/func_80057CC8/rejected/permuter-s14-p1-shift-cast-alias.c.
- verdict: KILLED

## [s15] The ';; 86 preferences: 3' line in the greg dump reflects hard_reg_preferences[86], not hard_reg_copy_preferences[86]. dump_conflicts in global.c:1741-1745 iterates hard_reg_preferences[i] when emitting that line.
- mechanism: Read tools/gcc-2.7.2/global.c dump_conflicts: the ';; N preferences:' line iterates TEST_HARD_REG_BIT(hard_reg_preferences[i], j). hard_reg_copy_preferences is a distinct array populated only by set_preference(copy=1) and expand_preferences.
- probe: Read tools/gcc-2.7.2/global.c dump_conflicts (line 1702-1748). Cross-checked expand_preferences (line 798-841) and set_preference (line 1591-1675).
- result: CONFIRMED. The greg dump line reports hard_reg_preferences, not hard_reg_copy_preferences. Ledger s6/s7 phrasing 'copy-preference = {3}' is imprecise — it is hard_reg_preferences {3}. Refines s6/s7 mechanism attribution.
- verdict: CONFIRMED

## [s15] hard_reg_preferences[86] is populated by set_preference on BOTH insns 89 and 124 with the FIRST operand of each PLUS SET_SRC. Post-population it is {v0, v1}; the greg dump shows {v1} only because prune_preferences removes v0 due to 86's v0 conflict.
- mechanism: set_preference (global.c:1591) is called from mark_reg_store on every SET during global_conflicts. When SET_SRC is an expression (GET_RTX_FORMAT[0]=='e'), it walks src = XEXP(src, 0) — the FIRST operand — and sets copy=0. For insn 89 (plus 112 88), src=reg112, reg_renumber[112]=v0 → hard_reg_preferences[86] |= {v0}. For insn 124 (plus 129 130), src=reg129, reg_renumber[129]=v1 → hard_reg_preferences[86] |= {v1}. prune_preferences (global.c:851) then ANDs out hard_reg_conflicts, and 86 conflicts with v0 (from s6/s7 sched1 insn-115 hoist), so v0 is removed. Post-prune: {v1}.
- probe: Traced set_preference logic. Read RTL for insns 89, 124 in tmp/grind/func_80057CC8/s15/func_lreg (lines 427-432, 513-518). Verified reg_renumber via post-alloc greg dispositions: 112 in 2, 88 in 6, 129 in 3, 130 in 4 (tmp/grind/func_80057CC8/s15/func_greg lines 29-40). Read prune_preferences (global.c:851-877) and dump_conflicts ordering (global_alloc calls prune_preferences BEFORE dump_conflicts).
- result: CONFIRMED. Both v0 and v1 propagate; v0 is pruned by the sched1-induced 86-vs-v0 conflict, leaving {v1} in the dump. The pref set is NOT intrinsically {v1}.
- verdict: CONFIRMED

## [s15] s7's downgrade of the 'consumer-of-ang_prev-early / eliminate insn-115 hoist' axis to necessary-but-NOT-sufficient is mechanism-wrong. Eliminating the sched1 hoist of insn 115 (removing the v0 conflict) IS sufficient to steer 86 to v0.
- mechanism: s7 wrote: 'even without the hoist, pseudo 86's pref {3} still steers away from v0.' That assumed pref {3} was intrinsic. Per hypothesis 2, pref {3} is post-prune; the pre-prune set is {v0, v1}. Removing the v0 conflict restores hard_reg_preferences[86] to {v0, v1}. find_reg (global.c:921-1140) selects best_reg by iterating reg_alloc_order (MIPS: v0=2 before v1=3 among caller-saves) and then confirms against the pref set. With v0 in the pref set and not in the conflict set, find_reg picks v0. Target's p1=v0 is reachable IF a legitimate pure-C form suppresses sched1's hoist of insn 115 (and $v0, $v0, 0xFFF finalizing ang_prev).
- probe: Read find_reg (global.c:921-1140) two-pass structure. Confirmed reg_alloc_order (MIPS caller-save order: 2,3,4,5,6,7,8,9,10,11,...). Cross-checked hard_reg_copy_preferences[86]: empty (no reg-reg SET involves 86; both defs are PLUS SETs); expand_preferences never fires for 86.
- result: CONFIRMED (analytical). The 'insn-115 hoist elimination' axis is RE-OPENED as a live frontier item, subject to finding a legitimate pure-C form that defers or suppresses the mask hoist without cheat coercion.
- verdict: CONFIRMED

## [s15] hard_reg_copy_preferences[86] is empty (neither set_preference nor expand_preferences ever populates it for pseudo 86).
- mechanism: set_preference sets hard_reg_copy_preferences only when copy=1, which happens only when SET_SRC is a bare REG (not an expression). Both defs of 86 (insn 89, insn 124) have PLUS SET_SRC. expand_preferences gates on `XEXP(link, 0) == SET_SRC(set)` — again requires a bare REG SET_SRC. Neither fires for 86.
- probe: Read set_preference (line 1601-1602 sets copy=0 for expression sources). Read expand_preferences (line 824 gates on XEXP(link,0) == SET_SRC(set)). Traced 86's def-use: no REG-REG SET where 86 is dest OR source.
- result: CONFIRMED. 86's copy_prefs empty. find_reg falls through to general hard_reg_preferences after skipping the copy-pref restriction (line 1057-1091).
- verdict: CONFIRMED

## [s16] The sched1 hoist of insn 115 (and $v0,$v0,0xFFF) from RTL position (114,117) to post-sched (145,147) is caused by sched.c adjust_priority calling birthing_insn_p on insn 115 during T-4's schedule of call2 (insn 147). birthing_insn_p returns TRUE because pseudo 78 (ang_prev) has reg_n_sets==1 and is live at insn 115; priority is boosted to LAUNCH_PRIORITY = 0x7f000001 (sched.c:2571 via max_priority=0x7f000001 set at sched.c:3985 for call2). At T-5 the ready list = {115 (0x7f000001), 143 (4), 145 (4)} and rank-for-schedule picks 115 by highest priority, placing it in the slot immediately before call2.
- mechanism: sched.c line 3985 sets INSN_PRIORITY(147)=LAUNCH_PRIORITY when call2 is picked at T-4. schedule_insn calls adjust_priority(115) at sched.c:2627. adjust_priority (sched.c:2534) with n_deaths=0 (REG_DEAD notes stripped) reaches case 0 (sched.c:2565); birthing_insn_p returns TRUE for insn 115 because pattern is SET, SET_DEST is pseudo 78 (REG), pseudo 78 is currently live, and reg_n_sets[78]==1 (candidate.c's ang_prev is a single-def local). INSN_PRIORITY(115) := max_priority = 0x7f000001. This inflated priority dominates 143/145's base priority 4 at T-5.
- probe: Fresh cc1 -da dumps under tmp/grind/func_80057CC8/s16/; grep for insn 115 in func_sched shows priority = 2 base but ready-list entry '115 (7f000001)' at T-5. Cross-referenced sched.c (LAUNCH_PRIORITY=0x7f000001 at line 187, LOW_PRIORITY_P macro at 189, priority boost at 3985, adjust_priority at 2534, birthing_insn_p at 2496).
- result: Ready-list dump at T-5 shows exactly {115 (7f000001), 143 (4), 145 (4)}; pick order 115 first. Post-sched linear order places 115 immediately before call2 (147). v0 live range extends across insn 124 (p2 addu redefining pseudo 86). greg conflict list ';; 86 conflicts: 72 74 75 77 84 86 87 2 4 16 17 29' includes hard reg 2 (v0). Confirms and refines s15's re-opening of F4.
- verdict: CONFIRMED

## [s16] Splitting ang_prev's assignment at the C source from `ang_prev = X & 0xFFF;` into two sequential writes `ang_prev = X; ang_prev &= 0xFFF;` will bump reg_n_sets[78] to 2 at sched1 time, defeat birthing_insn_p, keep insn 115 at base priority, lose the T-5 pick to 143/145, fire early near call1, shrink v0's live range, remove 86's v0 conflict, and restore find_reg's pick to v0 by reg_alloc_order.
- mechanism: birthing_insn_p (sched.c:2517) tests reg_n_sets[i] == 1 explicitly. Two source-level SETs to the same variable normally survive as two RTL SETs unless cse1/combine can substitute-and-delete. If both SETs reach flow.c's life analysis, reg_n_sets settles at 2, birthing_insn_p returns FALSE for both, no priority boost occurs, insn 115 stays at base priority 4 tied with 143/145, and the tie-break at T-5 does not favor 115.
- probe: Applied `ang_prev = single_game_getEnemyCharId(...); ang_prev &= 0xFFF;` (two source-level assignments) to src/text1b.c; re-ran sandbox --disable all and re-ran cc1 -da dumps.
- result: Score = 3 (byte-neutral, no gradient); target_insns=build_insns=111 (identical insn count); fresh sched dump shows insn 116 (renamed 115 due to +1 statement) STILL gets INSN_PRIORITY 0x7f000001 at the T-5 ready list. cse1/combine folded the two SETs into one before flow.c's life analysis, restoring reg_n_sets[78]==1. The intermediate ang_prev value has no intervening use, so combine substitutes freely. Route A of the mechanism-derived lever family is measured DEAD.
- verdict: KILLED

## [s16] F4 (from s15) — 'eliminate the sched1 hoist of insn 115 via a pure-C form that defers or suppresses the mask' — is closable by a legitimate pure-C axis this session.
- mechanism: F4 mechanism (s15): the v0 conflict on pseudo 86 originates in sched1's hoist of insn 115; if defeated, hard_reg_preferences[86]={v0,v1} pre-prune survives to find_reg, which picks v0 by reg_alloc_order (v0 = 2 precedes v1 = 3 in MIPS caller-save order). Would produce target's p1=v0.
- probe: s16 forensics named the mechanism (birthing_insn_p + LAUNCH_PRIORITY boost). Enumerated the three theoretical routes to defeat: (A) multi-set ang_prev with combine-surviving reg_n_sets>1 — measured DEAD (this session, split-mask form folded by combine); (B) inline ang_prev at use — anonymous temp still has reg_n_sets==1; (C) make pseudo 78 dead before insn 124 — impossible because insn 124's schedule position is fixed by p2's own dep chain, and 115's dep to call1 forces it to fall after call1.
- result: All three mechanism-consistent routes are structurally blocked at the semantic level. Route A requires a dead intervening use of ang_prev with no semantic purpose — same class as the 2026-06-22 rejected split-init-off-accumulation form ([[no-new-park-categories]] cheat-by-any-spelling). Route B and C are impossible by construction. F4 is downgraded from re-opened to KILLED-by-construction unless a novel axis emerges from Kengo transplant (F3).
- verdict: KILLED

## [s17] F3 Kengo transplant surfaces a structurally-equivalent 2-neighbour arena-angle-interpolation function whose C shape reveals a novel lever.
- mechanism: Kengo (PS2, Marionation engine reuse) shares provenance with BB2; if any Kengo function does the same mid-angle-with-0x800-wrap-flip + sin/cos-table scale, its C shape (as decomp source) transplants directly to func_80057CC8.
- probe: Inspected the Kengo corpus stored under Kengo/. Directory contains only the PS2 disc extraction (Kengo - Master of Bushido (USA).bin/.cue plus Kengo/disc/{DATA,IRX,...}) and symbol-table dumps (kengo_functions.txt, kengo_globals.txt, kengo_debug_full.txt, kengo_func_names_sorted.txt). find Kengo -name '*.c' -or -name '*.h' returns ZERO source files. Grepped kengo_func_names_sorted.txt for arena|angle|Bound|Neighb|jiai|dohyou|ring|circle|edge terms: hits are unrelated (coli_check_point_inside_triangle, coli_check_circle_hit_line, camera_check_line_cross_3d, rob_angle_target, rob_angle_camera — none are 2-neighbour arena-boundary angle-interpolation).
- result: Kengo corpus is a PS2 executable + symbol dump ONLY. No C source exists to transplant from. Even if a name-similar function surfaces (rob_angle_target etc.), no source-level shape is recoverable without decompiling Kengo itself (out of scope for this pipeline).
- verdict: KILLED

## [s17] F6 SOTN corpus contains a canonical `X = call(...); X &= K;` split-mask idiom that survives combine into sched1's reg_n_sets test, giving a byte-neutral 2-def form on ang_prev that defeats the birthing_insn_p priority-boost path named in s16.
- mechanism: s16 forensics named sched.c adjust_priority + birthing_insn_p as the mechanism boosting insn 115 (and $v0, $v0, 0xFFF) to LAUNCH_PRIORITY. Route A (multi-set ang_prev) was measured DEAD (combine folded the split back before flow.c) — but a domain-specific SOTN idiom might structurally survive combine by an untested pattern (e.g. an intervening RTL barrier from a specific expression shape).
- probe: Inspected the SOTN corpus available in this repo at tmp/sotn-decomp/src. The tree contains ONLY the psxsdk subset (~175 files across main/psxsdk/lib{api,c,card,cd,etc,gpu,gs,gte,snd,spu}). No dra/menu/weapon game-domain source is present. Grepped for `X &= 0xFFF;` and `X = Y & 0xFFF;` line patterns: 3 total hits across 3 files, all in psxsdk (libspu/s_gva.c, libgte/geo_01.c, libsnd/sstick.c) — none are 12-bit-angle domain code that would transplant to func_80057CC8. The SOTN pattern the ledger hypothesised (game-side `X = call(); X &= 0xFFF;` masked-angle idiom that survives combine) is not evidenced in the accessible corpus.
- result: F6's premise (a domain-relevant SOTN idiom exists that survives combine differently than our tested split-mask form) is unsupported by the accessible corpus. Combined with s16's measured Route A kill, the birthing_insn_p defeat axis remains structurally blocked.
- verdict: KILLED

## [s17] Deferring the `& 0xFFF` masks from the ang_prev/ang_next assignment sites to a post-both-calls position places the mask insns outside the call2 delay slot, giving GCC's dbr.c a different scheduling target and potentially altering pseudo 78 (ang_prev) live-range at insn 124 (p2 addu redef of pseudo 86).
- mechanism: Novel rederive-modality angle not previously measured: candidate.c applies masks IMMEDIATELY at each call return (`ang_prev = call1(...) & 0xFFF;`). Target places `andi $s0, $v0, 0xFFF` in call2's delay slot at 80057DA0 — so the mask fires post-p2-setup in target. Deferring the mask in C source to `ang_next &= 0xFFF; ang_prev &= 0xFFF;` after both calls tests whether GCC's scheduler still lands the ang_prev mask in call2 delay slot, or whether the deferred form regresses.
- probe: Applied the deferred-mask form: `ang_prev = call1(...); ang_next = call2(...); ang_next &= 0xFFF; ang_prev &= 0xFFF;` (masks moved from expression tails to standalone statements after both calls). Ran `& tools/wteng.ps1 main sandbox func_80057CC8 --disable all`.
- result: score=40, target_insns=111, build_insns=114 (+3 insns). GCC does NOT hoist the deferred masks back into call2's delay slot or the call2 return; it emits them as separate post-call2 andi insns, cascading through the ternary and scale-multiply blocks. Deferred-mask position destroys the target scheduling window rather than matching it. Saved memory/grind/func_80057CC8/rejected/rederive-deferred-mask-at-uses.c.
- verdict: KILLED

## [s18] Fresh m2c decompile of asm/funcs/func_80057CC8.s reveals a novel C shape unlike candidate.c whose transplant closes or reduces the p1 addu coalescing gap.
- mechanism: m2c reconstructs the target's structural intent from RTL/asm patterns. If m2c's output uses a shape our candidate.c doesn't (e.g. combined-shift, cur-pointer, split-p precomputes), applying that shape to src/text1b.c may reveal a lever the hand-derivation missed.
- probe: Ran wsl python3 tools/m2c/m2c.py --target mips-gcc-c --valid-syntax asm/funcs/func_80057CC8.s. Saved output to tmp/grind/func_80057CC8/s18/m2c_output.c. Examined shape differences vs candidate.c.
- result: m2c produces (a) combined-shift `((s32)(x<<0x10)>>0xE)` for p1/p2 offset (vs candidate's `((<<16)>>16)<<2`), (b) `temp_v0 = arg1*4 + temp_a2; temp_s4 = *(u16*)temp_v0` cur-pointer for cx/cy reads, and (c) TWO separate C locals (`temp_v0_3`, `temp_v1_2`) for the p1 and p2 addresses precomputed before either call. Shape (c) is the exact two-local split forbidden by Judge s10 binding. Shape (a) and (b) are novel structural axes worth measuring in this session (rederive modality).
- verdict: CONFIRMED

## [s18] Rewriting candidate.c's p-address shift chain from `((s32)(x<<16)>>16)<<2` to m2c's combined form `(s32)(x<<16)>>14` (algebraically identical, syntactically distinct) shifts combine.c's substitution surface and moves the p1 addu coalescing decision.
- mechanism: s2 tested two shift spellings (`<<16>>16<<2` and `(s32)(s16)x<<2`); both fold identically. The combined `<<16>>14` form is a THIRD variant not previously measured — expresses the sign-extend and *4 scale in one arithmetic-shift-right rather than a shift-right-then-shift-left pair. Combine.c pattern-matches on RTL operator tree shape.
- probe: Applied p1 = (s16*)(((s32)(prev_idx<<16)>>14) + (s32)table) and p2 = (s16*)(((s32)(next_idx<<16)>>14) + (s32)(*(s16**)(arg0+4))) to src/text1b.c. Ran `& tools/wteng.ps1 main sandbox func_80057CC8 --disable all`.
- result: score=3 (byte-neutral, no gradient); target=build=111 insns; rules_dropped=7; cheat_asm_stripped=395. GCC 2.7.2 combine.c folds all three shift-chain spellings to the same RTL. Extends s2's spelling-fold finding (two variants -> three variants). Saved rejected/rederive-combined-shift-16-14.c.
- verdict: KILLED

## [s18] Introducing a `u16 *cur` pointer local for cx/cy reads (m2c's shape) unifies the current-node address into a fresh pseudo whose lifetime shape may perturb the p1 addu allocation window and close the gap.
- mechanism: m2c's `temp_v0 = arg1*4 + temp_a2` shape treats the current node's address as a stored intermediate rather than inlining `((s32)table + arg1*4 + K)` at each read. A `u16 *cur = (u16*)((s32)table + arg1*4); cx = cur[0]; cy = cur[1];` block introduces a fresh RTL pseudo (`cur`) whose live range covers only the two subsequent reads. Semantically legitimate (a genuine 'current node's address' intermediate — not an alias for `p`, not a lifetime-only holder). If it shifts pseudo 86's schedule, could close the gap.
- probe: Applied `{ u16 *cur = (u16 *)((s32)table + arg1 * 4); cx = cur[0]; cy = cur[1]; }` block replacing the two inline cx/cy loads in src/text1b.c. Ran sandbox --disable all.
- result: score=4 (regression +1 from candidate baseline 3); target=build=111 insns. The `cur` pointer introduces a fresh pseudo whose live range interferes with p1/p2's addu scheduling window; codegen regresses rather than closing. NOT a cheat-by-spelling (`cur` has genuine semantic purpose — one node's address vs `p` which is 2 different neighbours' addresses) — measurably a worse form. Saved rejected/rederive-cur-pointer-cx-cy.c.
- verdict: KILLED

## [s18] The decomp.me corpus (3754 gcc2.7.2-cdk/psx scratches cached in tmp/decomp_me_corpus) contains at least one function structurally similar to func_80057CC8 (2-neighbour arena-boundary angle interpolation with 12-bit angle mask + sin/cos scale) whose C shape transplants directly.
- mechanism: s17 F3 (Kengo transplant) killed because Kengo has no source. F6 (SOTN) killed because accessible SOTN slice is psxsdk-only. decomp.me is a THIRD corpus with 3754 cached scratches under the BB2-compatible compilers; a structurally-similar match could produce a transplantable C shape.
- probe: Grepped tmp/decomp_me_corpus/*.json for `& 0xFFF` (12-bit angle mask signature): 5 hits. Inspected the first 3 hits' source code (`func_801E2820`, `func_8006A370`, `func_80149D90`). None are 2-neighbour arena-angle-interpolation; all use the mask for unrelated bit-extract purposes.
- result: No structurally-similar function in the cached decomp.me corpus. F3-equivalent corpus route (decomp.me instead of Kengo/SOTN) is measured dead. All THREE corpus-transplant routes now KILLED (Kengo unfindable, SOTN accessible slice psxsdk-only, decomp.me cache has no structural match). Confirms the rederive modality's corpus-transplant sub-axis is exhausted.
- verdict: KILLED

## [s19] The s1-s18 ledger merged names an impossibility surface: single-C-variable `p` cannot reach target's p1=v0 (s6/s7/s15/s16 chain: sched1 hoist -> v0 conflict; set_preference walks FIRST PLUS operand -> {v0,v1} pre-prune, {v1} post-prune; hard_reg_copy_preferences[86] empty). All two-C-local spellings closed by 25-form rejected bank + Judge 2026-07-19 17:09. All rederive corpora (Kengo/SOTN/decomp.me) unfindable or no structural match. Exactly ONE un-measured sanctioned cell remains (F5 corner: double-swap p1 AND p2 to table+offset).
- mechanism: Merge of s1-s18 evidence: (i) pseudo 86 = /v-marked DECL_RTL(p) with two SETs in BB4; local-alloc skips 86 for multi-def-in-BB, promotes to global-alloc pool. (ii) sched1's insn-115 hoist (adjust_priority + birthing_insn_p + LAUNCH_PRIORITY, reg_n_sets[78]==1) injects v0 into 86's conflicts. (iii) set_preference walks FIRST PLUS operand at each SET, populating hard_reg_preferences[86] pre-prune to {v0,v1}; prune_preferences drops v0 by conflict, leaves {v1}. (iv) hard_reg_copy_preferences[86] empty because both SETs have expression SET_SRC. (v) find_reg picks v1. Any single-pseudo binding is locked to v1.
- probe: Analytical synthesis only (session modality=synthesis, matches s10 precedent). Re-read evidence.md + hypotheses.md (350 lines) + 27 rejected forms. Cross-checked Judge 2026-07-19 17:09 binding against the 25-form rejected bank + 3 additional s17/s18 rederive rejects. Verified docs/grind/decisions.md contains no OWNER-ESCALATION entry for func_80057CC8 (only the 17:09 Judge FAIL).
- result: Frontier consolidated to (1) F5 corner double-swap (structural, s20), (2) local-alloc block_alloc bail-out forensics (s21), (3) OWNER-ESCALATION filing contingent on (1) KILLED + (2) either KILLED-by-construction or the identified C form measured dead (s22+). Synthesis document at tmp/grind/func_80057CC8/s19/synthesis.md.
- verdict: CONFIRMED

## [s20] F5 corner: writing BOTH p1 = (table + offset) AND p2 = (table_expr + offset) simultaneously either dominates via p2-swap (=> 9) or exposes novel addsi3 reassociation behavior at a distinct value.
- mechanism: s8 established p1-swap byte-neutral (3) and p2-swap regresses (3->9) via the p2 addu's PLUS operand order steering combine.c's late reassoc reach at insn 124. The double-swap corner tests whether p1's byte-neutrality persists when combined with p2's regression, OR whether the combination unlocks a new codegen shape.
- probe: Verified HEAD src is exactly the F5-corner shape (both p-adds spelled table_expr+offset, git status clean, s3 pin present but score-invisible under --disable all mask). Ran `& tools/wteng.ps1 main sandbox func_80057CC8 --disable all` — score=9, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=397.
- result: F5 corner measures 9 — matches p2-swap-alone value; no novel reassoc behavior. p1 operand order remains scoring-inert even under the p2-swap regression, confirming p2's PLUS operand order fully determines late-reassoc reach.
- verdict: KILLED

## [s21] local-alloc's block_alloc bail-out on pseudo 86 is a scoring-heuristic (C-tweakable) rather than a semantic requirement (untweakable), so a C form presenting 86 as single-def-use per fragment could get 86 into local-alloc.
- mechanism: local-alloc.c:470-478 checks reg_basic_block[i]>=0 && reg_n_deaths[i]==1 to set reg_qty[i]=-2 (local-alloc-eligible); else reg_qty[i]=-1 (skip -> global-alloc). Pseudo 86 has reg_n_deaths==2 per s7 lreg dump ('Register 86 used 6 times across 10 insns in block 4; dies in 2 places').
- probe: Read tools/gcc-2.7.2/local-alloc.c:470-478 and 1978-2029 (reg_is_set / reg_is_born). Confirmed the check is a single hard gate on reg_n_deaths==1 with NO fall-through; every pseudo with two or more deaths is unconditionally promoted. Two-SET-of-p C forms produce two distinct RTL live-ranges (last-use death of first SET + last-use death of second SET) => reg_n_deaths>=2 => triggered without exception.
- result: The bail-out is unconditional-semantic. Any C form that assigns p twice (which target-asm requires by having two DIFFERENT hard regs for p1 and p2) produces reg_n_deaths>=2 and gets skipped by local-alloc. Only two-C-local forms produce two distinct pseudos (Judge-banned). Single-C-local forms are separately killed by s6/s7 (expand_preferences + sched1-hoist).
- verdict: KILLED

## [s21] A self-referential SET (`p = p + (off_next - off_prev)`) could reduce pseudo 86's reg_n_deaths to 1 by making the second SET reference the first as source operand, keeping p's live-range contiguous.
- mechanism: If GCC's flow.c treats a self-referential SET (SET_DEST==reg appearing in SET_SRC) as a modification rather than a full rebirth, no REG_DEAD note fires at the source-side use of p, and reg_n_deaths could stay at 1. combine.c should also fold the (next-prev) delta cancellation with off_prev in p's value.
- probe: Applied self-referential form `p = (s16*)((s32)p + (((s32)(next_idx<<16)>>16)<<2 - ((s32)(prev_idx<<16)>>16)<<2))` to src/text1b.c line 11871; sandbox --disable all measured.
- result: sandbox=64; target_insns=111 build_insns=115 (+4 regression). GCC 2.7.2 does NOT fold the delta cancellation; combine.c treats the two shift-and-subtract chain as independent of the original off_prev inside p. Additionally, the self-ref adds a live-through requirement for prev_idx across call1, further disturbing scheduling. Byte-neutral single-death shape is not reachable; flow.c places REG_DEAD on p's use in the subtraction expression normally.
- verdict: KILLED

## [s22] A permuter chassis mutating the downstream ang_mid ternary structure (3 branch-sense alternatives) combined with the *arg2/*arg3 write-order (3 sequencing alternatives) under PERM_RANDOMIZE overlay surfaces a legitimate score-0 lever the prior five chassis (s4/s5-1/s5-2/s13/s14) missed. This axis has never been permuted; if any non-alias close exists in the search space it should appear here.
- mechanism: Prior chassis targeted upstream shape (prev_idx if-reload, p1/p2 addu operand order, p2 table-source) and one downstream axis (ang_mid ternary alone at s13). This chassis pairs the ternary with a *arg2/*arg3 write-order that has never been permuted, potentially perturbing sched1's fill of the call2 delay slot with insn 115 (and $v0,$v0,0xFFF finalizing ang_prev). If the write-order shift changes the ready-list neighbourhood at T-4/T-5 of call2, insn 115's LAUNCH_PRIORITY hoist might not win the T-5 slot, defeating the s6/s16-confirmed mechanism at its origin without any alias-holder.
- probe: Cloned s14/perm to s22/perm; replaced s14 PERM_GENERALs (already-killed shift-spelling and p2 table-source) with a 3-alt PERM_GENERAL on the ang_mid ternary (original / prev<=next-swap-with-else-first / prev<next-single-arm-rewrite) and a 3-alt PERM_GENERAL on the *arg2/*arg3 block (original / new_var-hoisted-out / *arg3-before-*arg2). Launched tools/permuter_campaign.py launch --func func_80057CC8 -j 6 --stop-on-zero. Waited in-turn ~13min (796.8s wall).
- result: iterations=4265, elapsed_s=796.8, finds_total=1, finds_new=1, best_new_score=0, procs_killed=0, pid_alive_at_harvest=false (self-exited on --stop-on-zero). base_score=15 permuter-scored (=sandbox floor 3). Score-0 close (output-0-1 at iter 4265, 317.1s post-launch) inspected: form is `s16 *new_var2; ... new_var2 = (s16*)((((s32)(prev_idx<<16)>>16)<<2) + (s32)table); p = new_var2;` — the PERM_RANDOMIZE overlay injected the p1-alias-holder local as a mutation OUTSIDE my defined PERM_GENERAL alternatives, and the ternary/write-order alternatives in the closing form were the original variants. Same cheat class as rejected/block-scope-alias-p1.c (2026-06-16 layer-1 FAIL, 2026-06-22 layer-2 FAIL) and rejected/permuter-{long_new_var2,s16-new_var2-p1,s16-new_var2-p2,s14-p1-shift-cast}-alias.c (s4/s5/s13/s14). Saved as memory/grind/func_80057CC8/rejected/permuter-s22-downstream-ternary-p1-alias.c.
- verdict: KILLED

## [s22] Cumulative permuter modality across s4/s5-1/s5-2/s13/s14/s22 exhausts the score-0 basin space accessible to the permuter from the candidate-floor-3 baseline; every closing form found by any chassis is a pointer-alias-holder in the cheat-by-any-spelling class.
- mechanism: SIX structurally distinct chassis — undirected random (s4), directed PERM_GENERAL enumeration over prev_idx-if/p1/p2 (s5-1), directed+random overlay of same (s5-2), downstream ternary alone (s13), shift-spelling + p2 table-source (s14), downstream-ternary + *arg2/*arg3 write-order (s22) — converge on identical closing form family: introduce a local (s16*, long, block-scoped, function-scoped, or via ternary direct-bind) that holds the p1 or p2 address expression, then bind `p = holder`. The permuter's mutation search space is dominated by this basin because it is the ONLY change that satisfies expand_preferences propagation into pseudo 86's copy-pref set without requiring a source-level split — but per Judge s10 binding + [[no-new-park-categories]], every spelling of this construct is closed.
- probe: Six chassis run cumulatively; each documented in ledger (s4-s22 with permuter outcome JSON entries). No permuter axis remains that could surface a legitimate lever without first requiring a rederive-modality structural change of C which would itself have to clear the Judge constraint independently.
- result: Permuter modality exhausted across six chassis. Combined kill-count for func_80057CC8 (s1-s22): 29 KILLED hypotheses spanning statement order, type width, declaration order, block scoping, named-intermediate, six permuter chassis, consumer-of-ang_prev-early, inline-both-call-sites, p1/p2 operand swap alone and combined (F5 corner), walking-pointer rederive, 2-iter for-loop rederive, F1 duplicated-arms A/B/C, F2 struct-typed arg0, split-mask ang_prev (Route A), deferred-mask, F3 Kengo (unfindable), F6 SOTN (accessible slice psxsdk-only), decomp.me corpus (no structural match), combined-shift <<16>>14, cur-pointer-cx-cy, self-referential-SET.
- verdict: KILLED

## [s23] Narrowing the existing single `p` s16* local from function scope to block scope (single DECL_RTL preserved, no alias holder, no rename, no split) changes pseudo 86's copy-preference chain enough to close the floor-3 gap.
- mechanism: Judge axis (1) asks for elimination of the bound `p` local so RTL-expand emits anonymous pseudos at call-arg sites. Block-scope is a strictly weaker variant of that axis — same variable, tighter lexical scope — that had NOT been measured across s1-s22. Hypothesized: DECL_RTL classification or allocno birth-point may be scope-sensitive within a single function; tighter scope could hypothetically shorten pseudo 86's live range enough to break the v1-copy-pref inheritance path from pseudo 129.
- probe: Applied candidate.c to src/text1b.c (verified baseline sandbox --disable all = 3). Moved `s16 *p;` decl from the function's top declaration block into an inner `{ ... }` wrapping the two p= assignments + both single_game_getEnemyCharId call sites. Re-ran sandbox --disable all.
- result: score=3 (byte-neutral with candidate function-scope form; delta=0, no gradient in either direction). Scope narrowing is a NULL lever on this RA wall.
- verdict: KILLED

## [s24] expand_preferences source-operand direction (which XEXP(src,0) at insn 89 / insn 124 propagates into 86's hard_reg_preferences) is a live C-level lever for shifting 86's post-prune pref set away from {v1}
- mechanism: set_preference (global.c:1591) only walks src = XEXP(src, 0) of a PLUS/binop when src is not a REG. Insn 89 = SET pseudo86 (PLUS pseudo112 pseudo88); walk picks pseudo112 (in v0 by local-alloc). Insn 124 = SET pseudo86 (PLUS pseudo129 pseudo130); walk picks pseudo129 (in v1 by local-alloc). Together set pre-prune hard_reg_preferences[86] = {v0, v1}; prune drops v0 (conflict from sched1's insn-115 hoist); post-prune = {v1}. Flipping the C-level `+` operand order at either insn would theoretically shift the walked operand.
- probe: Instrumented cc1 (tmp/gccdbg/cc1) with BB2_ALLOC_DEBUG + BB2_QTY_DEBUG + BB2_PRIO_DEBUG + BB2_RANK_DEBUG + BB2_SCHED_DEBUG on candidate.c preprocessed src/text1b.c. Located func_80057CC8's ALLOCDBG block via the 16-allocno signature matching s7 greg (lines 37630-37645 of tmp/grind/func_80057CC8/s24/cc1.err). Cross-check with s8/s20 C-level measurement of the two operand-swap alternatives.
- result: Direct measurement: block 4 local-alloc places pseudo112 in v0 (line 37607: got=2, qty=0, ord=0) and pseudo129 in v1 (line 37608: got=3, qty=5, ord=1). Global-alloc places pseudo79 at ord=0 hardreg=3 (pri=13333) and pseudo86 at ord=1 hardreg=3 (pri=12000). Pseudo 86 is ABSENT from QTYDBG (local-alloc skipped, confirming s21's reg_n_deaths==2 bail-out by direct-instrumentation absence). s8 measured p1 operand swap alone = 3 (byte-neutral: combine canonicalizes); p2 operand swap alone = 9 (regression); s20 measured F5-corner double-swap = 9 (regression). The expand_preferences walk direction is fixed by local-alloc's upstream placement of 112/129, which the C-level `+` swap does not alter.
- verdict: KILLED

## [s25] Pseudo 79 (s24 named ord=0 pri=13333 v1-sharing predecessor of pseudo 86) is DECL_RTL of C source variable ang_next, /v-marked, single-def at insn 150 = (set (reg/v:SI 79) (and:SI (reg:SI 2 v0) 0xFFF))
- mechanism: grep pseudo 79 in tmp/grind/func_80057CC8/s24/text1b.i.lreg (candidate baseline lreg dump): insn 150 at line 21743 sets (reg/v:SI 79) from v0-return-of-call2 masked with 0xFFF. /v marker = user variable = DECL_RTL binding. greg conflict list `;; 79 conflicts: 72 74 75 78 79 84 87 2 29` (line 18124) includes hard reg 2 (v0). No `;; 79 preferences:` line — empty positive prefs. Allocated FIRST in greg order (ord=0), lands in $v1 via reg_alloc_order fallthrough because v0 is conflict-blocked and v1 is next non-conflicting caller-save. Target asm/funcs/func_80057CC8.s:59 `andi $v1, $v0, 0xFFF` — target ALSO allocates ang_next to $v1. Identical allocation.
- probe: grep 'reg.*79' text1b.i.lreg + read insn 150 + cross-check greg block 18113-18153 + target asm scan
- result: pseudo 79 identity CONFIRMED as ang_next; s24's descriptive placeholder 'predecessor sharing v1 by non-conflicting live range' is now named; allocation identical to target
- verdict: CONFIRMED

## [s25] Target has an extra early $v0-establishing addu (80057D0C) that our build lacks, and that early addu is why target's p1 addu at 80057D54 lands in $v0 while ours lands in $v1
- mechanism: Objdump of tmp/sandbox/func_80057CC8/text1b.o at candidate baseline shows b750 `addu $v0, $v0, $a2` — the cx/cy base addu, DEST=$v0. This is the exact analog of target's 80057D0C. Both target and ours have the same early cx/cy `addu $v0,$v0,$a2` at the same layout position, and both coalesce dest=$v0 (source $v0 reused in-place) because the cx/cy base pseudo is single-def, single-use (feeds two lhu loads at +0 and +2). Local-alloc handles that pseudo and coalesces $v0->$v0. Our build DOES have the 'extra' early addu.
- probe: wsl mipsel-linux-gnu-objdump -d --disassemble=func_80057CC8 tmp/sandbox/func_80057CC8/text1b.o + head -50; compared byte-by-byte with asm/funcs/func_80057CC8.s lines 18-22 and 38-45
- result: The early cx/cy base addu is IDENTICAL in target and ours ($v0 dest, in-place coalescing). It is not a distinguishing lever. Only the p1 pointer addu (b798/80057D54) actually diverges: target `addu $v0, $v0, $a2` vs ours `addu $v1, $v0, $a2`. p2 addu (b7cc/80057D88) matches; ang_prev delay-slot mask (b7e4/80057DA0) matches; ang_next mask (b7e8/80057DA4) matches. The 3-insn gap is entirely at pseudo 86's p1-addu allocation as s7/s21/s24 already established.
- verdict: KILLED

## [s26] A fresh rederive angle — reusing the same `p` local for the cx/cy base pointer AND the prev/next neighbor pointers (three SETs of pseudo 86 in BB4) — will redirect pseudo 86's copy-preference chain by giving set_preference three FIRST-PLUS-operand sources instead of two, opening a legitimate path to target's p1=v0 allocation.
- mechanism: Target asm at 80057D0C emits `addu $v0, $v0, $a2` for the cx/cy base with $v0 dest AND at 80057D54 for the p1 addu with $v0 dest — both same hard reg. Hypothesis: target's C reuses `p` for cx/cy AND neighbor reads. In our RTL a third SET of pseudo 86 at BB4 top would extend set_preference's walk to include the FIRST operand of the cx/cy addu (in target's placement this is v0 by natural allocation from arg1*4 + table), potentially expanding pre-prune hard_reg_preferences[86] and shifting global-alloc away from v1.
- probe: Rewrote src/text1b.c line 11837 candidate baseline: `p = (s16*)((s32)table + arg1*4); cx = *(u16*)p; cy = *(u16*)((s32)p + 2);` at BB4 top, then reused `p =` for both prev and next neighbor addresses (three SETs total). Ran `& tools/wteng.ps1 main sandbox func_80057CC8 --disable all`.
- result: sandbox score=3 -> 12, target_insns=111, build_insns=112 (+1 insn regression). Three-SET-of-p form is measurably worse than the two-SET candidate. The added cx/cy binding of p does NOT redirect pseudo 86's copy-pref origin (still {v1} from pseudo 129 via insn 124's addsi3 per s6/s7/s15/s16/s24 forensics) AND adds an insn by breaking the natural cx/cy-from-table addressing path. Saved memory/grind/func_80057CC8/rejected/rederive-reused-p-walking-cx-cy.c. Adjacent to but structurally distinct from s18 cur-pointer-cx-cy (which was fresh separate local, +1 to score 4) — this reused-p is a genuine rederive angle not measured across s1-s25. Semantically legitimate reuse (no cheat-class concerns), just codegen-wise dead.
- verdict: KILLED

## [s26] The s18 decomp.me corpus sweep was too narrow (3 of 5 `& 0xFFF` hits inspected); broadening the search to intersect the shift-and-scale signature `<<16>>16<<2` OR `>>0xE` with `0xFFF` mask exposes at least one structurally-matching 2-neighbor arena-angle-interpolation function whose C shape transplants directly.
- mechanism: s18 F3-equivalent kill on decomp.me was based on 3 of 5 `& 0xFFF` hits; the corpus has 3754 gcc2.7.2-cdk/psx/psyq3.5 scratches. A composite grep pattern intersecting the p-address shift-chain signature (`<<16>>16<<2` or the m2c-preferred `>>0xE`) with the 12-bit angle mask should surface any function that both indexes an s16[]*4 table AND masks the result to 12-bit angle range — the distinguishing structural signature of func_80057CC8.
- probe: Ran `grep -lE "<< *16.*>> *16.*<< *2" tmp/decomp_me_corpus/*.json | xargs grep -l "0xFFF|0xfff|4095"` yielding 11 candidates. Extracted name + description + relevant source lines via tmp/grind/func_80057CC8/s26/inspect.py. Manually classified each function's semantic domain.
- result: 11 candidates inspected: func_8002FF34 (font glyph descriptor extraction), FrogModeMovementCentring / FrogStartPolyPiecePop / CreateFrog / UpdateFrogMatrix / FrogCollectGoldFrog (Frog Frenzy character movement matrix work — `<<16` for fixed-point coord scaling, unrelated masks), func_800260D8 / func_80026124 (unrelated `0xFFFE0000` DMA constants), ChangeClearRCnt (root-counter clear), func_ptr_80170014 / main / func_80149D90 (unrelated). ZERO functions are 2-neighbor arena-boundary angle interpolation. All 12-bit masks are unrelated bit-extract purposes. Confirms and extends s18's F3-equivalent decomp.me kill from a 3-of-5 sample to an 11-of-11 exhaustive composite-signature sample. The corpus does NOT contain a structural match for func_80057CC8.
- verdict: KILLED

## [s27] Swapping only the two call blocks (ang_next computed first with p1-slot, ang_prev computed second with p2-slot) while keeping prev_idx/next_idx compute order unchanged surfaces a legitimate score-0 close because RA's copy-preference propagation into pseudo 86 at insn 124 might draw from the OTHER neighbor's operand set.
- mechanism: Per s6/s7/s15/s16/s24 forensics, hard_reg_preferences[86] pre-prune = {v0, v1}: v0 propagates via set_preference from insn 89's first PLUS operand (pseudo 112), v1 propagates via insn 124's first PLUS operand (pseudo 129). v0 is pruned by conflict from sched1's insn-115 hoist. If the semantic roles of the two SETs of pseudo 86 are swapped (next-first, prev-second), the pseudo bound to insn 124 changes identity — pseudo-129-equivalent now comes from prev_idx's shifted offset rather than next_idx's, altering which operand's live range interferes at insn 124 and potentially avoiding the v0 conflict origin at insn 115. Semantically identical (both calls are independent leaf angle computations); a distinct axis from s2's swap-if-block-order kill (which reordered index compute blocks) and from s3's hoist-p1-before-nextidx-block (which moved the p1 add across the next_idx if-boundary).
- probe: Applied candidate.c baseline to src/text1b.c (offset+table reassoc, no s3 pin; verified sandbox --disable all = 3). Swapped ONLY the two call blocks: p = (s16*)((<<16>>16<<2 next_idx) + table); ang_next = call(...) & 0xFFF; p = (s16*)((<<16>>16<<2 prev_idx) + *(arg0+4)); ang_prev = call(...) & 0xFFF; kept prev_idx/next_idx compute in original order (prev_idx if-reload first, next_idx block second). Ran `& tools/wteng.ps1 main sandbox func_80057CC8 --disable all`. Snapshot at tmp/grind/func_80057CC8/s27/call-order-swap.c.snapshot; rejected form saved to memory/grind/func_80057CC8/rejected/rederive-call-order-swap-next-first.c.
- result: score=16 (regression from baseline 3, +13 delta), target_insns=111, build_insns=112 (+1 insn). Swapping the call order breaks scheduling topology: the +1 insn regression signals loss of the target's delay-slot fill entirely (target packs prev-side sll into the next_idx bnez delay slot at 80057D48 per s3 finding; that pattern requires prev-side compute to precede next-side compute). Even without the delay-slot loss, the base swap adds a partial-reassoc gap analogous to s8's p2-swap-alone regression (3->9). The role-swapped identity of pseudo 86's two SETs did not open a v0-friendly allocation path.
- verdict: KILLED

## [s28] The s1-s27 ledger merged discharges every un-run sanctioned axis identified by the 2026-07-19 17:09 Judge FAIL — inline-expression form and p2 PLUS operand-order rearrangement are both empirically closed with measurements (s8 inline-both-calls=0 but cheat-class rejected in-session; s8/s20 operand-swap alone regresses 3->9 and F5-corner double-swap=9).
- mechanism: Judge FAIL left two frontier axes explicitly open. s8 measured axis (1) closes bytes with a form character-for-character identical to rejected/duplicate-address-expr-pseudo-inline.c per the [[no-new-park-categories]] 6-test checklist (no semantic purpose; CSE-collapsed emit; GCC-internals-only justification). s8/s20 measured axis (2) as byte-neutral on p1-swap alone and a regression on p2-swap alone (the addsi3 SRC1/SRC2 slot layout is fixed, source-level operand-order does not toggle it). No sanctioned axis remains un-measured.
- probe: Analytical synthesis (session modality=synthesis; matches s10/s19 precedent — no sandbox measurement). Re-read evidence.md 510 lines + hypotheses.md 428 lines + 32 rejected forms; cross-checked 17:09 Judge FAIL binding against the rejected bank; verified no sanctioned axis remains un-measured within the frozen SOTN-accepted list.
- result: Frontier consolidated to OWNER-ESCALATION filing per s10/s17/s19/s22/s23/s24/s25/s26/s27 ledger prescription. Escalation appended at docs/grind/decisions.md line 895 with two mutually exclusive options: (a) sanction inline-both-call-sites form as per-function narrow carve-out under [[duplicated-statement-into-arms]] adapted for compound-address duplication across independent call arg-lists; (b) accept INCOMPLETE-owner-accepted with candidate.c standing at sandbox --disable all = 3.
- verdict: CONFIRMED

## [s28] The pseudo-86 v1-binding is theorem-locked at the GCC-pass-source level: sched1 insn-115 hoist + set_preference FIRST-PLUS-operand walk at insn 124 + prune_preferences by conflict + empty hard_reg_copy_preferences + local-alloc.c:472 unconditional-semantic bail-out on reg_n_deaths>=2 together determine v1 for any single-C-local binding of p, and no C-source lever within the sanctioned set alters any element of the chain.
- mechanism: s6/s7/s15/s16/s21/s24 forensics chain (instrumented cc1 -da dumps + tools/gcc-2.7.2/{global,sched,local-alloc}.c source reads). Each step measured or source-read at the GCC-pass level; combined they enumerate every C-reachable perturbation surface within the sanctioned set. s26/s27 rederive-axis kills (reused-p walking + call-order-swap) close the last two novel rederive angles not covered by s8/s9/s17/s18.
- probe: s28 analytical synthesis only. No fresh sandbox runs; the theorem-lock is established by prior forensics + local-alloc source read + s21 direct-source confirmation of the reg_n_deaths==2 hard gate.
- result: Every C-source lever within the sanctioned set is either (i) byte-neutral (does not touch any element of the chain), (ii) a regression (breaks target scheduling topology), or (iii) a cheat-by-any-spelling (introduces lifetime-shaping alias holder). No fourth possibility exists inside the frozen SOTN-accepted list without owner ruling.
- verdict: CONFIRMED

## [s29] The 28-session lock is not "single-C-variable `p` cannot reach v0" but "a MULTI-SET `p` cannot" — if the prev-neighbour call site indexes the vertex table directly (`table[pi*2]`, `table[pi*2+1]`) so that the surviving pointer local has exactly ONE SET, the pseudo drops out of the global-alloc pool entirely and local-alloc coalesces it to target's register.
- mechanism: The ledger's own forensics (s6/s7/s15/s16/s21/s24) established the lock precisely: DECL_RTL(`p`) = pseudo 86 has TWO SETs in BB4, which makes local-alloc's block_alloc skip it (multi-def-in-BB, local-alloc.c:472 reg_n_deaths==1 test), promoting it to global-alloc, where set_preference walks the FIRST PLUS operand at each SET, giving pre-prune hard_reg_preferences[86] = {v0, v1}; sched1's insn-115 hoist injects the v0 conflict; prune_preferences leaves {v1}; find_reg picks v1 where target has v0. Every prior session attacked the PREFERENCE (operand order, scheduling topology, call order, scope) or tried to SPLIT `p` into two locals (closed by the 2026-07-19 Judge FAIL). The un-tried move is to remove the SECOND SET without introducing a second pointer local — i.e. to stop using a pointer local for one of the two lookups at all, by indexing the s16 vertex table directly as coordinate pairs. That is ordinary C (the table IS an array of x/y pairs), not a coercion: the multi-SET condition that forced global-alloc simply never arises.
- probe: Four spellings measured with `sandbox func_80057CC8 --disable all` from the s28 floor-3 candidate.c chassis, each applied to src/text1b.c:1524 in isolation. (V1) BOTH call sites indexed, second base obtained by re-assigning the SAME `table` local. (V3) prev call site indexed off `table`, next call site keeps the pointer local `p` (now single-SET). (V5) both call sites indexed, next off a FRESH base local `nt`. (V6) = V5 with the dead `s16 new_var` carrier deleted from the `*arg3` expression. Then a full `build` SHA1 check on the winner.
- result: CONFIRMED — MATCH. V1 = 6 (worse: re-assigning `table` re-creates a multi-SET pseudo on the BASE instead of the offset, moving the same defect). V3 = 0. V5 = 0. V6 = 0 (the `new_var` carrier was byte-inert and is deleted). Final body (V6 with the call renamed to the actual linked symbol `ratan2`, and the stale grind comment stripped): `sandbox --disable all` = 0, target_insns 111, build_insns 111, rules_dropped 0; full `build` = sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH. The 28-session floor of 3 was an artifact of the chassis all sessions inherited (a hand-rolled cast-heavy offset-pointer local reused across both lookups), not a property of the function. The impossibility statement banked at [s19] is now bounded correctly: it holds for a single MULTI-SET C pointer local, and is escaped by not needing one.
- verdict: CONFIRMED

## [s29] The `s16 new_var` carrier inside the `*arg3` store expression (`(s32)(new_var = *(&Judge + ...))`) is byte-inert on the matching chassis.
- mechanism: `new_var` is written by an embedded assignment whose value is immediately consumed by the enclosing expression; the variable itself is never read afterwards, i.e. a dead store to a local. On the old floor-3 chassis it was inherited without ever being tested in isolation.
- probe: V6 = V5 with the `new_var` declaration and the embedded assignment removed (`(s32)(*(&Judge + ...))`), `sandbox --disable all`.
- result: KILLED as a lever / CONFIRMED as removable — score unchanged at 0. The final committed body carries no dead store, so the candidate needs no dead-store-fake-exception claim and no /* FAKE */ annotation.
- verdict: CONFIRMED

## [s29b] The floor-3 residual is caused by the PREV-site SET of the pointer local `p`, and the function matches with that SET deleted (prev vertex read by ordinary array indexing) while the next-site `p` and its inline re-read of `*(s16 **)(arg0 + 4)` are kept exactly as the layer-2-PASSed baseline had them.
- mechanism: `p`'s DECL_RTL pseudo (86) carries one SET per site. With two SETs it fails local-alloc's `reg_n_deaths == 1` hard test (local-alloc.c:472, directly instrumented in s21/s24) and is handed to global-alloc, where the copy preference propagated from pseudo 129 selects $v1, emitting `addu v1,v0,a2` where target has the coalesced `addu v0,v0,a2`; that cascades to the two `lh` base registers = the 3. Reading the prev vertex as `table[(s16) prev_idx * 2]` removes that SET entirely, so pseudo 86 is single-SET, local-alloc keeps it, and it coalesces into $v0 like target.
- probe: v12 (`tmp/grind/func_80057CC8/s29b/v12.c`) applied to src/text1b.c in place of the INCLUDE_ASM line; `sandbox func_80057CC8 --disable all`, then full `build`.
- result: CONFIRMED — MATCH. score 0, target_insns 111 == build_insns 111, rules_dropped 0; full build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. Intermediate points: v8 (same but with an `s32 pi` index local) = 0; baseline = 3.
- verdict: CONFIRMED

## [s29b] The second load of the vertex-table base (`*(s16 **)(arg0 + 4)` inside `p`'s address expression) is required by the target bytes and cannot be replaced by reusing the already-loaded `table`.
- mechanism: the target itself performs both loads — asm/funcs/func_80057CC8.s:17 `lw $a2, 0x4($s2)` before the first `jal ratan2` and :50 `lw $a0, 0x4($s2)` after it. Reusing `table` across the call forces GCC to keep the base live in a callee-saved register over the call boundary, which changes the instruction count rather than just the allocation.
- probe: v7 (prev site indexed, next-site `p` derived from `table`) and v9 (both sites indexed off `table`, no `p` at all) — the two forms that eliminate the re-read; `sandbox --disable all` on each.
- result: KILLED as alternatives — v7 = 30 with build_insns 112, v9 = 30 with build_insns 112, against target_insns 111. Both are one instruction LONGER than target, so no allocation-level fix can reach 0 from either. The re-read is load-bearing program shape, not a duplicate. Banked as rejected/reload-elimination-p-derived-from-table-score30.c and rejected/both-sites-indexed-no-reload-score30.c.
- verdict: CONFIRMED (the re-read is necessary) / the reload-free family is KILLED

## s29 (2026-08-20) — escalation modality

- hypothesis: The residual 3 is not a property of "how the vertex-table base is shared
  between the two call sites" but of the existence of ANY user-declared pointer local for
  the derived vertex address. If the parameter carries its real record type and both
  pointer locals are deleted, GCC emits an anonymous address temp per reference, no
  DECL_RTL allocno exists to be mis-placed, and the allocation matches.
  mechanism: RTL-expand creates no user pseudo for a member+index reference, so
  local-alloc.c:472 `reg_n_deaths == 1` (the bail-out measured in s21/s24 that punted
  pseudo 86 to global-alloc, where pseudo 129's copy preference pinned it to `$v1`) is
  never consulted for that address; the address temp dies at its single use and coalesces.
  probe: replace the whole body with a `VertRing_57CC8 *ring` parameter form using
  `ring->verts[idx * 2]` at all three read sites; `sandbox func_80057CC8 --disable all`,
  then full `build`.
  result: score 0, target_insns 111 == build_insns 111, rules_dropped 0; full build sha1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH.
  verdict: CONFIRMED

- hypothesis: The record type alone is what matters; it is byte-equivalent whether reached
  through a typed parameter or a local cast from `u8 *arg0` (this was the s12 finding,
  which measured the struct as byte-neutral at score 3).
  mechanism: s12 argued combine.c sees the same addsi3 SET structure regardless of the
  source-level type, so `arg0->table` and `*(s16 **)(arg0 + 4)` fold identically.
  probe: same body as the matching form but with `u8 *arg0` retained plus
  `VertRing_57CC8 *poly = (VertRing_57CC8 *)arg0;` as the first statement.
  result: score 10 at the same 111 insns. The extra param->local copy reorders the
  prologue param-to-callee-save moves (`move s6,a2` emitted before `move s2,a0`), so
  `prev_idx` takes `$a2` and the base load takes `$a0`, the reverse of target. All ten
  diffs are that swap. Banked at rejected/struct-local-cast-from-u8ptr-score10.c.
  verdict: KILLED (the s12 byte-neutrality claim holds only for the member-access FOLD; it
  does not extend to how the parameter itself is spelled)

- hypothesis (inherited frontier, now moot): the escalation gates. Gate (a) canonical-asm
  needs STRONG scan_hand_coded signals.
  mechanism: n/a — evidentiary gate.
  probe: `python3 tools/scan_hand_coded.py --single func_80057CC8`.
  result: tier=LOW score=1/8, only S4 set, no S1/S2/S6. Gate (a) FAILS. Gate (b) was not
  reached because the floor dropped to 0; no disposition entry was filed and none is
  needed.
  verdict: KILLED (as a disposition path — the function is matched, not exhausted)

## s29 (2026-08-20) — escalation / disposition

### H-s29-1 — "the ban-compliant single-`table` form is a floor-3-class near-miss whose gap is register allocation"
- mechanism claimed by s1-s28: pseudo 86's p1 `addu` fails to coalesce (`addu v1,v0,a2` vs
  target `addu v0,v0,a2`), a pure allocation divergence, therefore closable by steering.
- probe: apply the ban-compliant form (one `table` local loaded once, both call sites
  indexed off it, no second load / no second local / no reassignment) and run
  `sandbox func_80057CC8 --disable all` on the current chassis.
- result: **score 30, target_insns 111, build_insns 112, rules_dropped 0.** The build is one
  instruction LONGER than the target. Allocation steering cannot delete an instruction.
- verdict: **KILLED.** The floor-3 framing was an artifact of forms that already contained a
  banned construct. The ban-compliant floor is 30 and the gap is structural, not allocative.

### H-s29-2 — "the target's two base loads are a compiler artifact, so a single-load C form can still reach 111 insns"
- mechanism: if the second `lw $a2/$a0,0x4($s2)` in the target were merely GCC re-materializing
  a value it could equally have kept live, a single-load C form would be reachable.
- probe: read the target directly (asm/funcs/func_80057CC8.s:17 and :50) and compare insn
  counts of held-live (H-s29-1, 112) versus re-materialized (candidate.c, 111) forms.
- result: the target genuinely loads the base twice, once per call site, because the
  intervening `ratan2` call clobbers memory. The held-live form pays exactly +1 insn for the
  callee-save materialization; the re-materialized form is exactly 111.
- verdict: **KILLED.** The two loads are load-bearing target shape, not artifact. Reaching 111
  requires re-materializing the base at the second site — which is the refused family.

### H-s29-3 — endgame-lock gate evaluation (the disposition question itself)
- probe (a): `tools/scan_hand_coded.py --single func_80057CC8`.
- result (a): tier=LOW score=1/8, only S4 set; every STRONG signal (S1/S2/S6) absent. **FAILS.**
- probe (b): grep `docs/reference/sotn-construct-index.md` (1,365 entries, sotn master
  aa53500226ee84be763f3e8702b27de06456b3a7) for reload / duplicate-load / second-read /
  repeated-member-deref / redundant-pointer-local classes.
- result (b): zero PSX hits; no class covers duplicated base-address materialization across an
  intervening call. Nearest family `dup_if_else_arm` is per-arm-of-one-diamond. **FAILS.**
- verdict: **CONFIRMED both-gates-fail** — the pre-decided case under the owner's 2026-07-27
  standing ruling. Disposition filed at docs/grind/decisions.md:8107; terminal park; nothing
  pending on the owner.

### Open frontier for any future re-attempt (single, concrete, measurable)
A lever qualifies only if it produces **build_insns == 111 without a second source-level
materialization of the vertex-table base**. Nothing in 29 sessions has produced such a form,
and the target's own two-load shape argues one may not exist. Do not re-open on a form that
merely lowers the score with a second materialization present — that is the refused family
regardless of spelling.

## [s29] The ban-compliant single-`table` form (one pointer local loaded once, both ratan2 call sites indexed off it, no second load / no second local / no reassignment) is a floor-3-class near-miss whose residual is pure register allocation and is therefore closable by steering pseudo 86's p1 addu coalescing.
- mechanism: s1-s28 attributed the gap to local-alloc.c:472's reg_n_deaths==1 bail-out on pseudo 86 punting its DECL_RTL pseudo to global-alloc, where pseudo 129's copy preference pins it to $v1 (addu v1,v0,a2) instead of target's coalesced $v0 (addu v0,v0,a2) — an allocation-only divergence with all other instructions target-identical.
- probe: Applied the ban-compliant form (tmp/grind/func_80057CC8/s29/formB.c) to src/text1b.c:1524 in place of the INCLUDE_ASM line and ran `sandbox func_80057CC8 --disable all` on the current chassis.
- result: score 30, target_insns 111, build_insns 112, rules_dropped 0. The build is ONE INSTRUCTION LONGER than the target. No allocation or scheduling steering can delete an instruction, so the residual is not allocative. The floor-3 framing was an artifact of forms that already contained a Judge-banned construct (every one of them re-materializes the base at the second call site).
- verdict: KILLED

## [s29] The target's two loads of the vertex-table base are a compiler artifact of a single-load source, so a C form that loads the base once can still reach 111 instructions.
- mechanism: If GCC merely re-materialized a value it could equally have kept live in a callee-save, then the held-live source form and the re-materialized source form would emit the same instruction count and differ only in allocation.
- probe: Read the target directly (asm/funcs/func_80057CC8.s:17 `lw $a2,0x4($s2)`, :50 `lw $a0,0x4($s2)`) and compared build_insns between the held-live form (H1, 112) and the no-base-local form (candidate.c, 111) on the current chassis.
- result: The target genuinely loads the base twice, once before each ratan2 call, because the intervening call clobbers memory. The held-live source form pays exactly +1 insn for the callee-save materialization the target does not have; the re-materialized form is exactly 111/111. The two loads are load-bearing target shape, not artifact.
- verdict: KILLED

## [s29] Endgame-lock gate (a): func_80057CC8 shows STRONG hand-coded-asm signals and qualifies for the canonical-asm grant path.
- mechanism: The 2026-07-27 standing ruling grants canonical-asm only on STRONG scan_hand_coded signals (S1 multu pacing / S2 empty-body branches / S6 BIOS jumptable).
- probe: python3 tools/scan_hand_coded.py --single func_80057CC8
- result: HAND_CODED tier=LOW score=1/8 (111 insns). Only S4 front-loads is set (4 loads in an 8-insn window at insn 15). S1 0 multu/mflo pairs; S2 no empty-body branches; S3 9 spills / 15 distinct regs (compiled range); S5 jaccard < 0.5 no sibling cluster; S6 no BIOS jumptable; S7 all callee-saves $sp-saved; S8 no redundant mask-before-shift. Canonical gate independently routes verdict=C. Gate FAILS.
- verdict: CONFIRMED

## [s29] Endgame-lock gate (b): an in-hand SOTN-master precedent exists for the closing construct (a second source-level materialization of a base pointer across an intervening call, used to steer register allocation).
- mechanism: The 2026-07-27 standing ruling grants a coercion/spelling family only on a citable SOTN-master precedent (file+line or commit); 'same spirit' does not qualify.
- probe: Grepped docs/reference/sotn-construct-index.md (1,365 entries, sotn-decomp master aa53500226ee84be763f3e8702b27de06456b3a7) for reload / duplicate-load / second-read / repeated-member-deref / redundant-pointer-local shapes, and reviewed the class table for any covering class.
- result: Zero PSX hits; no indexed class covers duplicated base-address materialization across an intervening call. The nearest family, dup_if_else_arm (958 hits, .claude/rules/duplicated-statement-into-arms.md), is duplication into the two arms of ONE control-flow diamond, not duplication across two independent sequential call argument lists — the exact distinction the owner drew when refusing this family for this function on 2026-07-20. Gate FAILS.
- verdict: CONFIRMED

## [s30] A pure-C form exists that emits exactly 111 instructions with NO second source-level materialization of the vertex-table base (the s29 frontier hypothesis, re-opened in solver modality).
- mechanism: the base expression is written ONCE, inside a `static inline` helper that
  computes the neighbour angle; GCC's inliner emits the helper body twice, and each inlined
  copy reloads `*(s16 **)(arg0 + 4)` after the intervening `ratan2` call because the call
  clobbers memory. The duplication is produced by compilation, not by the programmer, and
  it reproduces the target's own two loads (asm/funcs/func_80057CC8.s:17, :50).
- probe: wrote `static inline s16 *vert_base_57CC8(u8 *)` + `static inline s32
  vert_angle_57CC8(u8 *, s32 idx, s16 cx, s16 cy)`, called once per neighbour, centre vertex
  read through `s16 *ctr = vert_base_57CC8(arg0);`; applied to src/text1b.c:1665 and ran
  `sandbox func_80057CC8 --disable all`, `goal_from_tgt.py classify`, and a full `build`.
- result: **score 0, target_insns 111 == build_insns 111, rules_dropped 0**; classify ->
  "NO DIVERGENCE: the two streams are identical"; full build sha1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH. The base expression occurs
  exactly once in the whole translation unit.
- verdict: **CONFIRMED** — the frontier hypothesis is true and the function matches.

## [s30] The s29 ban-compliant single-`table` residual is upstream of every solver model (typed re-measurement of the s29 insn-count argument).
- mechanism: with the base cached in a local that stays live across `ratan2`, GCC must give
  it a callee-save register; that is a NINTH callee-save ($s8) with its own save, restore
  and `move` (3 insns) where the target pays a `move` plus a 1-insn reload.
- probe: applied rejected/s29-ban-compliant-single-table-no-reload-score30.c and ran
  `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_80057CC8`.
- result: ours 112 insns vs target 111; **FIRST DIVERGENCE: PRE-RA**, "next tool: none — the
  residual is upstream of every model"; ours-only `sw s8,56(#)` / `move s8,#` / `lw s8,56(#)`,
  target-only `move #,#` / `lw #,4(#)`.
- verdict: **CONFIRMED (typed FORECLOSURE of the RA and scheduler axes for the cached-base
  family)** — s29's conclusion was right and is now mechanically typed, not merely counted.

## [s30] `inverse_compose.py classify` is usable on an asm-until-matched (INCLUDE_ASM) function.
- mechanism: it compares `<stem>.hon.s` against `<stem>.tgt.s`, and `mkasm_honest.sh` builds
  `.tgt.s` from the CURRENT src/ plus regfix/asmfix rules.
- probe: ran it with the 112-insn form applied and zero rules on the function.
- result: reported "honest 112 insns, target 112 insns — FIRST DIVERGENCE: IDENTICAL" while
  the true target is 111 insns. It compared our stream against itself.
- verdict: **KILLED** — on an INCLUDE_ASM function the only honest backend is
  `goal_from_tgt.py` (object-level). Recorded so no future solver session spends a turn on
  the fictional verdict.

## [s30] The `s32 half` named intermediate (present in every form since s1) is load-bearing.
- mechanism: it names `(ang_prev - ang_next) / 2` as its own local, which was assumed since
  s1 to bias where the division result is materialized.
- probe: dropped `half` while keeping `ang_opp` (formH) and measured.
- result: score 0, 111 insns — unchanged.
- verdict: **KILLED** — `half` was never load-bearing; it is deleted from the candidate,
  reducing the annotated-construct surface to exactly one local.

## [s30] The `s32 base` named intermediate is cosmetic and can be collapsed into the expression.
- mechanism: `ang_mid = (ang_prev + 0x800) - ((s32)(ang_prev - ang_next) / 2);` should emit
  the same code as naming the sub-expression.
- probe: collapsed both intermediates (formG), measured, and ran `goal_from_tgt classify`.
- result: score 6 at 111 insns; classify -> PRE-RA with exactly one differing instruction
  shape, `ours addiu #,#,-2048` vs `target addiu #,#,2048`. GCC folded the constant across
  the subtraction into `ang_prev - (half - 0x800)`; the target instead materializes
  `addiu $v0,$s0,0x800` in the `beqz` delay slot (asm/funcs/func_80057CC8.s:62).
- verdict: **KILLED** — `ang_opp` is load-bearing through combine.c/cse.c constant
  re-association, and is therefore carried as a FAKE-annotated named intermediate under
  .claude/rules/no-new-park-categories.md:189 (+ the 2026-08-17 clarification at :193).

## [s30] The centre-vertex read spelling is free once the helper is in place.
- probe: three spellings of the two centre reads, helper held fixed.
- result: `ctr = base; cx = ctr[arg1*2]; cy = ctr[arg1*2+1]` -> 0;
  `ctr = base + arg1*2; cx = ctr[0]; cy = ctr[1]` -> 1;
  centre read through a `base + idx*2` accessor shared with the angle helper -> 6.
  All 111 insns.
- verdict: **KILLED** — it is not free; the first spelling is required. Banked at
  rejected/s30-inline-helper-ctr-plus-idx2-score1.c and
  rejected/s30-inline-helper-vert-accessor-for-centre-score6.c.

### Frontier after s30
The function MATCHES and the ledger frontier is closed. The only open question is a policy
one, stated in full in memory/grind/func_80057CC8/self_vet.md T5: whether a `static inline`
helper called twice — the programmer writing the base expression once and GCC's inliner
emitting it twice — is the "compound-address duplication across two calls" family the owner
refused on 2026-07-20. The measurements say the source contains exactly one materialization;
the refusal is worded about source-level duplication. That is the layer-1 / Judge call.


## s30b (2026-08-27) — forensics

**H-s30b-1 (KILLED, with mechanism).** "Some pure-C form materializes the vertex-table
base exactly once at source level and still emits the target's two `lw ...,0x4($s2)`
loads" — the ledger's live frontier hypothesis #1 since s28.
*Mechanism tested:* the only path in GCC 2.7.2 from one RTL load to per-use loads at the
original address is the REG_EQUIV / reg_equiv_mem path in `local-alloc.c
update_equiv_regs` (there is no gcse.c in this compiler, and reload spills to `N($sp)`
otherwise). *Probe:* read the gates in the compiler source, then exercise each one
directly — const-qualified base to set RTX_UNCHANGING_P (gate at local-alloc.c:583),
and base-read sinking to make `reg_basic_block[regno] >= 0` (gate in update_equiv_regs,
flow.c:2072). *Result:* both gates are real and both are reachable in C, but even with
the note PRESENT (formD, verified in `.lreg`) global alloc still gives the base pseudo a
callee-save and emits ONE load. Independently, the target's first base copy sits in the
call-clobbered $a2 and is dead before the `jal`, so its two loads are two distinct RTL
values; caller-saving cannot explain it either
(`CALLER_SAVE_PROFITABLE` = `4*calls < refs`, regs.h:165, fails at 1 call / 3-4 refs).
**Verdict: KILLED.** Do not spend another session searching for a single-materialization
spelling; the compiler has no mechanism that produces the target's shape from one.

**H-s30b-2 (KILLED).** "`const`-qualifying the base read is a codegen lever here."
*Probe:* formC (formB + `*(s16 *const *)`) and formE (formD without const).
*Result:* formC = 34/112 (worse than formB's 30/112); formD (const) and formE
(non-const) are identical at 59/104. The qualifier reaches RTL (`mem/u`) but changes
nothing downstream in this function. **KILLED as a lever**; retained only as the gate-1
key documented in evidence.md.

**H-s30b-3 (CONFIRMED, new).** "The ban-compliant single-read regime is pinned at 112
instructions" (s29's closing claim) is FALSE. *Probe:* three placements of the same
single base read. *Result:* 112 (before branches), 112 (between the if-blocks), 104
(after both). **CONFIRMED that the band is 104..112**, so the ban-compliant obstacle is
shape, not instruction budget — but no measured placement lands on 111 and every
placement below 112 scores far worse (59 vs 30).

**Frontier left for the next session.** The single-materialization axis is closed by
mechanism. What remains genuinely open and ban-compliant: the 104-insn regime is a
different codegen world (7 instructions BELOW target) that no prior session had ever
seen — every earlier session lived in the 112 regime. A session should map what those
7 instructions are (diff formD's emitted text against the target block by block) and ask
whether some intermediate arrangement — e.g. sinking only the base read while keeping the
centre reads early through the SAME single pseudo, or reordering the two index if-blocks
so the base def lands in the tail block without moving the centre reads — reaches 111
with a target-shaped body. That is a shape search inside a region the ledger has never
entered, not a re-run of the spelling search.

## [s30] A pure-C form materializes the vertex-table base exactly once at source level and still emits the target's two loads of 0x4($s2) (ledger frontier hypothesis #1, open since s28).
- mechanism: The only path in GCC 2.7.2 from one RTL load to per-use loads at the ORIGINAL address is the REG_EQUIV / reg_equiv_mem path created in local-alloc.c update_equiv_regs (this compiler has no gcse.c; reload otherwise spills to N($sp), which the target does not show). That note is gated on (1) RTX_UNCHANGING_P on the MEM, else validate_equiv_mem returns 0 at the intervening CALL_INSN (local-alloc.c:583), and (2) reg_basic_block[regno] >= 0, i.e. every reference to the pseudo confined to one basic block (flow.c:2072-2075).
- probe: Read both gates in tools/gcc-2.7.2/local-alloc.c + flow.c + global.c + regs.h, then exercised each in C and measured: formB baseline (30/112, zero REG_EQUIV notes); formC = formB with *(s16 *const *) so the dump prints mem/u (34/112, still zero notes - gate 2 fails); formD = base read sunk below both if-blocks so all references sit in one block (59/104, exactly one REG_EQUIV (mem/u:SI (plus:SI (reg/v:SI 72) (const_int 4))) note in .lreg); plus 7 standalone gate probes (equivprobe.c) isolating each gate.
- result: Both gates are real and both are reachable from C, but the note is NOT sufficient: in formD and in every probe where the note was present, global alloc still assigned the base pseudo a callee-save ($16) and reload never fell back to the equivalent MEM - one lw, not two. Rematerialization additionally requires the pseudo to FAIL allocation, which needs register pressure this function does not have. Independently, the target's first base copy sits in $a2 (call-clobbered) and is dead before the jal at :47, so its two loads are two distinct RTL values, and caller-saving cannot produce that either: global.c:1179 only retries with accept_call_clobbered when CALLER_SAVE_PROFITABLE(refs, calls) = 4*calls < refs (regs.h:165), which fails at 1 call crossed / 3-4 refs.
- verdict: KILLED

## [s30] const-qualifying the base read (*(s16 *const *)(arg0 + 4)) is a codegen lever for this function.
- mechanism: TREE_READONLY on the pointed-to object sets RTX_UNCHANGING_P on the emitted MEM, which is the gate at local-alloc.c:583 that a call otherwise trips.
- probe: formC (formB + const) and formE (formD without const), both measured with sandbox --disable all, plus RTL dump inspection for the mem/u flag.
- result: The qualifier reaches RTL - the load prints as (mem/u:SI (plus:SI (reg/v:SI 72) (const_int 4))) - but changes nothing downstream here: formC measures 34/112, WORSE than formB's 30/112, and formD (const) vs formE (non-const) are identical at 59/104.
- verdict: KILLED

## [s30] s29's closing claim that the ban-compliant single-read regime is pinned at 112 instructions, so the form is 'structurally short and unmatchable in kind'.
- mechanism: The instruction count in the single-read regime is set by whether a branch separates the base def from a later use: with a branch in between the pseudo is REG_BLOCK_GLOBAL, gets a 9th callee-save and pays save+move+restore; with all references in the tail block the whole allocation and the centre-read code change regime.
- probe: Three placements of the SAME single source-level base read, each measured: before the branches (formB/formC), between the two index if-blocks (formF), after both if-blocks (formD/formE).
- result: 112 insns / score 30 (before), 112 / 44 (middle), 104 / 59 (after). The ban-compliant band is 104..112, not a fixed 112, so the barrier is SHAPE and not instruction budget - though no measured placement lands on 111 and every sub-112 placement scores far worse.
- verdict: CONFIRMED

## s31 (2026-08-27) — forensics

## [s31] The 104-insn regime's 7-instruction deficit is an unexplained shape difference that some intermediate arrangement could close on 111 (ledger live frontier #1 after s30b).
- mechanism: proposed by s30b as the live frontier; the deficit had never been itemised.
- probe: normalised block-by-block diff of formD's honest stream against asm/funcs/func_80057CC8.s (tmp/grind/func_80057CC8/s31/norm2.py), plus cc1 -da dumps of formB and formD compared at .combine.
- result: the 7 decompose exactly as 4 (combine.c folding the HImode extension into the load once the read shares a basic block with its use — two `(sign_extend:SI (reg/v:HI N))` insns present in dumpsB/.combine, absent in dumpsD/.combine, replaced by `extendhisi2_internal` on a MEM) + 2 (one fewer callee-save, because the fold collapses each coordinate's raw and extended live values into one) + 1 (the target's second `lw 0x4($s2)`). Nothing in the deficit is free-floating shape.
- verdict: KILLED as an open shape question — the 104 regime is fully explained and contains no path to 111.

## [s31] The centre coordinates' u16-plus-(s16)-cast spelling is what forces the target's lhu + separate sll/sra pair.
- mechanism: assumed since s1; the u16 local was believed to keep an unextended value alive.
- probe: v3 — coordinates declared s16 and read via *(s16 *), everything else formB.
- result: honest stream BYTE-IDENTICAL to formB (diff -q clean), 112 insns. GCC keeps the raw HImode pseudo regardless of the C type because the closing `sh` truncates.
- verdict: KILLED — the u16 spelling is not a lever; basic-block placement is.

## [s31] Hoisting `scale = arg0[2] * 40` above the calls frees the 9th callee-save the cached base needs (ledger live frontier #2: pay back the +1 by reducing values live across the call).
- mechanism: arg0's only post-call use is arg0[2]; killing that use should let arg0 die before the call.
- probe: v1, built and counted.
- result: the hoist works (arg0 allocated to call-clobbered $t0, never saved) but `scale` inherits its live range; 9 values still cross the call, $fp is still the 9th callee-save, 112 insns unchanged.
- verdict: KILLED for relocation — but the underlying idea is CONFIRMED in the entry below: a value must be ELIMINATED or two MERGED, not moved.

## [s31] Forming the next-neighbour vertex ADDRESS above the first call (single base materialisation) reduces the live-across-call set to the target's 8 and lowers the floor.
- mechanism: `next_vert = &table[ni * 2]` consumes both `table` and `next_idx` before the call, so ONE value crosses where formB needed two; global.c then allocates exactly 8 callee-saves and no `sw`/`lw` pair for $fp.
- probe: v4/v7/v8/v9 (four spellings) + v5 (both addresses hoisted), each built and diffed; the candidate measured with `sandbox func_80057CC8 --disable all`.
- result: **score 26, build_insns 110, target_insns 111** — floor 30 -> 26, and a 110-insn regime no prior session had reached. All spellings byte-identical except `table + ni + ni`, which flips one addu's PLUS operand order and still scores 26.
- verdict: CONFIRMED — banked as candidate.c.

## [s31] Some ban-compliant arrangement reaches 111 instructions with a target-shaped body (the ledger's live frontier since s28).
- mechanism: the count band 104..112 was thought to contain a reachable 111.
- probe: instruction-multiset accounting of the best ban-compliant form against the target, on top of s30b's mechanism proof.
- result: the 110-insn candidate pays its whole difference from the target in ONE instruction — the second `lw 0x4($s2)` — while already matching the target's callee-save count. A ban-compliant form emits exactly one base load, and GCC 2.7.2 has no pass that turns one RTL load into two loads at the original address (s30b: REG_EQUIV notes present but insufficient; caller-save excluded by CALLER_SAVE_PROFITABLE). Any ban-compliant form reaching 111 must therefore carry an instruction the target does not have, i.e. cannot be byte-identical. All measured counts fall out of insns = (values live across the call) + (base loads) + 101: 112 = 9+1, 110 = 8+1, 104 = 7+1, target 111 = 8+2.
- verdict: KILLED — 111-with-target-bytes is unreachable by counting, not merely unfound by search. The residual is the policy question refused 2026-07-20 and standing-ruled 2026-07-27.

### Frontier after s31
The count model above leaves exactly one arithmetically-open cell: a form with 7 values live across
the call AND two base loads would be 110, and one with 8 live values and two base loads is the
target's 111 — but the second load is the banned construct, so the only ban-compliant cells are
{7,8,9} x {1 load} = {104,110,112}. The honest open work is therefore NOT another spelling search:
it is (a) checking whether the count model itself has an escape — a value that can be made live
across the call for free by being needed anyway (which would put a ban-compliant form at 111 with a
different instruction in the reload's place, still non-zero but a lower floor), and (b) driving the
26 down within the 110 regime by matching the target's register assignment and the surviving order
differences around the two calls (our address arithmetic sits before the first call; the target's
sits between the calls). Both are floor work, not match work; s31's accounting says the match itself
is gated on the refused duplication family.

## [s31] The 104-insn regime's 7-instruction deficit is an unexplained shape difference that some intermediate arrangement could close on 111 (ledger live frontier #1).
- mechanism: s30b found placement moves the count 112 -> 104 but never itemised which instructions vanish; the claim was that an intermediate placement could land on 111 with a target-shaped body.
- probe: Built the honest stream for formB and formD via tools/ra_solver/mkasm_honest.sh, extracted the function, normalised both sides to numeric registers / canonical opcodes / decimal immediates and unified-diffed against asm/funcs/func_80057CC8.s (tmp/grind/func_80057CC8/s31/norm2.py); then captured cc1 -da dumps for both forms (tools/grinder/dump.ps1) and compared .combine.
- result: The 7 decompose exactly: 4 = combine.c folding the HImode sign-extension into the load (dumpsB/.combine keeps two (sign_extend:SI (reg/v:HI 84|88)) insns; dumpsD/.combine has them folded into (sign_extend:SI (mem:HI (reg 105))) = extendhisi2_internal pattern 125), available only when the load and its extending use share a basic block; 2 = one fewer callee-save because the fold collapses each coordinate's raw+extended live values into one; 1 = the target's second lw 0x4($s2).
- verdict: KILLED

## [s31] The u16-plus-(s16)-cast spelling of the centre reads is what produces the target's lhu + separate sll/sra pair.
- mechanism: Assumed since s1 that the u16 local is what keeps an unextended value alive across the calls.
- probe: v3: coordinates declared s16 and read via *(s16 *), everything else identical to formB; honest streams diffed.
- result: Byte-identical to formB (diff -q clean), 112 insns. GCC keeps the raw HImode pseudo regardless of C type because the closing sh truncates. Placement, not type spelling, is the lever.
- verdict: KILLED

## [s31] Hoisting scale = arg0[2] * 40 above the calls frees the 9th callee-save the cached base needs (ledger live frontier #2).
- mechanism: arg0's only post-call use is arg0[2]; removing that use should let arg0 die before the first call and drop the live-across set from 9 to 8.
- probe: v1 built and counted; honest stream inspected for arg0's allocation.
- result: The hoist works at the level aimed at (arg0 lands in call-clobbered $t0 and is never saved) but scale inherits the live range: still 9 values across the call, still $fp as the 9th callee-save, still 112 instructions. Relocating a computation cannot shrink the live-across set.
- verdict: KILLED

## [s31] Forming the next-neighbour vertex ADDRESS above the first call (base still materialised exactly once) reduces the live-across-call set to the target's 8 and lowers the floor.
- mechanism: next_vert = &table[ni * 2] consumes both table and next_idx before the call, so ONE value crosses where formB needed two; global.c then allocates exactly 8 callee-saves and no sw/lw pair for $fp.
- probe: Four spellings (table + ni*2, &table[ni*2], (s16 *)((s32)table + ((s32)(s16)next_idx << 2)), table + ni + ni) plus a both-addresses-hoisted twin, each built and diffed; candidate measured with sandbox func_80057CC8 --disable all.
- result: score 26, build_insns 110, target_insns 111 (previous best 30 at 112). All spellings emit byte-identical text except table+ni+ni, which only flips one addu's PLUS operand order and also scores 26. Both-addresses-hoisted twin is likewise 110.
- verdict: CONFIRMED

## [s31] Some ban-compliant arrangement reaches 111 instructions with a target-shaped body (the ledger's live frontier since s28).
- mechanism: The measured count band 104..112 was believed to contain a reachable 111.
- probe: Instruction-multiset accounting of the new 110-insn form against asm/funcs/func_80057CC8.s, on top of s30b's mechanism proof that GCC 2.7.2 cannot rematerialise one RTL load as two loads at the original address.
- result: The 110-insn form already matches the target's callee-save count and pays its entire difference in ONE instruction: the second lw 0x4($s2). A single-materialisation form emits exactly one base load and no GCC 2.7.2 pass manufactures a second, so any ban-compliant 111-insn form must carry an instruction the target lacks and cannot be byte-identical. Model insns = (values live across the call) + (base loads) + 101 predicts every measured count with no free parameters: 112 = 9+1, 110 = 8+1, 104 = 7+1, target 111 = 8+2.
- verdict: KILLED

## s32 (2026-08-27) — rederive

| # | hypothesis | mechanism | probe | result | verdict |
|---|---|---|---|---|---|
| s32-H1 | The next-neighbour byte offset can be selected in the arms of the next-index test, re-using the sign-extension the range compare already needs, cutting the address cost to one instruction. | GCC 2.7.2's cse1 keeps the compare's `(sign_extend (subreg tmp))` available inside both arms, so `off = (s16)tmp * 4` folds to a single `sll` there; the later `addu` then coalesces onto the base pseudo's hard register because the base is dead at that point. | Wrote the arm-selected-offset form; `sandbox func_80057CC8 --disable all`. | **score 24, build_insns 108** (previous best 26 at 110). NEW FLOOR. | CONFIRMED |
| s32-H2 | The target's `$16..$19` callee-save mapping can be reached by re-ranking the call-crossing allocnos from the source side. | `allocno_compare` (tools/gcc-2.7.2/global.c:635) ranks by `floor_log2(n_refs)*n_refs/live_length`, and the first-ranked allocno takes `$16`. Reference counts and live ranges are both source-controllable. | Read the formula out of global.c, computed the ranks for the target and for six measured forms, then tried every source-side re-ranking that keeps a single base materialisation (address in the arms, offset in the arms, address before/after `pi`, explicit twin locals, prev-address local, twins used in the final adds). | Best achievable rank order is `cys, base, cxs, arg0`; the target's is `cys, cxs, arg0, next_idx`. Matching it needs the base pseudo at <=3 references and it structurally has 4 (def + centre + prev + next). Reducing to 3 requires a second base materialisation (banned) or deriving one neighbour address from another (measured worse). | **KILLED** |
| s32-H3 | Shrinking the live-across-call set below the target's 8 helps, if the values removed are the sign-extended centre twins rather than the base. | Computing the next-neighbour dx/dy before the first call kills the twins early; the target's 8 would become 7 and the sw/lw pair disappears. | vT: dx/dy precomputed before the first `ratan2`. | score 42 at 110 insns — combine folds the centre `lhu` pair into `lh` and the body drifts. Matches s31's 7-live 104-insn result (59). | **KILLED** |
| s32-H4 | Swapping the two index if-blocks (next first) re-ranks the allocnos favourably on the CURRENT chassis, even though s2 measured it dead in 2026-07 on a different one. | The next address could then be formed between the blocks, lengthening its live range and lowering its priority toward the target's `$19`. | vK (sll16/sra14 form) and vG/vM (offset-in-arms form). | 41 at 109 and 41 at 106 — GCC fuses the two `lbu 3(arg0)` reads once the blocks are adjacent. Dead on a third chassis. | **KILLED** |
| s32-H5 | The centre-coordinate type spelling or the twin-vs-raw choice in the two final adds is a lever in the new 108-insn regime. | s31 proved both inert at 112 insns; a different regime could expose them. | vX (`s16` centre, `*(s16 *)` reads) and vH (twins used in the final adds). | 24 / 108 and byte-identical text respectively. Inert. | **KILLED** |
| s32-H6 | Frontier hypothesis 2 (redirect the PLUS operand order at the neighbour-address `addu`) is worth score. | A named prev-address local changes which operand RTL-expand puts first. | vQ: `s16 *prev_vert = &table[pi * 2];`. | The operand order DOES flip (`addu $2,$17,$2` vs `addu $2,$2,$17`); score unchanged at 24. Reachable, worth zero. | **KILLED** |
| s32-H7 | The arm selection spelling (`off = expr; if (cond) off = 0;` vs explicit `if/else`) and the offset unit (bytes vs elements) are free choices. | Both are semantically identical. | vV (explicit if/else) and vE (element offset). | 26 at 108 and 27 at 109 — both strictly worse than the byte-offset / assign-then-override spelling. Not free. | **KILLED** |

## [s32] The next-neighbour byte offset can be selected in the two arms of the next-index test, re-using the sign-extension the range compare already needs, cutting the address cost to one instruction and coalescing the address onto the base pseudo's register.
- mechanism: cse1 keeps the compare's sign_extend of tmp available inside both arms, so `off = (s16)tmp * 4` folds to a single sll there; the later addu then reuses the base's hard register because the base is dead at that point.
- probe: Wrote the arm-selected-offset form (tmp/grind/func_80057CC8/s32/vB.c) and ran `sandbox func_80057CC8 --disable all`.
- result: score 24, build_insns 108, target_insns 111, rules_dropped 0 -- against the inherited s31 candidate re-measured at 26/110 on the same chassis this session.
- verdict: CONFIRMED

## [s32] The target's $16..$19 callee-save assignment can be reached by source-side re-ranking of the call-crossing allocnos.
- mechanism: GCC 2.7.2 assigns hard registers in descending allocno_compare priority (tools/gcc-2.7.2/global.c:635 = floor_log2(n_refs)*n_refs/live_length*10000*size), first-ranked taking the first free callee-save $16. Both n_refs and live_length are source-controllable.
- probe: Read the formula from global.c, computed ranks for the target and six measured forms from their emitted asm, then measured every re-ranking that keeps a single base materialisation: address selected in the arms (vA/vC), offset selected in the arms (vB), address before/after pi (vD/vJ), explicit centre-twin locals (vI), named prev-address local (vQ), twins used in the final adds (vH).
- result: The best reachable rank order is cys, base, cxs, arg0 (vB); the target's is cys, cxs, arg0, next_idx. Matching it requires the base pseudo at <=3 references (priority ~0.09, below arg0's ~0.14); the ban-compliant function structurally gives it FOUR (def + centre address + prev address + next address), where floor_log2 steps to 2 and the priority is ~0.24. Reducing to 3 requires either a second materialisation of *(s16 **)(arg0 + 4) (the Judge-banned family) or deriving one neighbour address from another (measured worse, vT).
- verdict: KILLED

## [s32] Shrinking the live-across-call set below the target's 8 helps if the values removed are the sign-extended centre twins rather than the base.
- mechanism: Computing the next-neighbour dx/dy before the first call kills both twins early, so 7 values cross the call and one sw/lw pair disappears.
- probe: vT: two s32 locals dxn/dyn assigned from next_vert[0]/[1] minus the centre twins, placed above the first ratan2 call; sandbox measured.
- result: score 42 at build_insns 110 -- combine then folds the centre lhu pair into lh and the whole body drifts. Consistent with s31's other 7-live regime (104 insns, score 59).
- verdict: KILLED

## [s32] Swapping the two index if-blocks (next-index block first) re-ranks the allocnos favourably on the CURRENT chassis, even though s2 measured it dead in 2026-07 on a different chassis.
- mechanism: With the next-index block first, the next address can be formed between the two blocks, lengthening its live range and lowering its allocno priority toward the target's $19.
- probe: vK (s31 sll16/sra14 address form) and vG/vM (s32 offset-in-arms form), both measured.
- result: 41 at 109 insns and 41 at 106 insns. GCC fuses the two `lbu 3(arg0)` reads once the blocks are adjacent and five instructions collapse. Dead on a third chassis.
- verdict: KILLED

## [s32] The centre-coordinate type spelling (u16 + cast vs s16) or using the sign-extended twins instead of the raw coordinates in the two final adds is a lever in the new 108-insn regime.
- mechanism: s31 proved both inert in the 112-insn regime; a different regime could expose the choice because combine's extension folding is placement-sensitive.
- probe: vX (s16 centre reads) and vH (twins in the final adds), both measured and text-diffed.
- result: vX scores the same 24 at 108 insns; vH emits BYTE-IDENTICAL text to vB. Both inert.
- verdict: KILLED

## [s32] Frontier hypothesis 2 inherited from s24/s28 -- redirecting the PLUS operand order at the neighbour-address addu -- is worth score.
- mechanism: A named prev-address local changes which operand RTL-expand emits first in the address addu.
- probe: vQ: a named prev-neighbour address local alongside next_vert; measured and diffed.
- result: The operand order does flip (addu $2,$17,$2 instead of addu $2,$2,$17; the target has addu $2,$2,$6) and the score is unchanged at 24. The lever is reachable and worth zero.
- verdict: KILLED

## [s32] The arm-selection spelling (assign-then-override vs explicit if/else) and the offset unit (bytes vs elements) are free choices within the winning regime.
- mechanism: Both pairs are semantically identical, so any difference is pure codegen.
- probe: vV (explicit if/else) and vE (element offset), both measured.
- result: 26 at 108 insns and 27 at 109 insns respectively -- both strictly worse than the byte-offset assign-then-override spelling. Not free: the if/else flips the branch sense against the target's bnez, and the element offset costs a separate scale instruction.
- verdict: KILLED


## s33 (2026-08-27) - rederive

- **H-s33-1: The vertex-table base pseudo occupies a callee-save seat because it carries
  four references (s32-E4's model), so reducing it to three by deriving one vertex address
  from another will demote it and reproduce the target's $16..$19.**
  probe: centre pointer `ctr = table + arg1*4`, next derived from `ctr` (vA); both
  neighbours derived from `ctr` (vC); plus a `-da` dump of the baseline to read the actual
  allocation order.
  result: vA 38 / 109 insns, vC 35 / 110 insns - both regressions. The dump
  (tmp/grind/func_80057CC8/dumps/text1b.greg) shows `;; 87 preferences: 17` and
  `87 in 17  88 in 17`: the callee-save seat belongs to the next-ADDRESS allocno (88), and
  the base (87) inherits it through a copy preference, so the premise was wrong.
  **verdict: KILLED (and the underlying model corrected).**

- **H-s33-2: Breaking the copy preference between the base and the call-crossing address
  will leave the base in a caller-save register, matching the target's `lw $a2,0x4($s2)`.**
  probe: write the address as `(s16 *)(off + (s32)table)` - offset as the leading PLUS
  operand (vM); the same flip at the centre address (vT) and at a named prev address (vR).
  result: vM scores **22 at 108 insns** (baseline 24 at 108) with the base emitted in `$6`
  and `lw $6,0x4($19)` / `addu $2,$2,$6` matching the target instruction-for-instruction.
  vR and vT also 22 - the flip only pays at the add whose result crosses the call.
  **verdict: CONFIRMED - new floor 22.**

- **H-s33-3: A deliberately 110-111-insn ban-compliant arrangement can beat the 108-insn
  floor, because the engine metric is dominated by per-instruction text agreement rather
  than instruction count (s32 frontier axis 2).**
  probe: the target-shaped form (arm-selected next INDEX live across the first call, address
  formed after it from the single base) (vE); address selected in the two arms (vH/vN/vU/vW);
  centre pair read as one 32-bit word (vL); sll16/sra14 pair in the flipped regime (vX/vY).
  result: vE 33 / 111 insns - structurally the closest ever recorded (115 vs 116 normalised
  lines) but NINE values cross the call, so arg0 falls to $20 and arg3 into $fp. vH/vN/vU/vW
  all 25 / 110; vL 38 / 110; vX/vY 28 / 109. Nothing above 108 insns beats 22.
  **verdict: KILLED.**

- **H-s33-4: The next-address allocno can be demoted below `arg0` by giving it exactly three
  references over a long live range, which would yield the target's $16/$17/$18 assignment.**
  probe: arm-select the next INDEX and form the address once with `* 4` (v3/v4) or with the
  target's sll16/sra14 pair (vX/vY); single-def ternary offset (vZ); address selected
  directly in the arms (vH, four refs but two defs).
  result: three-reference spellings 28/28/28/28 and 25; the four-reference
  address-in-arms form DOES demote it one rank (emitted $16 cys, $17 cxs, $18 next-address,
  $19 arg0 - the target's $16/$17 exactly) but costs two instructions and measures 25.
  The rank and the instruction count are coupled; nothing measured buys both.
  **verdict: KILLED for every spelling tried; the coupling itself is the new frontier.**

- **H-s33-5: Banked spelling scores from the pre-2026-08 (banned-reload) chassis transfer to
  the current ban-compliant chassis.**
  probe: re-measure s27's call-order swap and s32-E6's if-block swap in the flipped-PLUS
  regime (vG, vQ).
  result: call-order swap 54 at 112 insns (s27 recorded 16); if-block swap 41 at 106 insns
  (s32 recorded 41 - that one does transfer).
  **verdict: KILLED - chassis-relativity is real and asymmetric; re-measure before spending.**

## [s33] The vertex-table base pseudo occupies a callee-save seat because it carries four references (s32-E4's model), so reducing it to three by deriving one vertex address from another will demote it below arg0 and reproduce the target's $16..$19 assignment.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:635) ranks call-crossing allocnos by floor_log2(n_refs)*n_refs/live_length; at four references floor_log2 steps to 2 and the base outranks arg0.
- probe: vA: centre pointer ctr = table + arg1*4, cx/cy read through ctr, next address derived from ctr with a relative arm-selected offset. vC: both neighbours derived from ctr (prev = ctr + (pi - arg1)*4). Plus a cc1 -da dump of the s32 baseline (tmp/grind/func_80057CC8/run_dump.sh) to read the real allocation order instead of inferring it.
- result: vA scored 38 at build_insns 109; vC scored 35 at build_insns 110 (baseline 24 at 108). The emitted code merely moves the same four-reference merged pointer allocno onto ctr (addu $17,$7,$4 then lhu $20,0($17)), so the reference count never drops, while the relative-offset arithmetic (addu $8,$0,4 / subu $8,$0,$4 and the wrapped-prev (arg0[3]-1-arg1)*4) adds instructions. Decisively, the .greg dump shows ';; 87 preferences: 17' with dispositions '87 in 17  88 in 17': pseudo 88 (the next-neighbour ADDRESS) is the allocno that crosses the call and needs the callee-save seat, and pseudo 87 (the base) only inherits it via the copy preference. The premise attributed the seat to the wrong allocno.
- verdict: KILLED

## [s33] Breaking the copy preference between the base pseudo and the call-crossing address pseudo will leave the base in a caller-save register, matching the target's lw $a2,0x4($s2).
- mechanism: RTL-expand emits the address as plus(op0, op1) in source operand order; with the base leading, the allocator records a copy preference from the base to the address pseudo, and since the address crosses ratan2 and must be callee-save, the preference drags the base into the same callee-save register. Reversing the source operands removes the tie.
- probe: vM: next_vert = (s16 *)(off + (s32)table) instead of (s16 *)((s32)table + off), everything else identical to the s32 candidate. Controls: vT (same flip applied to the centre address instead), vR (named prev-neighbour address written with the flip), v2 (address formed after pi rather than inside the arms), v5/v6 (two local-declaration reorderings).
- result: vM scores 22 at build_insns 108 / target_insns 111 / rules_dropped 0 - a two-point improvement with NO instruction-count change. The base is emitted in $6 and the pair lw $6,0x4($19) / addu $2,$2,$6 now agrees instruction-for-instruction with the target's lw $a2,0x4($s2) / addu $v0,$v0,$a2 (asm/funcs/func_80057CC8.s:17,:19). vR, vT, v2, v5 and v6 all re-measure 22, so the payoff is specific to the one add whose result crosses the call, not to operand order generally.
- verdict: CONFIRMED

## [s33] A deliberately 110-111-instruction ban-compliant arrangement can beat the 108-instruction floor, because the engine metric is dominated by per-instruction text agreement rather than by the absolute instruction-count difference (s32 frontier axis 2).
- mechanism: s32 measured 24 at 108 insns, 25-26 at 110 and 27 at 109 - the score is not monotone in |build_insns - target_insns|.
- probe: vE: the most target-shaped form - arm-select the next INDEX, keep it live across the first ratan2, and form the next address AFTER that call from the single table value (111 insns). vH/vN/vU/vW: next ADDRESS selected directly in the two arms (110). vL: centre coordinate pair read as one 32-bit word (110). vX/vY: the s31 sll16/sra14 address pair re-introduced on top of the flipped-PLUS regime (109).
- result: vE 33 at 111 insns - its normalised diff against target is the closest structural match ever recorded for this function (115 target lines vs 116 ours, tail identical from andi $16,$2,4095 onward) but it puts NINE values across the call instead of eight, because table occupies the seat the target frees by reloading; arg0 is pushed to $20 and arg3 spills into $30/$fp. vH/vN/vU/vW all 25 at 110; vL 38 at 110; vX/vY 28 at 109. Nothing above 108 instructions beats 22.
- verdict: KILLED

## [s33] The next-address allocno can be demoted below arg0 by giving it exactly three references over a live range longer than ~30 insns, yielding the target's $16/$17/$18 assignment (cys / cxs / arg0).
- mechanism: arg0 carries 5 references over the whole ~100-insn body, priority ~0.10 under allocno_compare; a three-reference allocno needs live_length > ~30 to fall below it, a four-reference one would need live_length > ~57 which the function cannot supply.
- probe: v3/v4: arm-select the next INDEX then form the address once with (s16)tmp * 4 (both operand orders). vX/vY: same but with the target's sll16/sra14 pair (both orders). vZ: single-def ternary offset. vH: address selected directly in the two arms (four references, two defs).
- result: Every three-reference spelling regresses: v3 28, v4 28, vX 28, vY 28, vZ 25 - the index form costs the instructions the byte-offset form saves. The four-reference address-in-arms form (vH) DOES demote it one rank and emits $16 cys, $17 cxs, $18 next-address, $19 arg0 - reproducing the target's $16 and $17 exactly - but costs two instructions (arg1 + 1 is sunk into both arms, plus a load-delay nop) and measures 25 at 110 insns in all three spellings tried. The register rank and the instruction count are coupled; nothing measured buys both.
- verdict: KILLED

## [s33] Banked spelling scores from the pre-2026-08 (banned-reload) chassis transfer to the current ban-compliant chassis, so a form s27 recorded at 16 would now sit below the 22-24 floor.
- mechanism: The floor-3 baseline those numbers were measured against contained the Judge-banned second base load; the ban-compliant chassis is a different codegen regime.
- probe: vG: re-measure s27's call-order swap (compute ang_next first) in the flipped-PLUS ban-compliant regime. vQ: re-measure s32-E6's if-block swap in the same regime.
- result: vG scores 54 at 112 insns where s27 recorded 16 - the number does not transfer at all. vQ scores 41 at 106 insns, exactly matching s32-E6's 41, so that one does transfer. Chassis-relativity is real and asymmetric: a banked score must be re-measured before it is spent, and the direction of the error is not predictable.
- verdict: KILLED

## s34 (2026-08-27) - structural

## [s34] The next-address allocno can be demoted below arg0 by LENGTHENING its live range (s33 frontier hypothesis #1), and that demotion yields the target's $16..$19 mapping.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:635) = floor_log2(n_refs)*n_refs/live_length; s33 varied n_refs and the definition site but never pushed live_length past ~16 insns. Only live_length was left unexploited.
- probe: x1/x2/x3 hoist the `tmp`/`off` computation out of its basic block (before the prev-index if-block, before the vertex-table load, and with the address formed after `pi`); y1 defines the offset via the target's own `(s32)(tmp << 16) >> 14` pair so its def is the 2nd insn of the block; y5 drops the redundant `(s16)` narrowing so the def is the 1st insn. Each measured with sandbox, and the winning form re-dumped with tmp/grind/func_80057CC8/run_dump.sh to read text1b.lreg / text1b.greg.
- result: The lever WORKS and is worth NOTHING. y5 lengthens the offset allocno from 16 to 19 insns (text1b.lreg "Register 104 used 3 times across 19 insns"), which flips the printed global allocation order from `... 87 104 72 ...` to `... 87 72 104 ...` - arg0 is now allocated first. The dispositions are unchanged (`72 in 19`, `104 in 17`) because arg0 carries hard-register conflicts on 16, 17 AND 18 (text1b.greg `;; 72 conflicts: ... 16 17 18 29`), so $19 is its only legal seat at any rank. Hoisting out of the block entirely costs an instruction and measures 38/109 (x1/x2/x3) or 33/109 (w2/w3/w4).
- verdict: KILLED (the ranking axis is closed - rank does not determine this function's seats)

## [s34] Dropping the redundant `(s16)` narrowing from the byte-offset expression is byte-neutral and therefore inert.
- mechanism: the wrap test `(s16) tmp >= (s32)arg0[3]` already forces the sll16/sra16 pair, so both spellings compile to three insns; only the position of the offset's defining insn inside the block differs.
- probe: y5 (`off = tmp * 4`), y1 (`off = (s32)(tmp << 16) >> 14`), y6/y7 (both with the base-first PLUS order as controls), z20 (`off = tmp << 2`).
- result: NOT inert - y5 measures **20 at build_insns 108**, a two-point improvement over the s33 candidate's 22 at the same instruction count, with byte-identical register assignment. The def moves from 3rd to 1st in the block and lands where the target's `addu $s3,$v0,$zero` sits. y1 (def 2nd) measures exactly in between at 21. z20 re-measures 20 (identical text). y6/y7 confirm s33's PLUS flip is still load-bearing (23 / 22).
- verdict: CONFIRMED - new floor 20

## [s34] The register residual ($17 next-address / $18 cxs / $19 arg0 instead of $17 cxs / $18 arg0 / $19 next-index) is a global-allocno ranking problem.
- mechanism: s32/s33 attributed the seats to allocno_compare ordering among global allocnos.
- probe: read text1b.lreg's per-pseudo lines for the whole crossing set, then measure z2 (address selected in the wrap arm, no `off` pseudo, so the address becomes a multi-block/global allocno) and w1 (the same construct with the `(s16)` cast).
- result: WRONG PASS. The seats are decided in LOCAL-alloc: our next-neighbour address is a block-local call-crossing quantity (`Register 88 used 3 times across 4 insns in block 4; crosses 1 call`), so local-alloc seats it in $17 before the centre twins are placed, pushing cxs to $18 and leaving arg0 only $19. The target has no block-local call-crossing quantity at all (its next INDEX is defined in two blocks and used in a third). z2 removes ours and reproduces the target's $16 cys / $17 cxs EXACTLY at no instruction cost - but the now-global address carries four references, outranks arg0, takes $18, and displaces the vertex-table base from $6 to $4; net 21, one point worse than y5.
- verdict: KILLED as stated; replaced by the corrected local-alloc attribution (evidence s34-E4/E5)

## [s34] The PLUS/multiply operand-order flip that bought 24 -> 22 in s33 has unexploited twins at the other two-operand sites (s33 frontier hypothesis #2).
- mechanism: the .greg dump records one copy-preference line per tied pseudo; any tie whose partner is a callee-save allocno is a candidate for the same source-level flip.
- probe: w6 `base = 0x800 + ang_prev`; w7 `scale = 40 * arg0[2]`; w8 both final adds written `((...) >> 12) + cx/cy`; w5 `ang_mid = ang_prev + ((s32)(ang_next - ang_prev) / 2)`; w9 both final scale multiplies flipped.
- result: w6/w7/w8 all 22 (byte-inert), w5 23, w9 24. The only preference tie in this function worth anything is the one at the neighbour address whose result crosses the first call - and within the z2 regime even that one is worth nothing (z2b 21 = z2 21).
- verdict: KILLED

## [s34] The next-address allocno can be demoted below arg0 by LENGTHENING its live range (s33 frontier hypothesis #1), and that demotion yields the target's $16..$19 mapping.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:635) = floor_log2(n_refs)*n_refs/live_length; s33 varied n_refs and the def site but never pushed live_length past ~16 insns, leaving it the one unexploited free variable.
- probe: x1/x2/x3 hoist the tmp/off computation out of its basic block (before the prev-index if-block, before the vertex-table load, address after pi); y1 defines the offset via the target's own (s32)(tmp<<16)>>14 pair; y5 drops the redundant (s16) so the def is the block's first insn. Winner re-dumped with tmp/grind/func_80057CC8/run_dump.sh and read out of text1b.lreg / text1b.greg.
- result: The lever works and is worth nothing for the mapping. y5 lengthens the offset allocno from 16 to 19 insns (lreg: 'Register 104 used 3 times across 19 insns') and flips the printed global allocation order from '... 87 104 72 ...' to '... 87 72 104 ...', i.e. arg0 is now allocated FIRST. Dispositions unchanged (72 in 19, 104 in 17) because greg prints ';; 72 conflicts: ... 16 17 18 29' - arg0 has hard-reg conflicts on $16/$17/$18 and $19 is its only legal callee-save seat at any rank. Hoisting out of the block costs an insn: x1/x2/x3 = 38 at 109, w2/w3/w4 = 33 at 109.
- verdict: KILLED

## [s34] Dropping the redundant (s16) narrowing from the byte-offset expression is byte-neutral and therefore inert.
- mechanism: The wrap test '(s16) tmp >= (s32)arg0[3]' already forces the sll16/sra16 pair, so both spellings compile to the same three insns; only the position of the offset's defining insn inside the block changes.
- probe: y5 (off = tmp * 4), y1 (off = (s32)(tmp << 16) >> 14), z20 (off = tmp << 2), and y6/y7 as base-first-PLUS controls; all measured with sandbox --disable all.
- result: NOT inert. y5 = score 20 at build_insns 108 vs the s33 candidate's 22 at the same 108 insns, with byte-identical register assignment - the offset insn simply moves from 3rd to 1st in the block, landing where the target's 'addu $s3,$v0,$zero' sits (asm/funcs/func_80057CC8.s:29). y1 (def 2nd) measures exactly in between at 21. z20 re-measures 20. y6/y7 (23/22) confirm s33's PLUS flip is still load-bearing.
- verdict: CONFIRMED

## [s34] The register residual ($17 next-address / $18 cxs / $19 arg0 instead of the target's $17 cxs / $18 arg0 / $19 next-index) is a global-allocno ranking problem, as s32/s33 modelled it.
- mechanism: s32/s33 attributed all four callee-save seats to allocno_compare ordering among global allocnos.
- probe: Read text1b.lreg's per-pseudo n_refs/live_length lines for the entire call-crossing set, then measure z2 (address selected in the wrap arm, no off pseudo, so the address becomes a multi-block global allocno) and w1 (same construct with the (s16) cast).
- result: Wrong pass. The seats are decided in LOCAL-alloc: our next-neighbour address is a block-local call-crossing quantity ('Register 88 used 3 times across 4 insns in block 4; crosses 1 call'), so local-alloc seats it in $17 before the centre twins, pushing cxs to $18 and leaving arg0 only $19. The target has no block-local call-crossing quantity (its next INDEX spans three blocks). z2 removes ours and reproduces the target's $16 cys / $17 cxs EXACTLY at no instruction cost (108 insns; w1, the cast version, costs two insns and measures 25) - but the now-global 4-reference address outranks arg0, takes $18, and displaces the vertex base from $6 to $4. Net 21, one worse than y5.
- verdict: KILLED

## [s34] The PLUS/multiply operand-order flip that bought 24 -> 22 in s33 has unexploited twins at the other two-operand sites (s33 frontier hypothesis #2).
- mechanism: The .greg dump records one copy-preference line per tied pseudo; any tie whose partner is a callee-save allocno should be flippable at source level for free.
- probe: w6 base = 0x800 + ang_prev; w7 scale = 40 * arg0[2]; w8 both final adds as ((...) >> 12) + cx/cy; w5 ang_mid = ang_prev + ((s32)(ang_next - ang_prev) / 2); w9 both final scale multiplies flipped.
- result: w6/w7/w8 all 22 (byte-inert), w5 23, w9 24. The only tie in this function worth anything is the one at the neighbour address whose result crosses the first call; inside the z2 regime even that one is worth nothing (z2b 21 = z2 21).
- verdict: KILLED

## [s34] Re-ordering the two index if-blocks (next-index first) behaves differently on the ban-compliant 22-chassis than the 41 recorded by s32-E6/s33.
- mechanism: Chassis-relativity is asymmetric (s33-H5): some banked scores transfer, some do not.
- probe: z16 = y5 with the next-index block moved above the prev-index block.
- result: 41 at 106 insns - identical to the previously banked number. CSE fuses the two 'lbu 0x3($s2)' reads that the target keeps separate (asm/funcs/func_80057CC8.s:24 and :31). This one transfers.
- verdict: KILLED

## s35 (2026-08-27) - structural

### H1 - The address allocno can be demoted below arg0 by lengthening its live range (s34 frontier #1)
- statement: In the z2 regime (global, arm-selected next-neighbour ADDRESS - the only regime that
  reproduces the target's `$16 cys / $17 cxs` local-alloc result), moving the unconditional def of
  the address earlier lengthens its live_length, lowering its `allocno_compare` priority
  `floor_log2(4)*4/L = 8/L` below arg0's fixed `2*5/54 = 0.185`, so global-alloc would seat arg0
  in $18 and the address in $19 - the target's exact mapping.
- mechanism: global.c:635 allocno_compare; live_length and n_refs are printed per pseudo in the
  .lreg dump, and the resulting order + dispositions in .greg.
- probe: a1 (address def immediately after the single `lw 0x4(arg0)` at the top of block 0, wrap
  arm unchanged) and a2 (def below the centre reads but above the prev-index if); both dumped with
  `pwsh tools/grinder/dump.ps1 func_80057CC8` and read at tmp/grind/func_80057CC8/s35/a1.{lreg,greg}.
- result: The lever works and the outcome does not. live_length goes 21 (z2) -> 29 (a1), priority
  0.381 -> 0.276, but the threshold is 43.2 and 29 is the MAXIMUM the value can attain: its def is
  already at the earliest point at which the address can exist (it needs `table`) and its last use
  is the second `lh`. greg still prints `88 in 18 / 72 in 19`. Scores 29 / 29 (v0 = 20).
- verdict: KILLED

### H2 - The address allocno can be dropped to three references
- statement: Three refs would give `floor_log2(3)*3/21 = 0.143 < 0.185` and hand arg0 the $18 seat
  at the z2 live_length, without any hoisting.
- mechanism: same allocno_compare ranking; n_refs enters both as the log factor and the numerator.
- probe: enumeration of the reference sources rather than a measurement - a wrap-selected value has
  two defs by construction (straight-line def + arm def, or two if/else arms - a3 confirms the
  if/else spelling merely costs an insn, 25 at 109), and a two-coordinate vertex read has two uses
  by construction (`next_vert[0]`, `next_vert[1]`). Any copy introduced to absorb one use is
  propagated away before flow counts refs - d1 demonstrates the propagation directly (`off = tmp`
  in the arms vanishes; still 108 insns, score 27).
- result: 2 defs + 2 uses = 4 references with no free variable. Not spellable.
- verdict: KILLED

### H3 - arg0's own priority can be raised instead
- statement: Rather than demoting the address, raise arg0 above 8/29 = 0.276 by shortening its live
  range or adding references.
- mechanism: allocno_compare again; arg0 is 5 refs / 54 insns.
- probe: arithmetic on the measured inputs plus c6 (`scale = arg0[2] * 40` hoisted above the calls,
  the only statement that can shorten arg0's live range).
- result: Six references give 0.222 and seven give 0.259, both below 0.276, and the sixth reference
  is the banned second `lw` anyway; eight would be needed for the next log step. Shortening the
  range past L = 36 requires arg0 to die before the first call, which stops it being a callee-save
  candidate at all - c6 measures exactly that (arg0 allocated `$8`, score 32).
- verdict: KILLED

### H4 - Slot alignment inside block 4 is worth points (s34 frontier #2)
- statement: The remaining ~20 points are mostly positional, so re-ordering statements to move an
  insn into the target's slot should pay as y5 did in s34.
- mechanism: the engine metric scores per-instruction text agreement, so a slot move can pay with
  no register or count change.
- probe: normalised diff of the 20-form against target (s31/norm2.py), then e1/e2 (`cxs`/`cys` as
  named locals, so the sign-extension pair is emitted before the address formation), c3 (`scale`
  inlined at both uses), c9 (next-neighbour coordinate DIFFERENCES computed before the first call),
  d1/d2 (index carried in the arms, scale performed in the merge block, plain and with the target's
  own `sll 16 / sra 14` idiom).
- result: The premise is wrong. The diff has eighteen lines, twelve of which are pure register
  renames all caused by arg0 sitting in $19, and six of which are the banned-duplication block;
  no other insn in the function differs. Every re-ordering probe was worse: e1 22, e2 28, c3 34,
  c9 42, d1 27, d2 28.
- verdict: KILLED (as stated; the only genuinely loose slots left are inside the six-line
  duplication block, which is the refused construct)

## [s35] In the z2 regime (global, arm-selected next-neighbour ADDRESS - the only regime that reproduces the target's $16 cys / $17 cxs local-alloc result), moving the unconditional def of the address earlier lengthens its live_length, lowering its allocno_compare priority floor_log2(4)*4/L = 8/L below arg0's fixed 2*5/54 = 0.185, so global-alloc seats arg0 in $18 and the address in $19 - the target's exact mapping. (This is the s34 frontier hypothesis #1 carried into this session.)
- mechanism: tools/gcc-2.7.2/global.c:635 allocno_compare ranks allocnos by floor_log2(n_refs)*n_refs/live_length; both inputs are printed per pseudo in the .lreg dump and the resulting allocation order plus dispositions in .greg.
- probe: Forms a1 (address def placed immediately after the single `lw 0x4(arg0)` at the top of block 0, wrap arm unchanged) and a2 (def below the centre reads, above the prev-index if). Built with tmp/grind/func_80057CC8/s35/gen.sh, scored with `sandbox func_80057CC8 --disable all`, and dumped with `pwsh tools/grinder/dump.ps1 func_80057CC8`; dumps copied to tmp/grind/func_80057CC8/s35/a1.lreg and a1.greg.
- result: The lever works and the outcome does not. The address allocno's live_length goes 21 (z2) -> 29 (a1), priority 0.381 -> 0.276 - but the threshold is 43.2, and 29 is the MAXIMUM the value can attain, because its def already sits at the earliest point at which the address can exist (it needs `table`, i.e. the single lw) and its last use is the second `lh`. a1.greg still prints `88 in 18 / 72 in 19`. Scores: a1 29, a2 29, against v0 = 20 - the hoist buys the ranking input and still loses the seat, at 8 points of positional collateral.
- verdict: KILLED

## [s35] The next-neighbour address allocno can be dropped to THREE references, which at the z2 live_length gives floor_log2(3)*3/21 = 0.143 < arg0's 0.185 and hands arg0 the $18 seat with no hoisting at all.
- mechanism: Same allocno_compare ranking; n_refs enters both as the log factor and as the numerator, so 4 -> 3 refs is worth more than any achievable live_length change.
- probe: Enumeration of the value's reference sources, cross-checked by measurement: a3 spells the wrap selection as if/else instead of straight-line-def-then-arm (does the def count change?), and d1 introduces an `off = tmp` copy in the arms to absorb one of the two uses.
- result: Not spellable. A wrap-selected value has two defs by construction (a3's if/else merely costs an instruction: 25 at 109 insns) and a two-coordinate vertex read has two uses by construction (`next_vert[0]`, `next_vert[1]`). Any copy introduced to absorb a use is propagated away before flow counts references - d1 demonstrates it directly: the `off = tmp` copy vanishes, the form is still 108 insns and does NOT reproduce the target's `addu $19,$2,$0`, and it scores 27. 2 defs + 2 uses = 4 references with no free variable.
- verdict: KILLED

## [s35] Rather than demoting the address allocno, raise arg0's own priority above 8/29 = 0.276 - either by adding references or by shortening its live range.
- mechanism: allocno_compare on arg0's measured inputs (5 refs / 54 insns, from tmp/grind/func_80057CC8/dumps/text1b.lreg).
- probe: Arithmetic on the measured inputs, plus c6 - hoisting `scale = arg0[2] * 40` above the two ratan2 calls, which is the only statement in the function that can shorten arg0's live range.
- result: Foreclosed from both sides. Six references give 2*6/54 = 0.222 and seven give 0.259, both below 0.276 (eight would be needed for the next floor_log2 step), and the sixth reference is the banned second `lw 0x4(arg0)` anyway. Shortening the live range below L = 36 requires arg0 to die before the first call, which stops it being a callee-save candidate at all - c6 measures exactly that: arg0 is allocated `$8` (`addu $8,$4,$zero`, tmp/grind/func_80057CC8/s35/c6.hon.s line 6) and the score goes 20 -> 32.
- verdict: KILLED

## [s35] The remaining ~20 points are mostly positional (s34 frontier hypothesis #2), so statement re-ordering that moves an instruction into the target's slot should pay again the way s34's y5 did.
- mechanism: The engine metric scores per-instruction text agreement, so a slot move can pay with no register change and no instruction-count change - which is how s34 went 22 -> 20.
- probe: Full normalised diff of the 20-form against target (`python3 tmp/grind/func_80057CC8/s31/norm2.py asm/funcs/func_80057CC8.s tmp/grind/func_80057CC8/s35/v0.hon.s`), then six re-ordering forms: e1/e2 (`cxs`/`cys` as named locals so the sign-extension pair is emitted before the address formation, at two different positions), c3 (`scale` inlined at both use sites), c9 (next-neighbour coordinate DIFFERENCES computed before the first call so two values cross instead of the address), d1/d2 (index carried in the arms with the scale performed in the merge block, plain and with the target's own `sll 16 / sra 14` idiom).
- result: The premise is wrong. The diff is 115 target lines vs 113 ours with exactly EIGHTEEN differing lines: twelve are pure register renames, every one of them a consequence of arg0 sitting in $19 instead of $18 (prologue save+copy pair, the save-order slot, `lw $6,4(arg0)`, both `lbu 3(arg0)`, the late `lbu 2(arg0)`, the cxs sll/sra pair, both `subu $4,$4,cxs`, the arm's `addu ...,$0,$0`), and the other six are the banned-duplication block itself. No other instruction in the function differs - entry block, prev-index block, wrap-compare block, both ang_mid arms, both scale multiplies and both stores already match modulo register names. Every re-ordering probe measured worse: e1 22, e2 28, c3 34 at 112 insns, c9 42, d1 27, d2 28 at 109 insns.
- verdict: KILLED

## [s36] The one untested cell of s30b's rematerialization analysis - REG_EQUIV note PRESENT and register pressure PRESENT simultaneously - is constructible, and in it reload emits the target's second `lw 0x4($s2)` from a SINGLE source-level read of the vertex-table base, dissolving the 2026-07-20 refused-duplication residual entirely.
- mechanism: tools/gcc-2.7.2/local-alloc.c update_equiv_regs is the only producer of REG_EQUIV MEM notes in GCC 2.7.2 (there is no gcse.c and no rematerialization pass); with the note in place reload substitutes the equivalent MEM instead of a stack slot when the pseudo fails allocation ("if no registers are available, reload will substitute the equivalence", local-alloc.c:1040-1041), and the note additionally DOUBLES the pseudo's live_length for global-alloc ranking (`reg_live_length[regno] *= 2`, local-alloc.c:1063). s30b proved both note gates are C-level reachable (gate 1 via `*(s16 *const *)` setting RTX_UNCHANGING_P; gate 2 by confining every base reference to one basic block) but had never combined the note with the nine-values-against-eight-seats pressure that s33-E5 measured in the target-shaped, post-call-address regime.
- probe: Both horns built on the score-20 candidate and measured with `sandbox func_80057CC8 --disable all`, each dumped with `pwsh tools/grinder/dump.ps1 func_80057CC8` and the func's section sliced out of text1b.lreg. r1 (tmp/grind/func_80057CC8/s36/r1.c): both index if-blocks first, then ONE block holding the const-qualified base read, the centre coordinate reads and both ratan2 calls, with the next-neighbour address formed AFTER the first call from the carried index - the target's own regime. r4 (s36/r4.c): the score-20 candidate verbatim with only the base read const-qualified, i.e. the target's shape with gate 1 satisfied.
- result: Not constructible - the two ingredients are mutually exclusive. r1 DOES earn the note (`tmp/grind/func_80057CC8/s36/r1.lreg:384`, `(expr_list:REG_EQUIV (mem/u:SI (plus:SI (reg/v:SI 72) (const_int 4))))`, both gates passed) and still emits exactly ONE `lw 4(s4)` with the base in callee-save `$s0`: score 60 at 102 insns. The reason is structural - confining the base's references to one basic block also puts the centre `lhu` in the same block as its `(s16)` cast, combine folds the pair into a single `lh` (r1.hon.s lines 34/36), the raw-u16 centre values leave the live set, and the crossing count falls from the nine the pressure argument needs to seven against eight seats, so nothing fails allocation. r4 shows the other horn: in the target's shape the base's references straddle the entry block and the post-wrap block, gate 2 (`reg_basic_block >= 0`) fails, its .lreg carries ZERO REG_EQUIV notes, and the const qualifier is not even codegen-inert here - it costs three points (score 23 at 108 insns). The note needs one basic block; the pressure needs the `lhu` and its cast in two. Additionally the wrap-selected next-neighbour ADDRESS can never carry a note at all (update_equiv_regs bails at `reg_n_sets[regno] != 1`, local-alloc.c:1019-1021, and its SET_SRC is a PLUS not a MEM), so the live_length doubling is unreachable for the one pseudo whose ranking s35-E2 needed to move.
- verdict: KILLED

## [s36] The wrapped next index carried through the arms as a NARROWER type (s16/u16) gives the arm a genuine sub-word def that copy-propagation cannot fold through the sign-extension, reproducing the target's `addu $19,$2,$0` / `addu $19,$0,$0` arm pair at zero instruction cost - the way s34's y5 recovered two points. (This is the s35 frontier item #2 carried into this session.)
- mechanism: RTL-expand emission order inside a basic block plus copy-propagation's inability to fold a def whose mode differs from its use; s35's d1/d2 had the same-width `off = tmp` copy propagated away before flow counted references, leaving 108 insns and no arm copy.
- probe: r2 (`s16 ni = tmp; if (...) ni = 0; next_vert = (s16 *)(ni * 4 + (s32)table);`) and r3 (identical with `u16 ni`), both built on the score-20 candidate, measured with `sandbox func_80057CC8 --disable all`, r3 disassembled to tmp/grind/func_80057CC8/s36/r3.hon.s.
- result: The mechanism fires and the arithmetic still loses. r3 emits exactly the target's arm pair - `move s0,v0` at r3.hon.s:27 and `move s0,zero` at :34, matching `addu $s3,$v0,$zero` / `addu $s3,$zero,$zero` at asm/funcs/func_80057CC8.s:29/37 - and even lands the vertex base in the target's call-clobbered `$a2`. But the narrow carrier must be re-widened before the address add (`andi s0,s0,0xffff` + `sll s0,s0,0x2` for u16; the s16 form folds to the same cost), which puts the form at 110 insns, and the address is still formed BEFORE the call rather than after it. Both spellings measure 26 against the candidate's 20.
- verdict: KILLED

## [s36] The score-20 residual is two independent problems - twelve foreclosed register renames sitting alongside six policy-refused lines - so the register half might still yield to a lever the duplication half does not need. (This is the s35-E3 reading, carried into this session as the implicit frame for further register work.)
- mechanism: s35-E1's normalised diff attributed twelve of the eighteen differing lines to arg0 occupying $19 instead of $18, and s35-E2 foreclosed that seat by allocno_compare arithmetic - which left open the reading that a different lever might reach the seat without touching the duplication.
- probe: Direct reading of the target's own register assignment and liveness (asm/funcs/func_80057CC8.s:2-56) against the closed mechanism set: rematerialization (killed above, s36-E1), caller-saving (killed by s30's `CALLER_SAVE_PROFITABLE` arithmetic, `4 * calls < refs` with 1 call and 3-4 refs), and a ninth callee-save seat (measured dead at 33 by s33-E5).
- result: The premise is wrong - the eighteen lines are ONE residual. The target seats eight callee-saves ($s0=cys $s1=cxs $s2=arg0 $s3=next-INDEX $s4=cx-raw $s5=cy-raw $s6=arg2 $s7=arg3) and deliberately does NOT give the vertex base a seat: it holds the base in call-clobbered $a2 (asm/funcs/func_80057CC8.s:17), lets it die before the jal, and re-creates it with a fresh `lw $a0,0x4($s2)` at :50. arg0 can occupy $s2/$18 only if the next-neighbour address is not live across the first call, which requires post-call address formation, which requires the base to be available after the call, which - with rematerialization and caller-saving both closed - requires either a ninth callee-save seat (33) or the second source-level materialization the owner refused on 2026-07-20. The register-seat axis and the duplication axis are the same request; no ban-compliant lever exists that buys one without the other.
- verdict: KILLED

## [s36] The one untested cell of s30b's rematerialization analysis - REG_EQUIV note PRESENT and register pressure PRESENT simultaneously - is constructible, and in it reload emits the target's second `lw 0x4($s2)` from a SINGLE source-level read of the vertex-table base, dissolving the 2026-07-20 refused-duplication residual entirely.
- mechanism: tools/gcc-2.7.2/local-alloc.c update_equiv_regs is the only producer of REG_EQUIV MEM notes in GCC 2.7.2 (no gcse.c, no rematerialization pass); with the note in place reload substitutes the equivalent MEM rather than a stack slot when the pseudo fails allocation (local-alloc.c:1040-1041), and the note also DOUBLES the pseudo's live_length for global-alloc ranking (reg_live_length[regno] *= 2, local-alloc.c:1063). s30b proved both note gates are C-level reachable (gate 1 via *(s16 *const *) setting RTX_UNCHANGING_P; gate 2 by confining every base reference to one basic block) but never combined the note with the nine-values-against-eight-seats pressure s33-E5 measured in the target-shaped post-call-address regime.
- probe: Both horns built on the score-20 candidate, measured with `sandbox func_80057CC8 --disable all`, each dumped with `pwsh tools/grinder/dump.ps1 func_80057CC8` and the function's section sliced out of text1b.lreg. r1 (tmp/grind/func_80057CC8/s36/r1.c): both index if-blocks first, then ONE basic block holding the const-qualified base read, the centre coordinate reads and both ratan2 calls, with the next-neighbour address formed AFTER the first call from the carried index - the target's own regime. r4 (s36/r4.c): the score-20 candidate verbatim with only the base read const-qualified, i.e. the target's shape with gate 1 satisfied.
- result: Not constructible - the two ingredients are mutually exclusive. r1 DOES earn the note (tmp/grind/func_80057CC8/s36/r1.lreg:384, `(expr_list:REG_EQUIV (mem/u:SI (plus:SI (reg/v:SI 72) (const_int 4))))`, both gates passed) and still emits exactly ONE `lw 4(s4)` with the base in callee-save $s0: score 60 at 102 insns. Reason is structural - confining the base's references to one basic block also puts the centre `lhu` in the same block as its (s16) cast, combine folds the pair to a single `lh` (r1.hon.s lines 34/36), the raw-u16 centre values leave the live set, and the crossing count falls from the nine the pressure argument needs to seven against eight seats, so nothing fails allocation. r4 shows the other horn: in the target's shape the base's references straddle the entry block and the post-wrap block, gate 2 (reg_basic_block >= 0) fails, its .lreg carries ZERO REG_EQUIV notes, and the const qualifier is not even codegen-inert here - it costs three points (score 23 at 108 insns). The note needs one basic block; the pressure needs the lhu and its cast in two. Additionally the wrap-selected next-neighbour ADDRESS can never carry a note at all (update_equiv_regs bails at reg_n_sets[regno] != 1, local-alloc.c:1019-1021, and its SET_SRC is a PLUS not a MEM), so the live_length doubling is unreachable for the one pseudo whose ranking s35-E2 needed to move.
- verdict: KILLED

## [s36] The wrapped next index carried through the arms as a NARROWER type (s16/u16) gives the arm a genuine sub-word def that copy-propagation cannot fold through the sign-extension, reproducing the target's `addu $19,$2,$0` / `addu $19,$0,$0` arm pair at zero instruction cost the way s34's y5 recovered two points. (s35 frontier item #2.)
- mechanism: RTL-expand emission order inside a basic block plus copy-propagation's inability to fold a def whose mode differs from its use; s35's d1/d2 had the same-width `off = tmp` copy propagated away before flow counted references, leaving 108 insns and no arm copy.
- probe: r2 (`s16 ni = tmp; if (...) ni = 0; next_vert = (s16 *)(ni * 4 + (s32)table);`) and r3 (identical with `u16 ni`), both built on the score-20 candidate, measured with `sandbox func_80057CC8 --disable all`; r3 disassembled to tmp/grind/func_80057CC8/s36/r3.hon.s.
- result: The mechanism fires and the arithmetic still loses. r3 emits exactly the target's arm pair - `move s0,v0` at r3.hon.s:27 and `move s0,zero` at :34, matching `addu $s3,$v0,$zero` / `addu $s3,$zero,$zero` at asm/funcs/func_80057CC8.s:29/37 - and even lands the vertex base in the target's call-clobbered $a2. But the narrow carrier must be re-widened before the address add (`andi s0,s0,0xffff` + `sll s0,s0,0x2`), putting the form at 110 insns, and the address is still formed BEFORE the call rather than after it. Both spellings measure 26 against the candidate's 20.
- verdict: KILLED

## [s36] The score-20 residual is two independent problems - twelve foreclosed register renames sitting alongside six policy-refused lines - so the register half might still yield to a lever the duplication half does not need. (The s35-E3 reading, carried into this session as the implicit frame for further register work.)
- mechanism: s35-E1's normalised diff attributed twelve of the eighteen differing lines to arg0 occupying $19 instead of $18, and s35-E2 foreclosed that seat by allocno_compare arithmetic - leaving open the reading that a different lever might reach the seat without touching the duplication.
- probe: Direct reading of the target's own register assignment and liveness (asm/funcs/func_80057CC8.s:2-56) against the now-closed mechanism set: rematerialization (killed above), caller-saving (killed by s30's CALLER_SAVE_PROFITABLE arithmetic, 4*calls < refs with 1 call and 3-4 refs), and a ninth callee-save seat (measured dead at 33 by s33-E5). Cross-checked against the candidate's own frame: the score-20 form already saves all eight of $s0-$s7 (tmp/grind/func_80057CC8/s36/v0.hon.s), so the frame and save count already match the target and the diff is naming only.
- result: The premise is wrong - the eighteen lines are ONE residual. The target seats eight callee-saves ($s0=cys $s1=cxs $s2=arg0 $s3=next-INDEX $s4=cx-raw $s5=cy-raw $s6=arg2 $s7=arg3) and deliberately does NOT give the vertex base a seat: it holds the base in call-clobbered $a2 (asm/funcs/func_80057CC8.s:17), lets it die before the jal, and re-creates it with a fresh `lw $a0,0x4($s2)` at :50. arg0 can occupy $s2/$18 only if the next-neighbour address is not live across the first call, which requires post-call address formation, which requires the base to be available after the call, which - with rematerialization and caller-saving both closed - requires either a ninth callee-save seat (33) or the second source-level materialization refused on 2026-07-20. The register-seat axis and the duplication axis are the same request; no ban-compliant lever buys one without the other.
- verdict: KILLED


## [s37] A permuter campaign seeded from the CURRENT 108-insn candidate finds a ban-compliant arrangement scoring below 20 - or, failing to across several fresh-seed windows, supplies the independent confirmation the ledger lacks that 20 is the ban-compliant minimum on this chassis. (s36 frontier item #1.)
- mechanism: The four prior campaigns (s4, s5, s13) all ran against the RETIRED 111-insn / floor-3 chassis and converged on the pointer-alias-holder cheat basin; the 108-insn regime discovered in s31-s34 (single base materialisation, arm-selected byte offset, PLUS-operand flip, unnarrowed offset) is a structurally different search space that had never been permuted.
- probe: Workspace rebuilt from the s4 recipe at tmp/grind/func_80057CC8/s37/perm with two chassis corrections (`-mel` added to cc1; the retired `fix_lwl` stage removed; the `.align 3 -> 2` sed dropped because text1b is not in RODATA_ALIGN2_FILES). Sanity gate: base 108 insns / target 111. Campaign `108regime-s37` launched with `permuter_campaign.py launch -j 6`, waited in-turn across six `wait --dir` windows, harvested with `--stop`. 45,270 iterations, permuter base_score 783, eleven novel finds in the 545-768 band, no score-0.
- result: CONFIRMED as a lever, in the direction the frontier hoped for but from an unexpected member of the find set. The best-permuter-scoring finds are semantically broken (output-545-1 leaves next_vert uninitialised on the wrap fall-through; output-615-1 reads a never-assigned ang_next), but the semantics-preserving `next_vert = &Judge` family (output-758-2 / 768-1 / 768-2) is real: 768-2 measures engine score 17, and bisecting its three mutations isolates a single statement worth FOUR points. Floor 20 -> 16 at 108 insns. The campaign also discharges the negative half of the hypothesis: 45k iterations produced no score-0 and nothing else competitive.
- verdict: CONFIRMED

## [s37] The four-point gain from `next_vert = &Judge;` is the pointer alias to the sine table, and therefore has an annotation-free spelling as a fresh pointer local (the shape src/code6cac.c:2395/2424 already ships un-annotated on main).
- mechanism: If the gain came from holding `&Judge` in a register across the two store sites, any local of the right type would buy it, and a fresh local is ordinary C requiring no family claim.
- probe: Four measurements against the score-20 candidate, all built by tmp/grind/func_80057CC8/s37 and scored with `sandbox func_80057CC8 --disable all`: d.c (fresh `s16 *jt;` assigned after `scale`, both sites spelled `*(jt + ...)`), e.c (same with array-index spelling `jt[...]`), f.c (fresh local initialised at its declaration), h.c (reuse of `table`, also dead at that point, instead of `next_vert`). Cross-checked against i.c/j.c (reuse of `next_vert` consumed at only ONE site) and g.c (reuse placed before `scale`).
- result: KILLED, decisively. d.c 20, e.c 20, h.c 20, f.c 48 at 110 insns - no fresh local and no other dead local buys anything. But i.c 16, j.c 16 and g.c 16 - the reuse of `next_vert` pays regardless of which site consumes it or where the statement sits. The gain is the extra SET of the next_vert pseudo, not the pointer alias. There is therefore NO annotation-free spelling of this lever, and the code6cac.c judge_ptr precedent does not cover it (that is a fresh local, which measures 20 here).
- verdict: KILLED

## [s37] The mechanism of the extra SET is the s34-E4 local-alloc attribution: a second def in the FINAL basic block promotes the next-neighbour pseudo out of local-alloc's block-local set, so local-alloc no longer pre-seats it and the two centre twins reach the target's $s0/$s1.
- mechanism: tools/gcc-2.7.2/local-alloc.c seats block-local quantities before global-alloc runs; a pseudo whose references span more than one basic block loses the `in block N` tag in the .lreg dump and is left to global.c. s34-E4 INFERRED this attribution from the dump; it had never been tested by intervention.
- probe: `pwsh tools/grinder/dump.ps1 func_80057CC8` with the 16-form applied, function section of tmp/grind/func_80057CC8/dumps/text1b.lreg (begins line 15487); honest disassembly at tmp/grind/func_80057CC8/s37/b.hon.s.
- result: CONFIRMED. lreg now prints `Register 88 used 6 times across 4 insns; crosses 1 call; GR_REGS or none; pointer.` with NO block tag, where the score-20 form printed `used 3 times across 4 insns in block 4`. The predicted consequence is exactly what the bytes show: cxs moves out of $s2 into $s1 and the pair now matches the target's `$s0 cys / $s1 cxs` (asm/funcs/func_80057CC8.s:41-44). Seven of the eight callee-save seats now agree; only $s2/$s3 differ.
- verdict: CONFIRMED

## [s37] With the local-alloc pre-seating removed, the remaining $s2/$s3 swap (ours: address in $s2, arg0 in $s3; target: arg0 in $s2, next-INDEX in $s3) becomes reachable, because the address is now ranked by global-alloc rather than pre-placed.
- mechanism: global.c:635 allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length; whichever of the address and arg0 ranks first takes the first free callee-save seat.
- probe: Arithmetic on the fresh lreg numbers, cross-checked against the one-site-reuse variants that reduce the address's reference count.
- result: KILLED. arg0 is `Register 72 used 5 times across 54 insns; crosses 2 calls` -> 2*5/54 = 0.185. The address (pseudo 88) is 6 refs across 4 insns -> 2*6/4 = 3.0; the one-site reuse drops it to 5 refs -> 2.5. Its live_length stays at 4 because its two live ranges are disjoint and short, so no reference-count spelling can push it below arg0 - the gap is an order of magnitude, not a tie. The address can only stop competing for a callee-save seat by not crossing the call, which requires post-call address formation, which requires the vertex-table base after the call; rematerialization (s36-E1), caller-saving (s30) and a ninth callee-save seat (s33-E5, 33) are all closed, leaving only the second source-level materialization refused on 2026-07-20. The $s2/$s3 residual is the same policy question as before, now with seven of eight seats matched instead of five.
- verdict: KILLED

## [s37] A permuter campaign seeded from the CURRENT 108-insn candidate finds a ban-compliant arrangement scoring below 20 - or, failing to across several fresh-seed windows, supplies the independent confirmation the ledger lacks that 20 is the ban-compliant minimum on this chassis. (s36 frontier item #1.)
- mechanism: The four prior campaigns (s4, s5, s13) all ran against the RETIRED 111-insn / floor-3 chassis and converged on the pointer-alias-holder cheat basin; the 108-insn regime discovered in s31-s34 (single base materialisation, arm-selected byte offset, PLUS-operand flip, unnarrowed offset) is a structurally different search space that had never been permuted.
- probe: Workspace rebuilt from the s4 recipe at tmp/grind/func_80057CC8/s37/perm with two chassis corrections (-mel added to cc1; the retired fix_lwl stage removed; the .align 3->2 sed dropped because text1b is not in RODATA_ALIGN2_FILES). Sanity gate: base 108 insns / target 111. Campaign 108regime-s37 launched with permuter_campaign.py launch -j 6, waited in-turn across six `wait --dir` windows, harvested with --stop inside the session.
- result: 45,270 iterations, permuter base_score 783, eleven novel finds in the 545-768 band, no score-0. The best-permuter-scoring finds are SEMANTICALLY BROKEN (output-545-1 leaves next_vert uninitialised on the wrap fall-through; output-615-1 reads a never-assigned ang_next). The semantics-preserving `next_vert = &Judge` family (758-2 / 768-1 / 768-2) is real: 768-2 measures engine score 17, and bisecting its three mutations isolates a single statement worth FOUR points. Floor 20 -> 16 at 108 insns.
- verdict: CONFIRMED

## [s37] The four-point gain from `next_vert = &Judge;` is the pointer alias to the sine table, and therefore has an annotation-free spelling as a fresh pointer local (the shape src/code6cac.c:2395/2424 already ships un-annotated on main).
- mechanism: If the gain came from holding &Judge in a register across the two store sites, any local of the right type would buy it, and a fresh local is ordinary C requiring no family claim.
- probe: Four measurements against the score-20 candidate, all scored with `sandbox func_80057CC8 --disable all`: d.c (fresh `s16 *jt;` assigned after `scale`, both sites `*(jt + ...)`), e.c (same with array-index spelling), f.c (fresh local initialised at declaration), h.c (reuse of `table`, also dead there, instead of next_vert). Cross-checked against i.c/j.c (reuse consumed at only ONE site) and g.c (reuse placed before `scale`).
- result: d.c 20, e.c 20, h.c 20, f.c 48 at 110 insns - no fresh local and no other dead local buys anything. But i.c 16, j.c 16, g.c 16 - the reuse of next_vert pays regardless of which site consumes it or where the statement sits. The gain is the extra SET of the next_vert pseudo, not the pointer alias. There is NO annotation-free spelling of this lever, and the code6cac.c judge_ptr precedent does not cover it (that is a fresh local, which measures 20 here).
- verdict: KILLED

## [s37] The mechanism of the extra SET is the s34-E4 local-alloc attribution: a second def in the FINAL basic block promotes the next-neighbour pseudo out of local-alloc's block-local set, so local-alloc no longer pre-seats it and the two centre twins reach the target's $s0/$s1.
- mechanism: tools/gcc-2.7.2/local-alloc.c seats block-local quantities before global-alloc runs; a pseudo whose references span more than one basic block loses the `in block N` tag in the .lreg dump and is left to global.c. s34-E4 INFERRED this attribution from the dump; it had never been tested by intervention.
- probe: `pwsh tools/grinder/dump.ps1 func_80057CC8` with the 16-form applied; function section of tmp/grind/func_80057CC8/dumps/text1b.lreg (begins line 15487); honest disassembly at tmp/grind/func_80057CC8/s37/b.hon.s.
- result: lreg now prints `Register 88 used 6 times across 4 insns; crosses 1 call; GR_REGS or none; pointer.` with NO block tag, where the score-20 form printed `used 3 times across 4 insns in block 4`. The predicted consequence is what the bytes show: cxs moves out of $s2 into $s1 and the pair now matches the target's $s0 cys / $s1 cxs (asm/funcs/func_80057CC8.s:41-44). Seven of eight callee-save seats now agree; only $s2/$s3 differ.
- verdict: CONFIRMED

## [s37] With the local-alloc pre-seating removed, the remaining $s2/$s3 swap (ours: address in $s2, arg0 in $s3; target: arg0 in $s2, next-INDEX in $s3) becomes reachable, because the address is now ranked by global-alloc rather than pre-placed.
- mechanism: global.c:635 allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length; whichever of the address and arg0 ranks first takes the first free callee-save seat.
- probe: Arithmetic on the fresh lreg numbers, cross-checked against the one-site-reuse variants that reduce the address's reference count.
- result: arg0 is `Register 72 used 5 times across 54 insns; crosses 2 calls` -> 2*5/54 = 0.185. The address (pseudo 88) is 6 refs across 4 insns -> 2*6/4 = 3.0; the one-site reuse drops it to 5 refs -> 2.5. Its live_length stays at 4 because its two live ranges are disjoint and short, so no reference-count spelling pushes it below arg0 - the gap is an order of magnitude, not a tie. The address stops competing for a callee-save seat only by not crossing the call, which requires post-call address formation, which requires the vertex-table base after the call; rematerialization (s36-E1), caller-saving (s30) and a ninth callee-save seat (s33-E5, 33) are all closed, leaving only the 2026-07-20 refused second source-level materialization.
- verdict: KILLED


## [s38] The extra-SET lever generalises: another block-local call-crossing pseudo in the 16-form's .lreg can be promoted out of local-alloc by a C-level second SET in a later basic block, changing which seat is free when arg0 is placed. (s37 frontier item #1.)
- mechanism: local-alloc.c seats block-local quantities before global.c runs; the `in block N` tag in the .lreg dump is the observable, and s37 proved by intervention that a C-level second SET of a variable in a later basic block removes it. Each promotion changes the seat order global.c:635 allocno_compare then works over.
- probe: Re-read the 16-form's .lreg (tmp/grind/func_80057CC8/dumps/text1b.lreg, function section from line 15487) and enumerate every pseudo carrying BOTH an `in block N` tag and `crosses N calls`; then identify each one's C-level subject by reading its RTL def in the same dump.
- result: KILLED - the lever has no remaining subject. Exactly two pseudos carry both tags: 119 (`used 3 times across 11 insns in block 4; crosses 1 call`) and 129 (`used 3 times across 8 insns in block 4; crosses 1 call`). Their RTL defs identify them as the SIGN-EXTENDED centre coordinates - insn 103 sets 119 to `(ashiftrt (ashift (subreg (reg/v:HI 83)) 16) 16)` with `REG_EQUAL (sign_extend (reg/v:HI 83))`, and insn 118 sets 129 to the identical shape over reg/v:HI 86 - i.e. (s16)cx and (s16)cy, which are ALREADY seated in the target's $s0/$s1. Promoting them could only move them off a correct seat. The third pseudo the frontier listed, 115, is the PREV-neighbour address (`plus (reg 114) (reg/v:SI 87)`) and carries no `crosses` tag at all. Both 119 and 129 are compiler temps (no `reg/v` tag), so there is in any case no C-level variable whose second SET would promote them.
- verdict: KILLED

## [s38] The $s2/$s3 seat order can be flipped by source-level statement placement, because allocno_compare's inputs (n_refs, live_length) are both functions of where the C writes the definitions: hoisting `scale = arg0[2] * 40;` shortens arg0's live_length and raises its priority, and hoisting the next-vertex address block lengthens pseudo 88's live_length and lowers its 3.0.
- mechanism: global.c:635 allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length; the s37 arithmetic is arg0 at 2*5/54 = 0.185 against the address pseudo at 2*6/4 = 3.0, and the whole gap is carried by the address's live_length of 4.
- probe: Three ordinary-C re-orderings of the 16-form, each measured with `sandbox func_80057CC8 --disable all`: a_scale_top.c (`scale = arg0[2] * 40;` moved to just after `table = *(s16 **)(arg0 + 4);`), b_nv_top.c (the whole `{ tmp / off / wrap / next_vert = ... }` block moved to the same place), c_both_top.c (both).
- result: KILLED, and badly - 47 @108, 39 @106, 54 @106 respectively against the 16-form's 16 @108. The b/c forms also fall to 106 instructions, two below the 108-insn regime and five below the target's 111, so they are a different and worse regime rather than a near miss. The mechanism of the failure is visible in the dump and settles the axis: pseudo 88's live_length of 4 is NOT a property of where the C writes the assignment. Its def is `(insn 86 ...)` - a low uid, emitted early - yet it sits in the stream between insn 124 and call_insn 126, because GCC 2.7.2 runs sched1 BEFORE local-alloc and the first scheduling pass has already sunk the address add into the slot immediately preceding the jal. Source-level hoisting gives the scheduler MORE slack to sink it, not less. The only constructs that would pin the add in place are scheduling barriers, a forbidden family.
- verdict: KILLED

## [s38] s33-E5's ninth-callee-save regime (next-neighbour address formed AFTER the first call) deserves re-measurement on the current chassis, because it was last measured two regimes ago (at 33) and before the s37 promotion lever existed; and - critically - it is BAN-COMPLIANT, since carrying the single `table` local across the call is one source-level materialization, not two.
- mechanism: If the address is formed after the call it never crosses the call, so it is not a callee-save candidate at all and cannot take the seat arg0 needs. The cost is that `table` and the wrap-selected byte offset must both cross instead, making nine crossing quantities against eight callee-save seats.
- probe: d_postcall_addr.c - the wrap arms compute only a function-scope `s32 off`, and `next_vert = (s16 *)(off + (s32)table);` is placed between the two ratan2 calls. There is NO second read of `*(s16 **)(arg0 + 4)` anywhere in the form. Control: e_postcall_addr_nolever.c, identical but with the `next_vert = &Judge;` statement removed and the sine-table reads spelled `*(&Judge + ...)`.
- result: KILLED, and it produced an unexpected second result. d measures 31 at 110 insns - the axis has barely moved from s33's 33 and is still 15 points worse than the 16-form. The two extra instructions are the post-call `sll`/`addu` that the pre-call form gets for free in the scheduler's jal-adjacent slot. The control e measures 31 at 110 insns TOO, i.e. an identical score: the `next_vert = &Judge;` lever is worth exactly ZERO points in this regime. That is a genuinely new fact about the lever - its four points come entirely from removing local-alloc's pre-seating of the PRE-CALL address pseudo, and once no such pseudo exists the statement is inert. Any future session evaluating a structurally different regime must re-measure the lever there rather than assume it carries.
- verdict: KILLED

## [s38] The 16-form's `next_vert = &Judge;` construct falls squarely inside one frozen SOTN-accepted family, so a self-vet can claim it and the ledger's citation of .claude/rules/defeat-licm-hoist-var-reuse.md is correct. (s37 frontier item #2, taken up as mandated.)
- mechanism: The frozen list carries "variable reuse for codegen control" and "C-level pointer alias to a global" as separate sanctioned families; the construct is a pointer alias to a global carried in a reused existing local, so it should be covered by one or the other.
- probe: Both rule files read end to end, plus .claude/rules/pointer-alias-fake-exception.md, checked against the construct and against the s37 negative controls; docs/reference/sotn-construct-index.md grepped for PSX-tagged (untagged) precedent of each half and of the composite.
- result: KILLED as stated, and the inherited citation is affirmatively WRONG. defeat-licm-hoist-var-reuse.md is explicitly loop-scoped - staged-value-reused-variable.md's Related section says verbatim "[[defeat-licm-hoist-var-reuse]] - variable reuse inside loops (a different, loop-scoped mechanism; do not cite it for straight-line code)" - and func_80057CC8 contains no loop, so citing it is the right-construct/wrong-citation layer-1 FAIL shape. The two genuinely adjacent families each cover one half and neither covers the composite. staged-value-reused-variable.md's scope ("a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order") is satisfied on bounds 2/3/5/6 - next_vert is an existing local with a real job, its previous value is provably dead at the staging point, and the exhaustion ledger is 38 sessions deep - but bound 1 is open, because the staged value is a compile-time CONSTANT ADDRESS rather than a loaded datum like SOTN's `i = *scriptCur++;`, and the rule's named mechanism is sched.c adjust_priority/birthing_insn_p whereas ours is local-alloc.c block-locality. pointer-alias-fake-exception.md's scope ("a local pointer that provides a second C handle to a global - where using the global directly would be semantically identical") describes the VALUE exactly, but its canonical shape is a FRESH local, and s37 measured a fresh local at 20 - so the alias half is provably not the lever. PSX-tagged precedent exists for each half separately (docs/reference/sotn-construct-index.md:51/:81/:92/:97/:109 for the reuse half; SOTN src/dra/cd.c:539 `new_var3 = &g_Cd;` quoted in pointer-alias-fake-exception.md for the alias half) and for the composite NOT AT ALL. Under the brief's own instruction - "Torn between two adjacent families? That is what ruling-request is for" - s38 returns a ruling-request instead of a family claim.
- verdict: KILLED

## [s38b] The s37 `next_vert = &Judge;` extra-SET lever is a general local-alloc promotion tool, so it should also pay in the OTHER address-formation regimes the ledger banked at 21-25 on the pre-lever chassis (s32/s33/s34 arm-selected-address family), potentially beating 16.
- mechanism: The lever's dump-verified mechanism is removal of the `in block N` tag from the address pseudo, promoting it out of local-alloc's pre-seated set. If a regime's address pseudo is also block-local, the same statement should promote it and re-rank the seats the same way.
- probe: The two best pre-lever arm-selected-address forms were re-spelled with the lever appended verbatim (`next_vert = &Judge;` after `scale = arg0[2] * 40;`, sine reads re-spelled `*(next_vert + ...)`) and measured on the live chassis: rejected/s38b-arm-address-basefirst-plus-lever-score21.c (from s34-address-in-arm-base-first-plus-score21.c) and rejected/s38b-arm-address-nocast-twins-plus-lever-score21.c (from s34-address-in-arm-no-cast-twins-at-16-17-score21.c).
- result: KILLED. Both measure 21 at 108 insns - IDENTICAL to their pre-lever scores of 21. The lever is worth exactly zero points in the arm-selected-address regime, just as s38 measured it worth exactly zero in the post-call-address regime (31 with, 31 without). It pays only in the merge-block-offset regime of the 16-form, where the address pseudo is genuinely block-local (`in block 4`). The lever is therefore NOT a general tool: it is a one-shot removal of one specific local-alloc pre-seating, and any future session applying it to a structurally different regime must re-measure rather than assume.
- verdict: KILLED

## [s38b] The score-16 residual is a SINGLE supernumerary live-across-call quantity, and the pre-call/post-call address-formation choice is a closed two-horn dilemma in which no ban-compliant form can occupy both horns.
- mechanism: global.c:635 allocno_compare ranks callee-save candidates by floor_log2(n_refs)*n_refs/live_length and hands out $s0.. in that order; the number of quantities crossing the call decides whether 8 seats suffice; sched1 runs before local-alloc and fixes the address pseudo's live_length.
- probe: (i) full normalised diff of the 16-form against target (tmp/grind/func_80057CC8/s38b/base16.hon.s vs asm/funcs/func_80057CC8.s via tmp/grind/func_80057CC8/s31/norm2.py); (ii) objdump of the post-call regime's sandbox object to read its ACTUAL register map for the first time (tmp/grind/func_80057CC8/s38b/d31.ins, from rejected/s38-postcall-address-from-carried-table-score31-110insns.c re-measured at 31 / 110 this session).
- result: CONFIRMED, and it sharpens every prior statement of the residual. (a) The 16-form's diff against target contains NOTHING but one seat rotation applied to ~12 lines plus the address-formation block; ours is a single pre-call `addu $17,$17,$6`, the target's is four post-call insns `sll $3,$19,16 / lw $4,4($18) / sra $3,$3,14 / addu $3,$3,$4`, and that block IS the entire 108-vs-111 gap. (b) Horn 1 (pre-call, this form): exactly 6 quantities cross, matching the target, 8 seats suffice - but sched1 sinks the address def next to the jal so its live_length is 4, its priority is >= 0.5 for any n_refs, arg0's is 2*5/54 = 0.185, and the address therefore ALWAYS takes $s1, the seat the target gives cxs. arg0 cannot be raised (shortening its live range stops it crossing at all: s38 measured 47) and the address cannot be lowered (only a scheduling barrier pins its def). (c) Horn 2 (post-call): the measured map is $s0 cys, $s1 cxs - BOTH matching the target for the first time - then $s2 table, $s3 arg0, $s4 off; SEVEN quantities cross, GCC takes a ninth callee-save ($s8/$fp, `sw s8,56(sp)`), +2 insns, and the supernumerary `table` allocno displaces arg0 out of $s2. (d) The target occupies both horns only because arg0 does double duty as scale source AND base source via the post-call `lw 0x4($s2)`. Every ban-compliant substitute for that double duty was measured and costs exactly one supernumerary crossing quantity: centre-relative (s33, 38), prev-address-plus-delta (s32 next-differences, 42), carried table + offset (s38/s38b, 31). The residual is one extra live-across-call quantity whose elimination IS the 2026-07-20 refused second materialization.
- verdict: CONFIRMED

## [s38] The s37 `next_vert = &Judge;` extra-SET lever is a general local-alloc promotion tool, so it should also pay in the arm-selected-address regimes the ledger banked at 21-25 on the pre-lever chassis, potentially beating 16.
- mechanism: The lever's dump-verified mechanism is removal of the `in block N` tag from the address pseudo, promoting it out of local-alloc's pre-seated set; if another regime's address pseudo is also block-local the same statement should promote it and re-rank the seats.
- probe: The two best pre-lever arm-selected-address forms re-spelled with the lever appended verbatim and measured on the live chassis: rejected/s38b-arm-address-basefirst-plus-lever-score21.c and rejected/s38b-arm-address-nocast-twins-plus-lever-score21.c.
- result: Both measure 21 at 108 insns - identical to their pre-lever scores of 21. Combined with s38's post-call measurement (31 with the lever, 31 without), the lever is worth exactly zero points in every regime except the merge-block-offset regime of the 16-form, where the address pseudo genuinely carries `in block 4`. It is a one-shot removal of one specific local-alloc pre-seating, not a general tool; any future session applying it to a structurally different regime must re-measure rather than assume.
- verdict: KILLED

## [s38] The score-16 residual is a SINGLE supernumerary live-across-call quantity, and the pre-call/post-call address-formation choice is a closed two-horn dilemma in which no ban-compliant form can occupy both horns.
- mechanism: global.c:635 allocno_compare ranks callee-save candidates by floor_log2(n_refs)*n_refs/live_length and hands out $s0.. in that order; the number of quantities crossing the call decides whether 8 seats suffice; sched1 runs before local-alloc and fixes the address pseudo's live_length at 4 by sinking its def next to the jal.
- probe: (i) full normalised diff of the 16-form against target (tmp/grind/func_80057CC8/s38b/base16.hon.s vs asm/funcs/func_80057CC8.s via tmp/grind/func_80057CC8/s31/norm2.py); (ii) objdump of the post-call regime's sandbox object to read its ACTUAL register map for the first time (tmp/grind/func_80057CC8/s38b/d31.ins), after re-measuring that form at 31 / 110 on the live chassis.
- result: CONFIRMED. The 16-form's diff against target contains nothing but one seat rotation applied to ~12 lines plus the address-formation block (ours one pre-call `addu $17,$17,$6`, target four post-call insns `sll $3,$19,16 / lw $4,4($18) / sra $3,$3,14 / addu $3,$3,$4`), and that block IS the entire 108-vs-111 gap. Horn 1 (pre-call): 6 quantities cross, matching target, 8 seats suffice - but the address pseudo's live_length of 4 gives it priority >= 0.5 for any n_refs against arg0's 2*5/54 = 0.185, so it always takes $s1, the seat target gives cxs; arg0 cannot be raised (shortening its range stops it crossing at all, measured 47) and the address cannot be lowered (only a scheduling barrier pins its def). Horn 2 (post-call): the measured map is $s0 cys, $s1 cxs - both matching target for the first time - then $s2 table, $s3 arg0, $s4 off; SEVEN quantities cross, GCC takes a ninth callee-save ($s8/$fp, `sw s8,56(sp)`), +2 insns, and the supernumerary `table` allocno displaces arg0 out of $s2. The target occupies both horns only because arg0 does double duty as scale source AND base source via the post-call `lw 0x4($s2)` (asm/funcs/func_80057CC8.s:50), so the base costs it zero extra crossing quantities.
- verdict: CONFIRMED

## [s39] The ra_solver suite is unusable on func_80057CC8 because the function ships as INCLUDE_ASM (the s35/s36/s37 "tooling-blocked" finding).
- mechanism: `inverse_compose.py classify` derives its target stream from the current src plus regfix/asmfix; with zero rules that stream is our own build, so it reports IDENTICAL against a 108-insn phantom target.
- probe: Re-ran `inverse_compose.py classify text1b func_80057CC8` (reproduced the fictitious IDENTICAL verdict), then ran the OBJECT-level backend s30 had already banked for exactly this case: `tools/ra_solver/goal_from_tgt.py classify|goal text1b func_80057CC8`, which compares `tmp/sandbox/func_80057CC8/text1b.o` against `build/src/text1b.o` (built from HEAD, where the function is INCLUDE_ASM, i.e. the true 111-insn target).
- result: `goal_from_tgt.py` works today with no wiring and produced a full typed classification, a mechanically derived target disposition, and two inverse-solver verdicts. The blockage was backend selection, not tooling.
- verdict: KILLED — the "solver is tooling-blocked for this function" finding is retracted. Future solver sessions on any asm-until-matched function: use `goal_from_tgt.py`, never `inverse_compose.py`.

## [s39] The register residual of the current 16-form is RA-reachable, so an allocno-priority lever (inherited frontier item 3) could close it.
- mechanism: frontier item 3 proposed giving the next-address pseudo exactly 2 references and a live_length >= 11 so that `floor_log2(n)*n/live_length` falls below arg0's 0.185 and arg0 wins $s2 in `allocno_compare`.
- probe: `goal_from_tgt.py classify text1b func_80057CC8` with the 16-form applied to src/text1b.c.
- result: FIRST DIVERGENCE **PRE-RA** — ours 108 insns vs target 111, one-sided shapes `ours: sll #,#,0x2` / `target: move, lw #,4(#), sll #,#,0x10, sra #,#,0xe`. The instruction multisets differ, so the residual is upstream of both the RA and the scheduler models; no perturbation of allocation inputs can reach zero from this form.
- verdict: KILLED as a route to zero. (It remains conceivable as a score-lowering lever, but it cannot close the function, and the 16-form is not the closest multiset anyway — see the next entry.)

## [s39] Some ban-compliant form already reproduces the target's instruction MULTISET, which would make the whole residual an RA/scheduler question the solver can invert.
- mechanism: the target is 111 insns; the ledger's forms sit at 108-110, and the prose framing ("3-instruction gap", "one supernumerary crossing quantity") never tested multiset identity directly.
- probe: applied four banked forms in turn and ran `goal_from_tgt.py classify` on each (tmp/grind/func_80057CC8/s39/screen.txt).
- result: NO form reaches the multiset, but `s36-r2-s16-narrow-index-carrier` (110 insns) misses it by **exactly one instruction shape, `lw #,4(#)` x1**, with nothing else one-sided in either direction. `s35-index-copy-in-arms` (109) misses by `move` + `lw #,4(#)`. `s38-postcall-address` (110) misses by 14 shapes, all `$s8` save/restore traffic. The 16-form (108) misses by 5.
- verdict: CONFIRMED in the sharpest possible form — the multiset half of the residual is ONE load, `lw base,4(arg0)`, i.e. literally the post-call materialization of `*(s16 **)(arg0 + 4)` refused on 2026-07-20. Every other instruction the target executes is already emitted by a ban-compliant form.

## [s39] The s37 `next_vert = &Judge;` staged-value lever, worth 4 points in the merge-block-offset regime and 0 in two others, pays in the narrow-index-carrier regime too.
- mechanism: the second SET of the variable removes the pseudo's "in block N" tag, so local-alloc no longer pre-seats it ahead of global.c (s38b, dump-verified).
- probe: built s36-r2 + `next_vert = &Judge;` (variant vA) and measured; also the same lever spelled on `table` (vC), the PLUS-operand-order flip (vD) and the lever hoisted above `scale =` (vF).
- result: vA **26 -> 18** at an unchanged 110 insns (an EIGHT-point gain, the largest the lever has produced). vC 26 (zero). vD 18, vF 18 (both identical to vA). Banked: rejected/s39-narrow-index-carrier-plus-judge-lever-score18-110insns.c.
- verdict: CONFIRMED. The lever's value is regime-specific and now measured in four regimes (+4 / +8 / 0 / 0); it pays exactly where the affected pseudo is genuinely block-local. 18 does not beat the 16-form's floor, but vA is the structurally closest form in the ledger and is the right base for future work.

## [s39] The target's register disposition is reachable from the closest ban-compliant form by some perturbation of the modelled allocation inputs.
- mechanism: `inverse.py global` searches refs / live span / birth order / conflicts / preferences / calls-crossed for minimal perturbation sets that flip our assignment to target's, and reports a typed NEGATIVE when none exists within the depth bound.
- probe: `extract.py func_80057CC8 text1b` on variant vA -> model.json (16 allocnos, 65 dispositions); `goal_from_tgt.py goal --model` -> the FULL disposition `{72(arg0):$s2, 104(next-index):$s3, 88(next-address):$v1, 87(base):$a0}`; then `inverse.py global --goal <full> --depth 3` and, separately, the seat-pair subgoal `{72:$s2, 104:$s3} --depth 3`.
- result: **FULL disposition: NEGATIVE at depth 3** — "no perturbation of any modelled input reaches the target assignment... the flip is not produced by refs / live span / birth order / conflicts / preferences / calls-crossed at all". It also names why the helpful copy-preference is unavailable: pseudos 86 and 88 cross a call and `$a0` is call-used, so `prune_preferences` (global.c:897) strips the preference before `find_reg` runs. **Seat-pair subgoal: REACHABLE**, two minimal 2-atom vectors, `#1 {88 calls_crossed 1->0, 104 calls_crossed 0->1}` and `#2 {88 refs 6->1, 104 calls_crossed 0->1}`.
- verdict: KILLED for the full disposition (typed FORECLOSED, depth 3). CONFIRMED-but-inert for the seat pair: **every reachable vector contains the atom "the next-index must cross the call"**, which is possible only if the address is formed after the call, which requires the vertex-table base to exist after the call — i.e. the banned post-call re-read, or carrying `table` (a 7th crossing quantity, a ninth callee-save `$s8`, +2 insns, measured 31). The register half and the instruction-count half of the residual therefore have the SAME single cause, proven twice by independent tooling.

## [s39] The ra_solver suite is unusable on func_80057CC8 because the function ships as INCLUDE_ASM (the s35/s36/s37 'tooling-blocked' finding).
- mechanism: inverse_compose.py classify derives its target stream from the current src plus regfix/asmfix; with zero rules that stream is our own build, so it reports IDENTICAL against a 108-insn phantom target.
- probe: Reproduced the fictitious inverse_compose IDENTICAL verdict, then ran the OBJECT-level backend s30 had already banked for exactly this case: tools/ra_solver/goal_from_tgt.py classify|goal text1b func_80057CC8, which compares tmp/sandbox/func_80057CC8/text1b.o against build/src/text1b.o (built from HEAD, where the function is INCLUDE_ASM, i.e. the true 111-insn target).
- result: goal_from_tgt.py works today with no wiring and produced a full typed classification, a mechanically derived target disposition, and two inverse-solver verdicts. The blockage was backend selection, not tooling.
- verdict: KILLED

## [s39] The register residual of the current 16-form is RA-reachable, so an allocno-priority lever (inherited frontier item 3: give the address pseudo 2 refs and live_length >= 11) could close it.
- mechanism: priority = floor_log2(n_refs)*n_refs/live_length; arg0's is 0.185, so an address pseudo with 2 refs and live_length >= 11 would rank below arg0 in global.c:635 allocno_compare and yield the target's seat order.
- probe: goal_from_tgt.py classify text1b func_80057CC8 with the s38b annotated 16-form applied to src/text1b.c:1665.
- result: FIRST DIVERGENCE PRE-RA - ours 108 insns vs target 111, one-sided shapes `ours: sll #,#,0x2` / `target: move #,#, lw #,4(#), sll #,#,0x10, sra #,#,0xe`. The instruction MULTISETS differ, so the residual is upstream of both the RA and the scheduler models; no perturbation of allocation inputs can reach zero from this form.
- verdict: KILLED

## [s39] Some ban-compliant form already reproduces the target's instruction MULTISET, which would turn the whole residual into an RA/scheduler question the inverse solver can invert.
- mechanism: The target is 111 insns and the ledger's forms sit at 108-110; the prose framing ('3-instruction gap', 'one supernumerary crossing quantity') never tested multiset identity directly.
- probe: Applied four banked forms in turn and ran goal_from_tgt.py classify on each (tmp/grind/func_80057CC8/s39/screen.txt).
- result: No form reaches the multiset, but s36-r2-s16-narrow-index-carrier (110 insns) misses it by EXACTLY ONE instruction shape, `lw #,4(#)` x1, with nothing else one-sided in either direction. s35-index-copy-in-arms (109) misses by `move` + `lw #,4(#)`. s38-postcall-address (110) misses by 14 shapes, all $s8 save/restore traffic. The 16-form (108) misses by 5. The multiset half of the 39-session residual is therefore ONE load, `lw base,4(arg0)` - literally the post-call materialization of *(s16 **)(arg0 + 4) refused on 2026-07-20.
- verdict: CONFIRMED

## [s39] The s37 `next_vert = &Judge;` staged-value lever (worth 4 points in the merge-block-offset regime, 0 in two others) also pays in the narrow-index-carrier regime.
- mechanism: The second SET of the variable removes the pseudo's 'in block N' tag, so local-alloc.c block_alloc no longer pre-seats it ahead of global.c (dump-verified s38b).
- probe: Built s36-r2 + `next_vert = &Judge;` (variant vA) and measured; also the lever spelled on `table` (vC), a PLUS-operand-order flip (vD), and the lever hoisted above `scale = arg0[2]*40;` (vF).
- result: vA 26 -> 18 at an unchanged 110 insns, the largest gain the lever has produced (EIGHT points). vC 26 (zero - matching s37's finding that the lever is specific to the pre-seated pseudo, not to the act of staging). vD 18 and vF 18, identical to vA. Regime table now four deep: merge-block-offset +4, narrow-index-carrier +8, arm-selected-address 0, post-call-address 0. 18 does not beat the 16-form's floor, but vA is the structurally closest form in the ledger.
- verdict: CONFIRMED

## [s39] The target's register disposition is reachable from the closest ban-compliant form (vA) by some perturbation of the modelled allocation inputs.
- mechanism: inverse.py global searches refs / live span / birth order / conflicts / preferences / calls-crossed for minimal perturbation sets that flip our assignment to target's, and reports a typed NEGATIVE when none exists within the depth bound.
- probe: extract.py func_80057CC8 text1b on vA -> model.json (16 allocnos, 65 dispositions); goal_from_tgt.py goal --model -> the FULL disposition {72(arg0):$s2, 104(next-index):$s3, 88(next-address):$v1, 87(base):$a0}; then inverse.py global --goal <full> --depth 3, and separately the seat-pair subgoal {72:$s2, 104:$s3} --depth 3.
- result: FULL disposition: NEGATIVE at depth 3 - 'no perturbation of any modelled input reaches the target assignment ... the flip is not produced by refs / live span / birth order / conflicts / preferences / calls-crossed at all'. It also names why the helpful copy-preference is unavailable: pseudos 86 and 88 cross a call and $a0 is call-used, so prune_preferences (global.c:897) strips the preference before find_reg runs. Seat-pair subgoal: REACHABLE, two minimal 2-atom vectors - #1 {88 calls_crossed 1->0, 104 calls_crossed 0->1} and #2 {88 refs 6->1, 104 calls_crossed 0->1}. EVERY reachable vector contains the same atom: the next-index must become a call-crossing quantity, which is possible only if the address is formed after the call, which requires the vertex-table base to exist after the call - i.e. the banned post-call re-read, or carrying `table` (a 7th crossing quantity, a ninth callee-save $s8, +2 insns, already measured at 31).
- verdict: KILLED

## s40 (2026-08-27, forensics)

### H-s40-1 — CONFIRMED
- statement: The two `lw 0x4($s2)` loads in the target are not authored duplication; the
  number of base loads is decided by cse1, and cse1 keeps the second one only because a
  non-const call sits between the two reads.
- mechanism: `canon_hash` (tools/gcc-2.7.2/cse.c:1948) marks a MEM lacking
  `RTX_UNCHANGING_P` as `hash_arg_in_memory`; `cse_insn` (cse.c:7241-7246) calls
  `invalidate_memory (&everything)` at every non-`CONST_CALL_P` CALL_INSN, destroying that
  element's equivalence class. Reads on the same side of the call stay in one class and are
  folded; a read on the far side hashes into a fresh class and emits its own load.
- probe: isolated 4-function probe compiled with the project's exact cpp|cc1 flags
  (tmp/grind/func_80057CC8/s40/probe.c, probe.sh, probe.s + full -da set): pA two reads with
  a call between; pB the same two reads with no call; pC one read cached in a local with a
  call between the uses; pD two reads with a call between through `s16 *const *`. Plus the
  same count taken on the real function with vN in src (dump.ps1).
- result: pA=2 lw, pB=1, pC=1, pD=1. Per-dump: `.jump` pA=2 pB=2 pD=2 -> `.cse` pA=2 pB=1
  pD=1, so the fold is cse1's. On the real function vN's source contains FIVE base reads;
  `.jump`=5, `.cse`=2, `.loop`=2, `.combine`=2 — exactly the target's two.
- verdict: CONFIRMED.

### H-s40-2 — CONFIRMED
- statement: A form with NO cached vertex-table base local at all — every access spelled at
  its point of use — byte-matches on the CURRENT chassis.
- mechanism: with no local, cse1 folds the four pre-call reads into one load and lets the
  post-call read emit its own, reproducing the target's `lw $a2,0x4($s2)` (pre-call, into a
  call-clobbered register, dead before the jal) and `lw $a0,0x4($s2)` (post-call).
- probe: tmp/grind/func_80057CC8/s40/vN.c applied to src/text1b.c ->
  `sandbox func_80057CC8 --disable all`. Also re-measured the archived
  rejected/s29-asymmetric-...-score0-superseded.c on the same chassis.
- result: vN score **0**, target_insns 111, build_insns 111, rules_dropped 0. s29 form
  likewise score **0**, 111/111. Banked as
  rejected/s40-no-base-local-per-use-site-reads-score0-RULING-PENDING.c.
- verdict: CONFIRMED. The match has not decayed across any chassis change; the only thing
  between this function and COMPLETED-C is the 2026-07-20 ruling / banned_constructs entry 5.

### H-s40-3 — CONFIRMED (consequence, not a new measurement)
- statement: Cheat-checklist T1 ("any observable effect beyond what a simpler form
  produces") is answered YES for the no-local form, so the premise the ban rests on is false
  for that spelling.
- mechanism: the cached-base form is a strictly stronger assertion about the program — that
  `ratan2` cannot write `((s16 **)arg0)[1]`. C does not license it and GCC does not assume
  it; pD demonstrates that when the assertion IS made explicit (`const`), the compiler folds
  the loads and the byte match is lost.
- probe: pC vs pA/pD above; plus the framing check that in the no-local form nothing is "in
  scope" to be re-materialized, since no base local exists.
- result: the two forms are different programs, not two spellings of one program.
- verdict: CONFIRMED. NOTE the scope limit: this argument does NOT rehabilitate the
  `table` + `nt` two-local family or the s29 form (inline reload while a live cached copy
  exists) — those genuinely do re-materialize a value that is in scope. It applies only to
  the spelling that declares no base local at all.

## s40b (2026-08-27, forensics — re-run after the first s40 lost its outcome JSON)

- **H-s40b-1: "Some ban-compliant form (one source-level materialization of
  `*(s16 **)(arg0 + 4)`) can still reach distance 0, given a clever enough arrangement."**
  mechanism probed: whether any GCC 2.7.2 pass after RTL expansion can turn one source-level
  base read into the two `lw 0x4($s2)` the target emits (rematerialization, reload, cse2,
  combine, sched2, jump2, dbr).
  probe: full `-da` dump set over the ban-compliant 16-form (candidate.c) built with the
  project's exact flags; per-pass count of `(mem:SI (plus:SI (reg) (const_int 4)))` inside
  the func_80057CC8 section (`tmp/grind/func_80057CC8/s40b/{full.sh,count.py,t16.*}`).
  result: the count is **1 at every one of the thirteen passes** (rtl, jump, cse, loop, cse2,
  flow, combine, sched, lreg, greg, sched2, jump2, dbr).  The comparison build of the
  score-0 form goes 5 (rtl/jump) -> 2 (cse onward), i.e. passes only ever REMOVE loads.
  verdict: **KILLED — and killed in closed form, not by exhaustion.**  Emitted base-load
  count is monotone non-increasing in source-site count under this toolchain, so two emitted
  loads require >= two source-level sites.  Ban-compliance and distance 0 are mutually
  exclusive for this function.  No future session should spend a measurement looking for a
  ban-compliant zero; the honest ban-compliant floor is 16 and it is permanent.

- **H-s40b-2: "The first s40's score-0 claim was a mis-measurement / chassis drift."**
  probe: applied `tmp/grind/func_80057CC8/s40/vN.c` to src/text1b.c on today's HEAD chassis
  and ran `sandbox func_80057CC8 --disable all`.
  result: `score 0, target_insns 111, build_insns 111, rules_dropped 0, scorable true`.
  verdict: **KILLED (the claim is confirmed, the doubt is dead).**  The form byte-matches in
  honest pure C with zero rules.  Only the ban stands between this function and COMPLETED-C.

- Open question (NOT a hypothesis a session can measure): scope of banned_constructs entry 5 /
  the 2026-07-20 owner refusal as applied to the *no-local* spelling.  Carried as the
  session's `ruling-request`; wording in `tmp/grind/outcome_func_80057CC8.json`.  If the
  ruling comes back PASS, the closing form is
  `rejected/s40-no-base-local-per-use-site-reads-score0-RULING-PENDING.c` verbatim (measure
  once to confirm, write self_vet.md, submit).  If it comes back FAIL, H-s40b-1 above means
  the correct next disposition is an escalation packet whose decidable question is
  *routing* (accept floor 16 permanently vs. canonical-asm), NOT a family sanction — and the
  canonical gate has already answered that (verdict C, scan_hand_coded LOW 1/8).

## [s40c] The candidate.c header's register map for the 16-form is correct.
- mechanism: the header (written by s38b) states ours = "$s0 cys, $s1 next-ADDRESS, $s2 cxs, $s3 arg0" and derives the whole closed-form dilemma from it.
- probe: read the `.greg` "Register dispositions" table for the applied 16-form (tmp/grind/func_80057CC8/s40c/d16.greg) and cross-referenced each pseudo against its `.lreg` line.
- result: the 16-form is $s0=129 cys, **$s1=119 cxs**, **$s2=88 next-ADDRESS**, $s3=72 arg0. The header's map is the score-20 predecessor's (v20.greg reproduces it exactly).
- verdict: KILLED. Header corrected in candidate.c. Corollary CONFIRMED: the `next_vert = &Judge;` lever's four points ARE "cxs moves from $s2 to its target seat $s1" — without it pseudo 88 is block-local and local-alloc pre-seats it into $s1 ahead of global.c.

## [s40c] (inherited s35) arg0 cannot reach $s2 ($18) in any ban-compliant form — FORECLOSED in closed form.
- mechanism: s35 argued the next-address allocno's `floor_log2(n)*n/live_length` priority always exceeds arg0's in global.c:635 allocno_compare, so find_reg always hands it the first free callee-save.
- probe: censused the seat map + allocno inputs of nine banked regimes, derived the exact inequality, then CONSTRUCTED a form satisfying it — `x1.c` = the s32 next-test-first layout with `scale = arg0[2] * 40;` moved to just after the ang_next call (arg0 live_length 53 -> 38, priority 2*4/38 = 0.2105 > the address's 1*3/18 = 0.1667).
- result: `x1.greg` shows **$s2 = 72 (arg0), $s3 = 88 (next-address)**, with $s0/$s1 = cys/cxs and $s4-$s7 unchanged — the target's COMPLETE callee-save map from ban-compliant C, a first in 40 sessions. Measured score 56 at 106 insns.
- verdict: **KILLED (the foreclosure is retracted).** The seat map is reachable; it is only unaffordable in that block order, where the instruction-order cost (~40 points) swamps the seats (~4 points). Banked: rejected/s40c-nextfirst-scale-after-angnext-SEATFLIP-score56-106insns.c.

## [s40c] The seat flip can be carried into the affordable prev-test-first regime by shortening arg0's live range.
- mechanism: prev-first keeps arg0 at 5 refs (cse1 cannot fuse the two `lbu 3($s2)` loads because the prev-arm read is conditional and does not dominate). The only cross-block third-crosser spelling there is the h35 two-def address, n=4 / L=29 -> 0.2759, so arg0 needs 2*5/L > 0.2759, i.e. L < 36.2.
- probe: built and dumped `y1.c` (`scale` after the ang_next call) and `y2.c` (`scale` between the two ratan2 calls, the earliest placement that still leaves arg0 crossing a call); measured y2 in the sandbox.
- result: y1 arg0 = 5 refs / 39 insns = 0.256 (no flip); y2 arg0 = 5 refs / **37** insns = **0.2703** vs 0.2759 — **no flip, short by 2%**; y2 scores 42 at 108 insns. y2 is the extremum: arg0's live range strictly contains the address's (arg0 is set at the prologue param copy before `table` is loaded and dies at `arg0[2]` after the address's last `lh`), so every move that shortens arg0 shortens the address too and the required ratio L_88/L_72 > 0.8 cannot be reached (29/37 = 0.784).
- verdict: KILLED for the h35 family. Banked: rejected/s40c-prevfirst-h35-scale-after-angnext-noflip.c, rejected/s40c-prevfirst-h35-scale-between-calls-noflip-score42.c.

## [s40c] The target's own allocation numbers explain why the ban is what costs the seats.
- mechanism: dump the score-0 banned form (rejected/s40-no-base-local-per-use-site-reads-score0-RULING-PENDING.c), which reproduces the target byte-for-byte, and read its allocno inputs.
- probe: `z0.greg` + `z0.lreg`.
- result: arg0 (72) = **6 refs / 57 insns** -> 2*6/57 = 0.2105 -> $s2; next-INDEX (77) = **3 refs / 20 insns** -> 1*3/20 = 0.150 -> $s3. arg0's SIXTH reference is the post-call `lw $a0,4($s2)` (asm/funcs/func_80057CC8.s:50). Ban-compliant forms cap arg0 at five refs, AND the ban forces the third crosser from an index (2 arm defs + 1 post-call use, n=3) up to an address (n>=4 whenever it spans blocks), because forming the address after the call needs the base after the call.
- verdict: CONFIRMED — a third independent confirmation (after goal_from_tgt.py's one-`lw` multiset miss and inverse.py's calls_crossed atom) that the register half and the instruction half of the residual have the SAME single cause, now with the compiler's own priority arithmetic.

## [s40] The candidate.c header's register map for the 16-form ($s0 cys, $s1 next-ADDRESS, $s2 cxs, $s3 arg0) is correct.
- mechanism: s38b derived the whole closed-form dilemma from that map, and every later session quoted it without re-reading a dump.
- probe: Read the .greg 'Register dispositions' table for the applied 16-form (tmp/grind/func_80057CC8/s40c/d16.greg) and cross-referenced each pseudo with its .lreg line.
- result: The 16-form is $s0=129 cys, $s1=119 cxs, $s2=88 next-ADDRESS, $s3=72 arg0. The header's map is the SCORE-20 predecessor's (v20.greg reproduces it verbatim). Corollary: the `next_vert = &Judge;` lever's four points are exactly 'cxs moves from $s2 to its target seat $s1' - without the lever pseudo 88 has all three refs in block 4, local-alloc pre-seats it into $s1 ahead of global.c, and cxs is pushed down.
- verdict: KILLED

## [s40] (inherited from s35) arg0 cannot reach $s2 ($18) in any ban-compliant form - the next-address allocno always outranks it in global.c allocno_compare; FORECLOSED in closed form.
- mechanism: GCC 2.7.2 global.c orders global allocnos by floor_log2(n_refs)*n_refs/live_length and find_reg then takes the first free callee-save; s35 argued the address's priority is unconditionally the larger.
- probe: Censused seats plus allocno inputs for nine banked regimes with a new batch dumper (tmp/grind/func_80057CC8/s40c/batch.sh), derived the exact inequality, then CONSTRUCTED a form satisfying it: x1.c = the s32 next-test-first layout with `scale = arg0[2] * 40;` moved to immediately after the ang_next call, shortening arg0's live_length 53 -> 38 (priority 2*4/38 = 0.2105) against the address's 1*3/18 = 0.1667.
- result: x1.greg reads $s0 cys, $s1 cxs, $s2 arg0 (72), $s3 next-address (88), $s4/$s5 raw cx/cy, $s6 arg2, $s7 arg3 - the target's COMPLETE callee-save map, from ban-compliant C, a first in 40 sessions. Measured score 56 at 106 insns: the seats are worth about 4 points and the next-test-first order plus the early `scale` costs about 40.
- verdict: KILLED

## [s40] The seat flip can be carried into the AFFORDABLE prev-test-first regime (the target's own block order) by shortening arg0's live range.
- mechanism: Prev-first keeps arg0 at 5 refs because cse1 cannot fuse the two `lbu 3($s2)` loads (the prev-arm read is conditional and does not dominate the next-test read). The only cross-block third-crosser spelling available there is the h35 two-def address, n=4 / L=29 -> 0.2759, so arg0 needs 2*5/L > 0.2759, i.e. live_length < 36.2.
- probe: Built and dumped y1.c (`scale` after the ang_next call) and y2.c (`scale` BETWEEN the two ratan2 calls - the earliest placement that still leaves arg0 crossing a call at all); sandbox-measured y2.
- result: y1: arg0 5 refs / 39 insns = 0.256, no flip. y2: arg0 5 refs / 37 insns = 0.2703 against 0.2759 - NO FLIP, short by 2%; score 42 at 108 insns. y2 is the extremum: arg0's live range strictly contains the address's (arg0 is set at the prologue param copy before `table` is even loaded and dies at `arg0[2]` a few insns after the address's last `lh`), so every statement move that shortens one shortens the other; the required ratio L_88/L_72 > 0.8 cannot be reached (29/37 = 0.784), and moving `scale` any earlier makes arg0 stop crossing a call and leave the callee-save pool entirely (the s35 score-32/47 regime).
- verdict: KILLED

## [s40] The target's own allocation numbers can be read directly, and they will show WHY the 2026-07-20 ban costs the seats.
- mechanism: The score-0 banned form reproduces the target byte-for-byte, so its allocno table IS the original's.
- probe: Dumped rejected/s40-no-base-local-per-use-site-reads-score0-RULING-PENDING.c (z0.greg / z0.lreg).
- result: arg0 (72) = 6 refs / 57 insns -> 2*6/57 = 0.2105 -> $s2; next-INDEX (77) = 3 refs / 20 insns -> 1*3/20 = 0.150 -> $s3. arg0's SIXTH reference IS the post-call `lw $a0,4($s2)` at asm/funcs/func_80057CC8.s:50. Ban-compliant forms cap arg0 at five refs (param def, arg0+4, arg0[3] twice, arg0[2]) AND the same ban forces the third crossing quantity up from an index (2 arm defs + 1 post-call use, n=3, floor_log2 factor 1) to an address (n>=4 whenever it spans blocks, factor 2), because forming the address after the call requires the base after the call.
- verdict: CONFIRMED

## s41 (2026-08-27, forensics — the missing n=3 cross-block cell EXISTS; two new seat-flipping regimes)

Chassis re-measured at session start via the s40c census tooling; the banked 16-form's
`.greg` map is unchanged ($s0 = 129 cys, $s1 = 119 cxs, $s2 = 88 next-ADDRESS 6 refs / 4
insns, $s3 = 72 arg0 5 refs / 54 insns). Floor 16 stands; nothing this session beat it.
All seat numbers below come from `tmp/grind/func_80057CC8/s41/batch.sh` (the s40c census
script re-pointed at the s41 dump directory), i.e. from the `.greg` "Register
dispositions" table plus each pseudo's `.lreg` line — not from inference.

### H-s41-1 — KILLED. "Raising arg0 (pseudo 72) to SIX references by duplicating a real
arg0 statement into both arms of the ang_mid if/else flips $s2, per the s40c frontier."
- mechanism: `duplicated-statement-into-arms` is a sanctioned family; arg0 at n=6 has
  `floor_log2(6)*6/L = 12/L`, which clears the h35 address's `2*4/29 = 0.2759` for any
  L < 43.5, and y2 already sits at L = 37.
- probe: `w1.c` = y2 with `scale = arg0[2] * 40;` removed from between the calls and
  duplicated into both arms of the ang_mid if/else; `w2.c` = the same with the between-calls
  copy also retained (n = 7).
- result: arg0 does reach **6 refs**, but its live_length goes **37 -> 52** ("dies in 2
  places; crosses 2 calls"), because both arms sit AFTER both ratan2 calls. Priority
  12/52 = **0.2308 < 0.2759** — no flip; `$s2` stays pseudo 88 in both w1 and w2.
- verdict: **KILLED.** The duplication lever is self-defeating in this function: every
  placement that can host a duplicated arg0 read is post-call, and the L it buys costs more
  than the ref it adds. Generalised in H-s41-3.

### H-s41-2 — KILLED. "Entry-block statement motion can widen L_88/L_72 past the required
0.8 by shrinking the front gap between arg0's def and the address's def."
- mechanism: the flip needs `8/L_88 < 10/L_72`, i.e. `L_72 - L_88 <= 7`; y2 sits at
  37 - 29 = 8, one instruction away.
- probe: `a1.c` (prev_idx computed just before the prev-if), `a2.c` (entry block reduced to
  the table load + the address def; prev_idx late, cx/cy after both ifs), `a3.c` (cx/cy
  after both ifs, prev_idx first).
- result: a1 = 29/37, a2 = 27/35, a3 = 27/35. **The gap is invariantly 8** under every
  source-order permutation. Reading y2's `.s` shows why: sched1 interleaves six prologue
  callee-save `sw` stores between `move $19,$4` (arg0's def, insn 3) and `lw $4,4($19)`
  (the table load, insn 19), and those stores are not reachable by source motion.
- verdict: **KILLED.** The front gap is a scheduler artefact of the 9-register prologue,
  not a source-order degree of freedom.

### H-s41-3 — CONFIRMED (closed form). "Any sixth arg0 reference is either post-death (too
much L) or pre-death and live across a call (a supernumerary crossing quantity)."
- mechanism: cse1 fuses two identical `arg0[k]` reads unless a non-const CALL_INSN separates
  them (`tools/gcc-2.7.2/cse.c:7241-7246`, banked in s40b). So a surviving extra read must
  be on the far side of a ratan2 call from its sibling. Placed after arg0's current death
  it extends L (H-s41-1, 52); placed before it, the value it produces must survive the call
  and therefore occupies its own callee-save seat.
- probe: `c1.c` — arg0[2] read TWICE (once before the ang_prev call, once between the calls),
  the two products feeding `*arg2` and `*arg3` respectively. Measurement only; this is the
  duplicate-materialization cheat family and is NOT submittable.
- result: arg0 reaches **6 refs / 41 insns = 0.293** and **takes $s2**, with the address in
  $s3 — the arithmetic is exactly as predicted — but a seventh quantity now crosses and
  arg3 is pushed out of the eight listed callee-saves.
- verdict: **CONFIRMED.** This is the fourth independent derivation of the same single
  cause: the target's sixth arg0 reference is the post-call `lw $a0,0x4($s2)`
  (`asm/funcs/func_80057CC8.s:50`), which is BOTH a sixth arg0 ref AND the value the extra
  crosser would otherwise have to carry. It costs nothing precisely because it is the
  banned second materialization.

### H-s41-4 — CONFIRMED (new regime, the s40c frontier's "missing n=3 cross-block cell").
"A block order exists that gives the third crossing quantity a cross-block n=3 live range
and flips $s2/$s3."
- mechanism: a single-def address is block-local (local-alloc pre-seats it, the score-20
  regime) only because nothing branches between its def and its uses. Hoisting the
  wrap-if AHEAD of the prev-if puts the address's single def in the wrap-merge block and
  leaves the prev-if's branch between it and the two `lh` uses, so it becomes a global
  allocno with n = 3 (`floor_log2(3) = 1`, priority 3/L).
- probe: `d1.c` — wrap-if first (`off` defaulted then zeroed, `next_vert` defined once from
  `table + off`), the vertex count hoisted into `cnt` so `arg0[3]` is read once, prev-if
  second, `scale` between the calls. `d8.c` = d1 with the prev-if reading `arg0[3]`
  directly (cse1 fuses it against the dominating wrap-test read — identical dumps, so the
  second `lbu 3($s2)` is unrecoverable in this order).
- result: **$s0 = cys, $s1 = cxs, $s2 = 72 arg0 (4 refs / 36 = 0.222), $s3 = 90 address
  (3 refs / 18 = 0.167), $s4/$s5 = raw cx/cy, $s6 = arg2, $s7 = arg3** — the target's
  COMPLETE callee-save map, from ban-compliant C, at **106 instructions**. Measured
  `sandbox --disable all` = **score 45**. `d2.c` (same but `scale` after the ang_next call)
  loses the flip (arg0 4/53 = 0.151 < 0.167), confirming the arithmetic is what drives it.
- verdict: **CONFIRMED.** The cell exists and is ban-compliant. Its cost is the block
  order: wrap-before-prev is not the target's order, and it also forces cse1 to fuse the two
  `lbu 3($s2)` loads the target keeps. Banked as
  `rejected/s41-wrapif-first-previf-separates-address-SEATFLIP-score45-106insns.c`.

### H-s41-5 — CONFIRMED (second new regime, and the first flip in the TARGET's own block
order). "The horn-2 post-call-address regime plus the s40c live-range shortening flips
arg0 into $s2."
- mechanism: horn 2 (`rejected/s38-postcall-address-*.c`, score 31 @ 110) already has the
  target's block order (prev-if, wrap-if, call, post-call address formation) and the target's
  post-call `sll/sra 14/addu` shape; its defect is that the vertex-table base must be CARRIED
  across the call, which is one supernumerary crossing quantity and displaced arg0 from $s2
  to $s3. Shortening arg0's live range (`scale` between the calls) raises its priority above
  the carried base's.
- probe: `e1.c` = the s38 post-call form with `scale = arg0[2] * 40;` moved between the two
  ratan2 calls. `e2.c` = e1 with the wrap quantity kept as the INDEX and sign-extended /
  scaled post-call (`(((s32)(off << 16) >> 16) << 2)`, the target's own two insns).
- result: **e1: $s2 = 72 arg0 (5 refs / 37), $s3 = 87 table (4 / 30), $s4 = 89 off (3 / 20)**,
  $s5/$s6 raw cx/cy, $s7 arg2 — arg0 reaches $s2 in the target's block order, with arg3
  spilled to the ninth callee-save. Measured **score 41 @ 110 insns** (the parent form
  scores 31, so the flip is worth ~4 points and the `scale` move costs ~14). e2 loses the
  flip entirely ($s2 = 89, the index).
- verdict: **CONFIRMED.** Banked as
  `rejected/s41-postcall-address-scale-between-calls-SEATFLIP-score41-110insns.c`. The
  residual in e1 is now a single named quantity: `table` occupies $s3 (which the target gives
  to the next-INDEX) and forces a ninth callee-save. To reach the target's map from here,
  `off` (3 refs / 20 = 0.150) must outrank `table` (4 refs / 30 = 0.267) — a 1.8x gap, the
  first time the whole residual has been expressible as ONE inequality between two named
  allocnos in the target's own block order.

### H-s41-6 — KILLED. "Starting the address pseudo's live range at the table load (first def
`next_vert = table;`, branch sense inverted) shrinks the front gap enough to flip."
- probe: `b1.c` / `b2.c`.
- result: the gap does collapse (L_88 34 vs L_72 37, front gap 3), but GCC coalesces the two
  pointer pseudos, so pseudo 88 absorbs `table`'s references: **7 refs / 34 = 0.412**, far
  above arg0. No flip.
- verdict: **KILLED.** Banked as
  `rejected/s41-address-first-def-equals-table-coalesces-7refs-noflip.c`.

## [s41] Raising arg0 (pseudo 72) to SIX references by duplicating a real arg0 statement into both arms of the ang_mid if/else (the sanctioned duplicated-statement-into-arms family) flips $s2, as the s40c frontier proposed.
- mechanism: arg0 at n=6 has floor_log2(6)*6/L = 12/L, which clears the h35 address's 2*4/29 = 0.2759 for any live_length below 43.5, and the y2 extremum already sits at L=37.
- probe: w1.c = y2 with `scale = arg0[2] * 40;` removed from between the calls and duplicated into both arms of the ang_mid if/else; w2.c = the same with the between-calls copy also retained (n=7). Seats and .lreg lines read from the -da dumps via tmp/grind/func_80057CC8/s41/batch.sh.
- result: arg0 does reach 6 refs, but its live_length goes 37 -> 52 ('dies in 2 places; crosses 2 calls') because both arms sit after BOTH ratan2 calls: 12/52 = 0.2308 < 0.2759. $s2 stays pseudo 88 in both w1 and w2.
- verdict: KILLED

## [s41] Entry-block statement motion can widen L_88/L_72 past the required 0.8 by shrinking the front gap between arg0's def and the address pseudo's def (y2 misses the flip by one instruction).
- mechanism: The flip needs 8/L_88 < 10/L_72, i.e. L_72 - L_88 <= 7; y2 sits at 37 - 29 = 8.
- probe: a1.c (prev_idx computed just before the prev-if), a2.c (entry block cut to the table load plus the address def; prev_idx late, cx/cy after both ifs), a3.c (cx/cy after both ifs). Also re-read y2.s to locate arg0's def and the table load.
- result: a1 = 29/37, a2 = 27/35, a3 = 27/35 -- the gap is invariantly 8 under every source-order permutation. y2.s shows why: sched1 interleaves six prologue callee-save `sw` stores between `move $19,$4` (arg0's def, insn 3) and `lw $4,4($19)` (the table load, insn 19). The gap is a scheduler artefact of the nine-register prologue, not a source-order degree of freedom.
- verdict: KILLED

## [s41] Any sixth arg0 reference is either post-death (extending live_length past the flip threshold) or pre-death and live across a call (adding a supernumerary crossing quantity) -- so no ban-compliant sixth reference is free.
- mechanism: cse1 fuses two identical arg0[k] reads unless a non-const CALL_INSN separates them (tools/gcc-2.7.2/cse.c:7241-7246, banked in s40b), so a surviving extra read must be on the far side of a ratan2 call from its sibling.
- probe: c1.c -- arg0[2] read TWICE, once before the ang_prev call and once between the calls, the two products feeding *arg2 and *arg3 respectively. This is the duplicate-materialization cheat family; run as a MEASUREMENT ONLY and banked as not submittable.
- result: arg0 reaches 6 refs / 41 insns = 0.293 and TAKES $s2 with the address in $s3 -- the arithmetic behaves exactly as predicted -- but a seventh quantity now crosses and arg3 is pushed out of the eight callee-saves. The target pays neither price only because its sixth arg0 reference IS the banned second materialization `lw $a0,0x4($s2)` (asm/funcs/func_80057CC8.s:50), one instruction that is simultaneously the sixth ref and the carrier of the extra crosser.
- verdict: CONFIRMED

## [s41] A block order exists that gives the third crossing quantity a cross-block n=3 live range and flips $s2/$s3 -- the cell s40c named as missing.
- mechanism: A single-def address is block-local (so local-alloc pre-seats it, the score-20 regime) only because nothing branches between its def and its uses. Hoisting the wrap-if AHEAD of the prev-if puts the address's single def in the wrap-merge block and leaves the prev-if's branch between it and the two `lh` uses, making it a global allocno with n=3 (floor_log2(3)=1, priority 3/L).
- probe: d1.c -- wrap-if first (vertex count hoisted into `cnt`, `off` defaulted then zeroed, `next_vert` defined ONCE as table + off), prev-if second, `scale` between the calls. Controls: d2.c (`scale` after the ang_next call) and d8.c (prev-if reading arg0[3] directly instead of `cnt`). Censused with batch.sh, then applied and measured with `sandbox func_80057CC8 --disable all`.
- result: d1.greg = $s0 cys, $s1 cxs, $s2 = 72 arg0 (4 refs / 36 = 0.222), $s3 = 90 address (3 refs / 18 = 0.167), $s4/$s5 raw cx/cy, $s6 arg2, $s7 arg3 -- the target's COMPLETE callee-save map from ban-compliant C at 106 instructions; measured score 45. d2 loses the flip exactly as the arithmetic predicts (arg0 4/53 = 0.151). d8 produces byte-identical dumps: in this order the wrap-test read dominates, cse1 fuses the two arg0[3] loads, and the target's second `lbu 3($s2)` is structurally unrecoverable here.
- verdict: CONFIRMED

## [s41] The horn-2 post-call-address regime plus the s40c live-range lever flips arg0 into $s2 -- the first seat flip achieved in the TARGET's own block order.
- mechanism: Horn 2 (rejected/s38-postcall-address-no-judge-lever-score31-110insns.c) already has the target's block order and the target's post-call `sll / sra 14 / addu` address formation; its defect is that the vertex-table base must be carried across the call, one supernumerary crossing quantity that displaced arg0 from $s2 to $s3. Shortening arg0's live range raises its allocno_compare priority above the carried base's.
- probe: e1.c = the s38 post-call form with `scale = arg0[2] * 40;` moved between the two ratan2 calls; e2.c = e1 with the wrap quantity kept as a sign-extended INDEX scaled post-call (the target's own two insns). Censused with batch.sh; e1 applied and measured.
- result: e1.greg = $s2 = 72 arg0 (5 refs / 37), $s3 = 87 table (4 / 30), $s4 = 89 off (3 / 20), $s5/$s6 raw cx/cy, $s7 arg2, arg3 to the ninth callee-save; measured score 41 at 110 insns (parent form 31, so the flip is worth about 4 points and the `scale` move costs about 14). e2 loses the flip entirely ($s2 = 89, the index).
- verdict: CONFIRMED

## [s41] Starting the address pseudo's live range at the table load (first def `next_vert = table;`, branch sense inverted so the wrap-if arm supplies the offset form) shrinks the front gap enough to flip.
- mechanism: The gap L_72 - L_88 is what blocks the prev-first flip; defining the address at the table load should collapse it to the distance between arg0's param copy and the `lw`.
- probe: b1.c and b2.c (b2 additionally computes prev_idx late), censused with batch.sh.
- result: The gap does collapse (L_88 34 vs L_72 37, front gap 3), but GCC coalesces the two pointer pseudos, so pseudo 88 absorbs table's references: 7 refs / 34 = 0.412, far above arg0's 0.270. No flip; strictly worse.
- verdict: KILLED

# ===== s42 (rederive) — chassis re-measured at 16; the post-call-address regime FORECLOSED
# in closed form, and all three s41 frontier items killed =====

Baseline re-measured this session with candidate.c applied to src/text1b.c:
`sandbox func_80057CC8 --disable all` -> **score 16, target_insns 111, build_insns 108,
rules_dropped 0**.  The brief reported the chassis floor as "measurement unavailable"; the
ledger's 16 is current.  All seat/live-range numbers below are read off the `-da` dumps via
`tmp/grind/func_80057CC8/s42/batch.sh` (the s41 census script re-pointed at s42), i.e. from
the `.greg` "Register dispositions" table plus each pseudo's `.lreg` line.

## [s42] The in-file sibling idiom (func_80048530 / func_800611A4, both COMPLETED in
src/text1b.c) — all address arithmetic carried in an s32 base with ONE reused pointer
cursor walked to each site — is a structurally different C shape that may move the
$s2/$s3 seat assignment the candidate cannot reach.
- mechanism: rederive-modality sibling transplant.  The two solved neighbours in this very
  file both spell pointer walks as `base = <ptr>; ... p = off + base;` with a single reused
  cursor and an integer-typed base (func_80048530 even carries an owner-sanctioned FAKE for
  the `off + base` operand order).  The candidate instead uses an s16* `table` indexed
  (`table[pi*2]`) for the prev neighbour and a separate `next_vert` pointer for the next one
  — an asymmetric shape no solved sibling uses.
- probe: `a1.c` = `s32 vt = *(s32 *)(arg0 + 4);` with a single reused `s16 *p` cursor
  (`p = (s16 *)(off + vt)` for next, a block-scoped `q = (s16 *)(pi * 4 + vt)` for prev),
  the &Judge lever staged through `p`.  `a2.c` = a1 with the prev cursor spelled as element
  indexing `(s16 *)vt + pi * 2` instead of byte arithmetic.
- result: **a1 = score 16 @ 108 insns — the SAME score as the candidate, with a
  REGISTER-IDENTICAL disposition table** ($s0 = cys, $s1 = cxs, $s2 = 88 next-address
  "used 6 times across 4 insns; crosses 1 call", $s3 = 72 arg0 "5 times across 54 insns",
  $s4/$s5 raw cx/cy, $s6 arg2, $s7 arg3).  a2 = score 17 @ 108.
- verdict: **KILLED as an improvement, CONFIRMED as an equivalence.**  The 108-insn
  merge-offset regime's register map is INVARIANT under the whole s32-base/reused-cursor
  respelling: a completely different source shape lands on the identical allocation.  The
  sibling idiom is not a lever here.  Banked as
  `rejected/s42-sibling-idiom-s32-base-reused-cursor-REGEQUIV-score16.c` and
  `rejected/s42-sibling-idiom-elementindex-cursor-score17.c`.

## [s42] (s41 frontier item #1) Giving the wrap quantity a fourth-and-more cross-block
reference by REUSING THE PARAMETER `arg1` as the wrapped-offset carrier flips `off` above
`table` in global.c allocno_compare and completes the target's seat map in the post-call
regime.
- mechanism: s41 measured e1's `off` at 3 refs / 20 insns (priority 1*3/20 = 0.150) against
  the carried `table` at 4 refs / 30 (2*4/30 = 0.267).  Reusing the parameter `arg1` — which
  is already referenced by `prev_idx = arg1 - 1`, the two centre-coordinate reads and
  `tmp = arg1 + 1` before it is overwritten with the wrapped offset — raises the carrier to
  n >= 6 without any second materialization of `*(s16 **)(arg0 + 4)`.
- probe: `g1.c` = e1.c with the local `off` deleted and `arg1` itself assigned the wrapped
  byte offset (`arg1 = tmp * 4; if ((s16) tmp >= (s32)arg0[3]) arg1 = 0;`), the address then
  formed post-call as `(s16 *)(arg1 + (s32)table)`.
- result: the priority arithmetic works exactly as predicted — pseudo 73 (`arg1`) reaches
  **7 refs / 32 insns = 2*7/32 = 0.4375** and TAKES `$s2` — but it overshoots: it outranks
  arg0 too, so the map becomes $s2 = arg1-carrier, $s3 = 72 arg0, $s4 = 87 table, i.e. a
  three-way rotation, not the target's.  Worse, arg3 (pseudo 75) is still pushed to
  **hard reg 30 = `$fp`/`$s8`, a NINTH callee-save**.  Measured **score 50 @ 111 insns**.
- verdict: **KILLED.**  The window the flip needs is arithmetically unreachable: to sit
  between arg0 (0.270) and `table` (0.267) the carrier's priority must land inside a 0.003
  interval, and every integral (n, L) pair available to it (n = 3: 0.150; n = 4, L = 30:
  0.267 — an exact tie with `table`; n = 7: 0.4375) misses it.  Banked as
  `rejected/s42-postcall-arg1-reused-as-offset-carrier-9th-callee-save-score50.c`.

## [s42] (the generalisation that closes s41 frontier items #1 AND #2) EVERY ban-compliant
post-call-address form carries one supernumerary live-across-call quantity, takes a NINTH
callee-save ($fp/$s8), and therefore cannot be byte-identical to a target that saves only
$s0-$s7.
- mechanism: the target's carried set has exactly EIGHT members — arg0, the next-INDEX, raw
  cx, raw cy, cxs, cys, arg2, arg3 (`asm/funcs/func_80057CC8.s:3-16` stores $ra + $s0..$s7
  at 0x18..0x38, and the body contains no $fp/$s8 reference at all).  It gets away with
  eight only because the vertex-table base is RE-DERIVED after the call from arg0
  (`lw $a0,0x4($s2)`, `asm/funcs/func_80057CC8.s:50`) — the second source-level
  materialization the owner refused on 2026-07-20.  A ban-compliant form must instead CARRY
  the base, so its carried set is {arg0 or its surrogate} + {base} + {index} + the five
  others = nine.
- probe: five independent post-call forms, each attacking the ninth quantity from a
  different direction, censused for pseudo 75's (arg3's) hard register:
  `e1.c` (s41's seat-flip form, carried `table` + fresh `off`); `g1.c` (carrier = reused
  `arg1`); `h1.c` (`scale = arg0[2] * 40` hoisted ABOVE the prev-if so arg0 dies before the
  first call, index carried in the target's own `sll 16 / sra 14` shape); `h2.c` (same, byte
  offset instead of index); `h3.c` (h1 + the &Judge staging lever).
- result: **all five put pseudo 75 in hard reg 30 ($fp/$s8)**.  The h-family shows why the
  "kill arg0 early" idea cannot help: arg0 does die pre-call (pseudo 72 lands in $t0, a
  caller-save), but `scale` (pseudo 80) then becomes a crosser in its place — "used 3 times
  across 64 insns; crosses 2 calls" — so the count is unchanged at nine.  Scores: e1 41 @
  110, g1 50 @ 111, h1 55 @ 112, h2 54 @ 111, h3 55 @ 112.  Contrast the pre-call regimes,
  which need only eight: base16/a1 put arg3 in $s7 (hard reg 23) and use no $s8.
- verdict: **CONFIRMED (closed-form foreclosure).**  The post-call-address regime — the home
  of BOTH s41 frontier items (#1 "make `off` outrank `table`" and #2 "permuter campaign
  seeded from e1.c") — can never reach distance 0, because a $s8 save/restore pair and the
  frame-offset shift it forces are wrong bytes the target does not contain.  Its floor is
  the 31 already banked at s38.  Both frontier items are therefore dead independently of
  whether their seat flips succeed.  Banked as
  `rejected/s42-postcall-scale-hoisted-arg0-dies-early-scale-becomes-9th-crosser-score55.c`
  and `rejected/s42-postcall-byteoffset-arg0-dies-early-9th-callee-save-score54.c`.

## [s42] (s41 frontier item #3) A branch other than the prev-if can be made to sit between
the next-address's single def and its two `lh` uses, so d1's cross-block n=3 address can be
obtained WITHOUT d1's wrap-before-prev block order.
- mechanism: d1 proved the n=3 cross-block address only needs SOME branch in that window;
  s41 hypothesised another branch could legitimately occupy it in the prev-first layout.
- probe: enumeration of the branches this function's semantics actually admit before the
  ang_next argument reads, cross-checked against the already-banked prev-first controls
  `rejected/s40c-prevfirst-h35-scale-between-calls-noflip-score42.c` and
  `rejected/s40c-prevfirst-h35-scale-after-angnext-noflip.c`.
- result: the function has exactly TWO semantically-required conditionals before the
  ang_next reads — the prev-index wrap and the next-index wrap — plus the ang_mid if/else,
  which is strictly AFTER both `lh` uses and so can never separate them from the def.  A
  single-def next-address is by construction defined at or after the next-wrap merge, so the
  next-wrap branch is always BEFORE it and cannot separate it.  That leaves the prev-if as
  the only candidate separator, and putting the prev-if after the address def IS d1's
  wrap-before-prev order.  Any additional branch would have to be invented (a condition with
  no semantic purpose), which is the dead-conditional family.
- verdict: **KILLED by closed form.**  d1's n=3 cross-block address and the target's
  prev-first block order are mutually exclusive in ban-compliant C.  This is the same
  dilemma s38b stated for the address-formation horns, now proven one level lower, at the
  block-structure level.

## [s42] SYNTHESIS — the ban-compliant search space is now fully partitioned (three regimes,
each foreclosed by a distinct closed-form argument).
- **Regime A — pre-call address, prev-first (the candidate, base16/a1): 8 callee-saves,
  108 insns, score 16.**  Correct block order, correct save COUNT, but the seat assignment
  is rotated ($s2 = next-address, $s3 = arg0 where the target has $s2 = arg0, $s3 =
  next-index).  The flip is foreclosed by s41 H-s41-2 (the arg0-def-to-address-def gap is
  invariantly 8 instructions under every source permutation, because sched1 interleaves six
  prologue callee-save stores that source motion cannot reach) and s41 H-s41-1/3 (no free
  sixth arg0 reference exists).  s42 adds that the whole register map is invariant under a
  complete s32-base/reused-cursor respelling.
- **Regime B — pre-call address, wrap-first (d1): 8 callee-saves, 106 insns, score 45.**
  The target's COMPLETE seat map from ban-compliant C, but the wrong block order, and cse1
  fuses the two `arg0[3]` reads so the target's second `lbu 3($s2)` is unrecoverable.  s42
  proves the order is not separable from the map (frontier item #3 above).
- **Regime C — post-call address (e1/g1/h1/h2/h3): 9 callee-saves, 110-112 insns, score
  31-55.**  Target's address-formation shape and (in e1) target's block order, but a $s8
  save/restore pair the target does not have.  Foreclosed outright by s42.
- The target itself is in none of these: it is Regime C with only eight saves, which it
  achieves solely via the second source-level materialization of `*(s16 **)(arg0 + 4)`
  refused on 2026-07-20.  **Floor 16 is Regime A's floor and, with B and C foreclosed, the
  ban-compliant floor of the function.**

## [s42] The in-file sibling idiom (func_80048530 at src/text1b.c:312 and func_800611A4 at src/text1b.c:3252, both COMPLETED) - an integer-typed base plus ONE reused pointer cursor walked to each site, instead of the candidate's asymmetric table[pi*2]-indexed prev plus dedicated next_vert - is a structurally different C shape that may move the $s2/$s3 seat assignment the candidate cannot reach.
- mechanism: rederive-modality sibling transplant. Both solved neighbours in this very file spell pointer walks as `base = <ptr>; ... p = off + base;` with a single reused cursor and an integer-typed base (func_80048530 even carries an owner-sanctioned FAKE for the `off + base` operand order). A different RTL-expansion shape for the address chain could redirect the copy-preference and live-range inputs global.c consumes.
- probe: a1.c = `s32 vt = *(s32 *)(arg0 + 4);` with a single reused `s16 *p` cursor (`p = (s16 *)(off + vt)` for next, a block-scoped `q = (s16 *)(pi * 4 + vt)` for prev) and the &Judge lever staged through `p`; a2.c = a1 with the prev cursor spelled as element indexing `(s16 *)vt + pi * 2`. Seats censused from the -da dumps via tmp/grind/func_80057CC8/s42/batch.sh, then applied and scored with `sandbox func_80057CC8 --disable all`.
- result: a1 = score 16 @ 108 insns with a register-IDENTICAL disposition table to the candidate ($s0 = cys, $s1 = cxs, $s2 = pseudo 88 next-address 'used 6 times across 4 insns; crosses 1 call', $s3 = pseudo 72 arg0 '5 times across 54 insns', $s4/$s5 raw cx/cy, $s6 arg2, $s7 arg3). a2 = score 17 @ 108. The 108-insn merge-offset regime's allocation is invariant under a complete s32-base/reused-cursor respelling.
- verdict: KILLED

## [s42] s41 frontier item #1: giving the wrap quantity a fourth-and-more cross-block reference by REUSING THE PARAMETER arg1 as the wrapped-offset carrier flips `off` above the carried `table` in global.c allocno_compare and completes the target's seat map in the post-call regime.
- mechanism: s41 measured e1's `off` at 3 refs / 20 insns (priority 1*3/20 = 0.150) against the carried `table` at 4 refs / 30 (2*4/30 = 0.267). Reusing arg1 - already referenced by `prev_idx = arg1 - 1`, both centre-coordinate reads and `tmp = arg1 + 1` before being overwritten - raises the carrier to n >= 6 with NO second materialization of *(s16 **)(arg0 + 4).
- probe: g1.c = e1.c with the local `off` deleted and arg1 itself assigned the wrapped byte offset (`arg1 = tmp * 4; if ((s16) tmp >= (s32)arg0[3]) arg1 = 0;`), the address formed post-call as `(s16 *)(arg1 + (s32)table)`. Censused with batch.sh, then measured.
- result: The priority arithmetic works exactly as predicted - pseudo 73 (arg1) reaches 7 refs / 32 insns = 2*7/32 = 0.4375 and TAKES $s2 - but it overshoots arg0 (0.270) too, giving a three-way rotation ($s2 = arg1-carrier, $s3 = arg0, $s4 = table) rather than the target's map, and arg3 is still pushed to hard reg 30 ($fp/$s8). Score 50 @ 111 insns. The window the flip needs is 0.003 wide (between table's 0.267 and arg0's 0.270) and no integral (n, L) pair available to the carrier lands inside it.
- verdict: KILLED

## [s42] EVERY ban-compliant post-call-address form carries one supernumerary live-across-call quantity, takes a NINTH callee-save ($fp/$s8), and therefore cannot be byte-identical to a target that saves only $s0-$s7 - which forecloses s41 frontier items #1 and #2 outright, independently of whether their seat flips succeed.
- mechanism: The target's carried set has exactly eight members (arg0, next-INDEX, raw cx, raw cy, cxs, cys, arg2, arg3): asm/funcs/func_80057CC8.s:3-16 stores $ra + $s0..$s7 at 0x18..0x38 and the body contains no $fp/$s8 reference at all. It gets away with eight only because the vertex-table base is re-derived after the call from arg0 (`lw $a0,0x4($s2)`, asm/funcs/func_80057CC8.s:50) - the second source-level materialization refused by the owner on 2026-07-20. A ban-compliant form must instead CARRY the base, making the set nine.
- probe: Five independent post-call forms censused for arg3's (pseudo 75's) hard register in the .greg 'Register dispositions' table: e1.c (s41's seat-flip form, carried table + fresh off), g1.c (carrier = reused arg1), h1.c (`scale = arg0[2] * 40` hoisted above the prev-if so arg0 dies before the first call, index carried in the target's own sll-16 / sra-14 shape), h2.c (same with a byte offset), h3.c (h1 + the &Judge staging lever). All applied and scored.
- result: All five place pseudo 75 in hard reg 30 = $fp/$s8. Scores: e1 41 @ 110, g1 50 @ 111, h1 55 @ 112, h2 54 @ 111, h3 55 @ 112. The h-family shows the 'let arg0 die early' dodge cannot work: arg0 does become caller-save (pseudo 72 -> $t0) but `scale` (pseudo 80) crosses both calls in its place ('used 3 times across 64 insns; crosses 2 calls'), so the count is unchanged at nine. The pre-call regimes need only eight (base16/a1 put arg3 in hard reg 23 = $s7, no $s8).
- verdict: CONFIRMED

## [s42] s41 frontier item #3: a branch other than the prev-if can be made to sit between the next-address's single def and its two `lh` uses, so d1's cross-block n=3 address can be obtained WITHOUT d1's wrap-before-prev block order.
- mechanism: d1 proved the n=3 cross-block address only needs SOME branch in that window; s41 hypothesised another branch could legitimately occupy it in the target's prev-first layout, which would combine d1's exact seat map with the target's instruction order.
- probe: Enumeration of the conditionals this function's semantics actually admit before the ang_next argument reads, cross-checked against the already-banked prev-first controls rejected/s40c-prevfirst-h35-scale-between-calls-noflip-score42.c and rejected/s40c-prevfirst-h35-scale-after-angnext-noflip.c.
- result: Exactly two conditionals exist before the ang_next reads (the prev-index wrap and the next-index wrap), plus the ang_mid if/else, which is strictly AFTER both `lh` uses and can never separate them from the def. A single-def next-address is by construction defined at or after the next-wrap merge, so the next-wrap branch is always before it. Only the prev-if remains, and placing the prev-if after the address def IS d1's wrap-before-prev order. Any additional branch would have to be invented with no semantic purpose (the dead-conditional family).
- verdict: KILLED

## [s43] (s42 frontier item #2) A never-run permuter campaign inside Regime A -- the
108-insn merge-offset regime -- seeded from TWO source-distinct but register-equivalent
chassis can beat 16, because s42 proved the regime admits structurally distinct source
shapes at identical score and therefore has real unexplored shape freedom.
- mechanism: s37 already proved this function's 108-insn regime is permuter-productive
  (the 20 -> 16 find came from it), and permuter mutations move exactly the statement-order
  and expression-shape inputs that cse1 and sched1 consume.  s42 additionally showed a1.c
  (s32 base + one reused cursor) and base16.c/candidate.c (s16* table + dedicated
  next_vert) are wholly different source shapes landing on the SAME 108 instructions and
  the SAME allocation -- so the two shapes are independent seeds into the same basin, and a
  campaign from both explores strictly more of it than s37's single-chassis run.
- probe: two workspaces rebuilt with the s37 recipe (`tmp/grind/func_80057CC8/s43/`
  build_wsA.sh from memory/grind/func_80057CC8/candidate.c, build_wsB.sh from
  tmp/grind/func_80057CC8/s42/a1.c; both sanity-gated at base 108 / target 111 insns).
  Launched as campaigns `s43-regimeA-candidate` and `s43-regimeA-siblingidiom`, -j 4 each,
  waited IN-TURN over three fresh-seed windows (313 s + 542 s on A, 115 s + 542 s on B),
  then harvested with --stop.  Totals: 27,866 iterations on A and 27,827 on B = **55,693
  iterations**, 8 outputs (A: 758, 593; B: 758, 738, 676, 640, 593, 583).  Every output was
  read in full and hand-checked for semantic equivalence before any measurement.
- result: **Seven of the eight outputs are SEMANTICS-BREAKING, and six of the seven break
  in the SAME way** -- a value is read on a path where it was never assigned.  Four of them
  (A-593, B-593, B-583, and structurally B-676) achieve it by sinking the next-address
  assignment INSIDE the next-wrap `if` (brace depth 3 instead of the legal depth 2), so the
  address pseudo is undefined on the fall-through path; B-583 additionally sources it from
  an uninitialised `new_var2`; B-640 inlines the second `ratan2` into the `if` condition and
  leaves `ang_next` -- still read in the else arm -- never assigned; B-676 sinks
  `new_var3 = (s16) cx;` to the very end of the function while reading `new_var3` as a
  ratan2 argument near the top.  **Exactly ONE output is semantics-preserving: B-738**,
  which reuses the existing local `pi` to carry `cx` (`(s16)(pi = cx)` == `(s16) cx`, and
  the later `*arg2 = pi` == `*arg2 = cx`).  Applied and measured: **sandbox 23 @ 111
  insns** -- the first Regime-A form ever to reach the target's exact instruction count,
  but strictly worse than 16 in score.  Its greg census
  (`tmp/grind/func_80057CC8/s43/p738.greg`) says why: the reused `pi` becomes a NEW
  call-crossing allocno (pseudo 85, "used 5 times across 44 insns; crosses 2 calls") that
  takes $s2, which pushes the next-address (pseudo 88) UP to $s1 -- the seat the target
  gives cxs -- and arg0 (pseudo 72) DOWN to $s3.  The rotation moves further from the
  target, not toward it.
- verdict: **KILLED.**  s42 frontier item #2 is measured dead: 55,693 iterations across two
  source-distinct chassis surfaced no legal form below 16.  The finding that matters is the
  SHAPE of the improving basin -- every single improving find wins by leaving the
  next-address pseudo (or another crossing value) PARTIALLY DEFINED.  That is not a spelling
  the permuter happened to pick; it is the only thing that lowers the address allocno's
  priority, and it is unreachable in legal C by construction.  Banked as
  `rejected/s43-permuter-partial-def-address-SEMANTICS-BROKEN-perm593.c` and
  `rejected/s43-permuter-pi-reused-as-cx-carrier-score23-111insns.c`.

## [s43] The Regime-A seat rotation is not merely "the address outranks arg0" (s38b Horn 1)
but outranks it by more than an order of magnitude, so no live-range or reference-count
manipulation available to legal C can flip it.
- mechanism: global.c:635 allocno_compare orders by floor_log2(n) * n / live_length.  s43's
  two censuses give both operands directly.  In the score-16 chassis (b16 = a1.c) the
  next-address is pseudo 88, "used 6 times across 4 insns" -> floor_log2(6) * 6 / 4 =
  2 * 6 / 4 = **3.00**; arg0 is pseudo 72, "used 5 times across 54 insns" -> 2 * 5 / 54 =
  **0.185**.  The ratio is **16.2x**.  In the p738 chassis the address stretches to "6 times
  across 6 insns" -> 2.00, and arg0 to 57 insns -> 0.175: still **11.4x**.
- probe: the two `Register dispositions` + lreg tables produced by
  tmp/grind/func_80057CC8/s43/batch.sh over p738.c and s42/a1.c (banked as
  tmp/grind/func_80057CC8/s43/{p738,b16}.greg / .lreg).
- result: For the address to fall below arg0 it needs 2 * n / L < 0.185.  Its last use is
  fixed by semantics -- the second ratan2's two `lh` argument reads, which sit at roughly
  insn 55 of a 108-insn body -- so L <= 55 even if the def were hoisted to the function's
  first instruction, which it cannot be (the def depends on the next-wrap test, which
  depends on `arg0[3]`).  At n = 6 the best attainable is 12 / 55 = 0.218, still above
  arg0's 0.185; only n <= 4 (2 * 4 / 55 = 0.145) would clear it, and n is pinned at 6 by
  the two `lh` reads plus the address arithmetic.  Every one of those three quantities is
  semantically required.
- verdict: **CONFIRMED (closed-form).**  This sharpens s38b Horn 1 from a qualitative
  "always outranks" into a quantitative bound: the flip needs a >= 16x priority swing and
  legal C can move it by at most ~1.4x.  It also explains the s43 permuter result exactly --
  partial definition is the ONLY mutation that changes the address's priority by enough,
  because it removes the def from the dominant path rather than merely stretching it.

## [s43] The rederive-modality re-derivations (fresh m2c decompile of the target, plus the
prior sibling/Kengo/corpus axes) produce no shape that is not already banked.
- mechanism: rederive modality's mandate is a structurally DIFFERENT C shape, not a tweak.
- probe: fresh `python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f
  func_80057CC8 asm/funcs/func_80057CC8.s` on the current chassis (output in the s43
  session log); compared against the banked regime partition.
- result: m2c's output is Regime C by construction -- it emits `temp_a2 =
  M2C_FIELD(arg0, s32 *, 4)` for the centre/prev address AND a second, independent
  `M2C_FIELD(arg0, s32 *, 4)` for the next address, exactly reproducing the target's
  `lw $a2,0x4($s2)` / `lw $a0,0x4($s2)` pair.  That is the 2026-07-20 owner-refused second
  source-level materialization and the head of this function's banned_constructs list, so
  the machine re-derivation lands squarely on the banned form and offers no new legal shape.
  The only genuinely new spelling detail it contributes -- the prev-wrap test written as a
  bit test `if (temp_v1 & 0x8000)` rather than a signed compare -- is byte-equivalent to the
  candidate's `if ((s16) prev_idx < 0)` on this chassis (both emit `sll 16` + `bgez`).
  The sibling-transplant axis was spent at s42 (a1/a2, register-equivalent) and the SOTN /
  decomp.me corpus axis at s17 (unfindable in the accessible slice).
- verdict: **KILLED.**  The rederive ladder has no unspent rung for this function: machine
  re-derivation reproduces the banned form, sibling transplant is register-invariant, and
  the corpus is empty.

## [s43] (s42 frontier #2) A never-run permuter campaign inside Regime A, seeded from TWO source-distinct but register-equivalent chassis (candidate.c and s42 a1.c), can beat 16, because s42 proved the regime admits structurally distinct source shapes at identical score and therefore has real unexplored shape freedom.
- mechanism: s37 already proved the 108-insn merge-offset regime is permuter-productive (the 20->16 find came from it); permuter mutations move exactly the statement-order and expression-shape inputs cse1 and sched1 consume. s42 showed a1.c (s32 base + one reused cursor) and candidate.c (s16* table + dedicated next_vert) are wholly different sources landing on the same 108 instructions and the same allocation, so they are independent seeds into the same basin.
- probe: Rebuilt two workspaces with the s37 recipe (tmp/grind/func_80057CC8/s43/build_wsA.sh, build_wsB.sh), both sanity-gated at base 108 / target 111 insns. Launched campaigns s43-regimeA-candidate and s43-regimeA-siblingidiom at -j 4, waited IN-TURN over three fresh-seed windows (313s + 542s on A, 115s + 542s on B), harvested both with --stop. Every output read in full and hand-checked for semantic equivalence before measurement.
- result: 27,866 + 27,827 = 55,693 iterations, 8 outputs (A: 758, 593; B: 758, 738, 676, 640, 593, 583). SEVEN are semantics-breaking: A-593, B-593 and B-583 sink the next-address assignment inside the next-wrap if (brace depth 3 instead of the legal depth 2) so the address is undefined on the fall-through path (B-583 additionally sources it from an uninitialised new_var2); B-640 inlines the second ratan2 into the if condition leaving ang_next never assigned though the else arm reads it; B-676 sinks new_var3 = (s16) cx; to the last line while reading new_var3 as a ratan2 argument near the top. The ONE semantics-preserving find, B-738, reuses the existing local pi to carry cx ((s16)(pi = cx) == (s16) cx; *arg2 = pi == cx) and measures sandbox 23 @ 111 insns - the first Regime-A form at the target's exact instruction count, but its greg census puts $s1 = next-address (88), $s2 = the reused pi/cx carrier (85, 5 refs / 44 insns, crosses 2 calls), $s3 = arg0 (72), i.e. the rotation moves FURTHER from the target than the score-16 candidate.
- verdict: KILLED

## [s43] The Regime-A seat rotation is not merely 'the address outranks arg0' (s38b Horn 1) but outranks it by more than an order of magnitude, so no live-range or reference-count manipulation available to legal C can flip it.
- mechanism: global.c:635 allocno_compare orders by floor_log2(n) * n / live_length. The s43 lreg/greg censuses give both operands directly for two different chassis.
- probe: tmp/grind/func_80057CC8/s43/batch.sh over p738.c and s42/a1.c, reading the 'Register dispositions' table plus the lreg 'used N times across L insns' lines (banked as tmp/grind/func_80057CC8/s43/{p738,b16}.greg and .lreg).
- result: Score-16 chassis: next-address pseudo 88 = 6 refs / 4 insns -> 2*6/4 = 3.00; arg0 pseudo 72 = 5 refs / 54 insns -> 2*5/54 = 0.185; ratio 16.2x. p738 chassis: address 6/6 -> 2.00, arg0 5/57 -> 0.175; ratio 11.4x. The address's last use is pinned by semantics to the second ratan2's two lh argument reads at ~insn 55 of a 108-insn body and its def cannot precede the next-wrap test (which needs arg0[3]), so L <= 55; even there 2*6/55 = 0.218 still exceeds 0.185, and n cannot drop below 6 (two lh reads plus the address arithmetic are all semantically required). Legal C moves the ratio by at most ~1.4x against a required 16x.
- verdict: CONFIRMED

## [s43] The rederive ladder still has an unspent rung: a fresh m2c decompile of the target on the current chassis yields a structurally different, ban-compliant C shape.
- mechanism: rederive modality mandates a structurally DIFFERENT shape, not a tweak; m2c re-derives the source from the bytes independently of 42 sessions of ledger bias.
- probe: python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_80057CC8 asm/funcs/func_80057CC8.s on the live chassis, compared against the banked three-regime partition.
- result: m2c's output is Regime C by construction: it emits temp_a2 = M2C_FIELD(arg0, s32 *, 4) for the centre/prev address AND a second independent M2C_FIELD(arg0, s32 *, 4) for the next address, reproducing the target's lw $a2,0x4($s2) (asm/funcs/func_80057CC8.s:17) / lw $a0,0x4($s2) (:50) pair exactly - the 2026-07-20 owner-refused second source-level materialization and the head of banned_constructs. The machine decompiler thus independently confirms the ORIGINAL source performed that duplication. The only novel spelling it contributes, the prev-wrap test as if (temp_v1 & 0x8000) instead of if ((s16) prev_idx < 0), is byte-equivalent on this chassis (both emit sll 16 + bgez). Sibling transplant was spent register-invariantly at s42; the SOTN/decomp.me corpus axis was spent at s17 (unfindable in the accessible slice).
- verdict: KILLED

## [s44] (structural, 2026-08-27)

- **H-s44-1 — KILLED.** *Statement:* in the Regime-B (wrap-first) block order, a spelling
  exists in which the prev-wrap arm's vertex-count read is not dominated by the wrap test,
  so the target's second `lbu 3($s2)` survives cse1 and Regime B's 45-point residual drops
  substantially.  *Mechanism:* cse1 fuses two `mem:QI` reads of `arg0+3` when the first
  dominates the second (s41 d8); reading the count through a different rtx breaks the
  fusion.  *Probe:* `tmp/grind/func_80057CC8/s44/p1.c` — s41 d1 with the prev arm reading
  `(u8)(*(u16 *)(arg0 + 2) >> 8)` (same value on little-endian, `mem:HI` at +2, so
  unfusable).  *Result:* the second read DOES survive; measured **score 44 @ 109 insns**
  versus d1's 45 @ 106.  *Verdict:* KILLED — the fusion is worth exactly one point, so the
  29-point Regime-A-to-Regime-B gap is not the missing count read.  Banked as
  `rejected/s44-regimeB-distinct-count-read-second-lbu-survives-score44.c`.

- **H-s44-2 — CONFIRMED (a foreclosure).** *Statement:* Regime B cannot reach distance 0 at
  any score, because its block order is inverted relative to the target and s42 proved that
  inversion is structurally required by Regime B's own defining property.  *Mechanism:* a
  single-def next-address is defined at or after the next-wrap merge, so only the prev-if
  can separate def from uses (s42 closed form) — which forces prev-if AFTER wrap-if; the
  target's order is prev-if FIRST.  *Probe:* read the emitted branch order off
  `tmp/grind/func_80057CC8/s44/d1.s` (`bne $2,$0,.L277` on the next-wrap test precedes
  `bgez $2,.L274` on the prev-wrap test) against `asm/funcs/func_80057CC8.s:18-26` vs
  `:30-36`.  *Result:* inverted, as predicted.  *Verdict:* CONFIRMED — Regime B is
  foreclosed on block order alone, independently of the lbu and address-formation residuals.
  s43 frontier item #1 is spent.

- **H-s44-3 — CONFIRMED (closed form, supersedes the partition argument).** *Statement:*
  distance 0 for `func_80057CC8` requires two source-level materializations of
  `*(s16 **)(arg0 + 4)`, i.e. the construct refused by the owner on 2026-07-20, so no
  ban-compliant C form can match at any score.  *Mechanism:* the target forms the
  next-vertex address post-call from `$s3` (the next index) and a base loaded at
  `asm/funcs/func_80057CC8.s:50`; its live-across-call set is exactly the eight callee-saves
  established at `:3-16` (arg0, next-index, raw cx, raw cy, cxs, cys, arg2, arg3), none of
  which is or can be converted into the vertex-table base; GCC 2.7.2 can emit the second
  load only from a second unfusable source materialization (cse cannot fuse across the
  intervening `jal`) or by reload rematerialization, and rematerialization was enumerated and
  killed in s36.  *Probe:* register-file enumeration against the target listing plus the s36
  rematerialization result and the s43 m2c re-derivation (which independently emits the
  banned pair).  *Result:* no ban-compliant source of the post-call base exists.
  *Verdict:* CONFIRMED — **16 is the complete ban-compliant floor**, and the foreclosure is
  a property of the target, not of the three-regime partition.

- **Standing note for the next session.** There is no remaining structural axis.  Do NOT
  re-seed Regime-A permuter campaigns (s43), do NOT re-spell Regime C (s42), do NOT re-open
  the Regime-B order attack (this session).  Do NOT file an escalation packet asking to
  re-scope the 2026-07-20 refusal — that is auto-reject class under the owner's 2026-08-24
  second ruling (its YES would lower a standard).  The characterisation is complete; the
  disposition is the driver's call.

## [s44] In the Regime-B (wrap-first) block order a spelling exists in which the prev-wrap arm's vertex-count read is not dominated by the wrap test, so the target's second `lbu 3($s2)` survives cse1 and Regime B's 45-point residual drops substantially.
- mechanism: cse1 fuses two mem:QI reads of arg0+3 when the first dominates the second (s41 d8); reading the count through a different rtx breaks the fusion, restoring the target's two-lbu shape (asm/funcs/func_80057CC8.s:24 and :31).
- probe: tmp/grind/func_80057CC8/s44/p1.c - the s41 d1 Regime-B chassis with the prev arm reading `(u8)(*(u16 *)(arg0 + 2) >> 8)` (same value on little-endian, mem:HI at +2, unfusable with the mem:QI at +3); measured with sandbox --disable all and censused with s44/batch.sh.
- result: The second count read DOES survive - the frontier's requested separation is achievable. Measured score 44 @ 109 insns, against d1's 45 @ 106 and the candidate's 16 @ 108. One point.
- verdict: KILLED

## [s44] Regime B cannot reach distance 0 at any score because its block order is inverted relative to the target, and s42 proved that inversion is required by Regime B's own defining cross-block-address property.
- mechanism: A single-def next-address is defined at or after the next-wrap merge, so only the prev-if can separate def from uses - forcing prev-if AFTER wrap-if. The target's order is prev-if FIRST.
- probe: Read the emitted branch order off tmp/grind/func_80057CC8/s44/d1.s (`slt $2,$2,$6 / bne $2,$0,.L277` on the next-wrap test precedes `sll $2,$2,16 / bgez $2,.L274` on the prev-wrap test) against asm/funcs/func_80057CC8.s:18-26 (prev-wrap, with `lbu $v0,0x3($s2)` in the arm) vs :30-36 (next-wrap).
- result: Inverted exactly as predicted. Regime B's register map and the target's block order are mutually exclusive, so the lbu and 4-vs-1 address-formation residuals are moot.
- verdict: CONFIRMED

## [s44] Distance 0 for func_80057CC8 requires two source-level materializations of *(s16 **)(arg0 + 4) - the construct the owner refused on 2026-07-20 - so no ban-compliant C form can match, independently of the three-regime partition.
- mechanism: The target forms its next-vertex address AFTER the first ratan2 call from the next index in $s3 and a base loaded at asm/funcs/func_80057CC8.s:50 (`lw $a0,0x4($s2)`). Its live-across-call set is exactly the eight callee-saves established at :3-16 (arg0, next-index, raw cx, raw cy, cxs, cys, arg2, arg3; no $fp/$s8 anywhere, s42), and $a2 (the pre-call base) is dead across the call. None of those eight is, or can be arithmetically converted into, the vertex-table base, so the post-call base can only come from a fresh load of arg0. GCC 2.7.2 emits a second load of the same location only from (a) a second source materialization cse1/cse2 cannot fuse - and they cannot, an intervening jal invalidates memory refs - or (b) reload rematerialization, enumerated and killed in s36.
- probe: Register-file enumeration against the target listing (asm/funcs/func_80057CC8.s:3-16, :17, :49-52) combined with the s36 rematerialization kill and s43's independent m2c re-derivation, which emits exactly the banned pair.
- result: No ban-compliant source of the post-call base exists. 16 is the complete ban-compliant floor and the foreclosure is a property of the target, not of the partition.
- verdict: CONFIRMED
