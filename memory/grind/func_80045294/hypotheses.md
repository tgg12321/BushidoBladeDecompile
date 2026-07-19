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

## [s15] The sched2 emission order of the sw/move16/sll cluster is a direct deterministic function of the LUID ordering of the two source-level assignments (v1=a0<<4 vs i=a0), with no independent sched2 tiebreaker.
- mechanism: sched.c rank_for_schedule tiebreaker `INSN_LUID(tmp) - INSN_LUID(tmp2)` in backward-list-sched: higher-LUID insn picked first from ready list, emitted later in forward stream.
- probe: Dumped -da sched2 for candidate (v1-before-i) and H1 (i-before-v1). Candidate T-9 ready list `22 14 12 6` picks 22 (move16 LUID=22) first, emits sll (14) first in output; H1 T-9/T-10 ready lists `17 12 6`/`15 12 6` pick sll (17, LUID=17) before move16 (15, LUID=15), emit move16 first in output. Final .s confirms: candidate `sll;sw;move16`, H1 `sw;move16;sll` (matches target sched2 order at this cluster).
- result: H1 form reaches target sched2 order at the sw/move16/sll cluster. Sched2 tie mechanism is (b) — LUID-driven, no independent tiebreaker.
- verdict: CONFIRMED

## [s15] Every C-source shape that flips LUID at sched2 to reach target order also triggers cse.c BB-scoped operand substitution in the ashift, shortening a0's live range and demoting its global_alloc priority — the (a) mechanism is upstream-coupled to any LUID lever.
- mechanism: cse.c BB-scoped value-equivalence: when `(set new_pseudo a0_pseudo)` precedes `(ashift a0_pseudo K)` in the same basic block, cse rewrites the ashift's operand to the new pseudo (proven at cse pass output). a0_pseudo's reg_n_refs drops by one; global_alloc's priority scoring demotes it in the allocation queue.
- probe: Compared cse pass RTL: candidate insn 14 emits `(ashift (reg 72) 4)` — sll uses a0 (pseudo 72) directly; H1 insn 17 emits `(ashift (reg 75) 4)` — sll operand rewritten pseudo 72 -> 75 (i). Compared greg allocation queues: candidate `92 78 75 86 85 79 74 73 72 76 80` puts pseudo 72 at position 9 -> $18; H1 `92 75 76 86 85 79 74 73 77 80 72` puts pseudo 72 at position 11 (last) -> $21. RA rotation fingerprint matches s1 H1 kill (a0->$21, i->$16, sll operand=$16).
- result: CSE substitution proven at pass RTL. Every LUID lever that reaches target sched2 order goes through this substitution. Coupling confirmed.
- verdict: CONFIRMED

## [s15] The frontier's (a)/(b) discrimination framing resolves as: (b) is the mechanical sched2 driver, but (a) is upstream-coupled to every C-source LUID lever hand-derivation can reach in this function. Fix must be upstream of cse.c fall-through -- likely impossible in pure C without a genuine semantic BB boundary the function does not have.
- mechanism: cse.c processes single basic blocks (fall-through spans do not cross real branches). A real BB boundary between i=a0 and v1=a0<<4 would prevent the value-equivalence class from being seen. do-while(0) collapses in jump.c before cse.c runs (s10). statement-expression collapses in c-parse.y before RTL (s11). saTan0Init has no natural semantic conditional between the two statements to insert a real branch.
- probe: s10, s11 measurements (dowhile0-around-v1-after-i.c and stmt-expr-shift.c both KILLED). This session confirms cse.c is the coupling site via direct pass-output comparison, closing the last hand-derivation avenue via named GCC mechanism.
- result: Hand-derivation of a cse-defeating pure-C lever is dead. Only permuter-family mutations reaching different expand paths (PERM_RANDOMIZE) remain sanctioned.
- verdict: CONFIRMED

## [s16] sched1 (pre-reload) and sched2 (post-reload) could diverge in their ordering of insn 14 (sll) vs insn 22 (move16), opening a sched1-specific or sched2-specific intervention (e.g. -fno-schedule-insns / -fno-schedule-insns2) that avoids the s7/s15 cse.c (a) coupling.
- mechanism: sched1 runs after loop.c and combine.c but before reload; sched2 runs after reload has inserted callee-save/restore RTL insns. Different insn sets on the ready list could in principle produce different pick orders even under the same rank_for_schedule tiebreak. If one pass gave target order and the other didn't, disabling the offending pass alone might close the gap without needing a C-source LUID lever.
- probe: Read s3 cc1 -da dumps: tmp/grind/func_80045294/s3/base.i.sched (sched1) and base.i.sched2 (sched2). Compared block-0 forward stream at the sll/move16 cluster in both.
- result: sched1 block 0 (8 insns, total_time=8) ready-list at T-6 = `22 14 12` -- backward-list picks 22 first (LUID higher), sll(14) emitted at forward pos 2, move16(22) at forward pos 3. sched2 block 0 (18 insns after reload inserts 195-211; total_time=18) ready-list at T-9 through T-11 picks 22 at T-9 (pos 10), 211 at T-10 (pos 9, hoisted by 'greater potential hazard'), 14 at T-11 (pos 8). BOTH passes emit sll before move16 by the same LUID mechanism; sw's placement between them at sched2 is a hazard-hoist (not LUID) and is sched2-specific but does not decouple the sll/move16 order.
- verdict: KILLED

## [s17] m2c with alternate targets (mipsel-gcc-c, mips-ido-c) not tried in s8 surfaces a structurally different C shape than s8's mips-gcc-c reconstruction, providing a novel rederive lever.
- mechanism: m2c's reconstruction heuristics differ across --target selections (compiler family, ABI conventions, register-name idioms); if any target's decompilation biases toward a different init order or expression tree, it could reveal a hand-derivation axis not yet enumerated.
- probe: Ran `python3 tools/m2c/m2c.py --target mipsel-gcc-c --passes 3 --valid-syntax asm/funcs/saTan0Init.s` and same with --target mips-ido-c. Diffed both against s8's mips-gcc-c output (memory reference: s8 evidence entry).
- result: Both alternate targets produce first-loop init exactly `var_s1=0; var_s0=arg0; var_v1=arg0*0x10; temp_s4=...; temp_s5=...; if (var_s0<D_800A33AC) do..while`. Byte-identical to s8's reconstruction. This is the H1 shape (i-before-v1 + arg0*K), already banked as rejected/i-before-v1-init.c (s1 kill, score=11 via cse.c substitution) and rejected/shift-as-{signed,unsigned}-mult.c (s5 NEUTRAL). Three independent m2c targets converge on the same KILLED shape.
- verdict: KILLED

## [s17] Kengo debug/globals metadata for saTan0Init contains local-variable names, struct field layouts, or source-file annotations beyond the symbol+size entry s8 observed, providing a Kengo-source-inspired structural transplant hint.
- mechanism: kengo_debug_full.txt and kengo_globals_full.txt could carry DWARF-derived typing/local information that would reveal how the original devs structured saTan0Init even without shipped source.
- probe: `grep -c saTan0Init Kengo/kengo_debug_full.txt Kengo/kengo_functions_full.txt Kengo/kengo_globals_full.txt`.
- result: Zero hits in kengo_debug_full.txt and kengo_globals_full.txt; only kengo_functions_full.txt has the symbol name+size line. Kengo's exported metadata is empty for this function — no locals, no types, no field info. Kengo-transplant block is at debug-info level, not just source level. s8's conclusion is independently corroborated at a distinct evidence surface.
- verdict: KILLED

## [s17] A matched sibling function in text1a_c.c that uses the D_800EED10/14/18/1C table has the same guarded-do-while + shift-index-accumulator + sum!=0 branch + second base+index-loop shape as saTan0Init and its committed pure-C body is directly transplantable.
- mechanism: Sibling structural transplant: if a text1a_c.c neighbor uses the same table with a similar guarded-do-while + accumulator + branch idiom and is in matched state, its statement order / decl block / init spelling would be an evidence-backed rederive lever.
- probe: grep for D_800EED10/14/18/1C uses across text1a_c.c and inspected each sibling function body (saTan5TakeGetPos_80045694, func_800456F0, func_8004574C, func_800457A0).
- result: All four siblings use a single-target search shape (`if (*(s16*)((u8*)D_800EED10 + i) == a0) return ...; i += 0x10;`) with a `while (i < count*16)` bound. None has a sum accumulator, none has a two-loop shape, none has the guarded-do-while + branch-on-sum idiom. No transplantable structural analogue exists in the file for this shape family.
- verdict: KILLED

## [s18] m2c with --gotos-only (goto-only CFG representation, mips-gcc-c, passes=5) reaches a structural shape distinct from the natural if/do-while lowering, potentially disturbing the s7 cse.c BB-scoped substitution that couples LUID and RA rotation.
- mechanism: --gotos-only disables control flow generation and emits explicit gotos for every branch, producing a different textual C surface for the same asm. If the m2c IR-to-C printer routes through a different template that reorders the initial decls, a new structural axis might be produced.
- probe: python3 tools/m2c/m2c.py --target mips-gcc-c --passes 5 --gotos-only asm/funcs/saTan0Init.s -> tmp/grind/func_80045294/s18/m2c_gotosonly_p5.txt
- result: Init cluster BYTE-IDENTICAL to s8/s17: var_s1=0; var_s0=arg0; var_v1=arg0*0x10; temp_s4=...; temp_s5=temp_s4+arg1. The CFG surface differs (explicit loop_1/block_2 labels + gotos) but the decl init order is fixed by m2c's SSA-to-decl printer and is invariant across CFG representation.
- verdict: KILLED

