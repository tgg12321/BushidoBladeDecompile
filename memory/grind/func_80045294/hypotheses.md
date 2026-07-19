# Hypothesis ledger — func_80045294

## s1 (recon, 2026-07-18) — floor 2

- **H1 KILLED**: "init `i = a0` before `v1 = a0 << 4` reproduces target's move-before-sll order."
  Measured: score 2 → 11. RA rotates (a0→$21, s4→$18, s5→$20) and cse rewrites `sll` operand to
  $16. Rejected form: rejected/i-before-v1-init.c. Corollary CONFIRMED: original source has
  v1-before-i; residual is scheduler-side only.
- **H2 KILLED (neutral)**: "s5-before-count init order flips the sll/move16 tie." Score stays 2,
  same residual.
- **H3 KILLED (neutral)**: "count-before-i init order flips the tie." Score stays 2.
- **H4 KILLED**: "fold shift into s4 initializer, re-derive v1 after i (CSE reuse keeps one sll
  but reorders RTL)." Builds 84/83 insns — extra copy insn. rejected/cse-reuse-shift.c.

## Frontier (for s2, drill)

1. **Read the tie directly**: preprocess text1a_c.c, run cc1 with build flags + `-da` (and/or a
   BB2_SCHED_DEBUG instrumented cc1) on the current form; read INSN_PRIORITY of the sll vs the
   i=a0 move in sched2, and confirm the addu $21 (s5) chain is what elevates sll. Then derive a
   C spelling that changes the priority relation. Mechanism: sched.c priority = longest downstream
   dependence chain.
2. **Permuter campaign** with a CLEAN single-function target.o (build from target bytes @0x35A94;
   see diff_target.sh) sweeping prologue-region spellings: guard forms (`if (i<count) do..while`
   vs `while`/`for`), v1/s4/s5 expression spellings, statement placements — cross-product beyond
   the 4 hand probes. Fresh-seed discipline per feedback/permuter-fresh-seed-discipline.
3. **s5 chain restructuring**: the sll priority comes from sll→addu$1→lw$20→addu$21. Find a
   spelling where s5 = s4 + a1 is expressed so its addu doesn't extend sll's chain during sched
   (e.g. compute s5 from the reloaded value or a different equivalent expression) while still
   landing addu $21,$20,$19 in the beqz delay slot. NOTE: must stay pure-C semantic — no dead
   stores/barriers.

## [s1] Current candidate.c (v1-before-i init order) sustains sandbox=2 on the honest pure-C sandbox.
- mechanism: Sandbox --disable all runs the full build with regfix/asmfix disabled and cheat-asm stripped; the resulting objdump score is the honest pure-C distance.
- probe: tools/wteng.ps1 main sandbox func_80045294 --disable all
- result: score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 (file-wide). Artifact: tmp/grind/func_80045294/s1/sandbox_recon.txt.
- verdict: CONFIRMED

## [s1] The residual is purely a sched2 ordering of two independent prologue insns; RA and all downstream blocks are byte-identical.
- mechanism: GCC 2.7.2 sched2 priority = longest downstream dependence chain; sll $3,$18,4 feeds sll->addu $1->lw $20->addu $21(beqz delay), while move $16,$18 feeds only slt/beqz. Longer chain wins tie, so sll emits first in build; target emits move16 first.
- probe: tmp/grind/func_80045294/s1/diff_baseline.txt (prior s1) + current sandbox output shows 83 vs 83 insns with only the 3-insn prologue cluster diverging.
- result: Residual confirmed 3-insn cluster; every other insn matches.
- verdict: CONFIRMED

## [s1] Re-ordering C init to place `i = a0` before `v1 = a0<<4` reproduces target's move-before-sll order.
- mechanism: Statement ordering steers pseudo creation order and thus tree_LUID / RA priority.
- probe: rejected/i-before-v1-init.c built and sandboxed.
- result: Score rose 2 -> 11: RA rotated (a0->$21, s4->$18, s5->$20) and CSE rewrote sll operand to $16.
- verdict: KILLED

## [s1] Folding the shift into s4's initializer and re-deriving v1 after `i = a0` (CSE-reuse spelling) preserves insn count.
- mechanism: CSE should reuse the single sll; re-derived v1 becomes a copy, but net insn count depends on RA copy elision.
- probe: rejected/cse-reuse-shift.c built and sandboxed.
- result: 84 build_insns vs 83 target: one extra copy insn survived CSE.
- verdict: KILLED

## [s2] Rewriting the guard as a plain `while (i<count)` instead of `if (i<count) do..while(i<count)` flips or breaks the sched2 tie between sll and move16.
- mechanism: Different tree/RTL loop shape could change block0's insn set and thus the sched2 priority computation; SOTN uses `while` for guarded loops in matched forms.
- probe: Replaced the guarded do-while in src/text1a_c.c with a plain while; ran sandbox --disable all.
- result: score=2 build_insns=83 target_insns=83 — identical shape. GCC 2.7.2 lowers the two forms to the same CFG for this body.
- verdict: KILLED

## [s2] Delaying `s32 s5 = s4 + a1;` to inside `if (sum != 0)` (where s5 is actually consumed) still lets reorg.c fill the beqz delay slot with addu $21.
- mechanism: s5 is only used in the sum!=0 branch; hoisting its definition inward could let sched2 pick a different priority ordering, without harming delay-slot fill because $21 is still computed before its use.
- probe: Moved `s32 s5 = s4 + a1;` from the pre-guard init cluster to the top of the if(sum!=0) block; sandboxed.
- result: score rose 2 -> 13, build_insns dropped to 79 (target 83). Losing the pre-guard s5 computation removes the delay-slot filler and cascades 13 register-choice diffs downstream. Confirms s5 MUST be computed pre-guard so addu $21,$20,$19 lands in the beqz delay slot.
- verdict: KILLED

