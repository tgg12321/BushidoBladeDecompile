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