## [s18] m2c with --target mipsel-mwcc-c (Metrowerks CodeWarrior compiler dialect) applies a different codegen-model prior, potentially producing a distinct init order or arithmetic spelling than the gcc/ido dialects.
- mechanism: mwcc's SDA/GPR-usage priors differ from GCC 2.7.2's; the m2c dialect selector adjusts prologue/epilogue matching and stack-spill inference, which could shift how temporaries and s-regs are surfaced.
- probe: python3 tools/m2c/m2c.py --target mipsel-mwcc-c --passes 5 asm/funcs/saTan0Init.s -> tmp/grind/func_80045294/s18/m2c_mwcc_p5.txt
- result: Init cluster BYTE-IDENTICAL to the gcc/ido runs: var_s1=0; var_s0=arg0; var_v1=arg0*0x10; temp_s4=...; temp_s5=... . The dialect selector affects prologue/epilogue matching but not the decl init printer for this asm shape. The H1 shape convergence extends to a 4th m2c dialect.
- verdict: KILLED

## [s19] Scoped PERM_RANDOMIZE (wrapping only the decl+first-loop scope) reaches a mutation surface that unbounded random-mode (s13, full-TU chassis, 17773 iters) did NOT, and can break the sll/move16 sched2 tie.
- mechanism: The s18 ledger frontier claimed scoping PERM_RANDOMIZE to a specific block would let pycparser's randomizer invent intermediate locals / split assignments that unannotated random-mode could not enumerate. Falsifiable: examine decomp-permuter/src/randomizer.py's Region handling.
- probe: Read tools/decomp-permuter/src/randomizer.py:151-173 + perm/parse.py:81. `get_randomization_region` collects `_permuter randomizer start/end` pragma-bounded regions, returns `random.choice(ret)` per iter. If no annotation, returns `Region.unbounded()` (whole function).
- result: Scoped PERM_RANDOMIZE is a strict SUBSET of unbounded random-mode -- narrowing the region set can only reduce the randomizer's per-iter choice space, never expand it. s13's random-mode (17773 iters unbounded on full-TU chassis, plateau 60) already ran the maximally-scoped PERM_RANDOMIZE search. The s18 frontier framing was inverted; scoped PERM_RANDOMIZE cannot find gradients s13 didn't. Ledger evidence surface corrected.
- verdict: KILLED

## [s19] Minimal-base chassis + random-mode permuter (no directed annotations) is a distinct chassis/mode combination not yet empirically banked, and its ~40x-smaller AST surface may let pycparser's randomizer reach mutation depths that full-TU random-mode (s13) or minimal-base directed (s14) did not.
- mechanism: The pycparser randomizer walks the top-level AST each iter and applies structural transforms (decl reorder, statement swap, expression rewrites). A smaller AST means fewer no-op mutations on irrelevant top-level decls per iter, potentially deeper effective mutation reach on the function body at the same wall-clock. s14 confirmed the minimal-base chassis compiles + iterates cleanly at 6-9 iter/s; s13 confirmed random-mode plateaus on full-TU at 17773 iters. The COMBINATION is untested.
- probe: Not run this session (synthesis-only). Frontier deferred to s20: reuse tmp/grind/func_80045294/s14/perm_min/ chassis, strip PERM_LINESWAP + PERM_GENERAL annotations, launch via permuter_campaign.py launch --label s20-min-random-fresh1 -j 4 --stop-on-zero, harvest at natural iter limit (~5000 iters).
- result: Unmeasured this session. Prior evidence weakly suggests plateau=60 (s14 chassis-independent for directed; s13 mode-independent for full-TU) but the combination remains formally un-banked. THIS is the correct final sanctioned axis to measure before OWNER-ESCALATION -- not scoped PERM_RANDOMIZE as the s18 frontier claimed.
- verdict: (deferred to s20)

## [s19] All pass-level failure modes for hand-derivation of a target-order sched2 emission are already named and cited at pass output, and no new hand-derivation lever remains for s20+ to explore.
- mechanism: Consolidated across s1-s18 -- (i) s7/s15 cse.c BB-scoped operand substitution proven at cse pass RTL; (ii) s6 local_alloc/global_alloc pool split with no coalescer proven at greg dump; (iii) s16 sched1+sched2 both LUID-committed with reload-preserved LUIDs proven at sched1/sched2 dumps; (iv) s10/s11 no pure-C CFG-splitter defeats cse's BB view proven at score measurement (do-while(0) collapses in jump.c; stmt-expr collapses in c-parse.y). saTan0Init has no natural semantic conditional between i=a0 and v1=a0<<4.
- probe: Cross-checked every measured axis in s1-s18 against the four failure modes. Every rejected form in memory/grind/func_80045294/rejected/ (22 files) maps to exactly one of (i)-(iv). No hand-derivation axis remains that has not already been shown to hit one of these four walls.
- result: The hand-derivation modality is TERMINALLY exhausted. Rederive is exhausted across 8 sub-axes (s8/s9/s17/s18). Directed permuter is exhausted across 2 chassis (s13/s14). Random permuter is exhausted on 1 chassis (s13, full-TU). The one remaining sanctioned axis (s19 second entry above) is s20's minimal-base + random-mode combination. After that: OWNER-ESCALATION ripe.
- verdict: CONFIRMED

## [s18] m2c with --target mipsee-gcc-c + --no-stack-spill + --deterministic-vars applies PS2-era GCC priors, disables stack-spill temporary introduction, and gives variables ASM-location-stable suffixes, which may surface a different init ordering or expose a variable that hand-derivation would spell differently.
- mechanism: mipsee target changes ABI/register-usage priors; --no-stack-spill suppresses phantom temporaries; --deterministic-vars binds var suffixes to asm positions so the printer chooses different decl ordering when var creation LUIDs are position-derived instead of increment-derived.
- probe: python3 tools/m2c/m2c.py --target mipsee-gcc-c --passes 5 --no-stack-spill --deterministic-vars asm/funcs/saTan0Init.s -> tmp/grind/func_80045294/s18/m2c_mipsee_p5.txt
- result: Init cluster (with position-suffixed names) is: var_s1_8=0; var_s0_10=arg0; var_v1_11=arg0*0x10; temp_s4_17=...; temp_s5_22=... . The numeric suffixes confirm the ASM-position-derived decl creation ORDER is: sum(pos 8) -> i(pos 10) -> v1(pos 11) -> s4(pos 17) -> s5(pos 22). This is the H1 shape's exact init order with independent evidence from position-derived LUIDs.
- verdict: KILLED

## [s19] Scoped PERM_RANDOMIZE (wrapping only the decl+first-loop scope) reaches mutations that unbounded random-mode (s13, 17773 iters full-TU chassis) cannot.
- mechanism: s18 frontier claim: scoped region lets pycparser randomizer invent intermediate locals / split assignments unannotated random-mode cannot enumerate.
- probe: Read tools/decomp-permuter/src/randomizer.py:151-173 + perm/parse.py:81. get_randomization_region returns random.choice(regions) per iter; no annotation -> Region.unbounded() (line 172, whole function).
- result: Scoped PERM_RANDOMIZE is a strict SUBSET of unbounded random-mode -- narrowing region set narrows randomizer's per-iter choice space. s13 already ran maximally-scoped search. s18 frontier framing was inverted.
- verdict: KILLED

