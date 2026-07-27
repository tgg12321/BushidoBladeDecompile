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

## s2 (structural, 2026-07-27)
- H-s2-1 "a same-path pre-combine spelling can lift pointer flow refs byte-neutrally" — KILLED.
  Probe: 8 spelling variants under BB2_ALLOC_DEBUG. Result: pointer 12 refs/21176 in all;
  cse+cse2 run pre-flow and the function is one fall-through EBB — nothing survives to flow.
- H-s2-2 "reg_n_refs/live_length are frozen at flow time in this fork" — CONFIRMED.
  Probe: combine.c:52-57 + 2306-2337 read; mini_pb empirical (16 refs counted post-fold).
- H-s2-3 "hand-peeled first iteration + hdr load/test split flips pointer/counter" — CONFIRMED.
  Probe: pK in src, sandbox. Result: 13 -> 6; pointer $v1, counter $a0; residual = const
  unification + a6/$a1 swap.
- H-s2-4 "in-arm m2=-1 const-holder restores the v0/a1/a2 constellation" — CONFIRMED.
  Probe: pU in src, sandbox. Result: 6 -> 3; all five registers target-correct; residual =
  3-insn peel stub (sched1 li placement blocks full cross-jump merge).
- H-s2-5 "the stub yields to statement order / decl order / register kw / block scope" — KILLED.
  Probe: 12 + 120 + 4 variants. Result: stub invariant in all 136.
- H-s2-6 "do-while(0) around the peel gives a counter-free ref-lift" — KILLED.
  Probe: 3 wrapper placements. Result: counter double-weighted too (flip reverts) + merge lost.

## Live frontier (end of s2) — floor 3, candidate.c = pU form
1. Kill the 3-insn stub: make sched1 emit the peel's li m2,-1 FIRST in the peel block (full
   suffix match -> stub collapses to exactly target's li a1). Mechanism fully characterized;
   levers not yet found: read sched.c launch-before/ready-sort tie-breaks for the exact knob;
   find an in-block li consumer that survives combine; or a real branch that isolates the li
   in its own BB (none available in this CFG so far).
2. Directed permuter campaign from candidate.c (score-3 seed, 29/26): PERM sweeps over peel
   spelling, holder placement/type, loop test forms — mechanical exploration of exactly the
   stub-space. Permuter modality is the natural next rung.
