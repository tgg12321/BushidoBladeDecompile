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

## [s39] Changing the a0 parameter signature from s32 to u32 shifts the sll/move16 sched2 tie by re-typing pseudo 72's DECL_MODE.
- mechanism: Hypothesis: PARAM-signature-level sign vs unsigned might route through a different expand_shift path than operand-side or destination-decl-side casts (already killed at s5 and s12). If TYPE_UNSIGNED on the DECL propagates to the ashift RTX's mode, downstream cse/global might behave differently.
- probe: Edited src/text1a_c.c signature from `void func_80045294(s32 a0, s32 a1)` to `void func_80045294(u32 a0, s32 a1)`; body unchanged. Ran sandbox --disable all.
- result: score=2, target_insns=83, build_insns=83 -- byte-identical to baseline candidate.c. GCC 2.7.2 expand_shift normalises constant power-of-2 shifts to (ashift SI) regardless of param signedness; DECL_MODE stays SI on the arg pseudo.
- verdict: KILLED

## [s39] Changing BOTH a0 and a1 to u32 (rather than just a0) shifts the tie.
- mechanism: Composite param-signature axis; a1's type could ripple through s5's addu path.
- probe: Edited signature to `void func_80045294(u32 a0, u32 a1)`. Sandbox --disable all.
- result: score=2, 83/83 -- byte-identical to baseline. addu is signedness-agnostic in expand.
- verdict: KILLED

## [s39] Casting the address base as `(u32)&D_800EED14` (integer arithmetic) instead of `(u8 *)&D_800EED14` (pointer arithmetic) shifts s4's initializer LUID.
- mechanism: Different declared type at the addr-base surface could route through a distinct expand-addr path (PLUS on integer vs pointer). Un-tested by s1-s38 (which varied operand-side + destination-decl-side + arithmetic-tree shape but not the addr-base cast).
- probe: Edited s4's init to `s32 s4 = *(s32 *)((u32)&D_800EED14 + v1)`. Sandbox --disable all.
- result: score=2, 83/83 -- byte-identical to baseline. GCC fold reduces both spellings to the same (plus (symbol_ref) (reg:SI)) tree before tree_LUID is assigned.
- verdict: KILLED

## [s39] Split-init on s5 seeded with a1 (mirror of s3's a4-seed split-init) is a lever for the tie.
- mechanism: s3 killed `s32 s5 = s4; s5 += a1;` (seed s4, +1 copy displaces $21 addu). The mirror seed (a1) has not been tested; addu operand-order asymmetry (s2) suggests seed=a1 might land the operand-order differently.
- probe: Edited s5 init to `s32 s5 = a1; s5 += s4;`. Sandbox --disable all.
- result: score=3, 83/83 -- +1 diff versus baseline. Split-init on s5 is HARMFUL in both operand-order variants; a1-seed adds one addu-operand-order diff on top of the s3 s4-seed kill.
- verdict: KILLED