## [s2] Swapping operand order to `s32 s5 = a1 + s4;` (a1 before s4) perturbs the sched2 tie by changing addu $21's operand-choice order.
- mechanism: GCC 2.7.2 addu isn't canonicalized commutatively at RTL creation time; operand order can change register-choice heuristics.
- probe: Edited `s5 = s4 + a1` to `s5 = a1 + s4`; sandboxed.
- result: score=3 (up from 2), build_insns=83. Same insn count but +1 register-choice diff — addu $21 now uses different pseudo assignments. sll-vs-move16 tie unchanged. Net worse.
- verdict: KILLED

## [s3] GCC 2.7.2 sched2 assigns sll(a0<<4) a HIGHER INSN_PRIORITY than move(s0=s2) because its downstream chain is longer (s1/s2 CONFIRMED evidence).
- mechanism: sched.c INSN_PRIORITY = longest downstream data-dependence chain.
- probe: Ran `tools/gcc-2.7.2/build/cc1 <build-flags> -da base.i` on the current candidate; parsed tmp/grind/func_80045294/s3/base.i.sched2 (line 20111-20155 for block 0).
- result: sched2 dump shows insn 14 (sll) priority=1 AND insn 22 (move16) priority=1. BOTH have priority 1. The chain-length hypothesis is falsified by measurement — the priorities are equal, so the tie is not decided by priority at all.
- verdict: KILLED

## [s3] sched2 tie between equal-priority ready insns is broken by INSN_LUID with LOWER LUID emitted FIRST (naive tie-stability reading).
- mechanism: sched.c rank_for_schedule() final tiebreak returns INSN_LUID(tmp) - INSN_LUID(tmp2), i.e., INSN_LUID(y) - INSN_LUID(x).
- probe: Read tools/gcc-2.7.2/sched.c:2398-2456. Cross-referenced insn UIDs (sll=14, move16=22) against the T-9 ready-list order in sched2 dump: `now 22 14 12 6` (HIGHER LUID at front). qsort compare(x=22,y=14) = 14-22 = -8 < 0 → x=22 sorts BEFORE y=14. sched2 is backward-list-sched: picked-first = scheduled at deeper T = emitted LATER.
- result: Higher LUID is picked first at the ready list → emitted LATER in forward stream. Move16 (LUID 22) emitted at position 10; sll (LUID 14) at position 8. To flip to target order (move16 at 9, sll at 10), sll's assignment LUID must EXCEED move16's LUID — i.e., `i = a0` (source of move16) must appear BEFORE `v1 = a0<<4` (source of sll) in the C.
- verdict: CONFIRMED

## [s3] Splitting declaration from initialization (`s32 v1; s32 i = a0; ... v1 = a0<<4;`) decouples RA allocno priority from sched2 LUID.
- mechanism: Hypothesis: global.c's allocno priority might follow declaration LUID rather than assignment LUID; if so, declaring v1 first while assigning it later would keep RA correct (a0->$18) while giving move16 a lower LUID.
- probe: src/text1a_c.c edited to `s32 v1; s32 s4; s32 i = a0; s32 count; s32 s5; v1 = a0<<4; s4 = ...; count = D_800A33AC; s5 = s4 + a1;`. Sandboxed.
- result: score rose 2 -> 11 (identical shape to H1). RA rotated as under H1 (a0->$21, s4->$18, s5->$20) AND sll operand rewrote to $16. Confirmed: RA priority follows ASSIGNMENT LUID, not declaration LUID. sched2 LUID and RA priority are NOT independently steerable via decl/init split. Saved as rejected/decl-init-decouple.c.
- verdict: KILLED

## [s3] Splitting s5 into two-statement accumulation (`s32 s5 = s4; s5 += a1;`) is neutral or perturbs the sched2 tie beneficially.
- mechanism: User-sanctioned split-init-accumulation lever (feedback/split-init-accumulation-sanctioned.md). Two-insn form for s5 could shift the ready-list membership at the s5 scheduling step.
- probe: Edited to `s32 s5 = s4; s5 += a1;`. Sandboxed.
- result: score rose 2 -> 11. The extra copy insn breaks the addu $21 delay-slot fill AND cascades the same RA rotation as H1. Split-init-accumulation is HARMFUL here because target's RTL has NO extra copy at s5. Saved as rejected/split-init-s5.c.
- verdict: KILLED

## [s3] Init-order permutation `sum, v1, i, s4, count, s5` (i between v1 and s4, keeping v1-before-i) is neutral.
- mechanism: Additional axis of the H2/H3 neutral cluster in s1; the constraint from H1 is only v1-before-i, other permutations should be neutral.
- probe: Edited i's init to line 3 (between v1 and s4). Sandboxed.
- result: score = 2, build_insns = 83 (neutral). Confirms init position of i within {i, s4, count, s5} is a free axis as long as v1-before-i.
- verdict: KILLED

## [s3] Rewriting the guard+loop as `for (i = a0; i < count; ) { ... i += 1; ... }` (i's init inside the for-clause) shifts sched2's tiebreak.
- mechanism: GCC 2.7.2 expands for-init as the first stmt of the loop nest, which could steer i's assignment LUID differently than a bare `s32 i = a0;`.
- probe: Kept `s32 i;` at the pre-guard position; moved i's assignment into `for (i = a0; i < count; )`. Sandboxed.
- result: score = 2, build_insns = 83 (neutral). The for-init shape lowers to the same RTL LUID for i's assignment as the bare `s32 i = a0;` — no tie movement.
- verdict: KILLED