3. pK-track alternative: break the guard/loop const unification in the while-form without a
   holder (cse follows the entry jump only because the test label has LABEL_NUSES==1 —
   find a spelling that bumps the label's uses pre-cse without emitted bytes). If broken,
   pK+two-lis might reach 0 with one variable fewer.

## [s2] A same-path pre-combine spelling can lift pointer flow refs byte-neutrally (frontier-1)
- mechanism: cse+cse2 run before flow; whole function is one fall-through EBB
- probe: 8 spelling variants (cast-addr, split-addr, reload, RMW, decl swap, loop temp, while, lvalue) under BB2_ALLOC_DEBUG
- result: pointer stuck at 12 refs / pri 21176 in all 8; 6 byte-identical-and-inert, 2 byte-diverging
- verdict: KILLED

## [s2] reg_n_refs/live_length are frozen at flow time in this fork (frontier-2)
- mechanism: combine.c:52-57 documents non-adjustment; only a deleted insn's own dead dest is zeroed (2306-2337)
- probe: source read + mini_pb empirical (16 refs counted after combine folded the pair)
- result: confirmed frozen through global alloc
- verdict: CONFIRMED

## [s2] Hand-peeled first iteration + hdr load/test split flips pointer/counter allocation
- mechanism: peel = real-statement duplication counted by flow (+4 pointer refs), cross-jump re-merges; hdr split strips 3 counter refs (peel alone: counter 30909 still wins)
- probe: pK applied to src, sandbox --disable all
- result: 13 -> 6; pointer $v1, counter $a0; residual = const unification + a6 steals $a1
- verdict: CONFIRMED

## [s2] In-arm m2=-1 const-holder restores the v0/a1/a2 constellation
- mechanism: m2 4 refs @ livelen 16 = pri 5000 > a6 3809; guard keeps its own local li -> $v0; holder set before guard fails (cse steal + livelen 28 pri 3571)
- probe: pU applied to src, sandbox --disable all
- result: 6 -> 3; all five registers target-correct; 29/26 insns
- verdict: CONFIRMED

## [s2] The 3-insn peel stub yields to statement order / decl order / register kw / block scope
- mechanism: sched1 backward list scheduling places the no-in-block-consumer li in the peel tail; cross-jump suffix match stops there
- probe: 12 statement orders x 4 m2 positions, 120 decl permutations, register storage class, block-scope decl
- result: stub invariant in all 136 variants; in-block li consumers combine-fold (dependence severed), cross-block ones don't fold (wrong bytes)
- verdict: KILLED

## [s2] do-while(0) around the peel gives a counter-free ref-lift
- mechanism: loop notes double peel refs for BOTH pseudos; counter 35555 back above pointer 34782
- probe: 3 wrapper placements under BB2_ALLOC_DEBUG
- result: flip reverts AND peel stops cross-jump-merging (body emitted twice)
- verdict: KILLED

## s3 (structural, 2026-07-27)
## [s3] The sched1 li-placement stub has an identifiable source-reachable knob (frontier-1)
- mechanism: sched.c adjust_priority -> birthing_insn_p: single-set (reg_n_sets==1) live-dest SETs get LAUNCH_PRIORITY when launched by the block-end jump -> placed at block tail; n_deaths path dead (REG_DEAD notes stripped); the ONLY escape is reg_n_sets[dest] != 1
- probe: sched.c read (adjust_priority ~2552, birthing_insn_p ~2496, LAUNCH_PRIORITY assignment ~3985) + p5_SVA_m0.i.sched (insn 49 = li shows 0x7f000001)
- result: mechanism fully characterized; explains all 136 s2-invariant variants by design
- verdict: CONFIRMED

## [s3] A byte-neutral second set can bump reg_n_sets on a FRESH holder
- mechanism: hoped flow counts sets before dead-deleting them
- probe: flow.c read (final-pass dead insns NOTE'd + `goto flushed` BEFORE mark_set_regs) + pV (trailing dead `m2 = 0;`) + pX (m2 as unused 2nd param, dead entry copy) — both byte-identical to pU
- result: dead sets are deleted-before-counted; live second sets emit bytes; combine-deletable ones are the Judge-banned re-set chain
- verdict: KILLED

## [s3] Borrowing an existing dead local as the holder defeats the boost (staged-value-reused-variable mechanism)
- mechanism: 2nd set on the borrowed host -> birthing_insn_p false -> li placed by LUID first -> cross-jump full suffix match collapses the peel
- probe: pY (`hdr = -1;`, loop tests hdr) — sandbox
- result: 3 @ 26/26; structure+schedule+li placement exactly target; residual = hdr pseudo (load ∪ holder) forced off $a0/$v0 by counter/loop-temp conflicts -> $a1 drags lw dest + 2 andi srcs
- verdict: CONFIRMED

## [s3] Borrowing the test-result local instead reduces the residual to 2
- mechanism: test value touches only andi dest + beqz src in target (2 insns < hdr's 3)
- probe: p3 (explicit `tst = hdr & 0x8000` borrowed as holder) — probe asm + sandbox
- result: 4 (worse); borrow drops tst's pri below hdr's -> hdr allocates first and drifts into $v0 (no short test temp left to conflict it out) -> 4 diffs
- verdict: KILLED

## Live frontier (end of s3) — floor 3, candidate.c = pY (26/26)
1. Permuter campaign from pY (score-3 seed): both KNOWN holder families are proven 3-locked
   (fresh -> boost stub; borrowed -> register drag, host enumeration complete), so the
   campaign's value is discovering an UNKNOWN family/topology, not permuting within these.
2. If the campaign runs dry: this now has the endgame-lock shape — small residual, mechanism
   proven from compiler source, every sanctioned axis measured dead (s1 ref-lift/live-length,
   s2 decorations/dw0/consumer folds, s3 boost-defeat + borrow enumeration + pK-track).
   Disposition per endgame-lock-disposition-policy / 2026-07-27 standing auto-ruling.
3. (weak) Any base-form (do-while) pointer-ref lift that is neither a same-path decoration
   (cse-folds, s2) nor a peel (this family) — no known candidate class remains.

## [s3] The sched1 li-placement stub has an identifiable source-reachable knob
- mechanism: sched.c adjust_priority -> birthing_insn_p: single-set (reg_n_sets==1) live-dest SETs get LAUNCH_PRIORITY when launched by the block-end jump and sink to the block tail; REG_DEAD notes are stripped so the n_deaths path never fires; only escape is reg_n_sets[dest] != 1
- probe: sched.c source read (adjust_priority ~2552, birthing_insn_p ~2496, LAUNCH_PRIORITY ~3985) + s2 dump p5_SVA_m0.i.sched (insn 49 li shows 0x7f000001)
- result: mechanism fully characterized; explains all 136 s2-invariant variants by design
- verdict: CONFIRMED

## [s3] A byte-neutral second set can bump reg_n_sets on a fresh holder
- mechanism: would require flow to count sets before dead-deleting them
- probe: flow.c read (final pass NOTEs dead insns and goto flushed BEFORE mark_set_regs) + pV trailing dead m2=0 + pX m2-as-unused-2nd-param; both compiled and diffed vs pU baseline
- result: both byte-identical to baseline: dead sets deleted-before-counted; live second sets emit bytes; combine-deletable ones are the Judge-banned re-set chain
- verdict: KILLED

## [s3] Borrowing the existing dead hdr local as the holder defeats the boost (staged-value-reused-variable mechanism)
- mechanism: 2nd set on the borrowed host makes birthing_insn_p false; li placed first by LUID; cross-jump full suffix match collapses the peel
- probe: pY applied to src, instrumented cc1 probe + sandbox --disable all
- result: 3 @ 26/26 (was 29/26); structure, schedule and li placement exactly target; residual = merged hdr pseudo forced to $a1 by counter($a0)/loop-temp($v0) conflicts, dragging lw dest + 2 andi srcs
- verdict: CONFIRMED

## [s3] Borrowing the test-result local instead reduces the residual to 2
- mechanism: test value touches only andi dest + beqz src in target (2 insns vs hdr's 3)
- probe: p3 (explicit tst = hdr & 0x8000 borrowed as holder), probe asm + sandbox
- result: 4 (worse): borrowing drops tst's allocno pri (~3571) below hdr's (5714); hdr allocates first and drifts into $v0 because no short-lived test temp remains to conflict it out
- verdict: KILLED

## s4 (permuter, 2026-07-27)
## [s4] A directed permuter campaign from pY finds an unknown holder family/topology outside {fresh holder, borrowed holder}
- mechanism: both known families proven 3-locked (s3 theorem); only an unknown flip/holder topology could beat 3
- probe: pY-reseed-full-window campaign (24,290 iters, 26 min, -j 8) on top of the prior 32k-iter run — ~56k cumulative pY-basin iterations
- result: zero improving finds; the single output was a score-neutral uninitialized-read mutation
- verdict: KILLED

## [s4] The fresh-holder (pU) chassis has an undiscovered sub-3 mutation
- mechanism: permuter mutating holder placement/type/staging might discover a topology outside the s3 enumeration
- probe: pU-fresh-holder-random campaign, 24,074 iters / 26 min, 38 finds harvested
- result: basin converges onto the KNOWN borrow attractor (m2=hdr staging = pY family respell at 15; tst-borrow = p3 family at 25); nothing below 15; all finds inside the s3 3-lock theorem
- verdict: KILLED

## [s4] A base-form (do-while) pointer-ref lift that is neither a decoration nor a peel exists (frontier-3)
- mechanism: would flip pointer/counter without the while-form holder problem
- probe: base-random-classify campaign without stop-on-zero (25,262 iters / 26 min, every 0-find classified) + hand decomposition of the one novel topology (score-10 mask-fold false flip): pB1/pB3/pB4 probes
- result: only 0-attractor is the Judge-banned cancellation pair (2 independent re-finds); the score-10 mask-fold topology is semantics-divergent (unmasked counter) and its only semantics-correct spelling is the s1 guard-fold kill (17 @ 25 insns); split-init a6 and dw0 measured inert alone and combined
- verdict: KILLED

## Live frontier (end of s4) — floor 3, candidate.c = pY (26/26), permuter axis dead
1. NONE grindable. Every sanctioned axis measured dead: s1 ref-lift/counter-split, s2 decorations/dw0/live-length/consumer-folds, s3 boost-defeat/borrow-enumeration/pK-track, s4 permuter (4 basins, ~105k iterations). Disposition: OWNER-ESCALATION filed 2026-07-27 (docs/grind/decisions.md) per the standing both-gates-fail auto-ruling; terminal park as OWNER-ACCEPTED INCOMPLETE, re-attempt eligible.
2. ONLY reopening path (per the Judge constraint): an actually-exhibited SOTN-master (or VS/ESA) file+function citation of the adjacent-cancellation shape — gate-(ii) owner escalation.

## [s4] A directed permuter campaign from pY finds an unknown holder family/topology outside {fresh holder, borrowed holder}
- mechanism: both known families proven 3-locked by the s3 theorem; only an unknown flip/holder topology could beat 3
- probe: pY-reseed-full-window campaign, 24,290 iters / 26 min / -j 8, on top of the prior 32k-iter run (~56k cumulative on the basin), harvested + stopped in-session
- result: zero improving finds; single output was a score-neutral uninitialized-read mutation
- verdict: KILLED

## [s4] The fresh-holder pU chassis (29/26) has an undiscovered sub-3 mutation
- mechanism: random mutation of holder placement/type/staging could discover a topology outside the s3 borrow-host enumeration
- probe: pU-fresh-holder-random campaign, 24,074 iters / 26 min, 38 finds harvested and classified
- result: basin converges onto the known borrow attractor (m2=hdr staging = pY-family respell at 15; tst-borrow = p3 family at 25); nothing below 15; all finds inside the s3 3-lock theorem
- verdict: KILLED

## [s4] A base-form (do-while) pointer-ref lift that is neither a same-path decoration nor a peel exists (ledger frontier-3)
- mechanism: would flip pointer/counter without the while-form holder problem
- probe: base-random-classify campaign WITHOUT stop-on-zero (25,262 iters / 26 min, every 0-find classified) + hand decomposition of the one novel topology (score-10 mask-fold) via pB1/pB3/pB4 workspace probes
- result: only 0-attractor is the Judge-banned cancellation-pair class (2 independent re-finds, output-0-1/output-0-2); the score-10 mask-fold topology is semantics-divergent (counter runs unmasked, 0x8000 extra iterations; mask temp in $v0) and its only semantics-correct spelling is the s1 guard-fold kill (17 @ 25 insns); a6 split-init and dw0 measured inert alone and combined (26 stripped-diff lines = base)
- verdict: KILLED