## [s19] All pass-level failure modes for hand-derivation of a target-order sched2 emission on this function are now named and cited at pass output; no new hand-derivation lever remains.
- mechanism: Cross-check every measured axis s1-s18 against (i) s7/s15 cse.c BB-scoped substitution, (ii) s6 no-coalescer pool split, (iii) s16 sched1+sched2 LUID commit, (iv) s10/s11 no pure-C CFG-splitter. Every rejected form in memory/grind/func_80045294/rejected/ (22 files) maps to exactly one of (i)-(iv).
- probe: Enumerated rejected/*.c against the four failure modes; verified each maps.
- result: Hand-derivation modality TERMINALLY exhausted. Rederive exhausted across 8 sub-axes (s8/s9/s17/s18). Directed permuter exhausted 2 chassis (s13/s14). Random permuter exhausted 1 chassis (s13 full-TU). Only untested combination: minimal-base + random-mode (deferred s20).
- verdict: CONFIRMED

## [s20] Minimal-base chassis (~40x fewer AST nodes than full-TU) + unannotated PERM_RANDOMIZE region reaches sub-60 where s13's full-TU random-mode did not.
- mechanism: pycparser randomizer's per-iter mutation graph unconstrained by irrelevant top-level decls should reach deeper effective mutation depth on the function body at the same iter/s wall-clock rate; only chassis/mode combination not empirically banked.
- probe: Copied s14/perm_min chassis (63-line base.c, empty prologue_config, cheat-invisible sandbox target.o); stripped PERM_LINESWAP wrapper to make it unannotated random-mode; launched via tools/permuter_campaign.py -j 4 --stop-on-zero; harvested at 13977 iters (531.6s wall-clock).
- result: iterations=13977 across 4 jobs (~26 iters/s), 2 finds both at score=60 (base tie, seconds_since_launch 39s and 433s), best_new_score=60, no sub-60 variants discovered; well past the 5000-iter fresh-seed budget from the ledger.
- verdict: KILLED

## [s21] A comma-expression `s32 s4 = (v1 = a0<<4, deref(v1))` will make v1 the SOLE pseudo for the shift (no anonymous CSE-move copy) AND position the ashift LUID inside s4's init tree, reaching sw/move16/sll order at 83 insns — closing the gap left by cse-fold-anon-shift's +1-copy near-hit.
- mechanism: COMPOUND_EXPR routes the LHS side effect through expand_expr's expand_stmt path, so the assignment `v1 = a0<<4` executes as its own statement while producing the value that the RHS deref uses. If the ashift RTX were emitted as a subexpression of s4's init tree (rather than as a standalone stmt), its LUID would fall between s4's compute and the following i=a0 move16, flipping the sched1/sched2 LUID tiebreak.
- probe: Applied the comma-expression form to src/text1a_c.c:1602-1608 (baseline candidate + swap the two-stmt v1/s4 init for the comma form). Ran `& tools/wteng.ps1 main sandbox func_80045294 --disable all`. Read tmp/sandbox/func_80045294/text1a_c.o via mipsel-linux-gnu-objdump -Mno-aliases -drz to inspect the prologue byte order. Reverted src and reconfirmed baseline.
- result: score=2, target_insns=83, build_insns=83, rules_dropped=0. Prologue order sll(2630)/sw s0(2634)/move16(2638) — IDENTICAL to baseline candidate.c, not cse-fold-anon-shift's sw/move16/sll+copy shape. Same 2-insn residual, no shape change.
- verdict: KILLED

## [s22] A fresh-seed re-run of the s14 minimal-base + PERM_LINESWAP chassis discovers a sub-60 gradient the prior 5249-iter s14 directed budget missed.
- mechanism: s14 cumulative directed budget was split across two runs on the same chassis; fresh-seed discipline (permuter-directives Campaign discipline) says basins yield early or not at all, so 720 more iters with a new seed either surface a novel find or reconfirm the basin.
- probe: Re-cloned tmp/grind/func_80045294/s14/perm_min to s22/perm (unchanged PERM_LINESWAP over the {sum,v1,s4,i,count,s5} decl block). Launched permuter_campaign.py --label s22-lineswap-fresh -j 4 --stop-on-zero; harvested at 720 iters / 137.5s. Artifacts: tmp/grind/func_80045294/s22/perm/campaign.log + output-60-1/.
- result: iterations=720, finds_total=1 (all at score=60, basin-equivalent), best_new_score=60. First novel find at 29.3s after launch (score=60 basin tie). No sub-60 mutations discovered. Basin plateau reconfirmed on this chassis with a fresh seed.
- verdict: KILLED

## [s22] An orphaned prior-session campaign on an alt-chassis (cse-fold-anon-shift base, score=105, target-order sched2 but +1-copy) discovers a sub-105 gradient (approaching 60 or below) under unannotated random-mode.
- mechanism: cse-fold-anon-shift is a +1-copy near-hit with the target's sw/move16/sll schedule order already in place — its basin differs from the baseline chassis (score=60, wrong schedule order). If the permuter can eliminate the surviving copy via any single mutation, the +1 would drop and the score would collapse toward baseline or below.
- probe: Reclaimed orphaned campaign at tmp/grind/func_80045294/s22/perm_alt (PID 605, dead, launched 2026-07-19T10:37:45 by a prior discarded session). Harvested via permuter_campaign.py harvest --stop. Artifact: tmp/grind/func_80045294/s22/perm_alt/campaign.log.
- result: iterations=1310, elapsed=442.3s, finds_total=0, finds_new=0, best_new_score=null. Alt-chassis basin=105 plateaus terminally under 1310 iters of random-mode. The s6 pool-split copy is uneliminable by random permuter mutation (as predicted by s6 verdict: 'no cross-pass coalescer in GCC 2.7.2').
- verdict: KILLED

## [s23] The H1-shape (i-before-v1) minimal-base chassis under random-mode permuter reaches a sub-60 score, distinct from the baseline (60) and cse-fold-anon-shift (105) chassis basins measured to date.
- mechanism: The H1 shape triggers the s7 cse.c BB-scoped operand substitution at insn 17 (ashift 77) and the resulting RA rotation (a0->$21) that hand-derivation banks at score=11. Random-mode permuter mutations from that seed explore a distinct region of the search space than the baseline chassis (which starts already at the sched2-tie attractor) — if any sub-60 spelling exists that avoids the four pass-level walls, the H1 seed's mutation trajectory would be the first to reach it.
- probe: Built tmp/grind/func_80045294/s23/perm_h1/{base.c,compile.sh} reusing s20's minimal-base compile pipeline and empty overrides, but with the H1 body (i=a0 before v1=a0<<4). Launched via `permuter_campaign.py launch --func func_80045294 --dir tmp/grind/func_80045294/s23/perm_h1 --label s23-h1-random-fresh -j 4 --stop-on-zero`. Base score reported: 84 (novel — permuter's weighted scorer rates the H1 basin at 84 vs 60 for baseline and 105 for cse-fold-anon-shift; the s1-banked H1 objdump score of 11 corresponds to 84 in the permuter's regs*5+reorders*60+ins/del*100 metric). Polled at 10 min and ~26 min; harvested + stopped after 37313 iterations.
- result: 37313 iters, 7 new finds: output-79 x2 (22.7s, 49.6s), output-69 (312.5s), output-61 (92.5s), output-60 x3 (77.1s, 120.8s, 956.9s). best_new_score=60, no sub-60. The H1 basin collapses via random mutation to the SAME score=60 attractor already banked by s13 (full-TU random 17773 iters), s14 (minimal directed 720+4320 iters), s20 (minimal random 13977 iters), and s22 (fresh-seed lineswap 720 iters). No structurally new gradient below 60 surfaces from the H1 seed.
- verdict: KILLED

## [s24] The local-alloc pass output (base.i.lreg) directly shows the s6 pool-split conclusion at pass boundary rather than as a downstream greg inference — pseudo 76 (block-0-scoped anon ashift result in cse-fold-anon-shift) is claimed by local_alloc BEFORE global_alloc places pseudo 81 (declared multi-block v1), and the two land in distinct hard regs ($v0 vs $v1) so the (set 81 76) copy at insn 29 cannot fold to a self-move.
- mechanism: flow.c live-range analysis tags block-0-scoped pseudos with 'in block N'; local-alloc.c iterates those tagged pseudos and emits ';; Register N in M.' claim lines before global-alloc.c runs; global-alloc.c has NO visibility into local's earlier claims and cannot coalesce a global pseudo with a local one linked by a copy even when REG_DEAD is set on the source at the copy. GCC 2.7.2 has no register-coalescer pass (first added in GCC 3.x).
- probe: Read tmp/grind/func_80045294/s3/base.i.lreg (baseline, v1-before-i) lines 16291-16400 vs tmp/grind/func_80045294/s6/base.i.lreg (cse-fold-anon-shift) lines 16291-16400 for the func_80045294 region. Baseline: pseudo 75 (v1) has 11 uses / 17 insns with NO 'in block N' tag → global_alloc; no anon block-0 shift pseudo exists. cse-fold: pseudo 76 has '2 uses / 2 insns in block 0' tag → local_alloc claim `;; Register 76 in 2.`; pseudo 81 has 11 uses / 16 insns un-tagged → global_alloc claims $3 for it. Extracted slices to tmp/grind/func_80045294/s24/{baseline_lreg_slice.txt, csefold_lreg_slice.txt}.
- result: s6 conclusion upgraded from greg-inferred to lreg-pass-direct: local_alloc claims $v0 for the anon shift pseudo; global_alloc claims $v1 for declared v1; two distinct hard regs; copy survives. All four ledger pass-level walls (s6 pool-split, s7 cse.c BB-scoped substitution, s16 sched1+sched2 LUID coupling, s10/s11 no pure-C CFG-splitter) now have direct pass-output citations.
- verdict: CONFIRMED

## [s24] docs/grind/decisions.md contains an OWNER-ESCALATION entry for func_80045294 that would satisfy the 'owner-gated' emission precondition this session.
- mechanism: Contract permits owner-gated only when the escalation entry is already filed AND every remaining sanctioned axis is measured dead.
- probe: grep -n '80045294\|saTan0Init' docs/grind/decisions.md
- result: Zero hits. No entry filed. owner-gated is not emittable this session. The next-session frontier per s22-s23 ledger (owner-escalation-drafting) remains the sanctioned move.
- verdict: KILLED

## [s25] The s10 kill of do-while(0)-around-v1-after-i (score 2->12) was mechanism (a) jump.c collapse, OR mechanism (b) cse.c spanning fall-through edges; both were plausible from score alone.
- mechanism: Applied rejected/dowhile0-around-v1-after-i.c to src, dumped cc1 -da at every pass, compared base.i.rtl (pre-jump.c) vs base.i.jump (post) vs base.i.cse to witness each mechanism at the pass boundary where it fires.
- probe: cc1 -O2 -G0 -da on preprocessed src/text1a_c.c; extract func_80045294 slices from base.i.{rtl,jump,cse}; compare CFG structure and ashift operand at insn 20 across pass boundaries.
- result: BOTH mechanisms independently confirmed at pass output. Pre-jump.c RTL has 4 code_labels + 2 unconditional jump_insns + 2 barriers + 3 LOOP notes bracketing insn 20. Post-jump.c has ONLY the 3 LOOP notes; every CFG-splitting insn deleted (mechanism a fires). cse.c dump header 'Processing block from 2 to 28' spans the LOOP_BEG/CONT/END notes as one block (mechanism b would fire even without a). Post-cse.c insn 20 ashift operand rewrites from (reg 72) a0 to (reg 75) i, reproducing the s7 substitution fingerprint.
- verdict: CONFIRMED

## [s25] Wall (iv) 'no pure-C CFG-splitter defeats cse.c BB view' can be strengthened from s10/s11's score+source-cite evidence to direct pass-output evidence.
- mechanism: The s10 do-while(0) probe and s11 statement-expression probe both measured the kill via score only, then attributed the mechanism to jump.c collapse (s10) or c-parse.y collapse (s11) by reading pass source. s25 upgrades s10 to direct base.i.jump + base.i.cse dumps.
- probe: Same dump pipeline as above; compare CFG structure at each pass boundary against s10's source-level hypothesis.
- result: Wall (iv) now carries direct base.i.jump + base.i.cse pass-output citations matching the evidence tier of walls (i) s24 base.i.lreg / (ii) s7 base.i.cse / (iii) s15+s16 base.i.sched+sched2. All four pass-level walls now have direct dump citations. The two mechanisms (a) and (b) are STACKED (both fire independently), not alternatives — any future CFG-splitter proposal must survive both.
- verdict: CONFIRMED

## [s26] m2c with --stack-structs + --descending-regs biases the printer toward a distinct decl-order surface via stack-layout inference and register-descending sort, shifting the i vs v1 init position relative to prior s8/s17/s18 gcc/ido/mwcc/mipsee runs.
- mechanism: m2c's Formatting Options: --stack-structs emits a hand-annotable stack-layout struct which can reshape var creation order in the printer; --descending-regs sorts declared vars by descending register number rather than ascending SSA index, potentially yielding s2->s1->s0 (a0/v1/i in some translation) instead of the s0-first bias every prior run produced.
- probe: python3 tools/m2c/m2c.py --target mips-gcc-c --passes 5 --valid-syntax --stack-structs --descending-regs asm/funcs/saTan0Init.s > tmp/grind/func_80045294/s26/m2c_stackstructs_descregs.txt
- result: Init cluster BYTE-IDENTICAL to s8/s17/s18: var_s1=0; var_s0=arg0; var_v1=arg0*0x10; temp_s4=...; temp_s5=temp_s4+arg1. --stack-structs adds a struct _m2c_stack_saTan0Init preamble but does not shift the init printer's decl order; --descending-regs is a display-only sort that leaves per-var init-position selection to the SSA pass. This is the H1 shape banked KILLED at s1 (rejected/i-before-v1-init.c, score=11 via cse.c substitution).
- verdict: KILLED

## [s26] m2c with --context (populated with real BB2 types/signatures: SaTanEntry struct, D_800EED10[] array, gpu_DrawSync(s32) + func_800520B8(s32,s32,s32) prototypes) + --globals all resolves the D_800EED14/18/1C references through the array/struct type rather than as freestanding globals, potentially producing a structurally different init or loop-index scheme distinct from context-less s8/s17/s18 runs.
- mechanism: When m2c has a --context C file with real types, its printer emits struct-field references and canonical array-index arithmetic instead of raw byte-offset pointer arithmetic. If the initial (arg0 * 0x10) is recognized as an array-index computation and re-expressed as a subscript, decl order might invert (e.g. i first, then i*0x10 elided into the subscript).
- probe: Wrote tmp/grind/func_80045294/s26/ctx.c with SaTanEntry struct + D_800EED10[] array + gpu_DrawSync/func_800520B8 prototypes. Ran python3 tools/m2c/m2c.py --context tmp/grind/func_80045294/s26/ctx.c --globals all --target mips-gcc-c --passes 5 --valid-syntax asm/funcs/saTan0Init.s > tmp/grind/func_80045294/s26/m2c_context_globals_all.txt.
- result: Init cluster BYTE-IDENTICAL: var_s1=0; var_s0=chan; var_v1=chan*0x10; temp_s4=*(&D_800EED14+var_v1); temp_s5=temp_s4+delta. m2c honors the arg NAMES from context (chan/delta) but the offset arithmetic is emitted as raw pointer addition against &D_800EED14 rather than a subscript against D_800EED10[], because the target's asm loads via base+shifted-offset (not struct-field syntax) and m2c's array-recognizer does not rewrite. i-before-v1 order preserved -> this is the H1 shape banked KILLED at s1.
- verdict: KILLED

## [s26] The full-set of m2c formatting/analysis options can produce a rederive lever distinct from the H1 shape.
- mechanism: Cross-check with s8 (mips-gcc-c), s17 (mipsel-gcc-c + mips-ido-c + Kengo debug/globals + BB2 sibling), s18 (--gotos-only + mwcc + mipsee + no-stack-spill + deterministic-vars), and s26 above (--stack-structs+--descending-regs; --context+--globals all).
- probe: Convergence check across 10 independent m2c sub-axes total.
- result: All 10 m2c sub-axes converge on the H1 init order (var_s0/i-before-var_v1). The i-before-v1 shape is fixed by m2c's SSA-to-decl printer against the target's asm insn order (target has `move s0,s2` before `sll v1,s2,4`, and every m2c pass extracts s0's assignment stmt before v1's). No m2c flag, target, or context configuration shifts this to the v1-before-i shape that the current baseline candidate.c already carries. Rederive-via-m2c is exhausted along every printer-behavior axis available.
- verdict: CONFIRMED

## [s27] Kengo's saTan0Init at 0x00147dc8 is the PS2 successor of BB2 func_80045294 and its asm shape / structural idioms are transplantable as a rederive lever.
- mechanism: Same symbol name + same insn count (0x14c bytes = 83 insns) in Kengo's exported debug metadata (per s8/s17) suggested a same-body PS2 successor. If true, disassembling Kengo's function and running m2c on its EE-MIPS asm would yield a distinct C reconstruction, biased by PS2-era codegen priors, that could inform BB2's C shape.
- probe: Disassembled Kengo/disc/SLUS_200.21 saTan0Init region 0x147dc8..0x147f14 via mipsel-linux-gnu-objdump -d in WSL; saved to tmp/grind/func_80045294/s27/kengo_saTan0Init.s (90 lines). Compared semantic shape against BB2 func_80045294 body (candidate.c + banked s1-s26 evidence).
- result: Kengo saTan0Init is a one-shot initializer + memory-alloc pipeline: 9 jals (saRobInit, saTanMainDataCreate, tslGlobalMemAlloc(0x8000), saFidLoad-in-bltzl-retry, tslSmdInitObj, motion_SetMotion, motion_CalcMotion, saTan0InitPrim, saSePlaySeq(5,0,96,96)), 160-byte stack frame, six-iteration fixed-count loop, byte-flag stores + sound cue at exit. BB2 func_80045294 is a per-frame accumulator+dispatch: 2 jals (gpu_DrawSync, func_800520B8), ~32B frame, dynamic-count loop over D_800EED18, sum!=0 branch with fn-ptr dispatch, D_800A33A0 += a1 / D_800A33A4 -= a1 tail. Zero structural overlap. Same-name coincidence only.
- verdict: KILLED

## [s28] Every sanctioned modality axis for closing func_80045294's 2-insn sched2 residual to floor=0 is measured dead across s1-s27; no residual hand-derivation, rederive, forensics, or permuter axis remains to explore.
- mechanism: Cross-check of 23 rejected forms against four pass-level walls (s7/s15 cse.c BB-scoped substitution; s6/s24 local/global_alloc pool split; s16 sched1+sched2 LUID coupling; s10/s11/s25 no pure-C CFG-splitter defeats cse's BB view) shows every form maps to exactly one wall. Rederive across 11 m2c/Kengo/decomp.me/sibling sub-axes converges on H1 (KILLED) or NEUTRAL. Permuter across 7 chassis/mode combinations totals ~69590 iters with 0 sub-60 finds; three chassis basins (60/84/105) all measured terminal.
- probe: s28 baseline reconfirmation + consolidated modality-exhaustion table cross-referenced against every rejected/*.c form, every m2c configuration in s8/s17/s18/s26, every permuter campaign in s13/s14/s20/s22/s23, and the four pass-output-cited walls.
- result: Baseline reconfirmed at score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78. Consolidation complete; no axis surfaces that has not been measured dead. Consistent with s19/s20/s21/s22/s23/s24/s25/s26/s27 consensus that owner-escalation-drafting is the only remaining sanctioned move.
- verdict: CONFIRMED

## [s28] A synthesis-modality search for a residual hand-derivable axis missed by s1-s27 (some novel pure-C structural or arithmetic lever not enumerated in the 23-rejected-form bank) surfaces a new lever.
- mechanism: Systematic re-check of the four walls against every axis category: (A) statement position (H1/H2/H3, count-before-i, sum-init-at-end, s5-inside-sum-block, inner-block-defer-v1 -- all banked); (B) decl/init decoupling (decl-init-decouple, late-v1-assign-with-s4-inline, combined-decl-v1-s4 -- all banked); (C) arithmetic-form (shift-as-{signed,unsigned}-mult, shift-via-ptrdiff, u32-v1-cast, a0-alias-local -- all banked); (D) CFG splitters (dowhile0-around-v1-after-i, stmt-expr-shift, v1-guard-scoped -- all banked); (E) CSE folding (cse-fold-anon-shift, cse-reuse-shift, comma-expr-v1-assign -- all banked); (F) operand order (swap-s5-operands, split-init-s5, inline-count-no-local -- all banked).
- probe: Enumerated all six axis categories against the 23-rejected-form bank + the H2/H3 free-axis cluster measured neutral at s1/s3/s11. Every reasonable C-source-level variation on the six-decl cluster shape has been measured.
- result: No un-measured axis exists in any category. The rejected bank is closed against the reachable pure-C search space for this shape.
- verdict: KILLED

## [s29] Placing s32 sum=0 at decl position 1 (between v1 and s4) rather than 0 flips the sll/move16 sched2 tie.
- mechanism: sum's assignment LUID falls between v1's and s4's LUIDs; a global_alloc reg_n_refs priority shift could reallocate sum's pseudo away from $17 or reallocate v1's / s4's pseudo, moving the LUID relationship rank_for_schedule uses at the sll/move16 tiebreak.
- probe: Edited src/text1a_c.c decl block from 'sum,v1,s4,i,count,s5' to 'v1,sum,s4,i,count,s5'; ran & tools/wteng.ps1 main sandbox func_80045294 --disable all.
- result: score=2, target_insns=83, build_insns=83, rules_dropped=0, cheat_asm_stripped=78 -- NEUTRAL. Byte-identical to baseline; same 2-insn sched2 residual.
- verdict: KILLED

## [s29] Placing s32 sum=0 at decl position 2 (between s4 and i) rather than 0 flips the sll/move16 sched2 tie.
- mechanism: sum's assignment LUID falls between s4's and i's LUIDs; if the s7 cse.c BB-scoped substitution or the s6 pool split is sensitive to sum's LUID position within the pre-i decl cluster, the tie could shift.
- probe: Edited decl block to 'v1,s4,sum,i,count,s5'; ran sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 -- NEUTRAL. Byte-identical to baseline.
- verdict: KILLED

## [s29] Placing s32 sum=0 at decl position 3 (between i and count, first position AFTER i=a0's assignment) flips the sll/move16 sched2 tie or is likewise neutral.
- mechanism: Testing the boundary between the s12-measured LOAD-BEARING position 5 and the s29-measured NEUTRAL positions 1-2 to pin the transition to a specific LUID relationship (relative to i=a0's LUID).
- probe: Edited decl block to 'v1,s4,i,sum,count,s5'; ran sandbox --disable all.
- result: score=4, target_insns=83, build_insns=83 -- PERTURBS (rises by 2 register-choice diffs, same magnitude as s12's position-5 measurement). Boundary is EXACTLY the i=a0 assignment: sum-LUID before i-LUID -> free-axis; sum-LUID after i-LUID -> RA cascade.
- verdict: KILLED

## [s29] s12's evidence claim that 'sum's decl position is LOAD-BEARING at position 0' is correct as stated.
- mechanism: s12 measured sum-at-position-0 (score=2) and sum-at-position-5 (score=4) and concluded position 0 was singularly load-bearing. s29 checks whether the intermediate positions preserve or break the tie.
- probe: s29 measurements at positions 1, 2, 3 give scores 2, 2, 4 respectively.
- result: Refuted. Sum-position is a bounded free-axis: FREE for positions 0-2 (LUID before i=a0), PERTURBING for positions 3-5 (LUID after i=a0). The load-bearing boundary is i=a0's assignment, not sum's absolute position. Mechanism aligns with walls (i)/(ii): sum's post-i LUID demotes it in global_alloc reg_n_refs priority.
- verdict: KILLED

## [s30] Two-axis simultaneous move (sum@2 + s5/count-swap) shifts LUID relation and breaks sll-before-move16 tie without triggering post-i RA cascade.
- mechanism: s29 measured single-axis moves independently. Cross-product of two independent free-axis moves might surface a novel LUID relation neither single-axis captured.
- probe: Applied decl order v1,s4,sum,i,s5,count to src/text1a_c.c; sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 (unchanged from baseline).
- verdict: KILLED

## [s30] Two-axis simultaneous move (sum@1 + count-before-i) creates a new LUID configuration that inverts the sched2 tiebreak.
- mechanism: Combines s29's sum-position free-axis with s11's count-position free-axis simultaneously; may compose non-linearly at the ready-list evaluation.
- probe: Applied decl order v1,sum,s4,count,i,s5 to src/text1a_c.c; sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 (unchanged).
- verdict: KILLED

## [s30] Three-axis simultaneous move (sum@2 + s5-before-i + count-last) achieves LUID(sll) > LUID(move16) at sched1/sched2 through combined free-axis cascade.
- mechanism: Superposition of three free-axis moves; if any non-linearity exists in the LUID assignment for closely-packed decl statements it would surface here.
- probe: Applied decl order v1,s4,sum,s5,i,count to src/text1a_c.c; sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 (unchanged).
- verdict: KILLED

## [s31] A fresh-seed random-mode permuter campaign on the H1-chassis (base=84) would plateau at the same 60 basin as s13/s14/s20/s22/s23 with no sub-60 finds.
- mechanism: s23 already ran this exact chassis for 37313 iters producing plateau=60. Fresh seed adds a stochastic-independence sample per fresh-seed discipline; the four pass-level walls (cse.c BB-scoped operand substitution / local-alloc pool split / sched1+sched2 LUID coupling / no pure-C CFG-splitter defeats cse.c) predict plateau=60 again.
- probe: Copy s23/perm_h1 workspace to s31/perm_h1_freshseed, strip prior output-*/campaign_meta.json/log, launch via tools/permuter_campaign.py with -j 4, wait in-turn, harvest --stop.
- result: KILLED (against the strong form) — campaign found output-10-1 at 336.4s (well below the plateau of 60 measured across 4 prior campaigns totaling ~69590 iters). The strong form of the hypothesis (H1 basin sealed at 60) is refuted. Complete banked finds: [60,60,69,61,79,10,60,66,70,79] over 21218 iters / 779.6s.
- verdict: KILLED

