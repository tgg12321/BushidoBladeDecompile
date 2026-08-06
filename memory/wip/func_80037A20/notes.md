# func_80037A20 — WIP (memcard file-count via firstfile/nextfile)

## TL;DR
Honest pin-free floor = **13** (sandbox --disable all), same as HEAD's honest
distance. HEAD "matches" ONLY via two `register asm()` pins (`var_s0 asm("s0")`,
`var_s1 asm("s1")`) plus a forbidden `__asm__("" : "=r"(var_s1) : "0"(var_s1))`
opt-barrier — all three are cheats, so this function is INCOMPLETE.
`candidate.c` is the faithful pin/barrier-free body (33=33 insns, 13 diffs).

## The 13 diffs decompose into exactly TWO coupled problems
1. **s0<->s1 register swap (~12 of 13).** Target: pointer→s0, counter→s1.
   Ours: pointer→s1, counter→s0.
2. **`li s0,1` vs target `addiu s1,s1,1` (1 diff).** cse's FIRST pass const-props
   the dominating `var_s1 = 0` into the entry `++` (REG_WAS_0 note).

## Quantified escape (grid over the validated forward model, 2026-08-06)
ALLOCDBG ground truth: counter(pseudo 75) nrefs=8 livelen=14 pri=17142 → s0;
pointer(pseudo 74) nrefs=5 livelen=17 pri=5882 → s1. Priority is
`floor_log2(n)*n*10000/livelen`. Brute-forcing refs 1..20 × livelen 1..160 in
`tools/ra_solver`'s simulator, the goal (74→s0, 75→s1) is reached ONLY by:
- pointer refs **≥10** at its structural livelen 17 (has 5), or
- counter refs **≤4** at its livelen 14 (has 8), or
- counter livelen **≥41** at refs 8 (has 14).
All three are byte-forced by target's mandatory 33-insn do-while + entry-`++`
structure: s8 measured the pointer has only one if-block and one loop so no
byte-neutral duplication can reach 10+ refs, and s7's barrier probe showed
defeating the fold *raises* the counter to 22000 (adds a read-ref), entrenching
the swap rather than flipping it.

## The inverse solver's LEVER verdict is NOT C-reachable
`docs/grind/inverse-sweep-2026-08-06.md` names one 1-atom vector: pseudo 75
acquires a copy preference for `$s1`. Foreclosed at the mechanism level:
`global.c set_preference` records a preference only from a SET between a pseudo
and a **hard reg**, and `$s1` — like every callee-saved register — **never
appears in pre-RA RTL** (verified on this function's `.lreg` dump: only v0, a0,
a1, a2, a3, ra are present). `prune_preferences` (global.c:897) additionally
strips every call-used reg from a call-crossing allocno's preferences, so no
argument/return flow can leave a surviving one. Only a `register asm()` pin
could create it — the forbidden construct. NB the solver's atom space caps
refs deltas at +3/-2, which is why it reported pref_add as the *unique* 1-atom
vector; the refs routes above sit outside that window (and are the s7/s8 kills).

## rejected_forms (measured, do NOT re-derive)
- do-while w/ s1++ at loop top, no entry-++: WRONG COUNT (off by one).
- nextfile-in-if-condition: 34 insns (+1 nop), fold and swap persist.
- decl reorder (either direction): allocation UNCHANGED.
- firstfile inlined to a v0 temp: 13, unchanged.
- pointer-init-after-call: 16 (worse); GCC hoists the `la` regardless.
- counter-init-before-call: 15 (worse); order and dispositions unchanged.
- return-value split to drop counter refs: 13, `result` copy-propagated away.
- u32 counter narrowing: 13, width-invariant.
- do-while(0) around the entry `++`: collapses; fold and swap persist.
- 3 permuter chassis, ~21k iters: all converge to the same attractor, zero never
  approached.
- cc1psx calibration: output byte-identical to our fork (also swaps AND folds)
  ⇒ compiler-divergence escalation foreclosed.
- duplicated-statement-into-arms ref-lift: structurally unavailable (one if,
  one loop; pointer refs byte-forced at 5).

## Status
Every sanctioned axis (structural, permuter, compiler-divergence, instrumented
RA, and now the inverse-solver atom) is measured dead with a named mechanism.
Disposition remains the filed endgame-lock owner escalation.

## Floor
- HEAD honest distance: 13 (carries 2 pins + 1 opt-barrier cheat)
- candidate.c honest distance: 13 (zero cheats)