## [s40] Exhaustively enumerating all 6! = 720 permutations of the {sum, v1, s4, i, count, s5} decl block via PERM_LINESWAP on the minimal-base chassis (fresh seed, -j 8) will surface a score<60 basin not reached by s14's under-sampled 720-iter minimal-base run or by prior random-mode campaigns.
- mechanism: PERM_LINESWAP enumerates the ordered set of parenthesized lines; with 6 items the search space is fully covered by 720 iterations. If any decl permutation shifted the sll/move16 LUID delta or defeated the s7 cse.c BB-scoped operand substitution mechanism, it would surface as a sub-60 find in the exhaustive sweep.
- probe: python3 tools/permuter_campaign.py launch --func func_80045294 --dir tmp/grind/func_80045294/s40/perm_min_lineswap --label s40-min-lineswap-freshseed -j 8 --stop-on-zero (base score 60, PERM_LINESWAP over the 6-decl block); polled until natural iteration limit; harvest --stop
- result: 720 iterations in 699.4s, 1 new find at score=60 (28.3s from launch, decl order v1/s4/s5/sum/count/i preserving H1's v1-before-i constraint -- member of the s1/s3/s11/s12/s29/s30 free-axis equivalent basin). best_new_score=60; no sub-60 find.
- verdict: KILLED

## [s41] An exhaustive 5!=120 PERM_LINESWAP sweep over the 5-decl block {sum, v1, s4, i, s5} on a count-inlined chassis (D_800A33AC inlined in both loop guards, no `count` local -- s8 neutral-basin variant never permuted) will surface a sub-60 basin escape distinct from the s40 6-decl exhaustive sweep on the count-local chassis.
- mechanism: The count-inlined chassis eliminates one insn (the `count = D_800A33AC` load) from block 0 without changing the sll/move16 residual (s8 CONFIRMED score=2 unchanged). The removed LUID slot could open a distinct rank_for_schedule LUID window at sched2 for one of the 120 decl-order permutations, potentially breaking the sll/move16 tie for a form that under the count-local chassis was equivalent-basin. Exhaustive enumeration covers the full free-axis surface for this chassis.
- probe: python3 tools/permuter_campaign.py launch --func func_80045294 --dir tmp/grind/func_80045294/s41/perm --label s41-count-inlined-lineswap -j 4 (fresh seed on a new workspace copied from s13 template with count inlined and PERM_LINESWAP wrapping the 5-decl block). Polled 10 min via wsl-side sleep; harvest --stop with reason `s41-exhaustive-5perm-count-inlined`.
- result: 120 iterations (all 5! permutations enumerated), elapsed 635.4s, 1 new find at score=60 (permutation order v1/sum/i/s4/s5 -- member of the s1/s3/s11/s29/s30 free-axis basin extended to count-inlined chassis). best_new_score=60. No sub-60 find. Artifact: tmp/grind/func_80045294/s41/perm/output-60-1/source.c; rejected form banked as memory/grind/func_80045294/rejected/s41-count-inlined-perm-lineswap-v1-sum-i-s4-s5.c.
- verdict: KILLED

## [s42] The s41-frontier-listed OWNER-ESCALATION entry for func_80045294 can be filed by a non-permuter session (s42 forensics) consolidating the pass-level dossier already banked across s3/s6/s7/s15/s16/s33/s34, satisfying contract precondition (a) for future owner-gated emission.
- mechanism: docs/grind/decisions.md is not on the forbidden-file list (regfix/asmfix/.claude/rules/engine/tools/Makefile/*.ld). The frontier item explicitly identifies filing as the sole remaining actionable move outside permuter modality. Forensics-modality synthesis is the natural vehicle because every relevant GCC pass has already been dumped and named at pass-source level.
- probe: Appended entry to docs/grind/decisions.md dated 2026-07-19 with mutually-exclusive owner options (a) canonical-asm authorization mirroring func_80037540 oversized-locals carve-out (2026-07-13 18:05) vs (b) uphold pure-C bar (park at INCOMPLETE-owner-accepted). Dossier cites 6 pass-level walls (EXPAND emit_insn / combine multi-use block / cse.c BB-scoped subst / local_alloc-global_alloc pool split no-coalescer / sched1 rank_for_schedule LUID tiebreak / sched2 same-mechanism re-decision), 30-form rejected bank, 137,872-iter permuter across 11 chassis/mode combos, 14 rederive sub-axes, s29/s30 structural closure, s38/s39/s40/s41 recent axis kills.
- result: Entry filed; contract precondition (a) now satisfied. Owner-gated becomes emittable by subsequent sessions once PERM_INT-mode (s41 frontier item 2) is also measured dead.
- verdict: CONFIRMED

## [s42] Baseline is unchanged from s41 (floor=2, 83/83 insns, 0 rules, 78 cheat-asm-stripped file-wide, current in-src candidate is the s3 v1-before-i form).
- mechanism: sandbox --disable all runs the full build with regfix/asmfix disabled and cheat-asm stripped; the resulting objdump score is the honest pure-C distance.
- probe: & tools/wteng.ps1 main sandbox func_80045294 --disable all (artifact: tmp/grind/func_80045294/s42/baseline_recon.txt).
- result: score=2, target_insns=83, build_insns=83, scorable=true, rules_dropped=0, cheat_asm_stripped=78, disabled_o=tmp/sandbox/func_80045294/text1a_c.o. Identical to s41.
- verdict: CONFIRMED

## [s43] The sw $s0,16($sp) callee-save (insn 211) is minted by reload with the highest LUID (211) of the seven callee-save stores, and its position between sll(insn 14) and move16(insn 22) at sched2 forward emission is driven by (i) REG_DEP_ANTI 211 on insn 22 forcing sw→move in forward stream and (ii) hazard-hoist elevating memory ops in the T-10 ready list.
- mechanism: reload_as_needed/emit_prologue in GCC 2.7.2 iterates hard-regs per mips.md FUNCTION_PROLOGUE and emits callee-save stores at monotonically increasing LUIDs starting after every expand-emitted insn (LUIDs 197..211 vs pre-reload max ~32). sched2.rank_for_schedule (sched.c:2398-2456) hazard-hoists the ready-list to prioritize memory-op insns at the same-priority T-10 tick.
- probe: Read tmp/grind/func_80045294/s3/base.i.sched2 lines 20105-20368 for the block-0 initial priorities, ready-list trace T-1..T-18, and the post-sched2 RTL insn sequence with insn 211's insn_list 197 dep and insn 22's REG_DEP_ANTI 211.
- result: Directly observed at pass output: reload assigns LUID 211 to sw $s0; ready-list at T-10 emits 'insn 211 has a greater potential hazard, now 211 14 12 6'; forward emit order is sll(14) at pos 8, sw(211) at pos 9, move16(22) at pos 10 — matches build's 3-insn cluster and rules out sw hoisting further away.
- verdict: CONFIRMED

## [s43] Target's forward order 'sw ; move ; sll' is unreachable by any pure-C mutation through cc1's emission alone, because it would require BOTH LUID(sll) > LUID(move) AND LUID(move) > LUID(sw=211); pre-reload LUIDs cannot exceed reload-minted LUIDs by construction of GCC 2.7.2's reload pass.
- mechanism: Backward-list-sched at T-9 must pick sll to place it at pos 10 (LUID(sll) > LUID(move)); at T-10 the ready list is {move, 211(sw), 12, 6} and rank_for_schedule LUID-descending tiebreak (plus hazard-hoist for mem ops) forces sw to pos 9 unless LUID(move) > LUID(sw). But every pre-reload insn (LUID <= 32 in this function) is emitted before reload allocates LUIDs 197..211; there is no C-source axis that shifts a pre-reload insn's LUID above 197.
- probe: Analytical: if LUID(sll)>LUID(move), simulate the T-9..T-11 rank_for_schedule picks with insn 211's LUID unchanged and dep set intact — forward emission yields 'move ; sw ; sll' (not target). Combined with s7/s15's proof that LUID(sll)>LUID(move) itself forces cse.c BB-scoped substitution + RA rotation (banked KILLED at H1 and 30+ rejected forms), both required conditions for target order are independently impossible via pure C.
- result: Both conditions independently blocked at named GCC-pass level. Reload LUID monotonicity confirmed by tmp/grind/func_80045294/s3/base.i.rtl vs base.i.greg diff (pre-reload max LUID 32; reload adds 195, 197, 199, 201, 203, 205, 207, 209, 211).
- verdict: CONFIRMED

## [s44] Kengo saTan0Init at 0x00147dc8 size 0x14c=83 insns (s8 name+size identity claim) is the same source function as BB2 func_80045294 and a disassembly-based m2c rederive from Kengo's ASM could surface novel structural transplant leads.
- mechanism: Kengo/BB2 shared 'sa_tan0.c' source lineage would preserve statement order and idiom; PS2 GCC (newer) applied to the same C could produce ASM whose control flow reveals source ordering that BB2's GCC 2.7.2 sched2/cse.c obscured (s7 mechanism).
- probe: mipsel-linux-gnu-objdump -d --disassembler-options=no-aliases --start-address=0x00147dc8 --stop-address=0x00147f14 Kengo/disc/SLUS_200.21 > tmp/grind/func_80045294/s44/kengo_saTan0Init.asm; compare shape against BB2 candidate.c body.
- result: SHAPES DISJOINT. Kengo: 160-byte frame, saves s0-s8+ra (9 callee-saves), ~9 calls (saRobInit, saTanMainDataCreate, tslGlobalMemAlloc, saFidLoad-in-loop, tslSmdInitObj, motion_SetMotion, motion_CalcMotion, saTan0InitPrim, saSePlaySeq), single 6-iter init loop over object slots, terminates with two sb-through-gp stores. BB2: small frame, ~4 callee-saves, 2 calls (gpu_DrawSync + func_800520B8), TWO scan loops over per-slot table D_800EED10/14/18/1C, fn-ptr dispatch at +0x1C offset with s16 arg, terminates D_800A33A0 += a1; D_800A33A4 -= a1. 83-insn size match is coincidence (calling-convention frame + 9 vs 2 calls happen to sum equal). Symbol name 'saTan0Init' was reused by Kengo devs for a different function in the PS2 restructure; BB2's named_syms.txt saTan0Init label at 0x80045294 is either a BB2-side naming choice or a stale name transplant that did not verify shape.
- verdict: KILLED

## [s45] Rewriting s4's initializer from pointer-arith `*(s32*)((u8*)&D_800EED14 + v1)` to array-subscript `((s32*)&D_800EED14)[a0 << 2]` routes through expand_expr's ARRAY_REF path and could shift tree_LUID relative to the sll/move16 assignments.
- mechanism: expand_expr's ARRAY_REF handler in c-typeck.c/expand.c is distinct from the INDIRECT_REF+PLUS_EXPR path used by the pointer-arith spelling. If fold-const does not collapse the two paths to identical PLUS_EXPR trees, a distinct LUID sequence emerges.
- probe: Edited src/text1a_c.c line 1605 to `s32 s4 = ((s32 *)&D_800EED14)[a0 << 2];`; ran & tools/wteng.ps1 main sandbox func_80045294 --disable all. Artifact tmp/grind/func_80045294/s45/probe_array_subscript_s4.txt.
- result: score=2, target_insns=83, build_insns=83 -- NEUTRAL. Byte-identical to baseline. GCC 2.7.2 fold-const collapses ARRAY_REF-with-INT_CST-subscript to the same PLUS_EXPR pointer-arith tree at fold time. No LUID movement. 17th independent rederive sub-axis banked; rejected form saved to memory/grind/func_80045294/rejected/array-subscript-s4-init.c.
- verdict: KILLED

## [s45] Rewriting the first-loop val load from pointer-arith `*(s32*)((u8*)&D_800EED18 + v1)` to array-subscript `((s32*)&D_800EED18)[v1 >> 2]` on the RUNTIME v1 variable exercises a distinct expand path than sub-axis 17's constant-foldable subscript case, and could either be neutral (if fold-const cancels >>2/<<2) or perturb the sll/move16 tie.
- mechanism: Runtime-variable subscript defers scaling to expand-time; combine.c's post-expand pass would have to cancel the srl v1,v1,2 against the subsequent <<2 byte-scaling. If combine cannot cancel (because v1 has additional cross-BB uses), the srl survives and adds insns.
- probe: Edited src/text1a_c.c line 1612 to `s32 val = ((s32 *)&D_800EED18)[v1 >> 2];`; ran sandbox --disable all. Artifact tmp/grind/func_80045294/s45/probe_array_subscript_loop.txt.
- result: score=8, target_insns=83, build_insns=86 -- HARMFUL. Adds 3 extra insns per iteration; combine cannot cancel the srl/scale pair because v1 is a multi-use loop-carried pseudo. 18th independent rederive sub-axis banked as harmful; rejected form saved to memory/grind/func_80045294/rejected/array-subscript-loop-body.c.
- verdict: KILLED

## [s46] A PERM_INT-mode campaign wrapping the load-bearing shift constant in `a0 << 4` can lower the sandbox score below 60 by finding an integer-literal spelling that shifts the sll/move16 sched2 LUID delta without triggering cse.c BB-scoped substitution or the local/global_alloc pool split.
- mechanism: PERM_INT emits str(low + seed) for random seeds in [low, high]; combined with PERM_LINESWAP over the 6-decl block on the minimal-base chassis, the permuter would search a broader constant-lowering surface than the s5/s45 hand-derived arithmetic-form probes ((u32)a0*16u, a0*16, (u8*)0+a0, ARRAY_REF).
- probe: Empirically bracket the shift constant in src/text1a_c.c line 1604: change `a0 << 4` to `a0 << 3` and `a0 << 5` in turn, sandbox --disable all each time, compare against baseline score=2.
- result: shift=3 -> score=2 (83/83); shift=5 -> score=2 (83/83); baseline shift=4 -> score=2 (83/83). Three distinct shift immediates all yield the SAME score. The sandbox's objdump-based scorer does not distinguish the sll immediate value (the mispositioned sll at the wrong position + the missing sll at the target position sum to 2 diffs regardless of immediate). Therefore no PERM_INT enumeration over the shift constant can lower the score below 60 by construction. Sandbox re-runs post-revert confirm baseline restored at score=2; src/text1a_c.c is git-clean.
- verdict: KILLED

## [s47] The sched2 prologue-cluster order has a solution set BROADER than the H1 (i-before-v1) shape — some dependence-edge, priority, or unit/cost perturbation reaches target's order while leaving a0's reference count intact.
- mechanism: sched.c `rank_for_schedule` breaks the sll-vs-move16 tie on INSN_LUID only after priority and dependence class tie. A perturbation of the dependence graph (aliasing, expression split/merge, memory-access ordering) or of the machine cost (load vs move, addressing mode) could reorder the pick without touching source statement order — i.e. without opening the cse equivalence region that costs a0 its fourth reference.
- probe: `tools/sched_solver/extract.py text1a_c` (parity=True) then `perturb.py tmp/sched_solver_work/text1a_c.sched.json --func func_80045294 --pass 2 --block 0 --goal-order 32,31,28,25,19,203,201,199,14,22,211,12,209,6,205,4,207,197 --depth 1 --max 200` — an EXHAUSTIVE enumeration of all 960 single atoms across all five atom classes. Artifact tmp/grind/func_80045294/s47/sched_perturb_p2_allatoms_d1.txt.
- result: KILLED. Exactly 13 vectors reach the goal and all 13 encode one intent: the `i = a0` statement must precede the `v1 = a0 << 4` statement (5 luid swaps, 5 luid_moves, `luid_move 22 -> before 14`, and `add_dep 14 <- 22` in both true/data and anti-output kinds — the add_dep pair being the RTL spelling of "the shift reads i", i.e. the same C intent). There is NO order-only lever. The sched2 half of the residual is exactly the H1 shape and nothing else.
- verdict: KILLED

## [s47] Under the H1 chassis (target sched2 order already achieved, score 11), the register-allocation rotation is closable by a pure-C lever.
- mechanism: `ra_solver` models global.c exactly; if the rotation is driven by a modelled input (reg_n_refs, live length, birth order, conflicts, preferences) then inverse.py names the minimal perturbation and maps it to a C technique. H1 gives target's instruction ORDER for free, so closing the allocation would close the function.
- probe: `tools/ra_solver/extract.py func_80045294 text1a_c` on the H1 source (simulate.py: sort order MATCH, dispositions 11/11), then `inverse.py global tmp/ra_solver_work/func_80045294.model.json --goal {"72": 18, "77": 20, "80": 21} --depth 2 --top 12`. Comparison model extracted on the candidate chassis. Artifacts tmp/grind/func_80045294/s47/{h1.model.json,cand.model.json,ra_inverse_h1.txt}.
- result: REACHABLE IN THE MODEL, minimal solution size 1 atom, 5 distinct vectors — four `[refs_up] pseudo 72: refs 3->{4,5,6,7}` and one `[live_shrink] pseudo 72: livelen 23->15`. All 24 preference atoms are reported FORECLOSED (the goal registers $s2/$s4/$s5 are callee-saved, so they can never appear as hard regs in pre-RA RTL and set_preference can never record them — only a forbidden register-asm pin reaches them). The candidate-vs-H1 model diff isolates the cause exactly: pseudo 72 (a0) has nrefs_flow 4 on the candidate chassis and 3 on H1, with livelen_flow 24 and calls_crossed 2 IDENTICAL on both. The whole 2-insn residual is one reference count.
- verdict: CONFIRMED (the RA half is model-reachable; see the next hypothesis for whether the atom is C-spellable)

## [s47] The `refs_up pseudo 72: 3 -> 4` atom is spellable in honest C by giving a0 a fourth reference in a basic block where cse.c cannot substitute it away.
- mechanism: cse.c's operand substitution is BASIC-BLOCK scoped (the s7 wall). Under H1 the first block's `i = a0` makes reg 75 equivalent to reg 72, so the ashift operand is rewritten 72 -> 75 and a0 loses a reference. The SECOND loop's index shift (`v1 = i << 4`) is in a different region of the function; spelling it on a0 instead (`v1 = a0 << 4`) would restore the reference if cse's equivalence class does not reach there.
- probe: H1 first block + second-loop shift spelled on a0, applied to src/text1a_c.c and sandboxed --disable all. Artifact tmp/grind/func_80045294/s47/vA_sandbox.txt; form banked rejected/h1-second-loop-shift-on-a0.c.
- result: KILLED. score = 11, 83/83 — unchanged from plain H1. The second loop's own block re-establishes the equivalence (`i = a0;` immediately dominates the shift there too), so cse substitutes a second time and reg_n_refs(72) stays 3. Moreover the ORIGINAL contains exactly FOUR a0 references (def @8004529C, uses @800452B4, @800452B8, @8004532C, after which $s2 is redefined as the walking pointer @80045344), and all four sit inside the equivalence region that H1 opens — so there is no fifth site to spend and no honest algorithmic ref-lift available. The companion `live_shrink 24 -> 15` atom is likewise unreachable: a0's last reference in the original is after both calls, so no C form retires it earlier without deleting a reference the target contains.
- verdict: KILLED

## [s47] TYPED FORECLOSURE — the two conditions target's bytes require are mutually exclusive in pure C for this function.
- mechanism: (A) target's sched2 order requires `i = a0` before `v1 = a0 << 4` (unique intent of all 13 exhaustively-enumerated sched atoms); (B) target's allocation requires reg_n_refs(a0) == 4 (the single-atom RA inverse, with livelen and calls-crossed already identical). (A) is precisely the condition that lets cse.c convert one of a0's four references into a reference to `i`, forcing (B) to 3. Both models are exact, both solution sets are enumerated, and both remaining lever classes (preference atoms; a fifth a0 reference) are foreclosed — preferences by callee-save unreachability in pre-RA RTL, the fifth reference by the target's own instruction census.
- probe: composition of the two exhaustive searches above plus the direct H1 + second-loop-shift-on-a0 measurement.
- result: The residual is not a survey of 33 tried spellings but a proof over an enumerated lever space. Every banked rejected form is an instance of taking (A) without (B) (the score-11/12 H1 basin) or (B) without (A) (the score-2 candidate basin); no third basin exists within the modelled atom space.
- verdict: CONFIRMED

## [s47] The sched2 prologue-cluster order has a solution set broader than the H1 (i-before-v1) shape — some dependence-edge, priority, or unit/cost perturbation reaches target's order without touching source statement order, and therefore without opening the cse equivalence region that costs a0 its fourth reference.
- mechanism: sched.c rank_for_schedule breaks the sll-vs-move16 tie on INSN_LUID only after priority and dependence class tie, so a dependence-graph or machine-cost change (aliasing, expression split/merge, memory-access ordering, load-vs-move selection) could reorder the pick independently of LUID.
- probe: tools/sched_solver/extract.py text1a_c (parity=True, 152 funcs, 532 blocks) then perturb.py --func func_80045294 --pass 2 --block 0 --goal-order 32,31,28,25,19,203,201,199,14,22,211,12,209,6,205,4,207,197 --depth 1 --max 200 — an exhaustive enumeration of all 960 single atoms across all five atom classes. Model reports the block 'baseline exact'. Artifact tmp/grind/func_80045294/s47/sched_perturb_p2_allatoms_d1.txt.
- result: Exactly 13 vectors reach the goal and all 13 encode one C-level intent: the statement generating insn 22 (i = a0) must precede the statement generating insn 14 (v1 = a0 << 4). Namely 5 x 'luid swap 14 <-> {19,22,25,28,32}', 5 x 'luid_move 14 -> before {19,25,28,31,32}', 'luid_move 22 -> before 14', and 'add_dep 14 <- 22' in both true/data and anti-output kinds (the RTL spelling of 'the shift reads i'). No order-only lever exists. That intent is the H1 shape, re-measured this session at sandbox score 11.
- verdict: KILLED

## [s47] Under the H1 chassis (target sched2 order already achieved, score 11) the register-allocation rotation is closable by a pure-C lever.
- mechanism: ra_solver models global.c allocno priority + find_reg exactly; if the rotation is driven by a modelled input (reg_n_refs, live length, birth order, conflicts, preferences) then inverse.py names the minimal perturbation and maps it to C techniques. H1 gives target's instruction order for free, so closing the allocation would close the function.
- probe: tools/ra_solver/extract.py func_80045294 text1a_c on the H1 source; simulate.py reports 'sort order: MATCH' and 'dispositions: 11/11 match'; then inverse.py global --goal {"72":18,"77":20,"80":21} --depth 2 --top 12. Comparison model extracted on the candidate chassis. Artifacts tmp/grind/func_80045294/s47/{h1.model.json,cand.model.json,ra_inverse_h1.txt}.
- result: REACHABLE in the model at minimal size 1 atom, 5 distinct vectors: [refs_up] pseudo 72 (= a0) refs 3->4/5/6/7 and [live_shrink] pseudo 72 livelen 23->15. All 24 preference atoms reported FORECLOSED ($s2/$s4/$s5 are callee-saved and can never appear as hard regs in pre-RA RTL, so set_preference can never record them — reachable only by a forbidden register-asm pin). The two-chassis model diff isolates the whole 2-insn residual to one number: pseudo 72 has nrefs_flow=4 on the candidate chassis and 3 on H1, with livelen_flow=24 and calls_crossed=2 IDENTICAL on both.
- verdict: CONFIRMED

## [s47] The refs_up atom (pseudo 72: 3 -> 4) is spellable in honest C by giving a0 a fourth reference in a basic block where cse.c cannot substitute it away — specifically the second loop's index shift.
- mechanism: cse.c's operand substitution is basic-block scoped (the s7 wall). Under H1 the first block's 'i = a0' makes reg 75 equivalent to reg 72, so the ashift operand is rewritten 72 -> 75 and a0 loses a reference; the second loop's shift sits in a different block, so spelling it on a0 should restore the count.
- probe: H1 first block + second-loop shift spelled 'v1 = a0 << 4;' instead of 'v1 = i << 4;', applied to src/text1a_c.c, sandbox --disable all. Artifact tmp/grind/func_80045294/s47/vA_sandbox.txt; form banked memory/grind/func_80045294/rejected/h1-second-loop-shift-on-a0.c.
- result: score = 11, 83/83 — unchanged from plain H1. The second loop's own block re-establishes the equivalence ('i = a0;' immediately dominates that shift too), so cse substitutes a second time and reg_n_refs(72) stays 3. Furthermore the ORIGINAL contains exactly four a0 references (def addu $s2,$a0 @8004529C; uses @800452B4, @800452B8, @8004532C, after which $s2 is redefined as the walking pointer @80045344) and all four lie inside the equivalence region H1 opens — no fifth site exists to spend. The companion live_shrink atom is likewise unreachable: a0's last reference in the original is after both calls.
- verdict: KILLED

## [s47] TYPED FORECLOSURE — the two conditions target's bytes require are mutually exclusive in pure C for this function's reference set.
- mechanism: (A) target's sched2 order requires 'i = a0' before 'v1 = a0 << 4' (unique intent of all 13 exhaustively enumerated sched atoms); (B) target's allocation requires reg_n_refs(a0) == 4 (the single-atom RA inverse, live length and calls-crossed already identical). (A) is precisely the condition that lets cse.c convert one of a0's four references into a reference to i, forcing (B) to 3. Both models are exact, both solution sets enumerated, and both remaining lever classes foreclosed — preferences by callee-save unreachability in pre-RA RTL, a fifth a0 reference by the target's own instruction census.
- probe: Composition of the two exhaustive solver searches plus the direct H1 + second-loop-shift-on-a0 measurement, all this session.
- result: Every one of the 34 banked rejected forms is an instance of taking (A) without (B) (the score-11/12 H1 basin) or (B) without (A) (the score-2 candidate basin); the modelled atom space contains no third basin. This supersedes the ledger's seven-wall prose (s34/s43) with a proof over an enumerated lever space, and it states exactly what any future lever must do: supply a fourth reference to a0, or shorten its live range, under source order i-before-v1.
- verdict: CONFIRMED

## s48 (escalation modality — owner ruling 10 of the 2026-08-30 batch)

- [s48] **H-s48-1 — KILLED.** *Statement:* the pass-1 (sched1) block-0 input set admits a
  perturbation that changes which insns reach sched2, invalidating s47's claim that the
  pass-2 960-atom enumeration is exhaustive. *Probe:* exhaustive single-atom enumeration
  over sched_solver's pass-1 model for func_80045294 block 0
  (`tmp/grind/func_80045294/s48/pass1_probe.py`, 192 atoms) plus a goal-directed re-run
  (`pass1_goal.py`). *Result:* pass 1 is genuinely perturbable (28 distinct non-baseline
  orders), so the conditional was real — but exactly one of those orders flips the pass-2
  LUIDs of the two residual insns, and all 11 atoms reaching it spell the single C intent
  **i-before-v1** (either "the shift consumes i" or "the shift statement moves after
  `i = a0`"). *Verdict:* the pass-1 axis is a second MECHANISM for the intent s47 already
  named, not a new axis. The scheduling foreclosure is now unconditional across both passes.

- [s48] **H-s48-2 — KILLED (new axis).** *Statement:* the residual is closable by making
  `i` not be the value whose cse quantity displaces a0's — specifically by using a DISTINCT
  counter variable for the second loop, so that cse.c's `make_regs_eqv` prong (2)
  (`uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`, cse.c:856-857) is
  false and `qty_first_reg` stays reg 72 (a0), leaving the ashift operand as a0 on the
  H1 chassis where sched2 is already target-exact. *Mechanism:* read directly out of
  `tools/gcc-2.7.2/cse.c:826-882`, not inferred — this is the pass-source answer the s47
  frontier asked for. *Probe:* two spelled forms measured with `sandbox --disable all`:
  H1 + distinct `j` and base chassis + distinct `j`. *Result:* both **score 37,
  build_insns 80** against target 83. Shortening `i`'s live range re-rotates the whole
  callee-save allocation AND lets loop 2's `lw D_800A33AC` hoist out of the loop, deleting
  3 instructions the target keeps. *Verdict:* KILLED, and it kills the FAMILY, not just the
  spelling: the 83-insn target shape is contingent on the single reused counter, and the
  single reused counter is precisely what makes cse prong (2) true. Requirements (A) the
  shift's operand stays a0 and (B) the instruction count stays 83 are mutually exclusive
  at the source level.

- [s48] **Frontier retired.** All three s47 frontier items are now measured dead: item 1
  (fourth reference to a0 / cse operand choice) and item 3 (a lowering where `i` is not a
  value-copy of a0) by H-s48-2 plus the cse.c source reading; item 2 (the pass-1
  conditional) by H-s48-1. No successor frontier item is proposed, because the two
  foreclosures are now at the COMPILER-SOURCE level (cse.c:844-857 for the RA leg,
  the pass-1+pass-2 exhaustive atom enumerations for the scheduling leg) rather than at
  the "we tried N spellings" level. Future sessions must NOT re-open this by spelling
  another variant of i/v1 declaration order or another second-counter identity.

## [s48] The pass-1 (sched1) block-0 input set admits a perturbation that changes which insns reach sched2, invalidating s47's claim that the pass-2 960-atom enumeration is exhaustive.
- mechanism: sched.c reassigns INSN_LUID at the start of each scheduling pass, so the pre-reload emission order produced by sched1 sets the pass-2 LUIDs of every real insn. The 960-atom pass-2 proof is exhaustive only GIVEN the insn set/order reload hands sched2; a different sched1 output is a different pass-2 input.
- probe: Exhaustive single-atom enumeration over sched_solver's pass-1 model for func_80045294 block 0 (tmp/grind/func_80045294/s48/pass1_probe.py, 192 atoms, 8 insns, baseline simulator order-exact), then a goal-directed re-run listing every atom that reaches the one order which flips the pass-2 LUIDs of uid 14 (sll $v1,$s2,4) and uid 22 (addu $s0,$s2,$zero) (pass1_goal.py).
- result: Pass 1 is genuinely perturbable: 28 distinct non-baseline orders, so the conditional was real rather than a formality. But exactly ONE of those orders is the LUID-flipping one, and all 11 atoms reaching it are either add_dep 14 <- 22 (the shift consumes i) or a luid/luid_move atom placing the shift statement after `i = a0` in the source. All 11 spell the single C intent i-before-v1 that the pass-2 search already named.
- verdict: KILLED

## [s48] The residual is closable by using a DISTINCT counter variable for the second loop so that cse.c's make_regs_eqv prong (2) is false, leaving reg 72 (a0) as qty_first_reg and therefore keeping a0 as the ashift operand on the H1 chassis where sched2 is already target-exact.
- mechanism: Read directly from tools/gcc-2.7.2/cse.c:826-882. make_regs_eqv makes the copy destination (i, reg 75) displace the source (a0, reg 72) as qty_first_reg — and hence as canon_reg's substitution target — iff (1) the destination outlives the current cse EBB AND (2) uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]] (cse.c:851-857). Prong (1) is unfalsifiable in C because i is a loop induction variable; prong (2) is falsified with zero extra instructions by ending i's live range at the close of loop 1.
- probe: Two forms spelled and measured with `sandbox func_80045294 --disable all`: H1 chassis (i = a0 before v1 = a0 << 4) plus a distinct second-loop counter j, and the current chassis plus the same distinct j as a control. Sandbox object disassembled to attribute the delta.
- result: Both forms score 37 with build_insns 80 against target_insns 83. The shortened live range re-rotates the entire callee-save allocation (a0 lands in $s1, the counter in $s2) and lets loop 2's `lw D_800A33AC` hoist out of the loop, deleting 3 instructions the target keeps inside it at 0x8004538C. Identical score on both chassis isolates the damage to the counter split itself, not to the i/v1 declaration order.
- verdict: KILLED

## [s48] A pure-C form exists that keeps the ashift operand as a0 (satisfying the RA leg) while keeping the 83-instruction target shape (satisfying the count leg).
- mechanism: The two legs are coupled through the same source fact. cse prong (2) is true precisely because one variable serves as the counter of both loops, and that same reuse is what forces GCC to keep loop 2's global reload inside the loop and to keep the extra copies that make up the 83-instruction shape.
- probe: Composite of the two probes above: cse.c source reading to enumerate the falsification routes for prong (2), then measuring the only instruction-free route.
- result: Every edit that falsifies prong (2) without emitting an instruction also shortens i's live range, and shortening i's live range is exactly what deletes the 3 instructions. The requirements are mutually exclusive at the source level.
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A. Ground: ra_solver inverse names the goal REACHABLE at one atom (refs_up 72: 3->4); the sanctioned duplicated-statement-into-arms ref-lift (which closed func_800324D0 on 2026-09-01) was never tried — the 'no fifth a0 reference site' theorem assumes a reference must occupy a target instruction slot, which byte-free cross-jump-remerged duplication defeats by design. Named probe: under the H1 (i-before-v1) chassis, duplicate a real a0-referencing statement per the family (2026-08-06 loop-tail extension), verify nrefs_flow(72)==4 via extract.py BEFORE spending a sandbox run.

## [s49] The owner's 2026-09-01 Ruling A named probe: under the H1 (i-before-v1) chassis, a [[duplicated-statement-into-arms]] ref-lift supplies the fourth reference to pseudo 72 (a0) byte-free, closing the RA leg that ra_solver named REACHABLE at one atom.
- mechanism: jump2's cross-jump runs AFTER global allocation, so a statement written into two arms is counted twice by flow.c's reg_n_refs (allocno-priority lift) and then re-merged to identical bytes. This is the mechanism that closed motion_SetMotion (2026-07-01) and func_800324D0 (2026-09-01). The 2026-08-06 control-transfer-tail extension permits the duplicated tail to end in a branch.
- probe: `i = a0;` written into BOTH arms of `if (sum != 0)` on the H1 chassis. Reference count verified with `tools/ra_solver/extract.py func_80045294 text1a_c` BEFORE the sandbox run as the ruling directs (tmp/grind/func_80045294/s49/v2.model.json). Form banked memory/grind/func_80045294/rejected/dup-i-eq-a0-into-sum-arms.c.
- result: The ref-lift is REAL — nrefs_flow(72) rises 3 -> 7, inside ra_solver's goal band {4,5,6,7}. But the sandbox measures **score 27 at build_insns 84** against target 83: the duplicate SURVIVES into the final bytes. Cross-jump cannot merge these two copies because they are not tails of two paths converging on a common continuation (the sum!=0 arm continues into the loop-2 guard, the sum==0 arm continues into the epilogue), so prerequisite 2 of the family (byte-neutrality) fails outright; the else-arm copy is additionally a dead store, which fails prerequisite 1 and routes the construct to [[dead-store-fake-exception]] instead. Structurally, this function has NO in-family duplication site at all: the only convergence points are the epilogue (contains no a0 reference and cannot acquire one byte-free) and the loop-2 body tail (a0 is dead there — $s2 has been reused as the walking pointer at 0x80045344); every other duplication would duplicate the two CALLS, which the family's Non-extension clause excludes explicitly.
- verdict: KILLED

## [s49] The `refs_up 72: 3->4` atom is reachable byte-cheaply by spelling the SECOND loop's guard on the parameter (`if (a0 < D_800A33AC) { i = a0; ... }`), because that a0 read is not dominated by a live `i = a0` copy and therefore escapes cse.c's substitution.
- mechanism: cse's operand substitution is basic-block scoped (the s7 wall). i is clobbered by loop 1, so at the second loop's guard the 75-equiv-72 quantity from block 0 is dead; a fresh a0 read there is a genuine reference to pseudo 72. Raising reg_n_refs(72) raises a0's allocno priority in global.c, which ra_solver's inverse named as the single atom that restores target's allocation on the H1 chassis.
- probe: form built, extract.py run (tmp/grind/func_80045294/s49/v1.model.json), sandbox run, sandbox object disassembled (tmp/grind/func_80045294/s49/v1_disasm.txt). Form banked rejected/h1-second-loop-guard-on-a0.c. Control with `i = a0;` restored in front of the guard measured by extract.py only, per the ruling's "verify nrefs before spending a sandbox run" instruction (v1d.model.json, rejected/h1-i-then-guard-on-a0.c).
- result: **CONFIRMED for the RA leg, and it corrects the s48 record.** nrefs_flow(72) = 4 exactly, and block 0 is emitted as `sw $s0,0x10($sp) ; move $s0,$s2 ; sll $v1,$s0,0x4` — target's prologue-cluster ORDER *and* target's REGISTER ALLOCATION (a0 -> $s2, i -> $s0) simultaneously, for the first time in 49 sessions. The score-11 H1 callee-save rotation is gone; the form scores **5 at 84 instructions**. s48's claim that the refs-up route was dead is wrong as stated: refs_up is reachable and does fix the allocation. Two residuals survive it. (1) block 0's shift still reads $s0 (i) where target reads $s2 (a0): reg_n_refs is a global.c input and does not touch cse.c's canonical-register choice. (2) hoisting `i = a0` into the guard's arm costs one instruction (84 vs 83); the control form (guard on a0 with `i = a0` left in front of it) is back to 83 instructions but drops nrefs_flow(72) to 3, because cse substitutes that a0 too.
- verdict: CONFIRMED (RA leg) / KILLED (as a closing form)

## [s49] With the RA leg closed by the guard-on-a0 ref-lift, the remaining block-0 residual (ashift operand $s0 vs $s2) is closable by making a0's last reference postdate i's last reference, falsifying prong (2) of cse.c's make_regs_eqv.
- mechanism: `tools/gcc-2.7.2/cse.c:842-857` — reg 75 (i) displaces reg 72 (a0) as `qty_first_reg`, and hence as canon_reg's substitution target, iff `uid_cuid[regno_last_uid[75]] > uid_cuid[regno_last_uid[72]]`. i's last reference is inside loop 2; a0's last reference is the loop-2 guard, which precedes it. Counting loop 2 with a distinct variable j pulls i's last reference back into loop 1, inverting the inequality — the one combination s48 never built (s48 measured the distinct counter WITHOUT the guard-on-a0 ref-lift).
- probe: H1 chassis + guard on a0 + distinct loop-2 counter j; extract.py (v3.model.json, nrefs_flow(72) = 6) then sandbox. Form banked rejected/h1-guard-on-a0-distinct-j.c.
- result: **score 38 at build_insns 81** against target 83. The s48 deletion reproduces unchanged and is independent of the guard spelling: with a distinct loop-2 counter the `lw %gp_rel(D_800A33AC)` that the target keeps INSIDE loop 2 at 0x8004538C hoists out of the loop and three instructions disappear. Keeping a0 canonical for the shift requires shortening i, and shortening i deletes instructions the target has.
- verdict: KILLED

## [s49] Disposition summary — the residual is now attributed to ONE compiler decision, and it is not a lever
The 2-instruction residual decomposes into exactly two source-level requirements that are provably
incompatible at the C level:
  (A) sched2 prologue-cluster order requires source order `i = a0` BEFORE `v1 = a0 << 4`
      (s47/s48 exhaustive atom enumerations over BOTH scheduler passes: every one of the
      13 pass-2 and 11 pass-1 goal-reaching atoms spells that single intent and no other).
  (B) block 0's `sll` must read a0's register, i.e. cse.c must keep reg 72 as `qty_first_reg`
      of the 72/75 quantity (cse.c:842-857).
Under (A) the copy `i = a0` establishes the quantity before the shift, and reg 75 wins
`qty_first_reg` because its last reference (loop 2) postdates a0's. The three routes to invert
that are now each measured dead: extend a0's last reference past loop 2 (no byte-free site
exists — s49 duplication probe), shorten i's last reference (deletes 3 instructions — s48 and
s49 v3), or lift a0's reference count (s49 v1: fixes the ALLOCATION, does not touch the
canonical-register choice, and costs one instruction). Note the two legs are now decoupled:
the register ALLOCATION is solved (v1), and the only thing left is which of two registers
holding the identical value the shift reads.

## [s49] Owner Ruling A named probe: under the H1 (i-before-v1) chassis, a duplicated-statement-into-arms ref-lift supplies the fourth reference to pseudo 72 (a0) byte-free, closing the RA leg ra_solver named REACHABLE at one atom.
- mechanism: jump2's cross-jump runs AFTER global allocation, so a statement written into two arms is counted twice by flow.c's reg_n_refs (allocno-priority lift) and then re-merged to identical bytes -- the mechanism that closed motion_SetMotion (2026-07-01) and func_800324D0 (2026-09-01); the 2026-08-06 control-transfer-tail extension permits a duplicated tail ending in a branch.
- probe: `i = a0;` written into BOTH arms of `if (sum != 0)` on the H1 chassis; nrefs verified with tools/ra_solver/extract.py BEFORE the sandbox run as the ruling directs (tmp/grind/func_80045294/s49/v2.model.json), then `sandbox func_80045294 --disable all`. Form banked memory/grind/func_80045294/rejected/dup-i-eq-a0-into-sum-arms.c.
- result: The ref-lift is REAL -- nrefs_flow(72) rises 3 -> 7, inside ra_solver's goal band {4,5,6,7}. But the sandbox measures score 27 at build_insns 84 against target 83: the duplicate SURVIVES into the final bytes. Cross-jump cannot merge the copies because they are not tails of two paths converging on a common continuation (the sum!=0 arm continues into the loop-2 guard, the sum==0 arm into the epilogue), so prerequisite 2 of the family (byte-neutrality) fails outright; the else-arm copy is additionally a dead store, failing prerequisite 1 and routing the shape to dead-store-fake-exception. Structurally this function has NO in-family duplication site: its only convergence points are the epilogue (no a0 reference, cannot acquire one byte-free) and the loop-2 body tail (a0 dead -- $s2 reused as the walking pointer at 0x80045344); every larger duplication would duplicate the two CALLS, excluded by name in the family's Non-extension clause.
- verdict: KILLED

## [s49] The ra_solver atom refs_up 72: 3->4 is reachable by spelling the SECOND loop's guard on the parameter (`if (a0 < D_800A33AC) { i = a0; ... }`), because that a0 read is not dominated by a live `i = a0` copy and so escapes cse.c's basic-block-scoped substitution.
- mechanism: i is clobbered by loop 1, so at the second loop's guard the 75-equiv-72 quantity from block 0 is dead and a fresh a0 read there is a genuine reference to pseudo 72. Raising reg_n_refs(72) raises a0's allocno priority in global.c -- the single atom ra_solver's inverse named as restoring target's allocation on the H1 chassis.
- probe: Form built and applied to src/text1a_c.c; extract.py (tmp/grind/func_80045294/s49/v1.model.json); `sandbox func_80045294 --disable all`; sandbox object disassembled (tmp/grind/func_80045294/s49/v1_disasm.txt). Control with `i = a0;` restored in front of the guard measured by extract.py only, per the ruling's verify-before-sandbox instruction (v1d.model.json). Forms banked rejected/h1-second-loop-guard-on-a0.c and rejected/h1-i-then-guard-on-a0.c.
- result: CONFIRMED for the RA leg and it CORRECTS the s48 record. nrefs_flow(72) = 4 exactly, and block 0 is emitted as `sw $s0,0x10($sp) ; move $s0,$s2 ; sll $v1,$s0,0x4` -- target's prologue-cluster ORDER and target's REGISTER ALLOCATION (a0 -> $s2, i -> $s0) simultaneously, for the first time in 49 sessions; the score-11 H1 callee-save rotation is gone. Form scores 5 at 84 instructions. s48's claim that the refs-up route was dead is wrong as stated. Two residuals survive: (1) block 0's shift still reads $s0 (i) where target reads $s2 (a0) -- reg_n_refs is a global.c input and does not touch cse.c's canonical-register choice; (2) hoisting `i = a0` into the arm costs one instruction (84 vs 83), and the control that recovers 83 instructions drops nrefs_flow(72) back to 3 because cse substitutes the guard's a0 too.
- verdict: CONFIRMED

## [s49] With the RA leg closed by the guard-on-a0 ref-lift, the remaining block-0 residual (ashift operand $s0 vs $s2) is closable by making a0's last reference postdate i's last reference, falsifying prong (2) of cse.c's make_regs_eqv.
- mechanism: tools/gcc-2.7.2/cse.c:842-857 -- reg 75 (i) displaces reg 72 (a0) as qty_first_reg, and hence as canon_reg's substitution target, iff uid_cuid[regno_last_uid[75]] > uid_cuid[regno_last_uid[72]]. i's last reference is inside loop 2, a0's is the loop-2 guard which precedes it. Counting loop 2 with a distinct variable j pulls i's last reference back into loop 1 and inverts the inequality -- the one combination s48 never built (s48 measured the distinct counter WITHOUT the guard-on-a0 ref-lift).
- probe: H1 chassis + guard on a0 + distinct loop-2 counter j; extract.py (v3.model.json, nrefs_flow(72) = 6) then `sandbox func_80045294 --disable all`. Form banked rejected/h1-guard-on-a0-distinct-j.c.
- result: score 38 at build_insns 81 against target 83. The s48 deletion reproduces unchanged and is independent of the guard spelling: with a distinct loop-2 counter the `lw %gp_rel(D_800A33AC)` the target keeps INSIDE loop 2 at 0x8004538C hoists out of the loop and three instructions disappear. Keeping a0 canonical for the shift requires shortening i, and shortening i deletes instructions the target has.
- verdict: KILLED

## [s50] The block-0 rotation is a sched.c LUID tiebreak coupled to a cse ref-count collapse -- and the coupling breaks on the allocation side

### H-s50-1 (KILLED, class) -- declaration order alone cannot satisfy both halves
**Statement.** With loop 2 spelled `i = a0; v1 = i << 4;` (candidate.c's
spelling), no declaration order of {sum, i, v1, s4, count, s5} in block 0
reaches below score 11 whenever i is declared before v1.

**Mechanism.** sched2 schedules block 0 backwards; all block-0 insns are
INSN_PRIORITY 1 (priority() at sched.c:1497 measures distance from the block
START as `priority(pred) + insn_cost(pred) - 1`, which is flat over latency-1
chains) and all ready candidates at the deciding tie are class 3 against a `sw`
last_scheduled_insn, so rank_for_schedule falls through to the LUID compare at
sched.c:2461-2463. The target's order needs the shift's LUID above `i = a0`'s,
i.e. i declared first. But `i = a0` then makes i the cse quantity's
qty_first_reg (cse.c:842-857, because i outlives the EBB and outlives a0), so
canon_reg rewrites `a0 << 4` to `i << 4` and a0's reg_n_refs falls 4 -> 3.
global.c allocno_compare's `floor_log2(n_refs)*n_refs/live_length` then collapses
a0's priority 8 -> 3 and rotates the callee-saves.

**Probe.** All 30 legal orderings generated and measured
(tmp/grind/func_80045294/s50/v/P00..P29): P00-P05 (sum before i) score=11,
P06-P29 (i first) score=14, all at build_insns=83. Plus dumps_A/.greg read for
the allocation order and the rewritten `(ashift (reg 16 s0) 4)`.

**Result.** KILLED. The declaration-order axis is closed by exhaustive
measurement, and both halves of the coupling are now named with file:line
predicates rather than described as "an RA plateau".

kill_scope: class
predicate_cite: cse.c:855
measured_on: candidate.c chassis at HEAD 2026-09-03 (score 2, 83 insns), zero
FAKE constructs present, loop 2 spelled `i = a0; v1 = i << 4;`.

### H-s50-2 (CONFIRMED) -- a0's fourth reference is recoverable in the loop-2 preheader's fresh cse EBB
**Statement.** Respelling the loop-2 preheader as `v1 = a0 << 4; i = a0;`
restores a0's fourth reg_n_refs and therefore the target's complete callee-save
allocation, which makes the i-first block-0 order viable for the first time.

**Mechanism.** cse processes the loop-2 preheader as a separate extended basic
block (`;; Processing block from 70 to 109` in the .cse dump). i has been
clobbered by loop 1, so no a0/i equivalence exists at the top of that block; an
`a0 << 4` placed before that block's `i = a0` is not canonicalized to i and
counts as a genuine a0 reference. a0 returns to 4 refs, allocno priority returns
to 8, and a0 is allocated $s2 again.

**Probe.** Form I_a0shift_before_i (i-first block 0 + that loop-2 respelling)
measured score=2, build_insns=83; objdump diff against asm/funcs/func_80045294.s
shows the ONLY differences are the two `sll` operands at idx 9 and idx 41 --
prologue ordering, the full callee-save allocation and every stack slot match.
Control I_shiftfirst (i-first, unrespelled loop 2) = 11; control
B_a0shift_before_i (v1-first, respelled loop 2) = 3.

**Result.** CONFIRMED. Banked as
memory/grind/func_80045294/rejected/s50-i-first-a0ref-in-loop2-preheader.c as an
equal-floor ALTERNATE chassis, not a rejection. candidate.c (operands right,
schedule wrong) and this form (schedule right, operands wrong) are exactly
complementary at score 2.

### H-s50-3 (KILLED, instance) -- buying a0's fourth reference from the loop-2 guard costs an instruction
**Statement.** Spelling the loop-2 guard `if (a0 < D_800A33AC)` ahead of
`i = a0`, on either block-0 chassis, raises build_insns to 84.

**Mechanism.** The target emits `move $s0,$s2` then `slt $v0,$s0,$v0`; testing
a0 instead emits the `slt` on $s2 and still requires the `move`, so the compare
cannot double as both the a0 reference and the target's `slt` operand.

**Probe.** I_a0shift_in_guard score=5 insns=84; B_a0shift_in_guard score=27
insns=84.

**Result.** KILLED for these two spellings.

kill_scope: instance
measured_on: HEAD 2026-09-03 chassis, i-first and v1-first block-0 heads, zero
FAKE constructs present.

## [s50] With loop 2 spelled `i = a0; v1 = i << 4;`, no declaration order of {sum, i, v1, s4, count, s5} in block 0 reaches below score 11 whenever i is declared before v1.
- mechanism: sched2 schedules block 0 backwards. Every block-0 insn is INSN_PRIORITY 1, because priority() (sched.c:1434-1522) accumulates over LOG_LINKS as `priority(pred) + insn_cost(pred) - 1`, i.e. distance from the block START, which is flat over latency-1 chains; only the two insns fed by an `lw` reach 2, and neither the `a0 << 4` shift (insn 14) nor `i = a0` (insn 22) is one of them. Both are also class 3 against the `sw` last_scheduled_insn, so rank_for_schedule falls through to the LUID compare at sched.c:2461-2463 (descending LUID, head picked, highest LUID emitted last). The target's `sw $s0 / move $s0,$s2 / sll` order therefore requires the shift's LUID to exceed `i = a0`'s, i.e. i declared first. But `i = a0` then makes i the cse quantity's qty_first_reg (cse.c:842-857: i outlives the EBB `Processing block from 2 to 36` and outlives a0, whose last reference is the loop-2 preheader's own `i = a0`), so canon_reg rewrites `a0 << 4` into `i << 4` and a0's reg_n_refs falls 4 -> 3. global.c allocno_compare's `floor_log2(n_refs)*n_refs/live_length` then collapses a0's priority from floor_log2(4)*4 = 8 to floor_log2(3)*3 = 3, dropping a0 from 9th to last in the allocation order and rotating the callee-saves a0 $s2->$s5, s4 $s4->$s2, s5 $s5->$s4.
- probe: Generated and measured all 30 legal orderings of {sum, count} interleaved into the fixed chain i < v1 < s4 < s5 (tmp/grind/func_80045294/s50/v/P00..P29, one sandbox run each). Read tmp/grind/func_80045294/s50/dumps_base/text1a_c.sched2 for the T-9 ready-list tie and the flat priorities, dumps_A/text1a_c.greg for the allocation order and the rewritten `(ashift (reg 16 s0) (const_int 4))`, and tools/gcc-2.7.2/{sched.c,cse.c,global.c} for the three predicates.
- result: P00-P05 (sum declared before i) score=11, P06-P29 (i declared first) score=14, all at build_insns=83. Baseline candidate.c = score 2 / 83. The i-first order does produce the target's prologue structure insn-for-insn (`addiu sp,-48 / sw s5,36 / move s5,a0 / sw s3,28 / move s3,a1 / sw s1,20 / move s1,zero / sw s0,16 / move s0,s5 / sll v1,s0,4 / sw ra,40 ...`), so the scheduling half of the target is reachable; the entire cost is the callee-save rotation caused by a0 losing its shift reference. The 49-session 'wall' is therefore a genuine two-sided coupling, now named with file:line predicates on both sides rather than described as an RA plateau: the schedule wants i-first, the allocation wants v1-first.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis (candidate.c pasted over src/text1a_c.c:1445, sandbox score 2 / 83 insns), zero FAKE constructs present, loop 2 spelled `i = a0; v1 = i << 4;`
- predicate_cite: cse.c:855

## [s50] Respelling the loop-2 preheader as `v1 = a0 << 4; i = a0;` restores a0's fourth reg_n_refs and with it the target's complete callee-save allocation, making the i-first block-0 order reach score 2.
- mechanism: cse processes the loop-2 preheader as a separate extended basic block (`;; Processing block from 70 to 109` in the .cse dump). i has been clobbered by loop 1, so no a0/i equivalence is live at the top of that block, and an `a0 << 4` placed ahead of that block's `i = a0` is not canonicalized to i. It counts as a genuine a0 reference, returning a0 to 4 refs and its global.c allocno priority to 8, so a0 is allocated $s2 again while block 0 keeps the i-first LUID order that the scheduler needs.
- probe: Built the 2x3 matrix of {block-0 head: v1-first / i-first} x {loop-2 preheader: `i=a0; v1=i<<4` / `v1=a0<<4; i=a0` / guard-on-a0} and measured each with sandbox --disable all, then objdump-diffed the winner against asm/funcs/func_80045294.s.
- result: I_a0shift_before_i measures score=2, build_insns=83. Its residual is ONLY the two `sll` operands: idx 9 build `sll $v1,$s0,4` vs target `sll $v1,$s2,4`, and idx 41 build `sll $v1,$s2,4` vs target `sll $v1,$s0,4`. The prologue sw/move interleave, the `sw $s0 / move $s0,$s2 / sll` ordering that candidate.c gets WRONG, the whole callee-save allocation (a0->$s2, a1->$s3, sum->$s1, i->$s0, s4->$s4, s5->$s5) and every stack slot are byte-exact. Controls isolate the effect: I_shiftfirst (i-first, unrespelled loop 2) = 11; B_a0shift_before_i (v1-first, respelled loop 2) = 3; B_shiftfirst = 2 (identical to candidate.c). This is the first time the floor of 2 has been reached from the i-first side at all, and candidate.c and this form are exactly complementary -- candidate.c has both sll operands right and the block-0 schedule wrong, this form has the schedule right and both operands wrong. Banked as memory/grind/func_80045294/rejected/s50-i-first-a0ref-in-loop2-preheader.c (equal-floor ALTERNATE chassis, not a rejection).
- verdict: CONFIRMED

## [s50] Buying a0's fourth reference from the loop-2 guard, by spelling it `if (a0 < D_800A33AC)` ahead of `i = a0`, raises build_insns to 84 on both block-0 chassis.
- mechanism: The target emits `move $s0,$s2` and then `slt $v0,$s0,$v0`; testing a0 instead emits the `slt` on $s2 and still requires the `move`, so the compare cannot serve as both the a0 reference and the target's `slt` operand.
- probe: Measured I_a0shift_in_guard and B_a0shift_in_guard with sandbox --disable all.
- result: I_a0shift_in_guard score=5 insns=84; B_a0shift_in_guard score=27 insns=84. Both exceed the target's 83 instructions, so the guard is not a free carrier for a0's fourth reference.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, both the i-first and v1-first block-0 heads, zero FAKE constructs present

## [s51] The block-0 emission order of `sll` vs `move $s0,$s2` equals their source order at BOTH scheduler passes, and the only tie-breakers above LUID are unreachable for this insn pair.
- mechanism: sched.c's `priority()` (tools/gcc-2.7.2/sched.c:1497) accumulates over LOG_LINKS (PREDECESSORS) as `priority(pred) + insn_cost(pred) - 1`, so a chain of latency-1 insns is flat at 1; a priority of 2 requires a predecessor whose `insn_cost` is 2, i.e. a LOAD feeding the insn. Both `v1 = a0 << 4` and `i = a0` have exactly one predecessor, the parameter copy `(set (reg 72) (reg:SI 4 a0))`, at priority 1, so both are priority 1. rank_for_schedule (sched.c:2408-2464) then compares class against `last_scheduled_insn`; class < 3 requires the candidate to be a LOG_LINKS predecessor of last_scheduled_insn with cost > 1 (again a load), and at the deciding step last_scheduled_insn is a prologue `sw`, so both are class 3. The comparison therefore falls through to `INSN_LUID (tmp) - INSN_LUID (tmp2)` at sched.c:2461-2463 and the pair keeps its RTL order.
- probe: Read the sched1 AND sched2 block-0 traces for the candidate.c chassis in one build (tmp/grind/func_80045294/s51/dumps_cand/text1a_c.sched and .sched2, produced with the instrumented cc1 at tools/gcc-2.7.2/cc1 with BB2_PRIO_DEBUG/BB2_RANK_DEBUG set; raw stderr in dumps_cand/sched_debug.txt). sched1 block 0 "from 4 to 32": every listed insn is priority 1 except 28 and 31 (both fed by an `lw`, priority 2); at T-6 the ready list is `22 (1) 14 (1) 12 (1)` and the sort leaves `22 14 12`, so 22 (`i = a0`) is picked and 14 (the shift) is emitted BEFORE it. sched2 block 0 "from 197 to 32" reproduces this exactly at T-9 (`ready list at T-9: 22 (1) 14 (1) 12 (1) 6 (1), now 22 14 12 6`) with last_scheduled_insn = 199 (`sw $ra`).
- result: CONFIRMED, and it EXTENDS the s50 result from sched2 to sched1. Previous sessions established the sched2 tie only; the open question that left was whether sched1 could reorder the pair so that sched2's freshly assigned LUIDs would favour the shift. It cannot: sched1 has the identical flat-priority/class-3 structure and preserves the source order, so the RTL order presented to sched2 is still the source order. Consequence: `sll` is emitted after `move $s0,$s2` if and only if the SOURCE writes `i = a0` before `v1 = a0 << 4` -- there is no scheduler-side lever, and raising the shift's priority would require its operand to come from a load, which `a0 << 4` cannot have.
- verdict: CONFIRMED

## [s51] A DEAD C-level reference to a0 placed after i's last reference raises uid_cuid[regno_last_uid[a0]] and so falsifies cse.c:855's clause (2), keeping a0 as qty_first_reg for block 0's shift on the i-first chassis.
- mechanism: cse.c:842-857 `make_regs_eqv` makes the copy destination i the quantity's canonical register only when `uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]]`. i's last reference is loop 2; a0's is the loop-2 preheader copy. A dead statement referencing a0 placed AFTER loop 2 would invert the inequality without being a real instruction, which is the s50 frontier's named "untried direction" (lengthen a0's last use rather than shorten i's).
- probe: Four placements of `dead = a0;` (fresh unused local) on the i-first chassis, each measured with `sandbox func_80045294 --disable all`: (D) immediately after block 0's shift, (E) in the loop-2 preheader after both calls, (F) inside loop 2's body tail, (B) as the function's last statement. Control = the same chassis with no dead reference (tmp/grind/func_80045294/s51/C_ifirst_plain.c). Then the (B) build was dumped end-to-end with the instrumented cc1 (tmp/grind/func_80045294/s51/dumps_B/) and the function extracted from the .rtl, .jump and .cse dumps.
- result: KILLED, and mechanically attributed. All four placements measure score=11 build_insns=83, byte-identical to the control -- the dead reference has zero effect anywhere. The dumps say why: the pre-cse RTL DOES carry it (`(insn 191 188 193 (set (reg/v:SI 74) (reg/v:SI 72)))` as the function's last insn, reg 74 = the dead local, reg 72 = a0), but in the .jump dump insn 188 links straight to note 193 -- the insn is already gone. tools/gcc-2.7.2/jump.c:568-584 is the deleter: `if (! reload_completed && after_regscan)` it removes any insn whose SET_DEST is a pseudo with `regno_first_uid[dest] == INSN_UID (insn)` and `regno_last_note_uid[dest] == INSN_UID (insn)`, i.e. every set-once/never-read pseudo, and toplev runs jump_optimize with after_regscan BEFORE cse. So no dead a0 reference ever reaches the pass that would read it, and block 0's `(ashift (reg 72) 4)` is still rewritten to `(ashift (reg 76) 4)` in the .cse dump. The complement is measured too: with a LIVE destination (`i = a0;` as the tail statement, memory/grind/func_80045294/rejected/s51-deadstore-i-eq-a0-tail.c) jump.c:577 does not fire, the store survives every pass into the bytes, and the build is 84 instructions / score 27. jump.c:577 splits tail a0 references into exactly two cases -- dead destination (deleted before cse, zero effect) and live destination (costs an instruction) -- with no third case.
- verdict: KILLED
- kill_scope: class
- predicate_cite: jump.c:577
- measured_on: HEAD 2026-09-03 chassis (candidate.c 2/83 and s50-i-first-a0ref 2/83 both re-measured this session), i-first block-0 head, one FAKE-annotated variant (s51-deadstore-i-eq-a0-tail.c) and four un-annotated dead-local variants

## [s51] The block-0 emission order of `sll $v1,$s2,4` vs `move $s0,$s2` equals their source order at the FIRST scheduler pass as well as the second, so sched1 cannot pre-reorder the pair to give sched2 favourable LUIDs.
- mechanism: sched.c:1497 priority() accumulates over LOG_LINKS (predecessors) as priority(pred)+insn_cost(pred)-1, flat at 1 over latency-1 chains; both insns have the single parameter-copy predecessor, so both are priority 1. rank_for_schedule (sched.c:2429-2441) then needs a cost>1 predecessor relation to last_scheduled_insn to drop either below class 3, which a reg-reg move against a prologue `sw` cannot be, so sched.c:2461-2463's INSN_LUID compare decides and preserves RTL order.
- probe: Built the candidate.c chassis with the instrumented cc1 (tools/gcc-2.7.2/cc1) under BB2_PRIO_DEBUG + BB2_RANK_DEBUG and read block 0 of BOTH scheduler dumps: tmp/grind/func_80045294/s51/dumps_cand/text1a_c.sched (sched1, block 0 'from 4 to 32') and .sched2 (block 0 'from 197 to 32'), plus the raw stderr in dumps_cand/sched_debug.txt.
- result: sched1's deciding step is `ready list at T-6: 22 (1) 14 (1) 12 (1), now 22 14 12` -- insn 22 (`i = a0`) is picked, so the shift (insn 14) is emitted before it, identical to sched2's T-9 `now 22 14 12 6` with last_scheduled_insn = 199 (`sw $ra`). sched1 hands sched2 the source order unchanged. Emission order first->last in sched2 is 197,207,4,205,6,209,12,14,211,22,199,201,203,19,25,28,31,32 against the target's ...12,211,22,14,199..., which is exactly the 2-instruction residual. Raising the shift's priority to 2 would require a LOAD predecessor, which `a0 << 4` cannot have without adding an instruction.
- verdict: CONFIRMED

## [s51] A DEAD C-level reference to a0 placed after i's last reference raises uid_cuid[regno_last_uid[a0]] and so falsifies cse.c:855's clause (2), letting a0 stay qty_first_reg for block 0's shift on the i-first chassis.
- mechanism: cse.c:842-857 make_regs_eqv promotes the copy destination i to the quantity's canonical register only when uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]]. i's last reference is loop 2, a0's is the loop-2 preheader copy; a dead statement referencing a0 after loop 2 would invert the inequality for free. This is the s50 frontier's named untried direction (lengthen a0's last use rather than shorten i's).
- probe: Four placements of `dead = a0;` (fresh unused local) on the i-first chassis, each measured with `sandbox func_80045294 --disable all` against the no-dead-reference control tmp/grind/func_80045294/s51/C_ifirst_plain.c: after block 0's shift (D), in the loop-2 preheader after both calls (E), inside loop 2's body tail (F), and as the function's last statement (B). The (B) build was then dumped end-to-end (tmp/grind/func_80045294/s51/dumps_B/) and the function extracted from .rtl, .jump and .cse.
- result: All four placements measure score=11 build_insns=83, byte-identical to the control -- zero effect anywhere. The dumps give the cause: .rtl carries the store as `(insn 191 188 193 (set (reg/v:SI 74) (reg/v:SI 72)))`, the function's last insn (reg 74 = the dead local, reg 72 = a0); .jump has insn 188 linking straight to note 193, so it is already deleted; .cse still shows block 0's shift rewritten to `(ashift (reg/v:SI 76) 4)` (reg 76 = i). tools/gcc-2.7.2/jump.c:568-584 is the deleter -- with after_regscan set (how toplev calls jump_optimize before cse) it removes any insn whose SET_DEST is a pseudo with regno_first_uid[dest] == INSN_UID(insn) and regno_last_note_uid[dest] == INSN_UID(insn). The complement is measured too: with a LIVE destination (`i = a0;` as the tail statement) the predicate does not fire, the store survives into the bytes, and the build is 84 instructions / score 27 (rejected/s51-deadstore-i-eq-a0-tail.c). jump.c:577 partitions tail a0 references into dead-destination (deleted pre-cse, inert) and live-destination (costs an instruction), with no third case.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis (candidate.c 2/83 and rejected/s50-i-first-a0ref-in-loop2-preheader.c 2/83 both re-measured this session), i-first block-0 head; four un-annotated dead-local variants plus one FAKE-annotated live-destination variant
- predicate_cite: jump.c:577

## [s51] s48's attribution of the distinct-loop-2-counter form's three missing instructions to a hoisted `lw %gp_rel(D_800A33AC)` is correct.
- mechanism: s48 recorded that splitting the two loop counters lets loop 2's global reload become loop-invariant and hoist out of the loop, deleting three instructions and taking the build from 83 to 80.
- probe: Re-measured rejected/h1-separate-second-loop-counter.c (score=37 insns=80, reproducing s48 exactly), disassembled the sandbox object and diffed it instruction-by-instruction against asm/funcs/func_80045294.s (tmp/grind/func_80045294/s51/build_disasm.txt, cmp.py).
- result: The load is NOT hoisted -- it is still inside loop 2 in the 80-instruction build. What actually happens is an allocation collapse: with two separate counter pseudos, loop 1's counter no longer has to survive the two calls, so GCC leaves it in the incoming $a0 (`addiu a0,a0,1`), block 0's `move $s0,$s2` copy disappears entirely, a0 is copied once into $s1, the count lands in $a1, and the $s5 save/restore pair drops out (frame 0x28 instead of 0x30). The three missing instructions are the deleted block-0 copy plus the $s5 save/restore pair. The kill stands but its reason is different, and the corrected reason is load-bearing: the SINGLE shared counter is what forces one pseudo to span block 0 through loop 2, which is what forces a callee-save allocation and the `move $s0,$s2` copy the target has.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, i-first block-0 head with a distinct loop-2 counter `j`, zero FAKE constructs present

## [s52] Reusing the parameter a0 as loop 2's walking pointer fixes block 0's schedule -- CONFIRMED, floor 2 -> 1

**Statement.** With loop 2's walking pointer assigned into the parameter `a0`
instead of a separate `s32 *ptr` local, the i-first block-0 source order
(`i = a0;` before `v1 = a0 << 4;`) emits the TARGET's prologue order
`sw $s0,0x10($sp) / addu $s0,$s2,$zero / sll $v1,...,4`, dropping the residual
from 2 to 1.

**Mechanism.** a0's pseudo now stays live through loop 2 (it is the pointer that
is loaded from, stored through and incremented every iteration), which changes
block 0's allocation pressure and, with it, the sched1/sched2 ready-list state at
the copy/shift tie. s51 established that the pair's emitted order is its source
order at both scheduler passes; the reuse is what makes the i-first source order
allocatable at 83 instructions instead of the 11-diff shape it produced before.
Independent corroboration in the target: `addu $s2,$v1,$v0` at 0x80045344
overwrites a0's register with the loop-2 pointer, so the shipped code really does
carry both values in $s2.

**Probe.** `tmp/grind/func_80045294/s52/F_a0ptr_ifirst.c` measured with
`sandbox func_80045294 --disable all`: score=1, target_insns=83, build_insns=83.
Control matrix on the same HEAD: separate-ptr/v1-first = 2, separate-ptr/i-first
= 11, a0-ptr/v1-first = 2, a0-ptr/i-first = 1.

**Result.** CONFIRMED. Banked as the new `memory/grind/func_80045294/candidate.c`.

## [s52] cse.c:855 crowns loop 2's counter over a0 whenever the block-0 copy precedes the shift -- KILLED (class)

**Statement.** In every form measured this session in which the surviving block-0
copy `i = a0` precedes the shift in RTL and `i` is the pseudo that loop 2's exit
test reads, cse's make_regs_eqv makes `i` the quantity's canonical register and
canon_reg rewrites the block-0 shift's operand from a0 to i, printing
`sll $v1,$s0,4` instead of the target's `sll $v1,$s2,4`.

**Mechanism.** tools/gcc-2.7.2/cse.c:842-857. The copy joins i to a0's quantity;
i wins qty_first_reg because its last reference (loop 2's exit test) is both past
`cse_basic_block_end` and later than a0's last reference. Confirmed insn-by-insn
on the new chassis: .rtl insn 17 is `(ashift (reg 72) 4)` and .cse insn 17 is
`(ashift (reg 75) 4)` (tmp/grind/func_80045294/s52/dumps_F/, F.rtl.fn/F.cse.fn).

**Probe.** Ten spellings measured on the new chassis. Clause-(2) attacks (make
a0's last reference postdate i's): a0's increment last in the body = 84/score 5;
exit test hoisted into a `more` temporary = 85/27; for(;;)+break = 85/29;
top-tested while with guard = 84/5; top-tested while without guard = 85/12;
s50's guard-on-a0 re-audited = 84/5. Clause-(1) attacks (shorten the copy dest's
live range): `t = a0; v1 = a0<<4; ...; i = t` = 83/2 (the shift keeps a0, but the
surviving copy moves after it -- the old rotation); `t = a0; i = t; v1 = a0<<4` =
83/11; `b = a0; i = b; v1 = b<<4` with b reused as the pointer = 83/6. Separate
loop-2 counter (which does invert clause 2) = 80/30, the s48 allocation collapse.

**Result.** KILLED. kill_scope: class -- the gate is the predicate at
cse.c:855, and every spelling that keeps the surviving copy ahead of the shift
with a counter that outlives block 0's EBB is rewritten by it. Not killed: forms
in which the shift is created after cse in block 0 (see frontier), or in which
the surviving copy's dest is not loop 2's counter without triggering the $a0
allocation collapse.

## [s52] loop.c strength reduction cannot supply block 0's shift -- KILLED (instance)

**Statement.** Leaving loop 1's offset to loop.c (writing the body as
`sum += *(s32 *)((u8 *)&D_800EED18 + (i << 4))` with no explicit offset local)
does not put an a0-reading shift into block 0: loop.c's giv initialisation is
emitted into loop 1's PREHEADER block, after block 0's guard branch, while block
0's own shift has already been canonicalised to i by cse1.

**Mechanism.** tmp/grind/func_80045294/s52/Agiv.loop.fn: loop.c emits
`(insn 192 (set (reg 109) (reg/v:SI 72)))`, `(insn 193 (set (reg 110)
(ashift (reg 109) 4)))` carrying `REG_EQUAL (mult (reg/v:SI 72) 16)` -- reg 72 is
a0, so the giv chain really is created post-cse on a0 -- but it sits between the
guard jump (insn 31) and loop 1's top label (insn 35). The target's shift is in
block 0, before the guard, because its result feeds the `D_800EED14` load.

**Probe.** A_giv (s4 indexed by a0) and B_giv_ishift (s4 indexed by i) both
measure score=11, insns=83; dumps in tmp/grind/func_80045294/s52/dumps_Agiv/.

**Result.** KILLED. kill_scope: instance -- measured on the pre-a0-pointer
chassis with zero FAKE constructs; the same experiment has not been re-run on top
of the new score-1 chassis, where block 0's allocation differs.

## [s52] Assigning loop 2's walking pointer into the parameter a0 (instead of a separate `s32 *ptr` local) makes the i-first block-0 source order `i = a0; v1 = a0 << 4;` emit the target's prologue order, lowering the honest floor from 2 to 1.
- mechanism: a0's pseudo now stays live through loop 2 (loaded from, stored through and incremented every iteration), changing block 0's allocation pressure and with it the sched1/sched2 ready-list state at the copy/shift tie. s51 established that the emitted order of the copy/shift pair is its source order at both scheduler passes, so writing `i = a0` first is what puts it first in the bytes; before this session the same i-first order was only allocatable as an 11-diff shape. The target corroborates the reuse: `addu $s2,$v1,$v0` at 0x80045344 overwrites a0's register with the loop-2 pointer, so $s2 really carries both values in the shipped code.
- probe: tmp/grind/func_80045294/s52/F_a0ptr_ifirst.c measured with `sandbox func_80045294 --disable all`; control matrix on the same HEAD: separate-ptr/v1-first = 2, separate-ptr/i-first = 11, a0-ptr/v1-first = 2, a0-ptr/i-first = 1.
- result: score=1, target_insns=83, build_insns=83. The only differing instruction is idx 9: target `sll $v1,$s2,4` vs build `sll $v1,$s0,4`. The prologue interleave, the complete callee-save allocation (a0->$s2, a1->$s3, sum->$s1, i->$s0, s4->$s4, s5->$s5, ptr->$s2, idx->$s1), the stack frame, both loops, every delay slot and the tail are byte-exact. Banked as the new candidate.c; the superseded score-2 form is banked at rejected/s52-superseded-candidate-score2-rotation.c. A second equal-floor spelling (count loaded before the shift, M_countfirst) also measures 1/83.
- verdict: CONFIRMED

## [s52] When the surviving block-0 copy `i = a0` precedes the shift in RTL and `i` is the pseudo that loop 2's exit test reads, cse's make_regs_eqv makes i the quantity's canonical register and canon_reg rewrites the block-0 shift's operand from a0 to i.
- mechanism: tools/gcc-2.7.2/cse.c:842-857. The copy joins i to a0's quantity; i wins qty_first_reg because its last reference (loop 2's exit test) is both past cse_basic_block_end and later than a0's last reference. Confirmed insn-by-insn on the new chassis: .rtl insn 17 is `(ashift (reg/v:SI 72) 4)` (a0) and .cse insn 17 is `(ashift (reg/v:SI 75) 4)` (i).
- probe: Ten spellings measured this session. Clause-(2) attacks (make a0's last reference postdate i's): a0's increment moved to the last body statement = 84/score 5; exit test hoisted into a `more` temporary = 85/27; for(;;)+break so the increment follows the test = 85/29; loop 2 as a top-tested while with the guard kept = 84/5; without the guard = 85/12; s50's guard-on-a0 re-audited on this chassis = 84/5. Clause-(1) attacks (shorten the copy dest's live range): `t = a0; v1 = a0<<4; ...; i = t` = 83/2 (shift keeps a0 but the surviving copy moves after it); `t = a0; i = t; v1 = a0<<4` = 83/11; `b = a0; i = b; v1 = b<<4` with b reused as the pointer = 83/6. A distinct loop-2 counter does invert clause (2) but reproduces the s48 allocation collapse at 80/30.
- result: Every spelling that keeps the surviving copy ahead of the shift while its dest outlives block 0's cse EBB is rewritten by the predicate; the six zero-cost attempts to buy a later a0 reference each cost an instruction (84 or 85) because loop 2's exit test is the last insn of the do/while body and nothing can be ordered after it. Left open: forms in which the block-0 shift is created after cse, and forms whose surviving copy dest is not loop 2's counter without triggering the $a0 collapse.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, both the separate-ptr and the new a0-as-pointer chassis, zero FAKE constructs present in any measured form
- predicate_cite: tools/gcc-2.7.2/cse.c:855

## [s52] Leaving loop 1's offset to loop.c strength reduction does not put an a0-reading shift into block 0, because loop.c emits the giv initialisation into loop 1's preheader block after the guard branch while block 0's own shift has already been canonicalised to i by cse1.
- mechanism: tmp/grind/func_80045294/s52/Agiv.loop.fn shows loop.c emitting `(insn 192 (set (reg 109) (reg/v:SI 72)))` and `(insn 193 (set (reg 110) (ashift (reg 109) 4)))` carrying `REG_EQUAL (mult (reg/v:SI 72) 16)` -- the chain really is created post-cse on a0 -- but between the guard jump (insn 31) and loop 1's top label (insn 35). The target's shift sits in block 0 before the guard because its result feeds the D_800EED14 load.
- probe: A_giv (s4 indexed by a0) and B_giv_ishift (s4 indexed by i), both measured with `sandbox func_80045294 --disable all`; dumps in tmp/grind/func_80045294/s52/dumps_Agiv/.
- result: Both score=11, insns=83, identical to the plain i-first control. This answers s51's frontier item 3: post-cse insn creation does exist in this function, but it lands in the wrong basic block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, pre-a0-pointer (separate `s32 *ptr`) chassis, zero FAKE constructs present

## [s53] Reusing loop 1's counter as loop 2's byte offset inverts cse.c:855 clause (2) and makes block 0's shift print a0's register while keeping the target's copy-before-shift emission order.
- mechanism: cse.c:842-857 make_regs_eqv promotes the copy destination i to qty_first_reg only when uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]]. On the a0-as-pointer chassis a0's last mention is loop 2's pointer increment; if loop 1's counter is ALSO loop 2's byte offset and its increment is written before the pointer's, i's last mention precedes a0's, the crown fails, and canon_reg leaves the block-0 shift reading reg 72 (a0). The copy still precedes the shift in RTL, so sched1/sched2's LUID tiebreak (sched.c:2461-2463) keeps the target's `sw $s0 / move $s0,$s2 / sll` order.
- probe: tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c measured with `sandbox func_80045294 --disable all`, disassembled and diffed against asm/funcs/func_80045294.s; RTL/cse dumps in tmp/grind/func_80045294/s53/dumps_A/text1a_c.{rtl,cse}.fn. Control B_reuse_i_as_idx_a0first (pointer increment written first, restoring clause (2)) = score 10.
- result: score=11, build_insns=83. Block 0's first ten instructions are byte-exact, including idx 8 `addu $s0,$s2,$zero` and idx 9 `sll $v1,$s2,4` — the first form in 53 sessions with the correct shift operand AND the correct prologue order. The .cse dump shows insn 17 keeping `(ashift (reg 72) 4)`. Banked as rejected/s53-crown-defeat-guard-operand-lost.c.
- verdict: CONFIRMED

## [s53] Under a single cse crown state, block 0's shift and the loop-1 entry guard print the same register, so any spelling whose block-0 copy precedes the shift in RTL misses one of the target's two operands.
- mechanism: cse keeps one qty_first_reg per quantity per extended basic block (tools/gcc-2.7.2/cse.c:842-857) and canon_reg rewrites every subsequent use of that quantity to it. Block 0's EBB is `;; Processing block from 2 to 36` (dumps_A/text1a_c.cse.fn), and it contains BOTH the shift (.rtl insn 17) and the loop-1 entry guard (.rtl insn 31). The target needs the shift to print a0's register ($s2) and the guard to print i's ($s0). Measured in both crown states: with the crown succeeding (candidate.c) insn 17 is rewritten 72 -> 75 and insn 31 stays 75 (shift wrong, guard right, score 1); with the crown failing (A_reuse_i_as_idx) insn 17 stays 72 and insn 31 is rewritten 75 -> 72 (shift right, guard wrong, score 11).
- probe: Ten forms measured this session with `sandbox func_80045294 --disable all`: candidate.c 1/83; A_reuse_i_as_idx 11/83; B_reuse_i_as_idx_a0first 10/83; C_sepctr 31/80; D_base_idxfirst 3/83; K0 decl-split control 1/83; K1 `a0 = a0;` statement 1/83; K2 `(a0 = a0) << 4` 1/83; K3 (A + self-assign) 11/83; N_sum_as_idx 31/80; plus G1..G4 loop-1 restructurings 1/83 each. The only device that can flip the canonical mid-EBB is a real write to reg 72 (cse's invalidate -> delete_reg_equiv promotes reg_next_eqv), and the only zero-instruction spelling of such a write, a self-assignment, is folded away by expand and never reaches RTL (K1/K2 byte-identical to K0).
- result: KILLED for the copy-before-shift geometry. The escape that remains measured-open is the shift-before-copy geometry (H_a0ptr_vfirst, 2/83), where cse processes the shift before the equivalence exists and both operands come out right; there the residual is purely the sched2 emission order.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer, both crown states, zero FAKE constructs present in any measured form (K1/K2/K3 carried an un-annotated self-assignment that expand deletes)
- predicate_cite: tools/gcc-2.7.2/cse.c:855

## [s53] A self-assignment of the parameter (`a0 = a0;`) cannot flip block 0's cse canonical register, because expand never emits the store.
- mechanism: intended path was cse's invalidate() -> delete_reg_equiv(), which promotes reg_next_eqv to qty_first_reg when the current canonical register is written; a write to reg 72 between the shift and the guard would hand the quantity back to i. GCC 2.7.2's expand_assignment produces no insn when source and destination are the same pseudo, so no pass ever sees it (unlike the s51 dead-store case, where the insn existed in .rtl and jump.c:577 deleted it).
- probe: K1_selfassign_stmt.c (statement form, block 0 declarations split so the statement can sit between the shift and the guard) and K2_selfassign_inexpr.c (`s32 v1 = (a0 = a0) << 4;`) against the decl-split control K0_declsplit_ctrl.c, each measured with `sandbox func_80045294 --disable all`. K3 applied the same device to the crown-defeated chassis.
- result: K0 1/83, K1 1/83, K2 1/83 — all three identical, residual unchanged at idx 9. K3 11/83, identical to A_reuse_i_as_idx. Banked as rejected/s53-selfassign-a0-inert.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, both the i-first (candidate.c) and the crown-defeated (form A) block-0 heads

## [s53] No loop-1 control-flow spelling makes the entry guard a post-cse insn, so restructuring loop 1 cannot separate the guard's operand from the shift's.
- mechanism: the guard that the target emits at idx 18 is created by jump.c's duplicate_loop_exit_test (called at jump.c:626), and toplev.c:2827 runs that jump_optimize with after_regscan = 1 BEFORE cse_main (toplev.c:2865). The duplicated test therefore exists as ordinary RTL when cse walks block 0 and is canonicalised with everything else.
- probe: Four loop-1 spellings on the a0-as-pointer chassis, each measured with `sandbox func_80045294 --disable all`: G1 top-tested `while` with the offset left to loop.c; G2 guarded do-while with the offset left to loop.c (s52's A_giv shape re-measured on the new chassis); G3 `for`; G4 candidate.c's explicit offset with a top-tested `while`. Disassembly of G1 and G4 compared against asm/funcs/func_80045294.s.
- result: All four score 1 / 83 insns with the identical single residual at idx 9 (`sll $v1,$s0,4` vs `sll $v1,$s2,4`). Banked as rejected/s53-giv-while-loop1.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer, i-first block-0 head, zero FAKE constructs present

## [s53] The block-0 copy's destination must itself be live across DrawSync/func_800520B8, so a loop-1-only counter cannot carry it.
- mechanism: with loop 1's counter distinct from loop 2's and not reused later, it crosses no call; local-alloc leaves it in the incoming $a0, the copy `move $s0,$s2` is never emitted, and the $s5 save/restore pair drops out with it (the s48/s51 collapse, 80 instructions).
- probe: N_sum_as_idx.c (loop-2 offset carried by the dead `sum` instead, so the loop-1 counter stays short-lived) and C_sepctr.c (plain distinct counters), both measured on this chassis.
- result: Both 31 / 80 insns; the disassembly shows block 0 emitting `sw $s0 / move $s0,$zero / sll $v1,$s2,4` with no parameter copy at idx 8. Reusing the loop-1 counter as loop 2's OFFSET (form A) is the only measured way to keep the copy and defeat the crown at the same time. Banked as rejected/s53-sum-as-idx-alloc-collapse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer, distinct loop counters, zero FAKE constructs present

## [s53] Reusing loop 1's counter as loop 2's byte offset inverts cse.c:855 clause (2) and makes block 0's shift print a0's register ($s2) while keeping the target's copy-before-shift emission order.
- mechanism: make_regs_eqv (tools/gcc-2.7.2/cse.c:842-857) crowns the copy destination i only when uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]]. On the a0-as-pointer chassis a0's last mention is loop 2's pointer increment; writing the offset increment (carried by loop 1's counter) before it puts i's last mention first, the crown fails, and canon_reg leaves the block-0 shift reading reg 72. The copy still precedes the shift in RTL, so the sched.c:2461-2463 LUID tiebreak keeps the target's sw $s0 / move $s0,$s2 / sll order.
- probe: tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c measured with sandbox func_80045294 --disable all, disassembled and diffed against asm/funcs/func_80045294.s; RTL/cse dumps at tmp/grind/func_80045294/s53/dumps_A/text1a_c.rtl.fn and .cse.fn; control B_reuse_i_as_idx_a0first (pointer increment written first) measured too.
- result: score=11, build_insns=83. Block 0's first ten instructions are byte-exact against the target, including idx 8 addu $s0,$s2,$zero and idx 9 sll $v1,$s2,4 - the first form in 53 sessions carrying the correct shift operand together with the correct prologue order. The .cse dump shows insn 17 keeping (ashift (reg 72) 4). Control B = score 10. Banked as rejected/s53-crown-defeat-guard-operand-lost.c.
- verdict: CONFIRMED

## [s53] With one cse crown state per extended basic block, block 0's shift and the loop-1 entry guard print the same register, so a spelling whose block-0 copy precedes the shift in RTL misses one of the target's two operands.
- mechanism: cse keeps one qty_first_reg per quantity per extended basic block (cse.c:842-857) and canon_reg rewrites every later use of that quantity to it. Block 0's EBB is ';; Processing block from 2 to 36' (dumps_A/text1a_c.cse.fn) and contains both the shift (.rtl insn 17) and the loop-1 entry guard (.rtl insn 31). The target needs the shift on a0's register ($s2) and the guard on i's ($s0). Both crown states measured: crown succeeding (candidate.c) rewrites insn 17 from reg 72 to reg 75 and leaves insn 31 at reg 75; crown failing (form A) leaves insn 17 at reg 72 and rewrites insn 31 from reg 75 to reg 72.
- probe: Ten forms measured with sandbox --disable all this session: candidate.c 1/83, A_reuse_i_as_idx 11/83, B_reuse_i_as_idx_a0first 10/83, C_sepctr 31/80, D_base_idxfirst 3/83, K0 1/83, K1 1/83, K2 1/83, K3 11/83, N_sum_as_idx 31/80, plus G1-G4 at 1/83 each; RTL vs cse dumps read insn-by-insn in tmp/grind/func_80045294/s53/dumps_A/.
- result: KILLED for the copy-before-shift geometry. The only device that flips the canonical mid-EBB is a real write to reg 72 (cse invalidate -> delete_reg_equiv), and the sole zero-instruction spelling of such a write, a self-assignment, is folded away by expand and never reaches RTL. The shift-before-copy geometry (H_a0ptr_vfirst, 2/83) is NOT killed: there both operands come out right and the residual is purely sched2's emission order.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer, both crown states, zero FAKE constructs present (K1/K2/K3 carried an un-annotated self-assignment that expand deletes)
- predicate_cite: tools/gcc-2.7.2/cse.c:855

## [s53] A self-assignment of the parameter (a0 = a0;) placed between block 0's shift and the loop-1 guard leaves the build byte-identical to its control, because expand emits no store for it.
- mechanism: The intended path was cse's invalidate() -> delete_reg_equiv(), which promotes reg_next_eqv to qty_first_reg when the canonical register is written, handing the quantity back to i. GCC 2.7.2's expand_assignment produces no insn when source and destination are the same pseudo, so no pass sees it - unlike the s51 dead-store case, where the insn existed in .rtl and jump.c:577 deleted it.
- probe: K1_selfassign_stmt.c (statement form, block-0 declarations split so the statement can sit between shift and guard), K2_selfassign_inexpr.c (s32 v1 = (a0 = a0) << 4;), and K3_A_selfassign.c (same device on the crown-defeated chassis), each measured against the decl-split control K0_declsplit_ctrl.c with sandbox --disable all.
- result: K0 1/83, K1 1/83, K2 1/83 - all identical, residual unchanged at idx 9; K3 11/83, identical to form A. Banked as rejected/s53-selfassign-a0-inert.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, both the i-first (candidate.c) and the crown-defeated (form A) block-0 heads

## [s53] Four loop-1 control-flow spellings on the a0-as-pointer chassis all measure 1/83 with the identical idx-9 residual, so restructuring loop 1 does not make the entry guard a post-cse insn.
- mechanism: The entry guard the target emits at idx 18 is created by jump.c's duplicate_loop_exit_test (jump.c:626), and toplev.c:2827 runs that jump_optimize with after_regscan = 1 before cse_main at toplev.c:2865, so the duplicated test is ordinary RTL when cse walks block 0 and is canonicalised with everything else.
- probe: G1 top-tested while with the offset left to loop.c, G2 guarded do-while with the offset left to loop.c (s52's A_giv shape re-measured on the new chassis), G3 for, G4 candidate.c's explicit offset with a top-tested while; each measured with sandbox --disable all, G1 and G4 disassembled and diffed against asm/funcs/func_80045294.s.
- result: All four score 1 / build_insns 83 with the same single differing instruction at idx 9 (sll $v1,$s0,4 vs target sll $v1,$s2,4). Banked as rejected/s53-giv-while-loop1.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer, i-first block-0 head, zero FAKE constructs present

## [s53] A loop-1-only counter cannot carry block 0's copy: with the loop-2 offset given to the dead sum instead, the counter crosses no call, stays in the incoming $a0 and the copy is never emitted.
- mechanism: local-alloc leaves a short-lived pseudo copied from the parameter in the incoming $a0, so move $s0,$s2 is never emitted and the $s5 save/restore pair drops with it - the s48/s51 allocation collapse at 80 instructions.
- probe: N_sum_as_idx.c (loop-2 offset carried by the dead sum so the loop-1 counter stays short-lived) and C_sepctr.c (plain distinct counters), both measured with sandbox --disable all on this chassis; N disassembled and its block 0 read against the target.
- result: Both 31 / 80 insns, block 0 emitting no parameter copy at idx 8. Reusing the loop-1 counter as loop 2's OFFSET (form A) is the only measured way to keep the copy and defeat the crown at the same time. Banked as rejected/s53-sum-as-idx-alloc-collapse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer, distinct loop counters, zero FAKE constructs present

## [s54] synthesis — merged attack and frontier reset

**Where the function actually stands.** Floor 1 on HEAD 2026-09-03 with
`candidate.c` (a0-as-pointer, i-first). The whole residual is instruction idx 9:
target `sll $v1,$s2,4`, build `sll $v1,$s0,4`. Everything else — prologue
interleave, the whole callee-save allocation, both loops, delay slots, tail — is
byte-exact.

**The merged model (supersedes the s50-s53 per-lever framing).** Block 0 holds
three references to one cse quantity: the copy `i = a0` (insn 15), the shift
(insn 17) and the loop-1 entry guard (insn 31). cse canonicalises every reference
in an extended basic block to `qty_first_reg`, and block 0 is its own EBB
(`from 2 to 36`, measured s54). The target prints `$s2` for the shift and `$s0`
for the guard, i.e. TWO registers for one quantity in one EBB. Only three things
can produce that, and the ladder has now measured two of them dead:

  * R1 crown flip (form A) — measured 11/83 and structurally worse, because the
    crown test is global (`regno_last_uid`), so the same flip also breaks the
    loop-2 preheader shift at 0x80045338. s53 + s54.
  * R2 crown invalidation between insn 17 and insn 31 — CLOSED s54 at
    cse.c:6871-6902: a value-preserving write to the quantity is either rewritten
    to the class head (dest != head, cse.c:6878, stays a copy, invalidates
    nothing) or degenerates to a self-move and is deleted (dest == head). A
    value-CHANGING write invalidates but destroys the a0 value that loop 2's
    preheader re-reads, and preserving it elsewhere needs a seventh long-lived
    callee-save the target's 0x30 frame does not have.
  * R3 one of the two insns created AFTER both cse passes — OPEN. This is the
    frontier.

Everything else the ledger has accumulated (the sched.c LUID tiebreak that makes
emission order equal source order, jump.c:577's partition of dead a0 references,
the 80-instruction allocation collapse whenever loop 1's counter stops crossing
the calls) is consistent with this model and is what makes the score-1 corner the
best of the three corners.

### Frontier (reset, strongest first)

1. **Make the loop-1 ENTRY GUARD a post-cse insn.** If the guard does not exist
   when cse walks block 0, only the shift is canonicalised, the crown can be a0
   (shift right), and the guard — created later from loop 1's exit test — reads
   the counter pseudo directly (guard right). s53 established that
   `duplicate_loop_exit_test` is reached from the FIRST `jump_optimize`
   (toplev.c:2827, `after_regscan = 1`) for the four loop-1 spellings it tried,
   but jump.c's loop-exit-test duplication has preconditions
   (`duplicate_loop_exit_test` bails on insn count, on calls, on labels, on
   volatile refs, on unrecognised patterns). Next probe: read
   `tools/gcc-2.7.2/jump.c`'s `duplicate_loop_exit_test` bail-out list end to end
   and construct a loop-1 body that FAILS the precondition at the first
   `jump_optimize` and passes it at the second (toplev.c runs `jump_optimize`
   again after cse/loop). Measure only shapes that stay at 83 instructions;
   anything that adds a jump is already known to cost 1-2 insns (s52 G/J).

2. **Make block 0's SHIFT a post-cse insn that lands in block 0 rather than the
   loop-1 preheader.** s52 proved loop.c's strength reduction really does emit
   `(set (reg 109) (reg 72))` + `(set (reg 110) (ashift (reg 109) 4))` with a
   `REG_EQUAL (mult (reg 72) 16)` note, reading the UNCANONICALISED parameter
   pseudo — exactly the operand the target wants — but into loop 1's preheader,
   after the guard branch. Next probe: find whether the preheader block can be
   made to coincide with block 0. Candidates: a loop-1 shape whose entry guard is
   itself the loop's first test (so the preheader is empty and gets merged by
   `jump2`/cross-jump before sched1), or moving the `D_800EED14` load INSIDE loop
   1's guarded region so block 0 no longer needs its own shift at all and the
   only shift is the giv init. Read `dumps/text1a_c.loop` for the giv insertion
   point after each change and reject anything at build_insns != 83.

3. **Re-audit the assumption that the loop-2 preheader must re-read a0.** Both R1
   and R2 die on the same fact: the preheader's `addu $s0,$s2,$zero` (0x8004532C)
   forces a0's original value to stay live across DrawSync/func_800520B8 in the
   same pseudo that block 0's shift reads. If loop 2's counter could be
   re-established from a value the function already holds at that point (`s4`,
   `s5`, `sum`, or the `D_800EED14` base) without adding an instruction, a0's
   pseudo becomes free after block 0 and both R1 and R2 reopen with different
   register economics. `P4_a0_carries_count.c` (6/83) shows the frame has room to
   move the count into the parameter pseudo; the missing half is a zero-cost
   re-derivation of the counter. Measure any candidate with
   `sandbox func_80045294 --disable all` and read `dumps/text1a_c.greg`.

## [s54] A value-preserving register copy placed in block 0 between the shift and the loop-1 entry guard (spelled `a0 = i;`) moves cse's qty_first_reg off the parameter pseudo, so the already-processed shift keeps a0's register while the guard is canonicalised to i's.
- mechanism: cse's invalidate() calls delete_reg_equiv(), which promotes reg_next_eqv to qty_first_reg when the current canonical register is written; the promotion would let block 0 print two different registers for one quantity, which is what the target does (sll $v1,$s2,4 at idx 9 vs slt $v0,$s0,$a0 at idx 18).
- probe: Built the copy on both crown states and measured with `sandbox func_80045294 --disable all`, then read the cse dumps. BASE chassis (crown = i, a0 not the class head): P1_a0eqi_blk0.c = score 6, insns 83; dumps_P1/text1a_c.cse keeps the insn as `(insn 31 (set (reg/v:SI 72) (reg/v:SI 75)))`, insn 17 still `(ashift (reg/v:SI 75) 4)`, guard still `(lt (reg/v:SI 75) ...)`. Form A chassis (crown = a0, a0 IS the class head): A2_A_plus_blk0_inval.c = score 11, insns 83, output identical to plain A; A2.cse.fn shows the copy absent from the cse output (insn 28 links straight to insn 34) and the guard still `(lt (reg/v:SI 72) ...)`.
- result: No invalidation occurs in either case, and the two cases exhaust the possibilities for a value-preserving write. When the destination is NOT the class head, cse.c:6871-6902 rewrites SET_SRC to the head (the code's own comment: 'If both are registers that are not the head of their equivalence class, replace SET_SRC with the head of the class'), so the insn stays a live copy and touches no equivalence. When the destination IS the head, canon_reg rewrites the source to the head, the insn becomes `(set (reg 72) (reg 72))`, and the self-move is dropped before the assembler. A write with a DIFFERENT value does invalidate, but it destroys the a0 value that loop 2's preheader re-reads at 0x8004532C.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, both the BASE (a0-as-pointer, i-first, score 1) and form-A (i-as-loop-2-offset, score 11) block-0 crown states, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/cse.c:6881

## [s54] A mention of the parameter placed after loop 2 (`a0 = i;` as the function's last statement) flips cse's crown comparison in block 0 for free, because it makes the parameter pseudo's regno_last_uid postdate the loop counter's.
- mechanism: make_regs_eqv's clause 2 (cse.c:854-856) compares uid_cuid[regno_last_uid[new]] against uid_cuid[regno_last_uid[firstr]]; on the target's geometry i's last mention is loop 2's exit compare, so a later a0 mention would keep a0 canonical and leave block 0's shift reading a0's register. s51 showed dead references to a0 are deleted pre-cse by jump.c:577, so the store direction (into a0) was the untried half.
- probe: T1_tail_a0eqi.c (tail store only) and T2_tail_plus_blk0.c (tail store plus the block-0 invalidation attempt) measured with `sandbox func_80045294 --disable all`.
- result: T1 = score 27, insns 84; T2 = score 27, insns 84. Both materialise an instruction. jump.c:577's deletion predicate (regno_first_uid[dest] == INSN_UID (insn) && regno_last_note_uid[dest] == INSN_UID (insn)) cannot hold for a pseudo that has other mentions, and unlike the mid-block-0 copy there is no later reader to propagate the store into, so it survives to the bytes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first block-0 head (candidate.c geometry), zero FAKE constructs present

## [s54] Moving the preserved a0 into a fresh local `base` and reusing the parameter pseudo to hold D_800A33AC frees the parameter's live range and changes block 0's crown, which the target's own allocation supports because it keeps the count in $a0.
- mechanism: The target loads the count with `lw $a0, %gp_rel(D_800A33AC)($gp)` at 0x800452D4, so the parameter register is genuinely reused for the count in the shipped code; if the C mirrors that, the parameter pseudo's last mention falls back to loop 1's exit test and clause 2 of make_regs_eqv could stop crowning the loop counter.
- probe: P4_a0_carries_count.c (base = a0 as the $s2 carrier and loop-2 pointer, a0 = D_800A33AC, loop 1 tested against a0) measured with `sandbox func_80045294 --disable all`.
- result: score 6, insns 83. The register economy is sound (no extra callee-save, frame unchanged at 0x30), but the extra `base = a0` copy simply crowns base and then i, so block 0's shift is still canonicalised to the loop counter and the residual grows rather than shrinks. Banked as a chassis (rejected/s54-param-carries-count.c), not a lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, base-carrier variant of the a0-as-pointer geometry, zero FAKE constructs present

## [s55] synthesis — the residual is now ONE named pass (cse2) and ONE named predicate (cse.c:8039)

**Chassis check.** `candidate.c` applied to `src/text1a_c.c` measures
`sandbox func_80045294 --disable all` = **score 1, target_insns 83, build_insns 83**
on the HEAD 2026-09-03 chassis. Floor confirmed at 1; the whole residual is still
instruction idx 9: target `sll $v1,$s2,4`, build `sll $v1,$s0,4`.

**Kill re-audit (mandated).** `tools/fake_ablate.py --func func_80045294 --file
text1a_c --candidate memory/grind/func_80045294/candidate.c` reports "no
FAKE-annotated constructs found ... nothing to ablate", and every instance kill in
the s52-s54 block already records `measured_on: HEAD 2026-09-03 chassis ... zero
FAKE constructs present` — i.e. the same chassis measured above, with no FAKE
carrier that could have masked a lever. Nothing in the kill ledger is
FAKE-contaminated. The closest-to-target instance kill (s53 "four loop-1
control-flow spellings all measure 1/83") was re-measured directly:
`R1_while_loop1.c` (loop 1 respelled as a `while` loop, everything else
`candidate.c`) = **score 1, insns 83** — the kill stands unchanged on the current
chassis.

**CORRECTION to the s54 model.** s54 recorded "the crown test is global
(`regno_last_uid`)". That is only half of it. `make_regs_eqv`
(tools/gcc-2.7.2/cse.c:842-857) crowns the copy destination only when BOTH hold:

  (A) `uid_cuid[regno_last_uid[new]] > cse_basic_block_end`
      OR `uid_cuid[regno_first_uid[new]] < cse_basic_block_start`   — EBB-RELATIVE
  (B) `uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]` — global

More importantly, canonicalisation itself is per-EBB: `cse_main` resets the
quantity table at every extended-basic-block boundary, so **if block 0's copy
`i = a0` and block 0's shift `v1 = a0 << 4` sit in DIFFERENT EBBs, no equivalence
exists when the shift is processed and the shift keeps the parameter pseudo** —
without any crown flip, and therefore without disturbing the loop-2 preheader
(which needs the opposite crown). The s53/s54 framing ("one crown per EBB, so the
two operands are deadlocked") is correct only for insns that share an EBB; it does
not forbid splitting the EBB, and that direction had never been probed.

**MEASURED: the EBB split works at cse1.** `cse_end_of_basic_block`
(cse.c:8039-8058) ends an EBB at the first `CODE_LABEL`, and — only when
`after_loop == 0` — also at a `NOTE_INSN_LOOP_END`. A `do { i = a0; } while (0);`
emits exactly that note between the copy and the shift at zero instruction cost.
`B1_dowhile_after_copy.c`:

  * `dumps/text1a_c.cse` (cse1 output): block 0's shift is
    `(ashift:SI (reg/v:SI 72) ...)` — **reg 72 is the parameter pseudo**. This is
    the first time in 55 sessions that the target's shift operand has been
    produced on the copy-before-shift chassis.
  * `dumps/text1a_c.cse2`: the SAME insn is back to `(ashift:SI (reg/v:SI 75) ...)`
    (reg 75 = loop 1's counter).
  * final: score 6, insns 83; `sll v1,s0,0x4` in the emitted code, plus a 3-insn
    prologue rotation (`sw ra/s5/s4` hoisted ahead of `sw s0` / `move s0,s2` /
    `sll`).

**Why cse2 undoes it.** `toplev.c:2926` calls `cse_main (insns, max_reg_num (), 1,
cse2_dump_file)` — the third argument is `after_loop = 1`, which makes
cse.c:8055-8058 skip the `NOTE_INSN_LOOP_END` break. cse2's EBB for block 0
therefore spans the copy and the shift again, re-establishes the equivalence, and
`canon_reg` rewrites the shift's operand back to the crown (reg 75). cse2 is not
optional here: `-O2` sets `flag_rerun_cse_after_loop`.

**The closed-form statement of the residual (supersedes the s54 three-route model).**
For the target's two operands to print, block 0's copy and block 0's shift must be
in different cse EBBs at BOTH cse passes. At cse2 the only EBB terminator that
survives is a `CODE_LABEL` (cse.c:8039; the LOOP_END and SETJMP breaks are gated by
`!after_loop` and by `NOTE_INSN_SETJMP` respectively). Flipping (B) globally cannot
help, because (A) is true for the loop counter in both block 0's EBB and the
loop-2 preheader's EBB, so a single global (B) decides both and they need opposite
answers — this is the same wall s53 form A hit (11/83), now with a code-level
predicate instead of an empirical one. Hence:

  **The whole function reduces to: produce a `CODE_LABEL` between block 0's
  `i = a0` and block 0's `a0 << 4` that costs zero instructions and survives
  `jump_optimize`.**

That is a much narrower target than "an insn created after cse". It also explains
every prior result: the four loop-1 respellings (s53) and every declaration-order
permutation (s50) all leave the copy and shift adjacent inside one EBB, so none of
them could ever have moved idx 9.

### Frontier (reset, strongest first)

1. **A zero-instruction CODE_LABEL between the copy and the shift.**
   `jump_optimize` deletes a label as soon as `LABEL_NUSES` reaches 0 and deletes a
   jump whose target is the next insn, so the naive `goto`/`break` shapes collapse
   in the same pass (and the dead-goto label-pad spelling is a FORBIDDEN family —
   this must be a label a real construct needs). Next probe: read
   `tools/gcc-2.7.2/jump.c` label/jump deletion predicates end to end and
   enumerate which ordinary-C constructs leave a surviving `CODE_LABEL` with no
   emitted instruction — candidates to enumerate and measure: a one-case `switch`,
   a `do { ... } while (cond)` whose exit label is referenced by a `break` that
   jump.c cannot fold to a jump-to-next, and any shape where loop 1 own top label
   can be made to precede the shift while the shift still lands before the guard
   branch. Verify each in `dumps/text1a_c.cse2` (look for `(ashift:SI (reg/v:SI
   72)` in block 0) BEFORE looking at the score, and reject anything at
   `build_insns != 83`.

2. **Make cse2 crown flip by changing what `reg_scan` sees after loop.c.**
   `toplev.c:2925` re-runs `reg_scan` on the POST-LOOP RTL immediately before cse2,
   so cse2 clause (B) is evaluated on last-use positions that loop.c may have
   moved. If loop.c strength reduction / biv elimination made loop 2 counter
   (reg 75) die before the pointer pseudo (reg 72), cse2 would crown reg 72 and
   block 0's shift would keep the parameter. The cost is that the loop-2 preheader
   shift then flips the wrong way, so this is only worth measuring in combination
   with a cse1-side split (frontier 1) that protects the preheader. Next probe:
   diff `regno_last_uid` for regs 72/75 between `dumps/text1a_c.loop` and
   `dumps/text1a_c.cse2` on candidate.c, then try loop-2 shapes (pointer-compare
   exit test, idx merged into the pointer) that push reg 75 last use earlier.

3. **Re-open the block-0 prologue rotation that B1 exposed.** B1 shows the
   `sw ra / sw s5 / sw s4` cluster can be moved ahead of `sw s0 / move s0,s2 / sll`
   by a source-level change that adds no instructions. That is a free 3-insn
   scheduling degree of freedom in block 0 nobody has catalogued. If a future form
   fixes idx 9 but rotates the prologue the way B1 did, this is the knob that pays
   it back. Next probe: bisect which part of B1 (the split declarations, the
   do-while notes, or the LUID renumbering) causes the rotation, using
   `B0_control_splitdecl.c` (score 1, so NOT the declarations) as the control.

## [s55] A `do { i = a0; } while (0);` wrap of block 0's copy splits cse extended basic block at `NOTE_INSN_LOOP_END`, so cse1 leaves block 0's shift reading the parameter pseudo, but `cse_main` second call re-merges the block and rewrites the operand back.
- mechanism: `cse_end_of_basic_block` (tools/gcc-2.7.2/cse.c:8039-8058) terminates an EBB at a `CODE_LABEL` and, when `after_loop == 0`, also at a `NOTE_INSN_LOOP_END`; `cse_main` clears the quantity table at each EBB boundary, so a copy and a later shift in different EBBs share no equivalence and `canon_reg` cannot rewrite the shift operand. `toplev.c:2926` runs `cse_main (..., 1, cse2_dump_file)` with `after_loop = 1`, which disables the LOOP_END break.
- probe: `tmp/grind/func_80045294/s55/B1_dowhile_after_copy.c` (candidate.c with split declarations and `do { i = a0; } while (0);` between `sum = 0;` and `v1 = a0 << 4;`) measured with `sandbox func_80045294 --disable all`; control `B0_control_splitdecl.c` (identical but with a plain `i = a0;`) measured to isolate the declaration restructure; `pwsh tools/grinder/dump.ps1 func_80045294` read at `.cse` and `.cse2`.
- result: control B0 = score 1, insns 83 (the split-declaration restructure is byte-neutral). B1 = score 6, insns 83. `.cse` shows block 0's shift as `(ashift:SI (reg/v:SI 72) (const_int 4))` — the parameter pseudo, i.e. the target operand — proving the EBB split works and the crown never gets a chance to act. `.cse2` shows the same insn as `(ashift:SI (reg/v:SI 75) ...)`, and the emitted code is `sll v1,s0,0x4`. The extra 5 points over the control are a 3-insn prologue rotation (`sw ra/s5/s4` hoisted ahead of `sw s0`/`move s0,s2`/`sll`) plus the unchanged idx-9 operand.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry with split declarations, zero FAKE constructs present (the do-while(0) was measured un-annotated as a probe and is banked as rejected, not proposed)

## [s55] s53 kill of loop-1 control-flow respellings still holds on the current chassis.
- mechanism: re-audit mandated by the driver (floor flat 3 sessions, instance kills present). `tools/fake_ablate.py` reports no FAKE-annotated constructs in candidate.c, so no banked kill can have been masked by a FAKE carrier occupying a target pseudo.
- probe: `tmp/grind/func_80045294/s55/R1_while_loop1.c` (loop 1 respelled `while (i < count) { ... }`, everything else candidate.c) measured with `sandbox func_80045294 --disable all`.
- result: score 1, insns 83 — identical to candidate.c, same idx-9 residual. The s53 conclusion is re-confirmed, and the s55 model explains it: a control-flow respelling of loop 1 never inserts an EBB boundary between block 0's copy and block 0's shift, so it cannot touch idx 9.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE constructs present

## [s55] A do { i = a0; } while (0); wrap of block 0's copy splits cse's extended basic block at NOTE_INSN_LOOP_END, so cse1 leaves block 0's shift reading the parameter pseudo, but cse_main's second call re-merges the block and rewrites the operand back to the loop counter.
- mechanism: cse_end_of_basic_block (tools/gcc-2.7.2/cse.c:8039-8058) terminates an extended basic block at a CODE_LABEL and, when after_loop == 0, also at a NOTE_INSN_LOOP_END. cse_main clears the quantity table at each EBB boundary, so a copy and a later shift sitting in different EBBs share no equivalence and canon_reg cannot rewrite the shift's operand. toplev.c:2926 runs cse_main (insns, max_reg_num (), 1, cse2_dump_file) with after_loop = 1, which disables the LOOP_END break; -O2 (Makefile:35) enables that second run via flag_rerun_cse_after_loop.
- probe: tmp/grind/func_80045294/s55/B1_dowhile_after_copy.c (candidate.c with the six block-0 locals declared uninitialised and assigned in the same order, plus do { i = a0; } while (0); between sum = 0; and v1 = a0 << 4;) measured with `sandbox func_80045294 --disable all`; control tmp/grind/func_80045294/s55/B0_control_splitdecl.c (identical but with a plain i = a0;) measured to isolate the declaration restructure; `pwsh tools/grinder/dump.ps1 func_80045294` read at .cse and .cse2 and banked as s55/B1.cse and s55/B1.cse2.
- result: Control B0 = score 1, insns 83 — the split-declaration restructure is byte-neutral, so every difference belongs to the do-while. B1 = score 6, insns 83. B1.cse shows block 0's shift as (ashift:SI (reg/v:SI 72) (const_int 4)) — reg 72 is the parameter pseudo, i.e. the operand the target prints as sll $v1,$s2,4. That is the first time in 55 sessions the target's shift operand has been produced on the copy-before-shift chassis, and it is produced with no crown flip, so the loop-2 preheader is undisturbed. B1.cse2 shows the same insn back at (ashift:SI (reg/v:SI 75) ...), and the emitted code is sll v1,s0,0x4. The 5 extra score points over the control are a 3-insn prologue rotation (sw ra/s5/s4 hoisted ahead of sw s0 / move s0,s2 / sll) plus the unchanged idx-9 operand.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis (candidate.c = score 1, insns 83), a0-as-pointer i-first geometry with split declarations, zero FAKE constructs present; the do-while(0) was measured un-annotated as a probe and is banked as a rejected form, not proposed

## [s55] No NOTE-based extended-basic-block split of block 0 survives the second cse pass, because cse_end_of_basic_block only honours the NOTE_INSN_LOOP_END terminator when after_loop is zero and cse_main's second invocation passes after_loop = 1; only a CODE_LABEL terminates an EBB in both passes.
- mechanism: cse.c:8054-8056 guards the NOTE_INSN_LOOP_END break with `! after_loop`; cse.c:8060-8062 guards the only other note-based break on NOTE_INSN_SETJMP, which this function cannot reach (no setjmp). The unconditional terminator is the loop header itself, cse.c:8039, `while (p && GET_CODE (p) != CODE_LABEL)`.
- probe: Read tools/gcc-2.7.2/cse.c:8008-8110 and tools/gcc-2.7.2/toplev.c:2865, 2918-2931 end to end; confirmed empirically by the B1 .cse vs .cse2 dump pair above, where the identical insn carries reg 72 after cse1 and reg 75 after cse2 with no intervening source change.
- result: The residual reduces to a single requirement: a CODE_LABEL between block 0's copy (i = a0) and block 0's shift (a0 << 4) that costs zero instructions and survives jump_optimize's label/jump deletion. Note-based devices (any do/while/for wrapper) are excluded as a family by the after_loop guard, independent of spelling or nesting.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE constructs present; source-read of cse.c/toplev.c plus the B1 .cse/.cse2 dump pair
- predicate_cite: tools/gcc-2.7.2/cse.c:8055

## [s55] s53's kill of loop-1 control-flow respellings still holds on the current chassis: respelling loop 1 as a while loop measures 1/83 with the identical idx-9 residual.
- mechanism: Driver-mandated kill re-audit (floor flat 3 sessions, instance kills present). tools/fake_ablate.py reports no FAKE-annotated constructs in candidate.c, so no banked kill can have been masked by a FAKE carrier occupying a target pseudo; and the s52-s54 instance kills are all recorded on this same HEAD 2026-09-03 chassis.
- probe: `python3 tools/fake_ablate.py --func func_80045294 --file text1a_c --candidate memory/grind/func_80045294/candidate.c` (WSL), then tmp/grind/func_80045294/s55/R1_while_loop1.c (loop 1 respelled `while (i < count) { ... }`, everything else candidate.c) measured with `sandbox func_80045294 --disable all`.
- result: fake_ablate: 'no FAKE-annotated constructs found in memory/grind/func_80045294/candidate.c; nothing to ablate'. R1 = score 1, insns 83 — identical to candidate.c. The s55 model explains why: a control-flow respelling of loop 1 never inserts an EBB boundary between block 0's copy and block 0's shift, so it cannot touch idx 9.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE constructs present


## [s56] synthesis — merged attack after re-reading the whole ledger

### Kill re-audit (mandated, discharged)

`tools/fake_ablate.py` still reports no FAKE-annotated constructs anywhere in this
ledger, so the FAKE-ablation leg is vacuous (unchanged since s51). The two instance
kills that sat closest to the target were re-measured on the current HEAD 2026-09-03
chassis:

* s53's **H_a0ptr_vfirst** (v1-first, both operands correct, order wrong) — re-measures
  **2/83**, kill stands, and this session upgraded it to a class kill (below).
* s53's **A_reuse_i_as_idx** (crown flipped to a0) — re-measures **11/83**, kill stands,
  and its 11 points were decomposed for the first time: 1 = the loop-1 guard operand,
  10 = a pure `$s0`/`$s1` register-name swap in loop 2 caused by A's variable-sharing
  pattern, NOT by cse.

The s53/s55 loop-1 control-flow respellings were re-measured last session at 1/83 and
are subsumed by this session's exhaustive 120-permutation sweep of block 0, which is a
strictly larger enumeration of the same axis.

### [s56-H1] KILLED (class) — the v1-first geometry cannot emit the copy before the shift

**Statement.** On the v1-first (H) geometry the shift is emitted before the copy because
both insns carry `INSN_PRIORITY == 1` and `rank_for_schedule` falls through to its
`INSN_LUID` tiebreak, and the shift's priority cannot be raised while its operand remains
the incoming parameter.

**Mechanism.** `priority()` (tools/gcc-2.7.2/sched.c:1433-1521) derives `INSN_PRIORITY`
only from an insn's `LOG_LINKS` predecessors, `prev_priority = priority (x) +
insn_cost (x, prev, insn) - 1`, floored at 1. In block 0 the shift and the copy each have
exactly one real predecessor — insn 4, `move s2,a0`, priority 1, cost 1 — so both are
priority 1 (verified in the sched2 dump listing). `rank_for_schedule`
(sched.c:2408) then compares priority (sched.c:2418), then the last-scheduled-insn
dependence class (both class 3 here, as s51/s53 measured), then returns
`INSN_LUID (tmp) - INSN_LUID (tmp2)` at **sched.c:2464**, preserving RTL order.
Raising the shift's priority requires giving it a predecessor with a longer chain or a
multi-cycle cost, i.e. changing its operand away from the parameter — which is exactly
the byte the target fixes.

**Measured on.** HEAD 2026-09-03 chassis, a0-as-pointer geometry, zero FAKE constructs.
`H.c` = 2/83; dumps `tmp/grind/func_80045294/s56/H.sched2.fn`; the 120-form sweep
confirms every `V < I` permutation is >= 2.

**predicate_cite:** `tools/gcc-2.7.2/sched.c:2464`

### [s56-H2] KILLED (instance) — block-0 declaration order is a closed axis

**Statement.** No permutation of block 0's six initialised declarations reaches score 0
on the a0-as-pointer chassis: all 120 orderings permitted by the data dependences
`v1 < s4 < s5` were built and scored, giving 6 forms at 1, 53 at 2, 36 at 4 and 24 at 5.

**Measured on.** HEAD 2026-09-03 chassis, a0-as-pointer geometry, zero FAKE constructs;
`tmp/grind/func_80045294/s56/sweep.csv`.

### [s56-H3] KILLED (instance) — a do-while label between the copy and the shift is undone by loop.c, not by cse2

**Statement.** `do { v1 = a0 << 4; } while (i < a0);` splits block 0's extended basic
block at cse1 and makes the shift print the parameter pseudo (reg 72), but loop.c hoists
the loop-invariant shift above the label before cse2 and the shift reads the counter
again; the form measures 4/86.

**Mechanism.** cse.c:8039 terminates the EBB at any CODE_LABEL, and `text1a_c.cse2`
still contains `(code_label 18 ...)` — so the label itself survives. Between cse1
(toplev.c:2865) and cse2 (toplev.c:2926), loop.c (toplev.c:2895) recognises
`a0 << 4` as invariant in the do-while and moves it into the loop's preheader, i.e.
ahead of the label, restoring the single-EBB configuration in which `canon_reg`
(cse.c:2532-2572) rewrites reg 72 to the crown.

**This CORRECTS the s55 conclusion** that "at cse2 the only surviving EBB terminator is a
CODE_LABEL" was the blocker: the blocker is the shift's mobility, not the label's
survival.

**Measured on.** HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE
constructs. `memory/grind/func_80045294/rejected/s56-loop-label-split-undone-by-licm-hoist.c`.

### [s56-H4] KILLED (instance) — a never-taken `if` between the copy and the shift is folded before cse1

**Statement.** `if (i < a0) { sum = 1; }` placed between block 0's copy and its shift is
removed before cse1 runs — `text1a_c.cse` shows block 0 as one EBB (`from 2 to 46`) with
no code_label — so it costs nothing (1/83) and creates no split.

**Measured on.** HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE
constructs. `memory/grind/func_80045294/rejected/s56-ifjoin-label-folded-before-cse1.c`.

### [s56-E1] CONFIRMED — the original compilation had crown = i, so its block-0 shift escaped canon_reg

**Statement.** In the shipped code the parameter's register `$s2` is last mentioned at
loop 2's `addiu $s2,$s2,0x10`, which precedes the counter's last mention at loop 2's exit
`slt $v0,$s0,$v0`; clause (B) of `make_regs_eqv` (cse.c:854-857) therefore held for the
counter in the original compilation, i.e. the original's crown was i — the same crown the
candidate chassis produces — and block 0's shift nevertheless printed `$s2`.

**Consequence.** Crown-flipping is not the route (it is form A, and it costs the guard).
The original's block-0 shift was either in a different extended basic block from the copy
or created after cse2. Together with [s56-H1] this closes "the shift preceded the copy",
leaving exactly two live routes for the last instruction.

### Frontier reset for the next ladder pass

1. **A label between the copy and the shift that loop.c cannot hoist the shift across.**
   [s56-H3] proved the split works at cse1 and that only LICM undoes it. Probe: keep the
   shift OUT of any loop body and put a non-loop CODE_LABEL between the copy and the
   shift — [s56-H4] shows the carrier's condition must be one `jump_optimize`
   (toplev.c:2827) cannot resolve, so build it on `a1` or on a global rather than on a
   value cse knows equals `a0`; alternatively keep the do-while carrier but make the
   shift non-invariant inside it (e.g. shift a value the loop also writes). For every
   form, read `dumps/text1a_c.cse2` for `(ashift:SI (reg/v:SI 72)` BEFORE reading the
   score, and only then hunt for a zero-instruction carrier.
2. **Post-cse2 creation of block 0's shift.** The only remaining route besides the EBB
   split. Passes after cse2 (toplev.c:2926) that can create insns are jump_optimize
   (2929), thread_jumps (2935), combine, and the allocators. Probe: enumerate loop-1
   shapes in which loop.c's strength reduction emits the `a0 << 4` giv initialisation
   into block 0 rather than into loop 1's preheader (s52 measured it landing after the
   guard) — a giv init created at toplev.c:2895 is still canonicalised by cse2, so the
   useful variant is one where combine rebuilds the shift from the surviving
   `(set (reg 75) (reg 72))` copy, which requires the shift's operand pseudo to die at
   the shift.
3. **Form A's loop-2 register swap.** Form A is byte-exact through block 0's first
   seventeen instructions; ten of its eleven points are a `$s0`/`$s1` naming swap driven
   by its variable-sharing pattern. Probe: re-derive A with the target's sharing pattern
   (`sum` reused as loop 2's byte offset, `i` reused as loop 2's counter) and measure —
   note the tension that this pattern makes `i` the quantity's last mention and so
   re-crowns i; a form that keeps A's crown while adopting the target's allocation would
   isolate the guard as the single residual and give a second, independent one-instruction
   attack surface.

## [s56] On the v1-first (H) geometry the shift is emitted before the copy because both insns carry INSN_PRIORITY 1 and rank_for_schedule falls through to its INSN_LUID tiebreak, and the shift's priority cannot be raised while its operand remains the incoming parameter.
- mechanism: priority() (tools/gcc-2.7.2/sched.c:1433-1521) derives INSN_PRIORITY only from an insn's LOG_LINKS predecessors via prev_priority = priority(x) + insn_cost(x,prev,insn) - 1, floored at 1. In block 0 the shift (insn 14) and the copy (insn 22) each have exactly one real predecessor, insn 4 = move s2,a0 (priority 1, cost 1), so both are priority 1. rank_for_schedule (sched.c:2408) compares priority (sched.c:2418), then the last-scheduled-insn dependence class (both class 3, as s51/s53 measured), then returns INSN_LUID(tmp) - INSN_LUID(tmp2) at sched.c:2464, preserving RTL order. Lengthening the shift's predecessor chain requires changing its operand away from the parameter, which is the very byte the target fixes.
- probe: Applied tmp/grind/func_80045294/s56/H.c, measured `sandbox func_80045294 --disable all` = 2/83, then `pwsh tools/grinder/dump.ps1 func_80045294` and read the sched2 priority listing and scheduled insn chain (tmp/grind/func_80045294/s56/H.sched2.fn). Cross-checked against the 120-form block-0 permutation sweep, where every v1-before-i ordering measures 2 or worse.
- result: sched2 prints `insn[14]: priority = 1` and `insn[22]: priority = 1` for the shift and the copy, with only the two tail insns (28, 31) at priority 2. The emitted block-0 stream is sp / sw s2 / move s2 / sw s3 / move s3 / sw s1 / sum=0 / sw s0 / sll / move s0 / sw ra / sw s5 / sw s4 / ..., against the target's ... sw s0 / move s0 / sll / sw ra ..., i.e. insns 14 and 22 swapped and nothing else. The H geometry's entire 2-point residual is that swap.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer geometry, v1-first block-0 head, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s56] No permutation of block 0's six initialised declarations reaches score 0 on the a0-as-pointer chassis: the 120 orderings permitted by the data dependences v1 < s4 < s5 were each built and scored, giving 6 forms at 1, 53 at 2, 36 at 4 and 24 at 5.
- mechanism: Declaration/statement order inside block 0 fixes the RTL insn order, which fixes both cse's crown assignment for the a0 quantity (make_regs_eqv, cse.c:842-857) and the scheduler's LUID tiebreak (sched.c:2464). If the last instruction were a pure block-0 ordering artifact, some permutation would land at 0.
- probe: tmp/grind/func_80045294/s56/gen.py generated all orderings of S (sum=0), V (v1 = a0<<4), L (s4 = load), I (i = a0), C (count = D_800A33AC), F (s5 = s4+a1) satisfying V < L < F; tmp/grind/func_80045294/s56/sweep.ps1 applied and scored each with `sandbox func_80045294 --disable all`, recording tmp/grind/func_80045294/s56/sweep.csv.
- result: 120/120 forms measured, build_insns 83 throughout. Score histogram 1x6, 2x53, 4x36, 5x24; minimum 1. The six score-1 forms are exactly the orderings S < I < V < L < F with C free (CSIVLF, SCIVLF, SICVLF, SIVCLF, SIVLCF, SIVLFC) — count's position is byte-irrelevant, and candidate.c is one of the six.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer geometry, zero FAKE constructs present

## [s56] A do-while carrier placed between block 0's copy and its shift (`do { v1 = a0 << 4; } while (i < a0);`, which runs once because i == a0) splits the extended basic block at cse1 and makes the shift print the parameter pseudo, but loop.c hoists the invariant shift above the label before cse2 and the shift reads the counter again; the form measures 4/86.
- mechanism: cse.c:8039 terminates an extended basic block at any CODE_LABEL irrespective of after_loop, so the do-while's top label splits block 0 at both cse passes. But loop.c (toplev.c:2895), which runs between cse1 (toplev.c:2865) and cse2 (toplev.c:2926), recognises a0 << 4 as loop-invariant and moves it into the loop's preheader, i.e. ahead of the label, restoring the single-EBB configuration in which canon_reg (cse.c:2532-2572) rewrites reg 72 to the crown.
- probe: Built tmp/grind/func_80045294/s56/L1_label_probe.c, measured `sandbox func_80045294 --disable all`, then `pwsh tools/grinder/dump.ps1 func_80045294` and grepped the per-function regions of dumps/text1a_c.cse and dumps/text1a_c.cse2 for `Processing block`, `code_label` and `ashift`.
- result: score 4, build_insns 86 (the carrier costs slt + branch + nop). cse1: `;; Processing block from 2 to 18` then `;; Processing block from 20 to 33` with `(code_label 18 ...)` between them, and the shift printed `(ashift:SI (reg/v:SI 72) ...)` — the parameter pseudo, the target's operand, for only the second time in 56 sessions. cse2: `(code_label 18 ...)` is STILL present, but the shift now sits above it and reads `(reg/v:SI 75)`. This corrects s55's framing: the label survives cse2 fine; the shift's mobility is the blocker.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE constructs present

## [s56] A never-taken `if (i < a0) { sum = 1; }` placed between block 0's copy and its shift is folded away before cse1 runs, so it costs nothing and creates no extended-basic-block split.
- mechanism: jump_optimize at toplev.c:2827 resolves the comparison and deletes the branch and its join label; jump.c:270-271 then deletes any CODE_LABEL whose LABEL_NUSES has fallen to zero at the start of the next jump_optimize, so nothing survives into cse_main.
- probe: Built tmp/grind/func_80045294/s56/L2_ifjoin_probe.c, measured `sandbox func_80045294 --disable all`, then dumped and grepped the per-function regions of dumps/text1a_c.cse and .cse2.
- result: score 1, build_insns 83 — byte-identical to candidate.c. dumps/text1a_c.cse shows block 0 as a single EBB `;; Processing block from 2 to 46` with no code_label anywhere in the function's cse region, and the shift still reads (reg/v:SI 75). A non-loop label carrier therefore needs a condition jump_optimize cannot resolve.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer i-first geometry, zero FAKE constructs present

## [s56] The shipped code's own register layout shows the original compilation had cse crown = i, so its block-0 shift escaped canon_reg by extended-basic-block placement or by post-cse2 creation rather than by a crown flip.
- mechanism: make_regs_eqv's clause (B) (cse.c:854-857) is the global comparison uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]. In the target, the parameter's register $s2 is last mentioned at loop 2's `addiu $s2,$s2,0x10`, which precedes the counter's last mention at loop 2's exit `slt $v0,$s0,$v0`, so clause (B) held for the counter in the original build exactly as it does on the candidate chassis.
- probe: Read the full target disassembly (tmp/grind/func_80045294/s56/target.txt) and located the last mention of each register in the a0 quantity, then compared against the crown predicate in tools/gcc-2.7.2/cse.c:842-857.
- result: Confirmed: the original's crown was i, and block 0's shift still printed $s2. Combined with the class kill of the v1-first geometry above, the two remaining routes to the last instruction are (a) an extended-basic-block split that survives loop.c and cse2, and (b) creation of block 0's shift after cse2 (toplev.c:2926). Crown-flipping is form A, which costs the loop-1 guard.
- verdict: CONFIRMED

## [s56] Form A's eleven points decompose into one crown-priced instruction (the loop-1 entry guard operand) plus a ten-instruction $s0/$s1 register-name swap in the loop-2 preheader and body that follows from its variable-sharing pattern, not from cse.
- mechanism: Form A reuses i as loop 2's byte offset, so i and the loop-2 counter are distinct variables and the allocator gives the counter $s1 and the offset $s0; the target instead shares $s0 between the loop-1 and loop-2 counters and $s1 between sum and the loop-2 offset. The crown flip itself only mis-prints the guard.
- probe: Applied tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c, measured `sandbox func_80045294 --disable all`, and diffed the objdump of tmp/sandbox/func_80045294/text1a_c.o against target.txt instruction by instruction (tmp/grind/func_80045294/s56/d.sh, cur.txt).
- result: 11/83 re-confirmed on the current chassis. Block 0's first seventeen instructions are byte-exact INCLUDING `sll $v1,$s2,4`; the guard prints `slt $v0,$s2,$a0` against the target's `slt $v0,$s0,$a0`; the remainder is the naming swap (build `move s1,s2` / `slt v0,s1,v0` / `sll v1,s1,4` against target `addu $s0,$s2,$zero` / `slt $v0,$s0,$v0` / `sll $v1,$s0,4`).
- verdict: CONFIRMED

## [s57] solver — the first solver-suite pass ever run on this function

Modality: solver. `tools/ra_solver` + `tools/sched_solver` had never been run on
func_80045294 in 56 sessions (a grep of the whole ledger for `inverse_compose`,
`perturb.py` and `--target-object` returns nothing before this session). Chassis
re-verified first: `memory/grind/func_80045294/candidate.c` applied over
`src/text1a_c.c:1445` measures `sandbox func_80045294 --disable all` =
**score 1, target_insns 83, build_insns 83** on the HEAD 2026-09-03 chassis, so
the brief's "measurement unavailable" is resolved and the ledger floor of 1 stands.
The single differing instruction is build idx 9: ours `sll v1,s0,0x4`, target
`sll v1,s2,0x4` (`tmp/grind/func_80045294/s57/hon.txt`, `tgt.txt`).

### [s57-H1] KILLED (class) — the register-allocation layer is already AT the target

**Statement.** On the candidate chassis global alloc assigns
`{72:$s2, 73:$s3, 74:$s1, 75:$s0, 76:$v1, 77:$s4, 79:$a1, 80:$s5, 85:$s1, 91:$v0}`,
and that disposition is exactly the one the target's bytes require: 82 of the 83
instructions are byte-identical, and pseudo 75 ($s0) and pseudo 72 ($s2) each occur
in many of those matching instructions. No assignment of hard registers to pseudos
reaches the target, because the residual needs ONE reference of pseudo 75 to print
`$s2` while seventeen other references of the same pseudo keep printing `$s0`.

**Mechanism.** `global.c:1275` is `reg_renumber[allocno_reg[allocno]] = best_reg;`
— exactly one hard register per pseudo, function-wide. A per-reference hard
register does not exist in this allocator, so an operand divergence on one
reference of a pseudo that is correct on all its other references is outside the
RA model's reachable set by construction. `tools/ra_solver/inverse.py global
--goal '{"75": 18, "72": 16}'` (the naive RA reading of the residual, i.e. swap
$s0/$s2) does return 54 two-atom vectors, but the goal itself is wrong: applying
the swap renames both registers function-wide and breaks the eighteen currently
matching instructions at build idx 1, 2, 7, 8, 18, 24, 26, 38, 39, 41, 44, 46, 49,
61, 63, 64, 77 and 79.

**Probe.** `python3 tools/ra_solver/extract.py func_80045294 text1a_c --out
tmp/grind/func_80045294/s57/model.json` (model derived from the CANDIDATE body
applied to src, per solver operational rule 2), then the full instruction-level
diff of `tmp/sandbox/func_80045294/text1a_c.o` against `build/src/text1a_c.o` via
`engine.score.normalized_insns` (`tmp/grind/func_80045294/s57/streams.py`), then
`tools/ra_solver/inverse.py`. Report: `tmp/grind/func_80045294/s57/ra_report.txt`.

**Measured on.** HEAD 2026-09-03 chassis, candidate.c (a0-as-pointer i-first
geometry) at 1/83, zero FAKE constructs present.

**predicate_cite:** `tools/gcc-2.7.2/global.c:1275`

### [s57-H2] CONFIRMED — `inverse_compose.py classify` MIS-TYPES this residual as RA, and that is a pipeline-wide hazard for pseudo-substitution residuals

`python3 tools/ra_solver/inverse_compose.py classify text1a_c func_80045294
--target-object build/src/text1a_c.o --ours-object tmp/sandbox/func_80045294/text1a_c.o`
prints:

    FIRST DIVERGENCE: RA
      next tool: tools/ra_solver/inverse.py  (global / local)
      same instructions, different registers:
        ours  : sll v1,s0,0x4
        target: sll v1,s2,0x4

The verdict is wrong, and the reason is structural rather than local. `classify`
decides PRE-RA vs RA by comparing REGISTER-BLANKED instruction multisets: blanking
turns both spellings into `sll <r>,<r>,0x4`, the multisets match, and it concludes
"same instructions, different registers". But the difference here is which PSEUDO
the pre-RA RTL names (our `(ashift (reg 75))` against the target's
`(ashift (reg 72))`), not which hard register the allocator chose for a fixed
pseudo. Register blanking cannot distinguish those two situations and always
resolves the ambiguity in favour of RA, so **every cse/`canon_reg` operand-
substitution residual in this project will be typed RA by `classify`.** The
disambiguator is cheap and should be run whenever `classify` says RA on a residual
of one or two instructions: extract the RA model and check whether the divergent
register is already correct on that pseudo's OTHER references — if it is, the layer
is PRE-RA and the RA backend has nothing to search. This is the same class of
hazard as the func_80017848 s25 finding, but it survives the object path, so the
2026-08-30 `--target-object` escape does not fix it.

### [s57-H3] KILLED (instance) — form A's ten-instruction loop-2 "register swap" is not an allocation artifact, so s56 frontier item 3's premise is false

**Statement.** Form A (`tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c`,
re-measured this session at 11/83, build_insns 83) produces the register
disposition `{72:$s2, 73:$s3, 74:$s1, 75:$s0, 76:$v1, 77:$s4, 79:$a1, 80:$s5,
85:$s1, 91:$v0}` — identical in all 24 entries to candidate.c's, and identical to
what the target's bytes require. A's eleven-point residual therefore contains no
allocation difference at all: it is entirely a difference in which VALUE each
pseudo carries, decided before RA by A's variable-sharing pattern.

**Consequence.** s56 frontier item 3 ("Form A's ten-point loop-2 register-name swap
is an allocation artifact that can be removed independently of the crown") is
refuted. There is no allocator lever to pull. Making A's loop 2 print the target's
registers means giving pseudo 75 the loop-2 counter and $s1 the byte offset, which
IS the target's variable sharing, which is exactly what re-crowns i and destroys
A's block 0. Form A and candidate.c are the two endpoints of one pre-RA trade, not
two independent attack surfaces.

**Probe.** Applied A, `sandbox func_80045294 --disable all` = 11/83, full
instruction diff (differing indices 18, 38, 39, 41, 45, 51, 57, 61, 63, 64, 66),
then `python3 tools/ra_solver/extract.py func_80045294 text1a_c --out
tmp/grind/func_80045294/s57/modelA.json` and a field-by-field comparison of
`dispositions` against `model.json`. Two of the eleven points (idx 61 and 66) are
additionally just the source order of loop 2's two increments, not a naming issue
at all.

**Measured on.** HEAD 2026-09-03 chassis, form A geometry at 11/83, zero FAKE
constructs present.

### [s57-H4] KILLED (class) — on the v1-first (H) geometry no scheduler-input perturbation reaches copy-before-shift without a source reorder

**Statement.** An exhaustive depth-2 enumeration of the entire scheduler input
space for func_80045294's pass-2 block 0 on the H geometry (960 single atoms plus
pairs, over the `add_dep` / `del_dep` / `luid` / `luid_move` / `cost` / `unit`
classes) returns 40 goal-reaching vectors, and every one of them is either a `luid`
reorder — which is by definition the i-first geometry, whose cse pass then rewrites
the shift's operand to the crown and yields score 1 with the wrong operand — or an
`add_dep X <- Y` in which the added predecessor Y has a HIGHER LUID than X, i.e. a
dependence edge that contradicts the RTL order it would have to be built in and
that no C can spell. Pass 1 gives the same answer (12 vectors, the same two shapes).

**Mechanism.** `tools/sched_solver` models sched.c's list scheduler order- and
clock-exactly and reported `parity=True` on this TU (154 functions, 560 blocks,
3618 picks reproduced). In the H model, pass-2 block 0 has the shift as uid 14
(LUID 15, priority 1, cost 1, sole predecessor uid 4 = `move s2,a0`) and the copy
as uid 22 (LUID 16, priority 1, same sole predecessor), so `rank_for_schedule`
falls through to its `INSN_LUID (tmp) - INSN_LUID (tmp2)` tiebreak at
`tools/gcc-2.7.2/sched.c:2464` and preserves RTL order. Raising the copy's priority
above the shift's requires a new dependence edge, and the enumeration shows every
edge that would do it runs backwards against LUID.

**Probe.** `python3 tools/sched_solver/extract.py text1a_c --out
tmp/grind/func_80045294/s57/schedH.json` with
`tmp/grind/func_80045294/s53/H_a0ptr_vfirst.c` applied, then
`python3 tools/sched_solver/perturb.py ... --func func_80045294 --pass 2 --block 0
--goal-before 14:22 --depth 2 --max 60` and the same for `--pass 1`. Report:
`tmp/grind/func_80045294/s57/sched_report.txt`.

**Measured on.** HEAD 2026-09-03 chassis, H (v1-first) geometry at 2/83, zero FAKE
constructs present.

**predicate_cite:** `tools/gcc-2.7.2/sched.c:2464`

### Where this leaves the search

Both modelled layers are now typed and closed for this function: RA is at its
target (s57-H1, class, global.c:1275), and the scheduler cannot be perturbed into
the target order without the source reorder that costs the operand (s57-H4, class,
sched.c:2464, corroborating s56-H1 from an independent exhaustive enumeration).
The solver suite has nothing left to search here, and the entire remaining residual
lives in the one pass s52-s56 already named: cse's `canon_reg` substitution of
block 0's shift operand. The two live routes are unchanged (an extended-basic-block
split that survives loop.c, or creation of the shift after cse2), and s57 adds one
negative constraint to both: whatever route is taken must NOT change block 0's
statement order, because the sched model shows that order is the only
scheduler-visible lever and it is already spent at its best value.

## [s57] Global register allocation on the candidate chassis already produces the target's exact per-pseudo disposition, so no allocation reaches the one differing instruction.
- mechanism: global.c:1275 assigns exactly one hard register per pseudo for the whole function (`reg_renumber[allocno_reg[allocno]] = best_reg;`). The residual needs one reference of pseudo 75 to print $s2 while seventeen other references of the same pseudo keep printing $s0; a per-reference hard register does not exist in this allocator.
- probe: python3 tools/ra_solver/extract.py func_80045294 text1a_c (model from the candidate body applied to src), full normalized_insns diff of tmp/sandbox/func_80045294/text1a_c.o against build/src/text1a_c.o, and python3 tools/ra_solver/inverse.py global --goal (75->18, 72->16) --depth 2.
- result: dispositions {72:$s2, 73:$s3, 74:$s1, 75:$s0, 76:$v1, 77:$s4, 79:$a1, 80:$s5, 85:$s1, 91:$v0}; exactly one differing instruction (build idx 9, ours `sll v1,s0,0x4` against target `sll v1,s2,0x4`); the $s0/$s2 swap the naive goal asks for is reachable in 54 two-atom vectors but would break the eighteen matching instructions at idx 1,2,7,8,18,24,26,38,39,41,44,46,49,61,63,64,77,79. RA is at target.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, candidate.c a0-as-pointer i-first geometry at 1/83, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/global.c:1275

## [s57] tools/ra_solver/inverse_compose.py classify types this residual as RA, and the verdict is wrong because register blanking cannot distinguish a different pseudo from a different hard register.
- mechanism: classify separates PRE-RA from RA by comparing register-blanked instruction multisets. Blanking turns `sll v1,s0,0x4` and `sll v1,s2,0x4` into the same token, so a residual in which our pre-RA RTL simply NAMES a different pseudo (our `(ashift (reg 75))` against the target's `(ashift (reg 72))`) is indistinguishable from one in which the allocator assigned a fixed pseudo differently, and the tool always resolves that ambiguity toward RA.
- probe: python3 tools/ra_solver/inverse_compose.py classify text1a_c func_80045294 --target-object build/src/text1a_c.o --ours-object tmp/sandbox/func_80045294/text1a_c.o, cross-checked against the extracted RA model's dispositions.
- result: classify printed "FIRST DIVERGENCE: RA / next tool: tools/ra_solver/inverse.py" while the RA model is provably already at the target (s57-H1). Every cse/canon_reg operand-substitution residual in this project will be mis-typed the same way. Disambiguator: check whether the divergent register is already correct on the pseudo's OTHER references; if it is, the layer is PRE-RA. Unlike the func_80017848 s25 hazard this one survives the object path, so the 2026-08-30 --target-object escape does not fix it.
- verdict: CONFIRMED

## [s57] Form A's eleven-point residual contains no register-allocation difference: A produces the identical per-pseudo disposition to candidate.c and to the target, so its loop-2 register swap is a pre-RA value-to-pseudo assignment decided by A's variable sharing.
- mechanism: A reuses loop 1's counter as loop 2's byte offset and introduces a separate loop-2 counter, so pseudo 75 ($s0) carries the offset and $s1 the counter; the target has pseudo 75 carrying both loop counters and $s1 shared between sum and the offset. The allocator's map is the same in both builds; only which value each pseudo holds differs.
- probe: Applied tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c, `sandbox func_80045294 --disable all`, full normalized_insns diff, then python3 tools/ra_solver/extract.py func_80045294 text1a_c --out tmp/grind/func_80045294/s57/modelA.json and a field comparison of `dispositions` against the candidate's model.json.
- result: A = 11/83, build_insns 83; differing indices 18, 38, 39, 41, 45, 51, 57, 61, 63, 64, 66; dispositions identical to candidate.c's in all 24 entries. Two of the eleven (idx 61, 66) are merely the source order of loop 2's two increments. s56 frontier item 3 is refuted: there is no allocator lever, and adopting the target's sharing is exactly what re-crowns i and destroys A's block 0.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, form A geometry at 11/83, zero FAKE constructs present

## [s57] On the v1-first (H) geometry no perturbation of the scheduler's inputs reaches copy-before-shift in block 0 unless it reorders the source statements: all 40 depth-2 goal-reaching vectors are either a luid reorder or a dependence edge running backwards against LUID.
- mechanism: tools/sched_solver reproduces sched.c's list scheduler order- and clock-exactly (parity=True on this TU: 154 funcs, 560 blocks, 3618 picks). In H's pass-2 block 0 the shift is uid 14 (LUID 15, priority 1, cost 1) and the copy uid 22 (LUID 16, priority 1), both with the single predecessor uid 4 (`move s2,a0`), so rank_for_schedule falls through to the INSN_LUID tiebreak at sched.c:2464. Raising the copy above the shift needs a new dependence edge, and every such edge the enumeration finds has its predecessor at a HIGHER LUID than its successor, which no C can spell.
- probe: python3 tools/sched_solver/extract.py text1a_c --out tmp/grind/func_80045294/s57/schedH.json with H applied, then python3 tools/sched_solver/perturb.py --func func_80045294 --pass 2 --block 0 --goal-before 14:22 --depth 2 --max 60 (960 single atoms plus pairs over add_dep/del_dep/luid/luid_move/cost/unit), and the same for --pass 1.
- result: pass 2 returns 40 vectors, pass 1 returns 12; the only physically spellable class is `luid swap 14 <-> 22` / `luid_move`, which IS the i-first geometry (measured 1/83 with the wrong shift operand). Every add_dep vector (6<-22, 12<-22, 14<-22, 14<-25, 14<-31, 205<-22, 209<-22 and the rest) requires a predecessor of higher LUID than its successor. Independent exhaustive corroboration of the s56-H1 class kill.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, H (v1-first) geometry at 2/83, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s58] Global register allocation on the candidate chassis already produces the target's exact per-pseudo disposition, so the one differing instruction is not reachable by any allocation: it needs one reference of pseudo 75 to print $s2 while seventeen other references of the same pseudo keep printing $s0.
- mechanism: global.c:1275 is `reg_renumber[allocno_reg[allocno]] = best_reg;` - exactly one hard register per pseudo, function-wide. A per-reference hard register does not exist in this allocator, so an operand divergence on one reference of a pseudo that is correct on all its other references is outside the RA model's reachable set by construction.
- probe: python3 tools/ra_solver/extract.py func_80045294 text1a_c --out tmp/grind/func_80045294/s57/model.json (model derived from the CANDIDATE body applied to src, per solver operational rule 2); full instruction-level diff of tmp/sandbox/func_80045294/text1a_c.o against build/src/text1a_c.o via engine.score.normalized_insns (tmp/grind/func_80045294/s57/streams.py -> hon.txt, tgt.txt); then python3 tools/ra_solver/inverse.py global tmp/grind/func_80045294/s57/model.json --goal '{"75": 18, "72": 16}' --depth 2.
- result: Dispositions: 72:$s2 73:$s3 74:$s1 75:$s0 76:$v1 77:$s4 79:$a1 80:$s5 85:$s1 91:$v0 - exactly what the target's 82 matching instructions require. Exactly one differing instruction: build idx 9, ours `sll v1,s0,0x4` against target `sll v1,s2,0x4`. inverse.py does return 54 two-atom vectors for the naive $s0/$s2 swap goal, but the goal itself is wrong: the swap renames both registers function-wide and would break the eighteen currently matching instructions at build idx 1, 2, 7, 8, 18, 24, 26, 38, 39, 41, 44, 46, 49, 61, 63, 64, 77 and 79. It also reported 20 preference atoms FORECLOSED because $s0 and $s2 are callee-saved and can never appear as hard regs in pre-RA RTL from any C. The RA backend has nothing to search on this function.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, memory/grind/func_80045294/candidate.c (a0-as-pointer i-first geometry) applied at src/text1a_c.c:1445, sandbox score 1 / target_insns 83 / build_insns 83, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/global.c:1275

## [s58] tools/ra_solver/inverse_compose.py classify types this residual as RA and the verdict is wrong, because register blanking cannot distinguish 'our pre-RA RTL names a different PSEUDO' from 'the allocator chose a different hard register for the same pseudo'.
- mechanism: classify separates PRE-RA from RA by comparing REGISTER-BLANKED instruction multisets. Blanking turns `sll v1,s0,0x4` and `sll v1,s2,0x4` into the same token, the multisets match, and the tool reports 'same instructions, different registers'. Any cse/canon_reg operand substitution therefore presents identically to a genuine allocation divergence, and the ambiguity is always resolved toward RA.
- probe: python3 tools/ra_solver/inverse_compose.py classify text1a_c func_80045294 --target-object build/src/text1a_c.o --ours-object tmp/sandbox/func_80045294/text1a_c.o, cross-checked against the extracted RA model's dispositions and against the full 83-instruction diff.
- result: classify printed 'FIRST DIVERGENCE: RA / next tool: tools/ra_solver/inverse.py (global / local) / ours: sll v1,s0,0x4 / target: sll v1,s2,0x4' while the RA model is provably already at the target. Every cse/canon_reg operand-substitution residual in this project will be mis-typed the same way. Cheap disambiguator for future sessions: when classify says RA on a one- or two-instruction residual, extract the RA model and check whether the divergent register is already correct on that pseudo's OTHER references - if it is, the layer is PRE-RA. Unlike the func_80017848 s25 hazard (a text-path artifact) this one survives the object path, so the 2026-08-30 --target-object escape does not fix it.
- verdict: CONFIRMED

## [s58] Form A's eleven-point residual contains no register-allocation difference at all: form A produces the identical per-pseudo disposition to candidate.c and to the target, so its loop-2 register naming is a pre-RA value-to-pseudo assignment decided by A's variable sharing.
- mechanism: Form A reuses loop 1's counter as loop 2's byte offset and introduces a separate loop-2 counter, so pseudo 75 ($s0) carries the offset and $s1 the counter; the target has pseudo 75 carrying both loop counters and $s1 shared between sum and the offset. The allocator's map is the same in both builds; only which value each pseudo holds differs, and that is fixed before RA runs.
- probe: Applied tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c at src/text1a_c.c:1445, `& tools/wteng.ps1 main sandbox func_80045294 --disable all`, full normalized_insns diff, then python3 tools/ra_solver/extract.py func_80045294 text1a_c --out tmp/grind/func_80045294/s57/modelA.json and a field-by-field comparison of `dispositions` against the candidate's model.json.
- result: A = score 11, build_insns 83; differing build indices 18, 38, 39, 41, 45, 51, 57, 61, 63, 64, 66; dispositions identical to candidate.c's in all 24 entries. Two of the eleven points (idx 61 and 66) are merely loop 2's two increments in the other source order. This refutes s56 frontier item 3: there is no allocator lever to pull, and adopting the target's variable sharing is exactly what re-crowns i and destroys A's block 0. Form A and candidate.c are the two endpoints of one pre-RA trade, not two independent one-instruction attack surfaces.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-03 chassis, form A geometry (tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c) at score 11 / 83 insns, zero FAKE constructs present

## [s58] On the v1-first (H) geometry no perturbation of the scheduler's inputs reaches copy-before-shift in block 0 unless it reorders the source statements: all 40 depth-2 goal-reaching vectors at pass 2 (and all 12 at pass 1) are either a luid reorder or a dependence edge whose added predecessor has a higher LUID than its successor.
- mechanism: tools/sched_solver reproduces sched.c's list scheduler order- and clock-exactly and reported parity=True on this TU (154 functions, 560 blocks, 3618 picks). In H's pass-2 block 0 the shift is uid 14 (LUID 15, priority 1, cost 1) and the copy is uid 22 (LUID 16, priority 1), both with the single predecessor uid 4 (`move s2,a0`), so rank_for_schedule falls through to the INSN_LUID(tmp) - INSN_LUID(tmp2) tiebreak at sched.c:2464 and preserves RTL order. Raising the copy above the shift requires a new dependence edge, and every edge the enumeration finds runs backwards against LUID and so cannot be built in the RTL order it would apply to.
- probe: Applied tmp/grind/func_80045294/s53/H_a0ptr_vfirst.c, python3 tools/sched_solver/extract.py text1a_c --out tmp/grind/func_80045294/s57/schedH.json, then python3 tools/sched_solver/perturb.py tmp/grind/func_80045294/s57/schedH.json --func func_80045294 --pass 2 --block 0 --goal-before 14:22 --depth 2 --max 60 (960 single atoms plus pairs over the add_dep / del_dep / luid / luid_move / cost / unit classes), and the same for --pass 1.
- result: Pass 2 returns 40 vectors, pass 1 returns 12. The only physically spellable class is `luid swap 14 <-> 22` / `luid_move 22 -> before 14`, which IS the i-first geometry - already measured at 1/83 with the shift reading the wrong operand. Every add_dep vector (6<-22, 12<-22, 14<-22, 14<-25, 14<-31, 205<-22, 209<-22 and the rest) needs a predecessor of higher LUID than its successor. This is an independent, exhaustive corroboration of the s56-H1 class kill that does not depend on reading priorities out of a dump, and it adds a standing constraint on both live routes: whatever closes block 0 must not change block 0's statement order, because statement order is the only scheduler-visible lever for the copy/shift pair and candidate.c already has it at its best value.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, H (v1-first) geometry at score 2 / 83 insns, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s59] forensics — instrumented-cc1 pass attribution on the crown/guard/allocation trilemma

Modality: forensics (instrumented cc1 `-da` dumps, per-pass RTL slices, loop.c
movable report). Chassis re-verified at dispatch: `memory/grind/func_80045294/candidate.c`
pasted over `src/text1a_c.c:1445` measures `sandbox func_80045294 --disable all`
= **score 1, target_insns 83, build_insns 83**; residual is instruction idx 9 only
(target `sll $v1,$s2,4` = reg 72 / a0, build `sll $v1,$s0,4` = reg 75 / i).

Three forms were compiled with the instrumented cc1 (tools/gcc-2.7.2/cc1) and
sliced per pass into `tmp/grind/func_80045294/s59/dumps_{C,J,L}/fn.<pass>`:
  C = memory/grind/func_80045294/candidate.c (re-measured 1 / 83)
  J = rejected/s52-a0ptr-separate-loop2-counter-collapse-80.c (re-measured 30 / 80)
  L = rejected/s56-loop-label-split-undone-by-licm-hoist.c (the do-while label carrier, 4 / 86)

## [s59] Every distinct-loop-2-counter form loses exactly three instructions because the loop-1 counter pseudo stops crossing a call, so global.c's find_reg takes the call-clobbered path and gives it $a0, which frees a callee-saved pair and turns block 0's copy into a redundant reload of the incoming argument register.
- mechanism: `find_reg` (global.c:963-975) selects the initial `used1` set from `allocno_calls_crossed[allocno]`: a pseudo that crosses no call gets `fixed_reg_set` only (global.c:972), so the whole call-clobbered file is available, and since MIPS defines no `REG_ALLOC_ORDER` the default order 0,1,2,...,31 hands it `$4`/`$a0` long before any `$s` register. In form J reg 75 (the loop-1 counter) is dead after loop 1, so it is allocated `$4`; the greg dump shows `(insn 15 ... (set (reg/v:SI 4 a0) (reg/v:SI 18 s2)))` where form C shows `(set (reg/v:SI 16 s0) (reg/v:SI 18 s2))`. Because `$a0` still holds the incoming parameter at that point the copy is redundant and it is gone by the jump2 dump. Net: minus one copy insn, and one fewer callee-saved register in the frame (J emits `regs= 6/0`, five `$s` registers, against the target's six), i.e. minus two more for the missing sw/lw pair.
- probe: applied J over src/text1a_c.c:1445, `sandbox func_80045294 --disable all` gives score 30 / build_insns 80 / target_insns 83 (re-confirmed on the HEAD 2026-09-03 chassis; the s48-era measurement stands); dumped with tmp/grind/func_80045294/s59/dump.sh and compared `fn.greg` and `fn.jump2` against form C's.
- result: the collapse is fully attributed for the first time — earlier sessions recorded only "alloc collapse". It also fixes the price of the distinct-counter family: it is not a spelling accident but a consequence of `allocno_calls_crossed == 0`, so the only way to run a distinct loop-2 counter at 83 instructions is to give the loop-1 counter pseudo a genuine read after `func_800520B8` and before a0's last mention. The single such role available in this function is loop 2's `idx`, which is form A (`tmp/grind/func_80045294/s53/A_reuse_i_as_idx.c`, 11 / 83) — and form A then cannot satisfy the target's register layout, because the target puts the loop-1 counter in $s0 and `idx` in $s1 while form A shares one pseudo between the two roles.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer geometry, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/global.c:972

## [s59] A CODE_LABEL placed between block 0's copy and its shift makes the block-0 shift AND the loop-1 entry guard print the target's operands at the same time, so the s53 "two uses of one cse quantity need opposite registers" constraint is escapable rather than closed.
- mechanism: `cse_end_of_basic_block` (tools/gcc-2.7.2/cse.c, the `while (p && GET_CODE (p) != CODE_LABEL)` scan) terminates the extended basic block at the label, so the quantity created by `i = a0` (make_regs_eqv, cse.c:826-857) lives only in the FIRST extended block. Both consumers — the shift `v1 = a0 << 4` and the entry guard `i < count` — then sit in the SECOND extended block with an empty table, where `canon_reg` (cse.c:2557-2572) has no `qty_first_reg` to substitute and each insn keeps the pseudo the RTL generator gave it. This is why the crown choice stops mattering: the fix is not to win the crown, it is to make neither consumer see the class. Every previous session framed the label as a device for the shift alone and treated the guard as a separate, opposed requirement.
- probe: compiled form L with the instrumented cc1 and read `tmp/grind/func_80045294/s59/dumps_L/fn.cse`.
- result: at cse1 form L prints `(ashift:SI (reg/v:SI 72) (const_int 4))` for block 0's shift AND `(lt:SI (reg/v:SI 75) (reg/v:SI 78))` for the loop-1 entry guard — reg 72 = a0 and reg 75 = i, exactly the target's `sll $v1,$s2,4` / `slt $v0,$s0,$a0` pair. No other form in 59 sessions has had both right at once (form A gets the shift and loses the guard; candidate.c gets the guard and loses the shift). What still undoes it is downstream, in two steps: `fn.loop` shows LICM hoisting the shift out of the carrier as a fresh insn (uid `-1`) in the preheader still reading reg 72, and `fn.cse2` then rewrites that preheader insn to reg 75 because the hoisted insn is back in the first extended block.
- verdict: CONFIRMED

## [s59] The complete set of extended-basic-block terminators cse honours is CODE_LABEL, NOTE_INSN_LOOP_END (first pass only) and NOTE_INSN_SETJMP, so the only byte-free splitter reachable from C source would need a setjmp call, and every other split must be paid for with a real branch.
- mechanism: `cse_end_of_basic_block` scans `while (p && GET_CODE (p) != CODE_LABEL)` and breaks early on exactly two notes: `NOTE_INSN_LOOP_END` guarded by `! after_loop` (so cse2, invoked with after_loop = 1 at toplev.c:2926, ignores it — the s55 finding, now re-derived from the predicate itself rather than by measurement) and `NOTE_INSN_SETJMP`. Nothing else ends the block — not NOTE_INSN_LOOP_BEG, not NOTE_INSN_BLOCK_BEG/END, not a CALL_INSN (calls are handled by invalidate_for_call without ending the block).
- probe: read tools/gcc-2.7.2/cse.c `cse_end_of_basic_block` end to end and cross-checked against the three dump sets.
- result: this closes by predicate the "is there a zero-byte EBB splitter" question that s55/s56 left open by enumeration. A surviving CODE_LABEL is the only device, and a CODE_LABEL needs a live branch reference at cse time or jump_optimize deletes it before cse ever sees it. The route that remains is a carrier whose branch survives cse1 AND cse2 but is folded away by a LATER pass — combine's `nonzero_bits` can prove `(and (ashift x 4) 15) == 0`, and the post-combine jump pass then deletes the unreferenced label — which would leave the split in place for both cse passes at zero net instructions.
- verdict: CONFIRMED

## [s59] loop.c moves the carrier's shift on the ordinary movable path with an inequality satisfied by a factor of 1600, so the only C-level lever that can stop the hoist is `n_times_set` — a second, non-invariant store to the shift's destination inside the carrier.
- mechanism: `tools/loop_movables.py` reports the carrier loop (insns 17..33 in form L) as `insn_count=3 loop_has_call=False threshold=122`, with insn 21 (regno 76 = v1, the shift) scoring `122*1*40 = 4880 >= 3` and insn 26 (regno 80) scoring `119*1*1 = 119 >= 3`; both are `moved` on the middle disjunct of loop.c:1626-1631. savings, lifetime, insn_count and threshold are therefore all hopeless levers here (halving the threshold by putting a call in the carrier still leaves 61 >= 3). `scan_loop` admits a movable only when `n_times_set[dest] == 1` OR `consec_sets_invariant_p` succeeds (loop.c:705-709, a disjunct), so the movable can only be removed by storing to v1 twice inside the carrier with a set-chain that is NOT wholly invariant.
- probe: `python3 tools/loop_movables.py --func func_80045294 --file text1a_c --dumps tmp/grind/func_80045294/s59/dumps_L`, banked at tmp/grind/func_80045294/s59/loop_movables_L.txt.
- result: the LICM half of the label route now has a named single lever instead of a general "defeat LICM" goal. Note the trap this creates: a second store whose source is itself loop-invariant (a constant, a global load, another invariant local) satisfies `consec_sets_invariant_p` and the movable survives, so the second store must read something the carrier itself writes.
- verdict: CONFIRMED

## [s59] A CODE_LABEL placed between block 0's copy and its shift makes the block-0 shift AND the loop-1 entry guard print the target's operands at the same time in the cse1 dump, because both consumers land in an extended basic block whose table never saw the i = a0 quantity.
- mechanism: cse_end_of_basic_block (tools/gcc-2.7.2/cse.c, the `while (p && GET_CODE (p) != CODE_LABEL)` scan) ends the extended basic block at the label, so the quantity created by make_regs_eqv (cse.c:826-857) on `i = a0` lives only in the FIRST extended block. canon_reg (cse.c:2557-2572) then has no qty_first_reg to substitute for either the shift `v1 = a0 << 4` or the entry guard `i < count`, and each insn keeps the pseudo the RTL generator gave it. The crown choice, which 55 sessions treated as the battleground, stops mattering entirely.
- probe: Compiled rejected/s56-loop-label-split-undone-by-licm-hoist.c with the instrumented cc1 (tools/gcc-2.7.2/cc1, -da) and read the per-function cse1 slice tmp/grind/func_80045294/s59/dumps_L/fn.cse.
- result: fn.cse line 44 prints `(ashift:SI (reg/v:SI 72) (const_int 4))` for block 0's shift — reg 72 is the parameter a0, i.e. the target's `sll $v1,$s2,4` — and line 79 prints `(lt:SI (reg/v:SI 75) (reg/v:SI 78))` for the loop-1 entry guard — reg 75 is i, i.e. the target's `slt $v0,$s0,$a0`. No other form in 59 sessions has had both right at once: form A wins the shift and loses the guard, candidate.c wins the guard and loses the shift. The form still measures 4/86 because the split is undone downstream in two named steps (LICM hoist, then cse2 rewrite of the hoisted insn), not because the device fails.
- verdict: CONFIRMED

## [s59] Every distinct-loop-2-counter form loses exactly three instructions because the loop-1 counter pseudo crosses no call, so global.c's find_reg takes the call-clobbered path, gives it $a0, frees a callee-saved pair, and turns block 0's copy into a redundant reload of the incoming argument register that is gone by jump2.
- mechanism: find_reg selects its initial `used1` set from allocno_calls_crossed: a pseudo crossing no call gets fixed_reg_set only (global.c:972), so the whole call-clobbered file is available, and because MIPS defines no REG_ALLOC_ORDER the default 0,1,2,...,31 order reaches $4/$a0 long before any $s register. With reg 75 in $4 the block-0 copy becomes `$a0 = $s2`, redundant against the incoming parameter, and one fewer callee-saved register is needed.
- probe: Re-applied rejected/s52-a0ptr-separate-loop2-counter-collapse-80.c over src/text1a_c.c:1445 on today's chassis (sandbox --disable all = 30 / build_insns 80 / target_insns 83), dumped it with the instrumented cc1, and compared tmp/grind/func_80045294/s59/dumps_J/fn.greg and fn.jump2 against dumps_C's.
- result: dumps_J/fn.greg shows `(insn 15 12 17 (set (reg/v:SI 4 a0) (reg/v:SI 18 s2)))` where dumps_C/fn.greg shows `(set (reg/v:SI 16 s0) (reg/v:SI 18 s2))`; insn 15 is absent from dumps_J/fn.jump2; the emitted frame is `regs= 6/0` (five $s registers plus $ra) against the target's six $s registers. That is exactly the 3-instruction deficit (one copy plus one sw/lw pair). The distinct-counter family therefore cannot run at 83 instructions unless the loop-1 counter pseudo has a genuine read after func_800520B8 and before a0's last mention; the only such role in this function is loop 2's idx, which is form A, and form A then cannot match the target's layout because the target holds the loop-1 counter in $s0 and idx in $s1 while form A shares one pseudo between the two roles.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-03 chassis, a0-as-pointer geometry, zero FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/global.c:972

## [s59] cse honours exactly three extended-basic-block terminators — a CODE_LABEL, a NOTE_INSN_LOOP_END while after_loop is zero, and a NOTE_INSN_SETJMP — so a surviving CODE_LABEL is the only splitter that works in both cse passes and it always needs a live branch reference at cse time.
- mechanism: cse_end_of_basic_block scans `while (p && GET_CODE (p) != CODE_LABEL)` and breaks early only on those two notes; calls are handled by invalidate_for_call without ending the block, and NOTE_INSN_LOOP_BEG / NOTE_INSN_BLOCK_BEG / NOTE_INSN_BLOCK_END are not terminators. cse2 is invoked with after_loop = 1 (toplev.c:2926), which is why every NOTE-based split measured in s55 was re-merged.
- probe: Read tools/gcc-2.7.2/cse.c cse_end_of_basic_block end to end and cross-checked the prediction against the cse1/cse2 slices of all three forms dumped this session.
- result: The 'is there a zero-byte extended-block splitter' question, left open by enumeration in s55/s56, is now closed by the predicate itself: there is none reachable from C without a setjmp call. Any usable split must be paid for with a real branch at cse time, which leaves exactly one unexplored shape — a carrier whose branch survives cse1 and cse2 but is folded away by a later pass (combine's nonzero_bits can prove `(and (ashift x 4) 15) == 0`, and the post-combine jump pass then deletes the unreferenced label), so the split is present for both cse passes at zero net instructions.
- verdict: CONFIRMED

## [s59] loop.c moves the label carrier's shift on the ordinary movable path with the loop.c:1631 inequality satisfied by a factor of about 1600, so savings, lifetime, insn_count and threshold are all inert and n_times_set is the only remaining C-level lever against the hoist.
- mechanism: scan_loop admits a movable only when n_times_set[dest] == 1 or consec_sets_invariant_p succeeds (loop.c:705-709, a disjunct). The middle disjunct of loop.c:1626-1631 (threshold * savings * lifetime >= insn_count) is what moves the shift here, and the carrier's tiny insn_count makes it unbeatable through the other three terms.
- probe: python3 tools/loop_movables.py --func func_80045294 --file text1a_c --dumps tmp/grind/func_80045294/s59/dumps_L, banked at tmp/grind/func_80045294/s59/loop_movables_L.txt.
- result: Carrier loop insns 17..33: insn_count=3, loop_has_call=False, threshold=122; insn 21 (regno 76, the shift) scores 122*1*40 = 4880 >= 3 and insn 26 (regno 80) scores 119*1*1 = 119 >= 3, both `moved`. Putting a call in the carrier only halves the threshold to 61, still far above 3. The lever that survives is a second store to v1 inside the carrier whose consecutive set-chain is NOT itself invariant — a second store sourced from a constant, a global load or another invariant local satisfies consec_sets_invariant_p and the movable survives, so the second store must read a value the carrier itself writes.
- verdict: CONFIRMED