## [s31] The permuter's score=10 mutation is a semantically-VALID rewrite reachable in pure C — i.e. the H1 basin has a sub-60 spelling that a pure-C decomp could adopt.
- mechanism: Random-mode permuter mutations can preserve semantics accidentally; a sub-60 find could be a legitimate structural rewrite hidden in the H1 chassis. If so, translating it to canonical decomp form defeats the four pass-level walls.
- probe: Read tmp/grind/func_80045294/s31/perm_h1_freshseed/output-10-1/source.c and diff.txt; check whether the mutation preserves the first-loop terminator, sum accumulation, and callback dispatch semantics.
- result: The mutation inserts 'i = a0;' INSIDE the first do-while body (before v1+=0x10; i+=1). This RESETS i to a0 each iteration, so post-body i is always a0+1 regardless of iteration count. When a0+1 < count, the loop is non-terminating (infinite). When a0+1 >= count, it terminates after exactly 1 iteration (semantically differs from the correct <count-a0>-iteration loop for values that fed sum). Additional mutation: dead 's32 *new_var;' declaration and 'new_var = ...; ptr = new_var;' aliasing in the second loop — this branch is a benign codegen-only reshuffle but is not what dropped the score. The i-reset is what shifted LUID, and it is a SEMANTIC BREAK — no pure-C semantic-invariant construct can emit an equivalent mid-loop 'move t,a0' at that slot without either (a) actually corrupting the loop counter or (b) inline-asm register-pin (canonical-asm cheat class).
- verdict: KILLED

