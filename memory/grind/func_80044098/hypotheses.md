# Hypothesis ledger — func_80044098

## s1 (recon, 2026-07-27)
- H-s1-1 "split counter at the decrement lowers counter pri below pointer" — KILLED.
  Probe: `cnt = a4 - 1;` separate var, sandbox. Result: 17 (worse), combine guard-fold
  `beq a4,$0` + dec in delay slot + frame adjust. Mechanism generalizes to all
  counter-chain splits at/after the mask (see evidence.md guard-fold constraint).

## Live frontier (end of s1)
1. Byte-neutral pointer-ref lift (+2/+3 flow-time refs on the pointer pseudo):
   pre-combine RTL spellings of the v1-addressed accesses (integer-cast addressing
   `*(s32*)((s32)v1 - 4)`, compound/memory-RMW forms, re-load-after-store forms that
   CSE collapses) which combine folds back to the exact target insns while flow has
   already counted the extra refs. Verify each variant with BB2_ALLOC_DEBUG
   (need pointer pri ≥26470 solo, or ≥24705 with a load/test-only hdr split —
   NB v5 showed the hdr split alone perturbs allocation; re-test combined).
2. Determine whether reg_n_refs/livelen are frozen at flow time or updated by
   combine in this fork (read combine.c/flow.c reg_n_refs handling). If frozen,
   axis 1 widens to ANY combine-folded decoration; do this read FIRST to guide 1.
3. Directed permuter campaign from candidate.c (single tied-rename class,
   better-than-average permuter candidate per s0 notes; PERM sweeps over
   addressing spellings + statement order; fresh-seed discipline).

## [s1] Splitting the counter at the decrement (cnt = a4 - 1 as its own variable) drops the counter's allocno priority below the pointer's, flipping the v1/a0 assignment
- mechanism: global.c allocno priority = floor_log2(refs)*refs/livelen; the split moves 6 in-loop-weighted refs off the header variable
- probe: edited src to the split form, sandbox func_80044098 --disable all
- result: score 17 (floor 13), build 25 insns: combine folded the loop guard to beq a4,$0 with the decrement in the delay slot and a $sp frame adjust appeared; pre-decrement value live in a separate pseudo enables the fold. Generalizes to all counter-chain splits at/after the mask; retro-explains the s0 v5 rejection's beqz. Counter must stay ONE pseudo from mask through loop (min 11 refs, pri ~24400), so counter-side reduction alone can never undercut the pointer's 21176.
- verdict: KILLED
