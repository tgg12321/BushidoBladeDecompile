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