## [s31] The score=10 mutation reveals a new sanctioned axis to explore: pure-C constructs that force GCC to emit an extra 'move' instruction inside the first-loop body at the same slot the permuter injected 'i = a0'.
- mechanism: If a use of 'a0' or another register can be materialized into a genuine move inside the loop body via a semantic construct (e.g. a helper local re-initialized each iter for a legitimate downstream use), the same LUID shift may occur without semantic break.
- probe: Enumerate pure-C constructs that would legitimately emit a 'move t,a0' inside the loop body: (a) using a0 in an expression that GCC materializes into a temp — e.g. 'sum += val + (a0 - a0);' collapses via constant folding; (b) a mid-loop condition on a0 that GCC preserves — but a0 is loop-invariant so GCC hoists; (c) register-asm pin on a downstream local — this is the canonical-asm cheat class per inline-asm-injection rule; (d) volatile-tagged local — cheat class per volatile_cheats detector.
- result: All pure-C constructs that emit a mid-loop move of a0-into-a-register are either (i) collapsed by cse.c/fold_rtx before codegen (constant-fold, invariant-hoist) OR (ii) cheat-class (register-asm pin, volatile alias). This is the SAME four-wall stack from s28 synthesis, now hit from a fifth angle: the LUID-shift lever the permuter found requires bytecode-level codegen intervention no pure-C form can express. The four pass-level walls are hardened, not refuted, by the score=10 find.
- verdict: KILLED

