# Hypothesis ledger — func_80048530

## CONFIRMED
- H1 (s1): sibling func_800483DC base-routing idiom (mutate arg0 as walker +
  separate `base=arg0` local for final add) -> floor 10, from 11. Pure C.

## KILLED
- H2 (s1): give c,d the target's lhu+sll+sra shape via value spelling (either
  `(s16)c` at call or `((s32)(c<<16))>>16`) -> score 22. Register-scratch
  collision with v1-walker; NOT fixable by respelling c/d. Disproves the WIP
  conjecture that the c/d cascade would vanish once base routing was correct.

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
