# Hypothesis ledger — func_80048530

## CONFIRMED
- H1 (s1): sibling func_800483DC base-routing idiom (mutate arg0 as walker +
  separate `base=arg0` local for final add) -> floor 10, from 11. Pure C.

## KILLED
- H2 (s1): give c,d the target's lhu+sll+sra shape via value spelling (either
  `(s16)c` at call or `((s32)(c<<16))>>16`) -> score 22. Register-scratch
  collision with v1-walker; NOT fixable by respelling c/d. Disproves the WIP
  conjecture that the c/d cascade would vanish once base routing was correct.
- H3 (s2): defeat the lh-fold by loading c,d as u16 and sign-extending IN PLACE
  (single reused pseudo, reassignment) to avoid the symmetric-form cascade ->
  combine RE-FOLDS to lh, score 10. The lh-fold is un-defeatable for any
  single-use c/d. (rejected/inplace-shift-refolds-lh.c)
- H4 (s2, F1 re-examined): "make the walker dead-reusable at the d-load" is
  ALREADY TRUE in floor-10 — RTL shows `lh v1,2(v1)` reuses the dead walker,
  identical routing to target's `lhu v1,2(v1)`. F1 is not the lever; the gap is
  PURELY lh (1 insn) vs lhu+sll+sra (3 insns) for c,d.
- H5 (s2, F3): permuter campaign from floor-10, 10,593 iters, --stop-on-zero,
  no score-0. Candidates are floor-10-equivalent or semantic-breakers. Metric
  mis-aligned (identical source: 1020 base vs 750 output). Random mutation does
  not crack the combine-fold RA tie.

## THE WALL (s2, RTL-confirmed)
combine folds `sign_extend(mem:HI)` -> lh for single-use offset-0 loads. c,d
(stack args) always fold; a,b (register args) escape because ABI arg-setup
separates their load from their (s16) use. Forcing c,d to lhu+sll+sra requires
the symmetric `(s16)`-at-call form, which reverses c/d evaluation order and
evicts the walker v1->t0 (cascade, score 22). No pure-C spelling reconciles
"c,d as lhu+sll+sra" with "walker stays in v1". Remaining unexplored: a C
structure that separates c,d's load from their sext the way ABI arg-setup does
for a,b (e.g. forcing c,d through a register-resident intermediate before the
stack store) WITHOUT introducing a dead store.

## LIVE FRONTIER (see outcome frontier)
- F1: make the walker register DEAD-and-reusable at the d-load so GCC reuses it
  as the d sign-extend scratch (mirror target's `lhu v0,0(v1); lhu v1,2(v1)`).
  The walker's last live use must be the d-load itself; nothing (incl.
  `entry += base`) may reference the walker after. Probe: move `entry += base`
  and the `arg0 += 2` advances so the final pointer read (d) kills the walker;
  try loading d BEFORE c, or reading c/d off a fixed `arg0` base (no post-inc
  past the c load) so the pointer is dead at d.
- F2: fix the `move t0,v1` prologue scheduling (base copy before the sw stores).
  Probe: assign `base = arg0;` as the very first statement (before the index
  computation) so the copy schedules into the prologue region.
- F3: permuter campaign from the floor-10 base (candidate.c) — clean
  single-function target, --stop-on-zero, fresh-seed windows.

## [s1] Adopt COMPLETED-C sibling func_800483DC's base-routing idiom: mutate arg0 as the walking pointer and hold base=arg0 in a separate local for the final entry+=base.
- mechanism: Assigning base=arg0 before mutating arg0 forces GCC to copy arg0's reg (move t0,v1) and reuse arg0's reg as the walker (addu v1,v0,v1), matching target's 2nd-copy routing. Also lands move a1,a3 naturally in the beqz delay slot.
- probe: Rewrite func_80048530 mirroring the sibling; sandbox --disable all.
- result: score 10 (43 vs 47 insns), down from clean floor 12 / HEAD honest 11.
- verdict: CONFIRMED

## [s1] With base routing correct, give c,d the target's lhu+sll+sra shape (via (s16) at call or (x<<16)>>16) to close the 4-insn count gap.
- mechanism: Target loads c,d as lhu then sign-extends with sll/sra like a,b. Hoped the WIP-observed cascade to 22 was caused by wrong base routing and would vanish.
- probe: Two spellings sandboxed with base routing in place.
- result: Both score 22. c/d sign-extend needs v0+v1 scratch; collides with v1-walker, so GCC shifts walker v1->t0 and base t0->t1 (full register-rename cascade). Target reuses the DEAD walker v1 as the d-scratch (lhu v0,0(v1); lhu v1,2(v1)); our build won't.
- verdict: KILLED

## [s2] Frontier F1: make the walker register dead-and-reusable at the d-load so GCC reuses it as the d sign-extend scratch (mirror target's lhu v1,2(v1)).
- mechanism: The walker's last live use must be the d-load; then GCC allocates the d scratch onto the freed walker reg.
- probe: Disassembled the floor-10 build and read the RTL greg dump.
- result: Already satisfied at floor 10: the build emits `lh v0,0(v1); lh v1,2(v1)` — the dead walker v1 is already reused for d, identical routing to target's `lhu v1,2(v1)`. F1 is NOT the lever; the gap is purely lh (1 insn) vs lhu+sll+sra (3 insns) for c,d.
- verdict: KILLED

## [s2] Defeat the lh-fold by loading c,d as u16 and sign-extending IN PLACE (single reused pseudo, reassignment) to get lhu+sll+sra without the symmetric-form RA cascade.
- mechanism: A reassignment `c=(c<<16)>>16;` keeps c a single pseudo, hoping combine leaves lhu+sll+sra rather than folding to lh.
- probe: Edited src to the in-place-shift form; sandbox + disassembly.
- result: combine RE-FOLDS `sign_extend(subreg(zero_extend(mem)))` back to lh for single-use c,d -> `lh v0,0(v1); lh v1,2(v1)`, score 10 (unchanged, still 4 insns short). The lh-fold is un-defeatable for any single-use c,d spelling. rejected/inplace-shift-refolds-lh.c.
- verdict: KILLED

## [s2] Symmetric form ((s16) at the call) plus reversed c/d source order gives c,d the target lhu+sll+sra shape with the walker retained in v1.
- mechanism: Loading c,d as u16 and casting (s16) at the call creates a second pseudo combine cannot fold; source order steers which reuses the walker.
- probe: d-before-c source order with (s16) at call; sandbox + RTL greg dump.
- result: Score 22. RTL shows combine folds offset-0 c into lh (insn 91) but leaves offset-2 d as zero_extend+sll/sra; c(v1)+d(v0) occupy both temps so the walker is evicted v1->t0 and base t0->t1 (full register-rename cascade). Asymmetric fold (offset-0 folds, offset-2 does not) is the trigger.
- verdict: KILLED

## [s2] Frontier F3: a permuter campaign from the floor-10 base finds the combined register/scheduling lever to reach byte-match.
- mechanism: Clean single-function target at offset 0; randomization + directed macros explore the c/d scheduling/RA space.
- probe: Built a validated standalone permuter workspace (base.o == floor-10 bytes), ran launch -j6 --stop-on-zero, 10,593 iterations across fresh-seed wait windows, harvest --stop in-turn.
- result: No score-0 find. Best candidates are byte-identical to the floor-10 form or SEMANTIC-BREAKERS (drop the (s16) cast on c). The permuter diff metric is mis-aligned/noisy for this function (identical source scored 1020 at launch vs 750 as an output), giving no reliable gradient. Random C mutation does not crack the combine-fold RA tie.
- verdict: KILLED