## [s32] A fresh-seed random-mode permuter campaign on the baseline v1-before-i minimal chassis discovers a sub-60 gradient in ~15 min that the 5 prior baseline-family campaigns (s13 full-TU 17773 iters, s14 minimal directed 5249 iters, s20 minimal random 13977 iters, s22 lineswap 720 iters, s31 H1 21218 iters) did not.
- mechanism: Fresh-seed stochastic-independence sample per permuter-directives Campaign discipline: if a distinct RTL sub-region exists that the prior samples missed, a fresh RNG draw over 15+ min at ~40 iter/s (~40k iters) would surface it. Predicted plateau=60 by the four pass-level walls (s7/s15 cse.c BB-scoped substitution; s6/s24 local/global_alloc pool split; s16 sched1+sched2 LUID coupling; s10/s11/s25 no pure-C CFG-splitter defeats cse's BB view).
- probe: Copied s20/perm_min chassis to s32/perm_baseline_fresh (unchanged base.c: baseline v1-before-i decl order, no PERM_* annotations). Launched tools/permuter_campaign.py launch -j 4 --stop-on-zero (WSL PID 419); waited 15 min in-turn; harvested --stop.
- result: iterations=39420, elapsed=947.4s, base_score=60, finds_total=2 both at score=60 (32.2s, 307.2s), best_new_score=60, no sub-60. Basin plateau=60 reconfirmed at a 6th baseline-family campaign and 8th overall chassis/mode combination. Artifact: tmp/grind/func_80045294/s32/perm_baseline_fresh/campaign.log + output-60-{1,2}/.
- verdict: KILLED

## [s32] A prior-session orphaned campaign on the s32/perm_sum_at_end chassis (structurally distinct decl order: sum moved from position 0 -- this is the s29-KILLED sum-at-position-2 shape, which was measured NEUTRAL at score=2 in the sandbox) plateaus at a sub-60 score under random-mode permuter.
- mechanism: The sum-at-position-2 decl order is a bounded free-axis position measured NEUTRAL in the honest sandbox (s29). Under permuter weighted scoring, its base_score might differ from 60 if the LUID reshuffle affects sched2 output. If sub-60 finds emerge, they would indicate the sum-position axis has a permuter-visible gradient.
- probe: Reclaimed orphaned campaign at tmp/grind/func_80045294/s32/perm_sum_at_end (registered as s32-sum-at-2, launched 2026-07-19T12:07:43 by prior discarded s32 session, PID 428 dead). Harvested via permuter_campaign.py harvest --stop.
- result: iterations=772, elapsed=1191.7s, base_score=60, finds_total=1 at score=60 (43.7s), best_new_score=60. Sum-at-position-2 chassis basin also plateau=60; the s29-measured free-axis extends to permuter-visible search. 9th chassis/mode combination banked. Artifact: tmp/grind/func_80045294/s32/perm_sum_at_end/campaign.log + output-60-1/.
- verdict: KILLED

## [s32] Cumulative permuter evidence across 9 chassis/mode combinations totalling ~137,752 iters constitutes empirical closure of the pure-C search space for func_80045294's sched2 residual: every fresh chassis converges on the same score=60 basin (except cse-fold-anon-shift at 105 and the s31 semantic-break at 10).
- mechanism: Convergence across independent starting basins (baseline v1-before-i, H1 i-before-v1, cse-fold-anon-shift +1-copy, sum-at-position-2 free-axis) and modes (unbounded random, PERM_LINESWAP-directed, minimal-base, full-TU) is the strongest empirical closure signal the permuter can supply. Any legitimate pure-C sub-60 spelling reachable by random or directed mutation would have surfaced at least once across ~137k iters and 4 distinct basin geometries.
- probe: Cross-summed banked iter counts: s13 (17773) + s14 (5249) + s20 (13977) + s22 (720) + s22-alt cse-fold (1310) + s23 H1 (37313) + s31 H1-fresh (21218) + s32 baseline-fresh (39420) + s32 sum-at-2 (772) = 137,752 iters. Only sub-60 find in all of it: s31's score=10 mid-loop i-reset which is a documented semantic-break (rejected/perm-score10-mid-loop-i-reset-semantic-break.c).
- result: Cumulative permuter budget is ~2x the s28-synthesis ~69590 count and covers 9 chassis/mode combinations. Empirical closure now maximally strong at the permuter tier. Owner-escalation-drafting is more empirically supported than ever; the s31 semantic-break find remains the only sub-60 point in the entire search and it is not pure-C-expressible.
- verdict: CONFIRMED

## [s33] combine.c preserves both insn 14 (sll -> pseudo 75 v1) and insn 22 (move16 -> pseudo 78 i) as independent surviving insns on the baseline v1-before-i chassis, minting the LUID delta of 8 that sched1/sched2's rank_for_schedule tiebreak consumes (s3/s15 finding).
- mechanism: combine.c try_combine substitute-and-delete requires the pseudo being substituted to be single-use (or have LOG_LINKS pointing to a single user). Pseudo 72 (a0) has THREE cross-BB uses: insn 14 (ashift to v1 in block 0), insn 22 (move to i in block 0), insn 89 (i = a0 in block 4, second loop). combine's `combinable_i3pat` refuses to duplicate the def, so insn 4 stays as the a0 arg-save and insns 14 and 22 both survive as independent standalone SETs with LUIDs 14 and 22.
- probe: Read tmp/grind/func_80045294/s3/base.i.combine at ;;Function func_80045294 (line 12633); combiner statistics: 56 attempts, 45 substitutions, 2 successes. Insn 4 (set 72 a0) has REG_DEAD on a0 but pseudo 72 has three surviving uses at insns 14, 22, 89 (REG_DEAD 72 finally set at insn 89). Insn 14 emits (set 75 (ashift 72 4)) standalone. Insn 22 emits (set 78 72) standalone. Slice banked at tmp/grind/func_80045294/s33/combine_baseline_slice.txt.
- result: combine.c leaves the sll/move16 pair as two independent insns whose LUIDs (14 and 22) become the input to sched2's rank_for_schedule INSN_LUID tiebreak. The LUID delta of 8 is minted at combine.c's pass boundary, not at sched1's or cse.c's; sched1 inherits it verbatim, reload preserves it, sched2 emits based on it. This is a fifth direct pass-output citation upstream of the four ledger walls (cse.c BB substitution s7/s15; local/global_alloc pool split s6/s24; sched1+sched2 LUID coupling s16; no pure-C CFG-splitter defeats cse s10/s11/s25).
- verdict: CONFIRMED

## [s33] A pure-C form exists that reduces pseudo 78 (i) to single-use so combine.c can substitute pseudo 72 into every use of 78 and delete insn 22 (the move16), eliminating the sll/move16 tie entirely.
- mechanism: If pseudo 78's uses were consolidated to one, combinable_i3pat would allow (set 78 72) to be substituted forward and insn 22 deleted; the remaining ashift (insn 14) would be the only prologue LUID, no tie to break.
- probe: Enumerate pseudo 78's uses in the combine dump: insn 31 (slt 78 79 for first-loop guard), insn 48 (78 = 78 + 1 loop-carried increment), insn 57 (slt 78 79 for do-while terminator). Three uses, all loop-carried by definition. Making pseudo 78 single-use would require removing either the loop-terminator slt or the loop-carried increment; both are semantic to the accumulator loop's `while (i<count) { ... i += 1; }` shape and cannot be removed in pure C without corrupting semantics. The second-loop counterpart (insn 89) resets pseudo 78 = pseudo 72 again, likewise structurally required.
- result: Pseudo 78 (i) is a loop-carried counter with three uses in the first loop and multiple more in the second — combine.c cannot ever reduce it to single-use in any semantically valid pure-C form. The move16 emission at insn 22 (and its counterpart at insn 89) is structurally uneliminable at the combine.c pass boundary. This closes the newly-cited pass-level wall against any pure-C lever.
- verdict: KILLED

## [s33] combine.c's baseline behavior differs from cse-fold-anon-shift's behavior (s6 finding) — the +1-copy in cse-fold-anon-shift is a distinct combine-pass phenomenon from baseline, and the two dumps together define the pass-boundary origin of every rejected form's insn count.
- mechanism: s6 measured combine on cse-fold-anon-shift: 56 attempts / 45 substitutions / 2 successes; pseudo 76 has TWO uses (insn 18 address + insn 29 copy), so combine cannot substitute-and-delete, leaving the +1-copy behind. On baseline: same stat (56/45/2), but the two-use pseudo IS pseudo 72 (a0, 3 BB uses) and NO anonymous shift pseudo exists because the ashift RTX has its declared destination in the same insn. Baseline combine leaves NO surviving copy — the extra insn count differential between baseline (83) and cse-fold-anon-shift (84) originates at combine.c.
- probe: Compare tmp/grind/func_80045294/s6/base.i.combine (cse-fold-anon-shift form) against tmp/grind/func_80045294/s3/base.i.combine (baseline) at the ;;Function func_80045294 region. Baseline: insn 14 emits (set 75 (ashift 72 4)) — pseudo 75 (v1) is the shift's direct destination. cse-fold: insn 14 emits (set 76 (ashift 72 4)) into an anon pseudo, then insn 29 emits (set 81 76) — a copy that combine's try_combine cannot delete because pseudo 76 has two uses (insn 18 address computation + insn 29 copy).
- result: Baseline vs cse-fold combine behavior is fully accounted at the pass boundary: baseline lands the ashift directly into the declared pseudo (no copy); cse-fold splits ashift into an anon pseudo whose two-use pattern blocks combine's substitute-and-delete. The +1-copy insn count is minted at combine.c, then survives local_alloc/global_alloc because pool split assigns 76→$v0 (local) and 81→$v1 (global) without coalescing (s6/s24 wall). Both walls are chained: combine emits the copy; local/global split assigns different hard regs; no downstream pass folds them.
- verdict: CONFIRMED

## [s34] The 8-UID delta between sll (insn 14) and move16 (insn 22) consumed at sched2's rank_for_schedule LUID tiebreak is MINTED at the EXPAND pass by expand_expr walking s4's initializer, not at combine.c.
- mechanism: emit_insn assigns permanent INSN_UIDs sequentially during expand; combine can delete surviving insns via substitute-and-delete but never recycles UIDs. base.i.rtl (s3/base.i.rtl:14343-14378) shows expand emits insns 17 (symbol_ref temp) and 19 (mem-plus load) between v1's SET (insn 14) and i's SET (insn 22) — this two-insn interposition is the origin of the 8-UID delta.
- probe: Read s3/base.i.rtl lines 14343-14378 (expand output) and s3/base.i.combine lines 12656-12673 (post-combine). Confirmed insn 17 (symbol_ref temp for D_800EED14) is DELETED at combine (pseudo 77 was single-use, folded into insn 19's addressing) but insn 19 (s4 load) SURVIVES (pseudo 76 is downstream-live as call-arg to func_800520B8 at insn 82). LOG_LINK `insn_list 14` on insn 19 makes the sll → s4-load data dep explicit.
- result: The delta origin is EXPAND emit_insn, upstream of s33's combine.c wall. Combine is where the delta becomes IRREDUCIBLE (can't collapse either sll or move16), but the mint site is EXPAND.
- verdict: CONFIRMED

## [s34] Insn 19 (s4 = *(&D_800EED14 + v1)) can be relocated out from between insns 14 (sll) and 22 (move16) by pure-C restructuring without triggering the cse.c BB-scoped substitution wall (s7/s15) or the local/global_alloc pool split wall (s6/s24).
- mechanism: If insn 19 could be re-emitted before insn 14 or after insn 22, the between-insn count for sll → move16 would drop to 0 and the LUID delta at sched2 would collapse. Test whether any pure-C decl-cluster shape makes s4's load emit at a different position.
- probe: Enumerate structural options: (a) move insn 19 before insn 14 — impossible because expand_expr requires v1's SET (insn 14, defining pseudo 75) to precede any use of pseudo 75; insn 19's address computation `(plus 75 D_800EED14)` reads pseudo 75. (b) move insn 19 after insn 22 — requires `s4 = ...;` to follow `i = a0;` in the C, which is a strict sub-form of the H1 shape (i-before-v1) banked KILLED at score=11 via cse.c BB-scoped substitution rewriting the ashift operand from a0 to i, collapsing a0's live range (s1/s7/s15). (c) delete insn 19 — impossible; pseudo 76 is a live call-arg to func_800520B8 (insn 82 post-combine, via chain 19 → 28 s5-init → 82 call-arg-load).
- result: Insn 19's placement between insns 14 and 22 is structurally forced by s4's data dependence on v1 as address index. No pure-C form can relocate it without triggering one of the already-banked walls. This is the SIXTH pass-level wall (expand-side structural forcing) upstream of the five already ledger-cited.
- verdict: KILLED

## [s34] The 8-UID delta between sll (insn 14) and move16 (insn 22) consumed at sched2's rank_for_schedule LUID tiebreak is minted at the EXPAND pass by expand_expr walking s4's initializer, not at combine.c.
- mechanism: emit_insn assigns permanent INSN_UIDs sequentially during expand; combine can delete surviving insns via substitute-and-delete but never recycles UIDs. base.i.rtl (s3/base.i.rtl:14343-14378) shows expand emits insns 17 (symbol_ref temp) and 19 (mem-plus load) between v1's SET (insn 14) and i's SET (insn 22) — this two-insn interposition is the origin of the 8-UID delta.
- probe: Read s3/base.i.rtl lines 14343-14378 (expand output) and s3/base.i.combine lines 12656-12673 (post-combine). Confirmed insn 17 (symbol_ref temp for D_800EED14) is DELETED at combine (pseudo 77 single-use, folded into insn 19's mem addressing) but insn 19 (s4 load) SURVIVES because pseudo 76 is downstream-live as call-arg to func_800520B8 at insn 82. LOG_LINK insn_list 14 on insn 19 makes the sll→s4-load data dep explicit.
- result: Delta origin is EXPAND emit_insn, upstream of s33's combine.c wall. Combine is where the delta becomes IRREDUCIBLE, but the mint site is EXPAND.
- verdict: CONFIRMED

## [s34] Insn 19 (s4 load) can be relocated out from between insns 14 and 22 by pure-C restructuring without triggering the cse.c BB-scoped substitution wall (s7/s15) or the local/global_alloc pool split wall (s6/s24).
- mechanism: If insn 19 could be re-emitted before insn 14 or after insn 22, the between-insn count for sll→move16 would drop to 0 and the LUID delta at sched2 would collapse.
- probe: Enumerate options: (a) before insn 14 — impossible; expand requires v1's SET to precede any use of pseudo 75, which insn 19's address `(plus 75 D_800EED14)` reads. (b) after insn 22 — requires `s4 = ...;` to follow `i = a0;`, a sub-form of H1 (i-before-v1) KILLED at score=11 via cse.c BB-scoped substitution collapsing a0's live range. (c) delete insn 19 — impossible; pseudo 76 is a live call-arg to func_800520B8.
- result: Insn 19's placement between insns 14 and 22 is structurally forced by s4's data dependence on v1 as address index. No pure-C form can relocate it without triggering an already-banked wall. Sixth pass-level wall (EXPAND-side structural forcing) named upstream of the five ledger-cited walls.
- verdict: KILLED

## [s35] Reducing the shingle-search input from the whole target asm (s9, top sim 0.127) to the isolated 23-line prologue+block-0 slice (which contains the exact sll/move16/sw sched2 residual) will surface a corpus scratch whose C shape carries a byte-matching form of this cluster.
- mechanism: Prior s9 shingle-search may have been diluted by the two-loop dispatch tail (60 insns after block-0) that no other scratch shares; narrowing the fingerprint to the 23-line residual-region could raise similarity above the 0.15 relevance threshold and surface a novel scratch whose block-0 C shape produces sw/move16/sll in this order.
- probe: tools/decomp_me_scrape.py search --asm-file tmp/grind/func_80045294/s35/target_prologue_block0.s --corpus <corpus> --top 8 --shingle-k 3 across gcc272psx, gcc272cdk, psyq35 corpora (~5.4MB, ~1600 scratches total).
- result: Top hits: gcc272psx 6LWWm sim=0.122 (non-match, score=4200/4200); gcc272cdk hvTSS sim=0.111 (matched, but low similarity means block-0 doesn't share this function's cluster shape); psyq35 ajPwV sim=0.080 (non-match). Max cross-corpus similarity 0.122, LOWER than s9's whole-function 0.127 — narrowing input actually reduces overlap because most scratches don't share the exact 4-callee-save-preserve + shift-index-derivation prologue shape. All zero-score matched hits sit below 0.12 similarity, so their C shapes don't carry a transplantable form of the residual cluster.
- verdict: KILLED

## [s36] The sum!=0-branch tail-block (fn-ptr dispatch + second-loop shape) has a corpus twin above the s9-established 0.15 similarity threshold in one of the three PSX-GCC-2.7.2 local corpora, offering a transplant lever that whole-function and block-0-only searches (s9, s35) missed.
- mechanism: The tail block's `lw ; addu ; sw ; lui ; addu ; lw ; nop ; beqz ; nop ; lui ; addu ; lh ; jalr` fn-ptr dispatch skeleton is structurally distinctive; if any other scratch shares that closing shape, its C form would inform the whole-function residual by revealing what surrounding scaffolding lowers to that closing block.
- probe: Extract 39-line tail-block slice tmp/grind/func_80045294/s36/tail_block.s (from .L80045308 through .L800453A0 exclusive of the epilogue restore cluster); shingle-search across all three corpora at k=3 (loose) and k=4 (strict) via tools/decomp_me_scrape.py search.
- result: k=3 top = DqBYX 0.156 (gcc272cdk, func_8002D8CC, a 22-line fn-ptr dispatch wrapper with NO accumulator loop and NO gpu_DrawSync scaffolding — spurious skeleton-only match, verified by reading DqBYX's source_code); all other hits sub-0.14. k=4 collapses DqBYX to 0.056; max at k=4 is Z60NJ 0.069 in cdk and Tu4L2 0.055 in psyq35 — all well below the 0.15 threshold. Direct comparison to s9 confirms sparse-shingle signal is a k=3 artifact.
- verdict: KILLED

## [s36] The whole-function shingle search at k=5 (stricter than s9's k=3 default) surfaces a distant-but-genuine corpus twin that k=3 diluted via short-shingle noise; if any scratch crosses 0.15 at k=5, it is far more likely to be a real structural analogue.
- mechanism: Longer shingles reduce common-bigram noise (like `lw ; nop`, `addu ; addiu`) and reward runs of matching instruction sequences; a 5-shingle hit at >=0.15 would indicate a genuinely similar function-body flow.
- probe: Shingle-search asm/funcs/saTan0Init.s (whole 88-insn function) at k=5 across all three corpora.
- result: Max similarity across all three corpora at k=5 = 0.095 (KrpLx, gcc272cdk); psyq35 top = Tu4L2 0.083; gcc272psx top = 5t0dj 0.059. All hits well below the 0.15 threshold. Signal decreases monotonically from s9 (k=3 top 0.127) to k=5 (top 0.095), matching the s35 pattern that narrowing the search REDUCES the max similarity — this function's fingerprint is genuinely rare across the local corpus.
- verdict: KILLED

## [s37] Consolidated 36-session ledger reduces to a single non-grindable state: six pass-level walls cited at direct pass-output tier + 25-form rejected bank closed against reachable pure-C search + ~137,752-iter permuter budget across 9 chassis/mode combinations empirically closed + 14 rederive sub-axes killed + s30 formal structural free-axis closure. No sanctioned axis remains un-measured; the sole remaining sanctioned move is OWNER-ESCALATION drafting (docs/grind/decisions.md entry) followed by owner-gated on the next session.
- mechanism: Six pass-level walls chained upstream-to-downstream close every pure-C lever hand-derivation can reach: (1) EXPAND emit_insn insn 19 structural forcing (s34) — s4 load's data-dep on v1 forces it between insns 14 and 22, minting the 8-UID delta at expand; (2) combine.c substitute-and-delete refusal (s33) — pseudo 72 (a0) has 3 cross-BB uses so insns 14 and 22 survive with LUIDs 14 and 22; (3) cse.c BB-scoped operand substitution (s7/s15/s25) — any C-source LUID lever that reaches target sched2 order triggers value-equivalence class collapse of a0 into i, rewriting the ashift operand and collapsing a0's live range; (4) local_alloc/global_alloc pool split with no coalescer (s6/s24) — any anon-ashift near-hit lands the block-0-scoped pseudo in local_alloc's $v0 and the loop-carried v1 pseudo in global_alloc's $v1 without a coalescing pass to fold them; (5) sched1+sched2 rank_for_schedule LUID tiebreak (s3/s15/s16) — both passes consume LUID(sll)=14 vs LUID(move16)=22 identically, no independent tiebreaker; (6) no pure-C CFG-splitter defeats cse.c BB view (s10/s11/s25) — do-while(0) collapses in jump.c pass 1 AND cse_extended_basic_block spans fall-through edges (BOTH mechanisms fire independently); statement-expression collapses in c-parse.y before RTL. saTan0Init has no natural semantic conditional between i=a0 and v1=a0<<4 to insert a real branch.
- probe: Reconfirmed baseline at s37 (score=2, target=83, build=83, cheat_asm_stripped=78) via `& tools/wteng.ps1 main sandbox func_80045294 --disable all`; cross-checked all 25 rejected/*.c forms against the six walls; enumerated permuter chassis/mode combinations (baseline v1-before-i minimal, baseline v1-before-i full-TU, H1 i-before-v1 minimal, cse-fold-anon-shift, sum-at-position-2 free-axis; unbounded random, PERM_LINESWAP-directed, PERM_GENERAL-directed) totalling s13/s14/s20/s22/s22-alt/s23/s31/s32/s32-sum = 137,752 iters; enumerated rederive sub-axes (s8 mips-gcc-c, s17 mipsel-gcc-c, s17 mips-ido-c, s17 Kengo-debug/globals, s17 BB2-sibling, s18 --gotos-only, s18 --target mipsel-mwcc-c, s18 --target mipsee-gcc-c + --no-stack-spill + --deterministic-vars, s26 --stack-structs + --descending-regs, s26 --context + --globals all, s27 Kengo asm-level, s9 decomp.me whole-fn shingle-search, s35 block-0-only shingle-search, s36 tail-block-only k=3+k=4 + whole-fn k=5); enumerated structural free-axis: s29 sum-position bounded {0,1,2}, s30 two-axis + three-axis superposition all NEUTRAL.
- result: Baseline reconfirmed score=2 unchanged. Six pass-level walls chained: EXPAND -> combine -> cse -> local/global alloc -> sched1+sched2, with the do-while(0)/stmt-expr CFG-splitter path independently exhausted. 25 rejected forms map 1-to-1 onto the walls. Permuter empirically closed at 9 chassis/mode combinations converging on score=60 basin (except cse-fold-anon-shift at 105 and s31 semantic-break at score=10). Rederive dead across 14 independent sub-axes with the whole-corpus max sim = 0.127 (s9) collapsing to 0.095 at k=5. Structural free-axis formally closed at s30. Ledger state is TERMINAL for grinder modalities.
- verdict: CONFIRMED

## [s37] The next session's sanctioned move is drafting the OWNER-ESCALATION entry in docs/grind/decisions.md; no other sanctioned axis remains un-measured, and owner-gated emission is blocked THIS session because the entry is not yet filed (contract precondition).
- mechanism: Grinder contract permits owner-gated only when (a) an OWNER-ESCALATION entry for func_80045294 already exists in docs/grind/decisions.md AND (b) every remaining sanctioned axis is measured dead. Condition (b) is satisfied as of s28/s32/s34/s36; condition (a) is not (verified: `grep -c func_80045294 docs/grind/decisions.md` = 0). A synthesis session cannot file the escalation entry (owner-authored artifact). The next session -- whatever modality the driver picks -- should either (i) file the escalation entry then emit owner-gated the following session, or (ii) if the driver picks rederive/permuter/forensics/structural, note in its outcome that its measurement is redundant with the exhausted-modality banking and immediately move to consolidation.
- probe: `grep -c func_80045294 docs/grind/decisions.md` = 0 (verified this session at s37); s24 already banked the same result.
- result: owner-gated is not emittable this session; the driver-picked synthesis modality is discharged by consolidating the six-wall pass-chain and noting the escalation-drafting prerequisite. Frontier reset to a single top item: draft docs/grind/decisions.md OWNER-ESCALATION entry citing the six walls + 25-form bank + 137,752-iter permuter budget + 14 rederive sub-axes + s30 free-axis closure. Supplementary evidence entry: the s35/s36 rare-fingerprint signal (max sim 0.122 whole-fn / 0.156-collapsing-to-0.056 tail-block k=3-to-k=4 / 0.095 whole-fn k=5) reinforces that this function's shape is not a shared cross-project pattern in the local ~1600-scratch corpus.
- verdict: CONFIRMED

## [s37] Consolidated 36-session ledger reduces to a single non-grindable state: six pass-level walls, 25-form rejected bank, ~137,752-iter permuter empirical closure, 14 dead rederive sub-axes, s30 formal structural free-axis closure -- no sanctioned pure-C axis remains un-measured.
- mechanism: Six walls chained upstream-to-downstream: (1) EXPAND emit_insn insn 19 structural forcing [s34] -- s4 load's data-dep on v1 (pseudo 75) forces it between insns 14 and 22, minting the 8-UID delta. (2) combine.c substitute-and-delete refusal [s33] -- pseudo 72 (a0) has 3 cross-BB uses (insns 14/22/89) so insns 14 and 22 survive with LUIDs 14 and 22. (3) cse.c BB-scoped ashift-operand substitution [s7/s15/s25] -- any C-source LUID lever reaching target sched2 order triggers value-equivalence collapse of a0 into i, rewriting ashift operand and collapsing a0's live range. (4) local/global_alloc pool split with no coalescer [s6/s24] -- block-0-scoped anon-ashift pseudo lands in local_alloc's $v0 while loop-carried v1 lands in global_alloc's $v1; GCC 2.7.2 has no cross-pool coalescer (added in GCC 3+). (5) sched1+sched2 rank_for_schedule LUID tiebreak [s3/s15/s16] -- both passes consume LUID(sll)=14 vs LUID(move16)=22 identically; no independent tiebreaker. (6) No pure-C CFG-splitter defeats cse's BB view [s10/s11/s25] -- jump.c pass 1 collapses do-while(0) AND cse_extended_basic_block spans fall-through; statement-expression collapses in c-parse.y before RTL; saTan0Init has no natural semantic conditional between i=a0 and v1=a0<<4 to insert a real branch.
- probe: s37 reconfirmed baseline (score=2 target=83 build=83) via `& tools/wteng.ps1 main sandbox func_80045294 --disable all` (tmp/grind/func_80045294/s37/sandbox_baseline.txt); cross-checked all 25 rejected/*.c forms against the six walls (every form maps 1-to-1 onto one wall); enumerated permuter chassis/mode combinations (9 combinations, 137,752 iters); enumerated 14 rederive sub-axes; enumerated s29/s30 free-axis surface closure.
- result: Baseline score=2 unchanged. Six walls chained cover every reachable pure-C lever. 25 rejected forms exhaust the pure-C search space in categories A/B/C/D/E/F. Permuter empirical closure at 9 chassis/mode combinations converging on score=60 (except cse-fold-anon-shift at 105 and s31 semantic-break at score=10). Rederive dead across 14 sub-axes with rare-fingerprint signal (max whole-fn sim 0.127, block-0-only 0.122, tail-block k=3 0.156 collapsing to 0.056 at k=4, whole-fn k=5 0.095) confirming shape is not a shared cross-project pattern. Structural free-axis formally closed at s30.
- verdict: CONFIRMED

## [s37] owner-gated is emittable THIS session per contract precondition (docs/grind/decisions.md entry filed AND every sanctioned axis measured dead).
- mechanism: Contract permits owner-gated only when a filed OWNER-ESCALATION entry for func_80045294 exists in docs/grind/decisions.md AND every remaining sanctioned axis is measured dead. Second clause satisfied since s28/s32/s34/s36. First clause requires the entry file to exist.
- probe: `grep -c func_80045294 docs/grind/decisions.md` at s37.
- result: 0 hits, entry not filed (same result as s24). owner-gated is blocked. The synthesis modality is discharged by consolidation; next session must draft the escalation entry before owner-gated becomes emittable.
- verdict: KILLED

## [s38] Wrapping the entire function body in a bare outer `{ }` scope (no statement-position v1 assignment introduced, distinct from s4 inner-block-defer-v1) shifts the sll/move16 sched2 tie via a per-block allocno-ordering effect in global_alloc.
- mechanism: If global.c allocated pseudos in per-block passes, an outer scope would give the entire {sum,v1,s4,i,count,s5} cluster a distinct allocno bucket from the params, potentially shifting the reg_n_refs priority queue at the sll/move16 tiebreak.
- probe: Edited src/text1a_c.c func_80045294 body to `void func_80045294(s32 a0, s32 a1) { { s32 sum=0; ...; } }` -- outer bare-block wrapper, no other change. Ran sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83, rules_dropped=0. NEUTRAL. Byte-identical to baseline residual (same sll/sw/move16 vs sw/move16/sll cluster). Rejected form banked at memory/grind/func_80045294/rejected/outer-block-wrap.c.
- verdict: KILLED

## [s38] Plain `register s32 v1 = a0 << 4;` storage-class hint (NOT a `register T x asm("$N")` cheat pin -- ordinary C) biases GCC 2.7.2's RA priority or LUID assignment enough to shift the sll/move16 sched2 tie.
- mechanism: If GCC 2.7.2's local-alloc.c or global.c applied a priority boost for `register`-declared locals, v1's pseudo might allocno-rank higher or its assignment LUID might shift within expand_expr's tree walk.
- probe: Edited src/text1a_c.c to `register s32 v1 = a0 << 4;`; other decls unchanged. Ran sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83, rules_dropped=0. NEUTRAL. `register` without asm binding is purely advisory in GCC 2.7.2; RA decisions unchanged from baseline. Rejected form banked at memory/grind/func_80045294/rejected/register-storage-class-v1.c.
- verdict: KILLED
