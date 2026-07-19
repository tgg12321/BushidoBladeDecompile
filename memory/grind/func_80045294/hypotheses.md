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