## [s3] Comma-assignment sub-expression `s32 v1; s32 s4 = *(s32*)((u8*)&D_800EED14 + (v1 = a0<<4));` yields the target schedule while emitting sll directly into v1 (no extra copy).
- mechanism: The comma-assignment could steer GCC's tree so the sll's assignment LUID lives INSIDE s4's initializer tree (later than i=a0's LUID), while forcing the sll's result to land in v1's pseudo without a temp.
- probe: Edited to `s32 v1; s32 s4 = *(s32*)((u8*)&D_800EED14 + (v1 = a0<<4)); s32 i = a0; ...`. Sandboxed + objdump.
- result: score = 2, build_insns = 83 — IDENTICAL to baseline shape. objdump shows `sll v1,s2,4 ; sw s0,16(sp) ; move s0,s2` unchanged. GCC 2.7.2 desugars the comma-assignment sub-expression to the same tree as a preceding `s32 v1 = a0<<4;` — no LUID movement. Frontier hypothesis #1 killed at first probe.
- verdict: KILLED

## [s3] Inlining a0<<4 into s4's initializer (with v1 re-declared after as CSE-reuse) is a near-hit that gets the target's schedule ORDER right.
- mechanism: The anonymous subexpression `(a0<<4)` inside `s4 = *(...+(a0<<4))` creates the sll's pseudo at a LATER tree LUID than move16's; move16's LUID then wins the sched2 tie in target's direction (emitted later, matching target's `sw ; move16 ; sll` order).
- probe: Edited to `s32 s4 = *(s32*)((u8*)&D_800EED14 + (a0<<4)); s32 i = a0; ... s32 v1 = a0<<4;`. Sandboxed + objdump.
- result: score = 2, build_insns = 84 (target 83). Schedule order MATCHES target: `sw s0,16(sp) ; move s0,s2 ; sll v0,s2,4`. But the CSE-fold produces one extra insn: `move v1,v0` after the sll, because a0<<4 goes to an anonymous pseudo (v0) and v1 is a separate declared pseudo — RA emits a copy. Saved as rejected/cse-fold-anon-shift.c with the closing-lever hypothesis in comments.
- verdict: CONFIRMED

## [s4] Moving v1 into the guard-block scope (v1 declared inside `if (i<count) { s32 v1 = a0<<4; do { ... } }` with a0<<4 inlined anonymously into s4's file-scope init) will let CSE fold the two a0<<4 uses to one insn while removing v1's outer pseudo, avoiding the CSE-move copy that killed cse-fold-anon-shift.
- mechanism: Cross-scope CSE across (outer-block s4-init subexpression) and (inner-loop-scope v1-init) hypothetically shares the anon shift pseudo; removing v1's outer scope removes one pseudo class from RA priority accounting.
- probe: Edited src/text1a_c.c to move `s32 v1 = a0<<4;` inside the `if (i<count)` guard, inlined a0<<4 into s4's file-scope init. Sandbox --disable all.
- result: score=35, build_insns=84 (target 83). Cross-scope CSE did NOT fold: the loop's inner `v1 = a0<<4` produces a fresh pseudo, extra insn survives, RA cascades a 35-point register-choice diff.
- verdict: KILLED

## [s4] Declaring `s32 v1;` uninitialized before all other decls and assigning `v1 = a0<<4;` late (as a statement after s5's init) while s4's init inlines a0<<4 anonymously will decouple RA priority from sched2 LUID and give the target schedule at 83 insns.
- mechanism: Early decl gives v1 its RA priority slot; late statement pushes the explicit v1 assignment's LUID after i's, satisfying the sched2 tiebreak; s4's inline anon shift provides the actual sll for the schedule.
- probe: Edited src/text1a_c.c to `s32 v1; s32 sum=0; s32 s4 = *(base + (a0<<4)); s32 i = a0; s32 count = ...; s32 s5 = s4+a1; v1 = a0 << 4;`. Sandbox --disable all + objdump.
- result: score=2, build_insns=84. Schedule order MATCHES target (`sw $16 ; move $16 ; sll $2,$18,4`) — from a different C angle than cse-fold-anon-shift — but the CSE-move copy (`move $3,$2`) is present as always. Confirms the LUID lever works from a DIFFERENT SPELLING; the +1 insn blocker is inherent to any C with two tree-level a0<<4 expressions.
- verdict: KILLED

## [s4] Wrapping s4/s5 decls (and the whole function body) in an inner `{ }` block AFTER a statement-position `v1 = a0<<4;` will create a scoping boundary that decouples RA priority (which follows global.c allocno order across the whole function) from the ASSIGNMENT LUID axis, letting the late v1 assignment steer sched2 without rotating a0's register.
- mechanism: Hypothesized global.c might allocate pseudos scoped by block, so an outer-block v1 with a late statement position could have a low RA priority while the inner-block s4/s5/loop pseudos allocate independently.
- probe: Edited src/text1a_c.c: `s32 v1; s32 sum=0; s32 i=a0; s32 count=...; v1 = a0<<4; { s32 s4 = *(base+v1); s32 s5 = s4+a1; ... rest ... }`. Sandbox --disable all + objdump.
- result: score=11, build_insns=83. RA rotated EXACTLY like H1 (a0->$21, sll operand CSE-propagated to $16). global.c does NOT allocate per-block; it allocates all function-wide pseudos in one phase. THIRD angle confirming RA priority is coupled to assignment LUID at C-source level.
- verdict: KILLED

## [s5] Target's first-loop bytes reflect a walking-pointer variant, so restructuring v1 as a running pointer could byte-match.
- mechanism: If target used `lw val,0(ptr); addiu ptr,ptr,0x10;` the bytes would differ from a base+index form. A pointer-walk C spelling would then be a live structural axis.
- probe: Read tmp/grind/func_80045294/s1/diff_baseline.txt insns 22-29 (first loop body) directly. Target: `lui $1,0x0 ; addu $1,$1,$3 ; lw $2,0($1) ; ... ; addiu $3,$3,16` (v1 is $3). Base+index reload of hi(base) each iteration; $3 is a walking OFFSET, not a walking pointer. The current candidate already emits this shape.
- result: Target uses base+index addressing with `addu $1,base_hi,$3` and running offset $3. A walking-pointer restructure would produce `lw ,0($ptr)` (no addu) — different bytes. Cannot byte-match via pointer walk.
- verdict: KILLED

## [s5] An arithmetic-multiplier spelling `(u32)a0 * 16u` delivers a0<<4 through a different tree/LUID than a bare shift, potentially decoupling the RA/LUID coupling.
- mechanism: If GCC 2.7.2 handles unsigned multiplication via a different tree lowering than bare shift, the sll's assignment LUID could differ from a `v1 = a0<<4;` statement even at the same source position.
- probe: Edited to `s32 v1 = (s32)((u32)a0 * 16u);` and sandboxed --disable all. Artifact tmp/grind/func_80045294/s5/frontier2_expand_mult_fold.txt.
- result: score=2, target_insns=83, build_insns=83 — NEUTRAL. Identical bytes. GCC 2.7.2 expand_mult normalizes constant power-of-2 multiplication to (ashift RTX) BEFORE tree_LUID is assigned.
- verdict: KILLED

## [s5] Signed `a0 * 16` (matching the target's signed a0) may hit a different expand path than the unsigned form.
- mechanism: Signed vs unsigned multiplication may take different code paths in expand_mult; a MULT_EXPR vs SHIFT_EXPR distinction could alter tree_LUID assignment.
- probe: Edited to `s32 v1 = a0 * 16;` and sandboxed --disable all.
- result: score=2, target_insns=83, build_insns=83 — NEUTRAL. Same expand-time fold; identical RTL.
- verdict: KILLED

## [s5] Pointer-difference route `(s32)((u8*)0 + a0) * 16` may resist expand-time folding because it embeds pointer arithmetic.
- mechanism: The (u8*)0 + a0 sub-expression is a pointer computation; if front-end fold defers it past tree_LUID assignment, the outer *16 could land at a different LUID.
- probe: Edited to `s32 v1 = (s32)((u8 *)0 + a0) * 16;` and sandboxed --disable all.
- result: score=2, target_insns=83, build_insns=83 — NEUTRAL. GCC folds (u8*)0 + a0 -> a0 at the front-end; outer *16 folds to (ashift) identically.
- verdict: KILLED

## [s6] In cse-fold-anon-shift.c, combine.c substitutes insn 14 (ashift into anon pseudo 76) into insn 29 (v1 = anon), eliminating the copy.
- mechanism: combine.c can_combine_p/try_combine substitutes single-use pseudos into their user insn.
- probe: Applied cse-fold-anon-shift.c to src, ran cc1 -da, read tmp/grind/func_80045294/s6/base.i.combine at ;;Function func_80045294 (lines 12633-...).
- result: Combine stats 56 attempts, 45 substitutions, 2 successes. Insn 14 NOT substituted into insn 29 because pseudo 76 has TWO uses: insn 18 (memory address `(mem (plus (reg 76) SYM))`) and insn 29 (the copy). Multi-use pseudo blocks combine's inline substitution; combine leaves both insns intact and the copy survives.
- verdict: KILLED

## [s6] In cse-fold-anon-shift.c, GCC's register allocator coalesces pseudo 76 (anon ashift result) and pseudo 81 (declared v1) into the same hard reg, deleting the surviving copy in insn 29.
- mechanism: Hypothesized global.c coalesce logic would merge two pseudos linked by a copy where source dies (REG_DEAD 76 at insn 29).
- probe: Read tmp/grind/func_80045294/s6/base.i.greg lines 14006-14030. `;; 11 regs to allocate:` list is `93 81 78 87 86 79 74 73 72 75 80` — pseudo 76 is NOT in the global-alloc queue. Register dispositions show `76 in 2` (v0) and `81 in 3` (v1). Cross-referenced tools/gcc-2.7.2/local-alloc.c + global.c.
- result: Pseudo 76 dies within block 0 → handled by local_alloc() which picks $2 (first free scratch) BEFORE global_alloc() runs. Pseudo 81 is loop-live → handled by global_alloc() which assigns $3 by reg_n_refs priority WITHOUT visibility into local-alloc's earlier choice. GCC 2.7.2 has NO cross-pass coalescer (register coalescing pass was added in GCC 3+). combine_regs() in local-alloc.c only merges single-block-scope pseudo pairs. The inter-pool copy in insn 29 is thus structurally uneliminable.
- verdict: KILLED

## [s6] The +1-copy near-hit family (cse-fold-anon-shift + late-v1-assign-with-s4-inline + decl-init-decouple + inner-block-defer-v1) shares one root cause identifiable at pass level, not merely at surface-C level.
- mechanism: Cross-form pattern check: every near-hit has (a) two tree-level a0<<4 subexpressions producing two distinct expand-time pseudos, one confined to block 0 and one loop-carried, and (b) the block-0 pseudo becomes the address-base of s4's init while the loop-carried one is the declared v1.
- probe: Compared cse-fold-anon-shift.c greg vs baseline candidate.c greg (tmp/grind/func_80045294/s3/base.i.greg). Baseline: `11 regs to allocate: ... 75 ...`, `75 in 3` — pseudo 75 IS in global queue, no anon pseudo splits off, no copy. Cse-fold: two pseudos as described. Every rejected +1-copy form shares this two-pool split.
- result: Root cause is the local_alloc/global_alloc pass split, not the surface C. Any C spelling with two tree-level a0<<4 subexpressions where one use is loop-carried and one is block-0-local produces the pass-split condition and the surviving copy. Frontier #2 (make global.c coalesce them) has NO mechanism to reach it in GCC 2.7.2 — the coalescer doesn't exist.
- verdict: CONFIRMED

## [s7] The RA rotation observed in every one-tree-late-assign near-hit (i-before-v1-init, decl-init-decouple, late-v1-assign-with-s4-inline, inner-block-defer-v1) is caused by global.c reg_n_refs LUID-driven allocno priority alone (s6 wording).
- mechanism: s6 named global_alloc reg_n_refs priority. Not measured against baseline priority queue numerically; live-range contribution not disentangled from n_refs contribution.
- probe: Applied decl-init-decouple rewrite (s32 v1; s32 s4; s32 i=a0; s32 count; s32 s5; v1=a0<<4; s4=...) to src/text1a_c.c. cc1 -da dumped tmp/grind/func_80045294/s7/base.i.cse + base.i.greg. Compared cse pass output against baseline s3 dumps.
- result: REFINED: rotation greg allocation queue is '92 77 75 86 85 78 74 73 76 79 72' with pseudo 72 (a0) at position 10 vs. baseline's '92 78 75 86 85 79 74 73 72 76 80' at position 8. But the primary driver is UPSTREAM: cse.c substituted pseudo 77 (i) for pseudo 72 (a0) in the ashift operand at insn 17 because (set 77 72) at insn 15 established value-equivalence. That fold eliminated a0's second read, cutting live range from insn 4->22 to insn 4->15. The live-length collapse dominates the reg_n_refs collapse in allocno_compare, demoting a0 to end of queue where only $21 remains.
- verdict: KILLED

## [s7] A C-level intervening statement between i=a0 and v1=a0<<4 could break CSE's value-equivalence of pseudos 72 and 77, preventing the ashift-operand substitution and preserving a0's long live range.
- mechanism: cse.c's value-numbering treats pseudos linked by (set N M) as equivalent from that point forward until either is redefined. Redefining a0 or introducing a memory clobber that invalidates the equivalence class would prevent the fold.
- probe: Reviewed cse.c substitution logic + verified insn 15 (set 77 72) is what enables the fold at insn 17. Enumerated C-level interventions: (a0 = a0;) self-assign leaves value-equivalence intact; (void)a0; is compiled away pre-CSE; if (a0==a0) guard folds to true and disappears; modifying a0 (a0 = f(a0)) would change semantics because a0 is used later in the function as a parameter.
- result: No pure-C statement can decouple a0's value from i's value between the assignment and the shift without semantic change. The CSE fold fires deterministically whenever i=a0 precedes v1=a0<<4 in a straight-line block. Mechanism is in cse.c, upstream of both sched2 and global_alloc.
- verdict: KILLED

## [s8] m2c-reconstructed init order (i=arg0; v1=arg0*0x10) produces a structurally-different lever than the current v1-before-i shape.
- mechanism: m2c reconstructs from asm without knowledge of sched2/RA constraints; its statement order might reveal a form that the hand-derivation missed.
- probe: Ran tools/m2c/m2c.py on asm/funcs/saTan0Init.s (--valid-syntax --target mips-gcc-c). Read the reconstruction; then applied its distinctive difference — 'i = arg0; v1 = i << 4;' (Novel 1, with ashift operand spelled `i` instead of `a0`, to test whether cse.c would substitute 77 (i) back to 72 (a0) and preserve a0's long live range) — to src. Sandboxed --disable all. Artifact: tmp/grind/func_80045294/s8/m2c_output.txt + rejected/i-before-v1-with-i-as-shift-operand.c.
- result: score=11, build_insns=83, identical rotation shape to s3 decl-init-decouple. cse.c does NOT substitute 77->72 back; it accepts (ashift 77) as-emitted, and a0's live range still collapses at insn 15 (i=a0). Third confirmation of the cse.c BB-scoped substitution mechanism (s7's finding).
- verdict: KILLED

## [s8] Dropping the `count` local and inlining D_800A33AC in both loop guards (per m2c reconstruction) reduces RA priority-list pressure and shifts the sll/move16 sched2 tie.
- mechanism: One fewer pseudo in the global_alloc priority queue; also matches target's asm shape which uses a scratch reg ($a0) for count in first loop and RELOADS D_800A33AC after the call for the second loop — suggesting original source used D_800A33AC inline.
- probe: Replaced `s32 count = D_800A33AC;` + `if (i < count) do ... while (i < count);` with `if (i < D_800A33AC) do ... while (i < D_800A33AC);` in the first loop; sandboxed --disable all.
- result: score=2, build_insns=83, target_insns=83 — NEUTRAL. GCC's licm hoists the D_800A33AC load into a loop-invariant pseudo regardless of the C spelling; RTL is identical to the cached-local form. Removing the C decl does not change codegen. Same 2-insn residual (sched2 tie).
- verdict: KILLED

## [s8] A Kengo (PS2 successor) source-file transplant for saTan0Init (Kengo's function at 0x00147dc8, size 0x14c = 83 insns, same as BB2 — matched by name+size) surfaces the original C shape.
- mechanism: Kengo debug symbols name the source file as `src/sato/sa_tan0.c`; if the actual source or a closely-matched decomp existed, transplanting its statement order and idioms could reveal the pre-cheat-vetted form the original devs wrote.
- probe: Verified Kengo has `saTan0Init` at 0x00147dc8 in src/sato/sa_tan0.c (147dc8+14c = 147f14). Searched Kengo/ dir tree: only kengo_functions_full.txt (symbol names + sizes + source-file NAMES, no source) + the ELF + disc/ are shipped. No Kengo source is available in this repo.
- result: Kengo transplant is unavailable — only symbol metadata exists, not source. The saTan0Init name + size confirm the current C function boundary is correct.
- verdict: KILLED

## [s9] The decomp.me corpus contains a scratch whose asm shingles closely match saTan0Init and whose committed pure-C body offers a novel structural transplant (guarded do-while + shift-index accumulator + sum!=0 branch + second base+index loop) for the sll-vs-move16 sched2 tie.
- mechanism: decomp.me score=0 scratches are pure-C bodies that closed against a target with the SAME toolchain family (gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5); a high-similarity asm shingle match would signal that the closing C shape is transplantable to our function.
- probe: Installed curl_cffi in .venv (s8 blocker) and ran tools/decomp_me_scrape.py search --asm-file asm/funcs/saTan0Init.s --corpus <root> for all three local corpora under tmp/grind/cpu_side_move_dir_4/s44/corpus/. Inspected the top four score=0 matches by JSON body.
- result: Top similarity across all three corpora peaks at 0.127 (gcc272cdk/Z60NJ = func_8002DBF8, GP_SAVE_SCRATCH wrapper around a sfx dispatch, no loop). Other top matched (score=0) hits: KrpLx = 4-arg guard-chain no-loop, 5t0dj = billboard slot allocator single-block, pQrLs = i&0xFF field-match search loop. None has the accumulator+guard+shift-index+sum-branch shape of saTan0Init. No structural transplant available. Artifact: tmp/grind/func_80045294/s9/decomp_me_shingle_search.txt.
- verdict: KILLED

## [s10] do-while(0) wrap around `v1 = a0 << 4;` after `i = a0;` (owner-sanctioned per do-while-zero-exception, 2026-07-06, for any codegen effect incl. RA) creates a basic-block boundary between insn 15 (set 77 72) and insn 17 (ashift ...) that defeats cse.c's BB-scoped operand substitution (s7 root cause), preserves a0's long live range, and blocks the H1-family RA rotation while giving sll a higher LUID than move16 for target sched2 order.
- mechanism: cse.c's cse_basic_block walks JUMP-terminated blocks; a do-while(0) emits NOTE_INSN_LOOP_BEG + body + conditional-jump-on-0 at RTL creation, which should force a JUMP_INSN between the two assignments. cse.c enters the wrap's block with a fresh value-numbering table, cannot know 77 == 72, and emits (ashift 72) at insn 17 -- preserving pseudo 72's second read and its global_alloc position 8 in the allocno queue.
- probe: Applied to src/text1a_c.c: decls-separated + s32 i=a0 first + do { v1 = a0<<4; } while(0) + FAKE annotation + rest as baseline. Sandbox --disable all. Diff/RTL not dumped this session (scope: single measured probe for synthesis-reset).
- result: score=12, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Essentially the H1-family RA rotation (score 11) with +1 register-choice noise. The wrap did NOT create a cse-visible BB boundary. Two mechanism-plausible explanations both dead-end for hand-derivation: (a) jump.c pass 1 folds while(0) to unconditional fall-through BEFORE cse.c runs, collapsing the two would-be BBs; (b) cse_extended_basic_block spans fall-through edges, so cse's value-equivalence class propagates across the wrap's fall-through anyway.
- verdict: KILLED

## [s11] A comma-separated declarator list `s32 v1 = a0<<4, s4 = *(...);` (packing v1 and s4 onto one `s32` statement) shifts tree_LUID between v1's and s4's assignments differently than two separate single-declarator statements, and could perturb the sched2 sll/move16 tie.
- mechanism: GCC 2.7.2 c-parse.y processes a declarator list under one DECL context; if declarator-list packing routed the two initializers through a different intermediate tree construction than two separate DECL_STMTs, the assignment LUIDs -- which the s3-measured sched2 tiebreak (INSN_LUID delta in rank_for_schedule) depends on -- could land differently.
- probe: Edited src/text1a_c.c line 1604 from `s32 v1 = a0 << 4;\n    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);` to `s32 v1 = a0 << 4, s4 = *(s32 *)((u8 *)&D_800EED14 + v1);`. Ran `tools/wteng.ps1 main sandbox func_80045294 --disable all`.
- result: score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 -- IDENTICAL to baseline. Declarator-list packing produces the same LUID sequence as separate DECL_STMTs; no sched2 tie movement.
- verdict: KILLED

## [s11] Placing `s32 count = D_800A33AC;` decl BEFORE `s32 i = a0;` (the last un-permuted position in the s1/s3 {i,s4,count,s5} free-axis cluster, with v1-before-i preserved) shifts sched2's tie between sll and move16.
- mechanism: s1/s3 measured H2/H3 and the s3 `i between v1 and s4` variant as neutral, but did NOT explicitly measure count-before-i with s4 kept between v1 and count. If any pseudo LUID reshuffle in that ordering nudged rank_for_schedule's INSN_LUID tiebreak between sll (LUID 14) and move16 (LUID 22), the sled tie could invert.
- probe: Edited src/text1a_c.c to order `sum, v1, s4, count, i, s5`. Ran sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 -- IDENTICAL to baseline. The free-axis inference from s1/s3 extends to this specific permutation; count's decl position within the {i,s4,count,s5} cluster is neutral as long as v1-before-i is preserved.
- verdict: KILLED

## [s11] Wrapping `a0 << 4` in a GCC statement-expression `s32 v1 = ({ a0 << 4; });` creates a compound-statement rvalue that emits a fresh cse-visible basic block between i=a0 (insn 15) and the ashift (insn 17), preventing cse.c's operand substitution (s7 mechanism) via a different C-front-end lowering path than the s10-killed do-while(0) wrap.
- mechanism: Statement-expression is a GCC extension (c-parse.y stmt_expr rule). Hypothesis: if the compound statement lowers with NOTE_INSN_BLOCK_BEG/BLOCK_END or an intermediate CLEANUP_POINT_EXPR that emerges as a JUMP or block boundary at RTL creation, it would defeat cse_basic_block's straight-line value-equivalence for pseudos 72 and 77 -- independent of jump.c's while(0) folding path (s10 mechanism a) or extended-basic-block spanning (s10 mechanism b).
- probe: Edited src/text1a_c.c to `s32 v1 = ({ a0 << 4; });`. Ran sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 -- IDENTICAL to baseline. GCC 2.7.2's c-parse.y collapses `({ single_expr; })` to the bare expression at parse time; the resulting tree/RTL is byte-identical to `s32 v1 = a0<<4;`. No cse boundary emitted, no LUID shift. Independently corroborates s10's mechanism-agnostic conclusion via a different lowering path.
- verdict: KILLED

## [s12] Moving `s32 sum = 0;` from decl position 0 to position 5 (after v1/s4/i/count/s5) is a neutral extension of the {i,s4,count,s5} free-axis cluster (s1/s3/s11).
- mechanism: Free-axis inference: if {i,s4,count,s5} positions are all sched2-neutral (only v1-before-i binds), sum's position may also be free since sum's assignment is a constant.
- probe: Edited src/text1a_c.c to place `s32 sum=0;` after `s32 s5=s4+a1;`. Ran `wteng sandbox func_80045294 --disable all`.
- result: score=2 -> 4, build_insns=83. Two register-choice diffs surface; sched2 sll/move16 residual persists. sum's assignment LUID after s5's rotates sum's RA away from $17, cascading through the accumulator loop.
- verdict: KILLED

## [s12] Introducing a named intermediate local `s32 aa = a0;` and deriving both `v1 = aa<<4` and `i = aa` from the alias shifts sched2's INSN_LUID tiebreak between sll and move16 by inserting an extra pseudo-creation LUID between a0's arg-set and the shift.
- mechanism: cse.c's value-equivalence tables may treat the aa pseudo as a distinct value-class until the (set aa a0) is folded, delaying the substitution that collapses a0's live range; alternatively the added assignment LUID shifts the sll and move16 LUIDs symmetrically.
- probe: Edited src/text1a_c.c to insert `s32 aa=a0;` before `s32 v1=aa<<4;` and changed `s32 i=a0;` to `s32 i=aa;`. Ran sandbox.
- result: score=2, build_insns=83 -- NEUTRAL. cse.c value-numbers {a0, aa, i} into the same class; the added pseudo does not shift the LUID delta between sll (insn 14) and move16 (insn 22). Rules out named-alias intermediate as a LUID lever for this tie.
- verdict: KILLED

## [s12] Declaring v1 as `u32` with an explicit `(u32)a0` cast on the shift operand routes through a different expand-shift path than `s32 v1 = a0 << 4;`, shifting the ashift RTX's tree_LUID position or its combine-visibility.
- mechanism: The width/signedness axis at the destination-type surface is untested by s5's arithmetic-form probes (which only varied operand-side type: `(u32)a0*16u`, `a0*16`, `(u8*)0+a0`). If the destination type ripples into expand_shift's TRUNCATE/ZERO_EXTEND handling, a distinct RTL sequence could emerge.
- probe: Edited src/text1a_c.c to `u32 v1 = (u32)a0 << 4;`; other decls unchanged. Ran sandbox.
- result: score=2, build_insns=83 -- NEUTRAL. Byte-identical bytes to baseline. cc1 expand_shift produces `(ashift (reg:SI a0) (const_int 4))` regardless of destination declared type; the constant shift is signedness-agnostic. Downstream address arith identical.
- verdict: KILLED

## [s13] Permuter workspace for func_80045294 is un-blockable by using the FULL preprocessed src/text1a_c.c as base.c (bypassing import.py's pycparser choke on whole-func __asm__ + cpp-conflict decls) plus a compile.sh that pipes base.c to cc1 via stdin (positional-filename fails silently with cc1 exit=33; stdin form emits full asm despite the same warnings) and extracts func_80045294's region between .ent/.end.
- mechanism: s4/s5 blocker was import.py's parse of the raw TU, not cc1's ability to compile it. Bypassing import.py + feeding cc1 via stdin bypasses both. Extraction via awk on .ent/.end + wrapper prelude assembling with as -O1 -G0 produces a per-function base.o without needing single-function isolation.
- probe: Wrote tmp/grind/func_80045294/s13/setup_workspace.sh + perm/compile.sh; verified base.o extracts + assembles; permuter loads it and reports base score.
- result: Workspace loads and runs the permuter at 17773 iters/668s with 0 compile errors on the base pass and normal error rate on mutated forms (~4-5%%).
- verdict: CONFIRMED

## [s13] The engine sandbox's honest floor of 2 for func_80045294 is caused entirely by the disabled `prologue_fix` per-function entry (tools/prologue_config.json line 280) — a TRACKED cheat listed in queue.json as `prologue_fix: 1`, not counted in the `rules: 0` field. When my workspace passes standard PROLOGUE_CONFIG, base.o byte-matches target.o (0 insn diff). Passing an empty PROLOGUE_CONFIG (matching sandbox `disable=all` empty_overrides) reproduces the 2-insn sw/move16/sll sched2 tie exactly.
- mechanism: engine/pipeline.py L73-80 threads PROLOGUE_CONFIG/DELAY_SLOT_RA_FUNCS/FRAME_FIX_FUNCS env vars into prologue_fix.py; cheats.empty_overrides points them at empty JSON/txt files. prologue_fix.py's per-function entry for func_80045294 reorders sw s0 ; addu s0,s2 ; sll v1,s2 into target order after cc1 emits sll ; sw s0 ; addu s0,s2. Without prologue_fix's reorder, cc1's natural sched2 output IS the 2-off residual.
- probe: Built base.o with standard prologue_fix (byte-matches target.o), then with empty PROLOGUE_CONFIG env vars (produces sll ; sw s0 ; addu s0,s2 order — 2-insn diff, matching the sandbox exactly).
- result: Base score in workspace = 60 (permuter weighted score for the 2-insn shift), matches sandbox honest floor. Permuter now sees the same landscape as the engine sandbox.
- verdict: CONFIRMED

## [s13] Random-mode permuter over the full-TU base.c does NOT reach a score-60 basin escape in 17773 iters / 668s (fresh seed, chassis=s13-random-fresh1).
- mechanism: decomp-permuter's default randomization pass covers structural rewrites (decl reorder, expression rewrites, statement swaps) without PERM_* directives. In this basin, sched2 tie is coupled to source LUID / RA / cse.c substitution axes measured dead across s1-s12; random mutations that don't route through a different expand/cse/global path cannot shift sll's LUID above move16's without triggering the s6 pool-split copy or s7 cse.c substitution rotation.
- probe: python3 tools/permuter_campaign.py launch --func func_80045294 --dir tmp/grind/func_80045294/s13/perm --label s13-random-fresh1 -j 4 --stop-on-zero; polled 10 min then harvest --stop.
- result: 17773 iterations, 668s, 3 finds total, 2 new, ALL AT SCORE 60. Best_new_score = 60. output-60-2 = swap sum/v1 decl order (equivalent-basin, s1 free-axis). output-60-3 = broken UB mutation (v1 undeclared read, permuter false-match).
- verdict: KILLED

## [s13] The specific directed macros the ledger frontier calls for (PERM_STMT_LIST + PERM_ADD_SUB + PERM_DUMMY_COMMA_EXPR + PERM_REORDER_DECLS) DO NOT EXIST in tools/decomp-permuter/src/. Only PERM_LINESWAP, PERM_GENERAL, PERM_VAR, PERM_INT, PERM_IGNORE, PERM_ONCE, PERM_RANDOMIZE, PERM_FORCE_SAMELINE, PERM_LINESWAP_TEXT, PERM_PRETEND, PERM_FACTORIES are available. Prior sessions authored the frontier text without verifying decomp-permuter's actual macro set.
- mechanism: grep -rE 'PERM_[A-Z_]+' tools/decomp-permuter/src/ enumerates the available macros. The four ledger-cited names are aspirational placeholders. Closest usable substitutes: PERM_LINESWAP (decl-block reorder), PERM_GENERAL (arbitrary expression variants).
- probe: grep for PERM_* in decomp-permuter/src; cross-checked with tools/permuter_annotate.py's HINTS registry (which lists no matching hint slugs).
- result: s13 frontier item #1 must be re-phrased around the ACTUAL macro set. A future session should hand-annotate the func_80045294 body in base.c with PERM_LINESWAP around the decl block + PERM_GENERAL around the a0<<4 expression / s4/s5 init expressions and re-launch a directed campaign.
- verdict: CONFIRMED

## [s14] The directed permuter campaign using PERM_LINESWAP over the {sum, v1, s4, i, count, s5} decl block plus PERM_GENERAL over the `a0<<4` and `s4+a1` expressions can escape the score=60 basin that random-mode cannot (ledger frontier #1).
- mechanism: PERM_LINESWAP + PERM_GENERAL are the actual usable substitutes for the aspirational PERM_STMT_LIST/PERM_ADD_SUB the earlier ledger named. Focused directed search on the specific decl/expression cluster the s6/s7 pass analysis identified as coupled to the sll/move16 sched2 tie should surface novel near-hits invisible to broad random mutation.
- probe: Relaunched the prior (04:24) s14 directed chassis (tmp/grind/func_80045294/s14/perm) with a fresh seed via `permuter_campaign.py launch --label s14-relaunch-lineswap-general -j 4 --stop-on-zero`. Campaign ran to natural iter limit (4320 iters in 478s) — no sub-60 finds, one novel score-60 output. Cumulative directed-mode budget across both s14 runs on the full-TU chassis: 5249 iters, 0 sub-60 hits.
- result: score=60 plateau confirmed across 5249 directed iters on the full-TU chassis (929 s14-orig + 4320 s14-relaunch); no gradient found by PERM_LINESWAP + PERM_GENERAL over the identified decl/expression cluster.
- verdict: KILLED

## [s14] A minimal single-function base.c (~63 lines: typedefs + extern decls for D_800EED10/14/18/1C, D_800A33A0/4/AC, gpu_DrawSync, func_800520B8, then the func_80045294 body only) eliminates the parse-noise floor of the 2524-line full-TU chassis AND is a structurally-different chassis for fresh-seed discipline (ledger frontier #2).
- mechanism: The full-TU base.c has ~2000 lines of unrelated cpp-expanded code from common.h/sound.h/game.h/code6cac.h plus other definitions in text1a_c.c. A minimal base with only the immediate deps lets the compile pipeline run without irrelevant AST/randomizer overhead and gives pycparser's random-mode a cleaner mutation surface, potentially reaching mutation depths the full-TU chassis cannot in the same wall-clock. Also satisfies the fresh-seed discipline requirement that a directed sweep run on a chassis structurally different from prior chassis.
- probe: Built tmp/grind/func_80045294/s14/perm_min/{base.c,compile.sh}. base.c is 63 lines (vs 2524 in full-TU) with the same PERM_LINESWAP annotation over the 6-decl free-axis cluster. compile.sh reuses the same pipeline (cc1 -> prologue_fix with empty gates -> maspsx -> multu_pad -> awk-extract -> as). Smoke-compiled cleanly to base.o=1684 bytes. Launched via `permuter_campaign.py launch --label s14-minimal-base-lineswap -j 4 --stop-on-zero`. Campaign ran 720 iters in 112.4s; 0 novel finds, best_new_score null (no improvement over base=60).
- result: Minimal chassis compiles + runs the permuter cleanly. Zero novel finds and zero sub-60 in 720 iters. The mutation-space benefit did not materialize into a gradient — the directed macros are hitting the same plateau on both chassis. Chassis-independent basin confirmed.
- verdict: KILLED
