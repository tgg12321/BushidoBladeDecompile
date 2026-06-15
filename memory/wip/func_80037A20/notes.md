# func_80037A20 — WIP (memcard file-count via firstfile/nextfile)

## TL;DR
Honest pin-free floor = **13** (sandbox --disable all), identical to HEAD's
honest distance. HEAD "matches" ONLY via two `register asm()` pins
(`var_s0 asm("s0")`, `var_s1 asm("s1")`) + a forbidden opt-barrier
`__asm__("" : "=r"(var_s1) : "0"(var_s1))` that the 2026-04-04 commit message
admits was added to "block constant propagation." All three are cheats; this
function is INCOMPLETE. candidate.c is the faithful pin/barrier-free body
(33=33 insns, 13 diffs).

## The 13 diffs decompose into exactly TWO coupled problems
1. **s0<->s1 register swap (~12 of 13 diffs).** Target: var_s0(ptr)->s0,
   var_s1(counter)->s1. Our pin-free build: ptr->s1, counter->s0. The counter
   out-prioritises the pointer for the preferred callee-saved reg s0 because it
   is incremented in the loop AND is the return value (live to function end),
   while the pointer's address-load lives from the top too. Tied loop-weighted
   refs -> global.c allocno-priority tiebreaker. Decl reorder (var_s1 first /
   var_s0 first) does NOT flip it. This is the [[register-alloc-pure-c]]
   "confirmed limit" class (global.c:624 tiebreaker).
2. **`li s0,1` vs target `addiu s1,s1,1` (1 diff).** The faithful structure
   requires an entry `var_s1++` (counts the firstfile hit = slot 0) SEPARATE
   from the loop's `var_s1++`. Since `var_s1 = 0;` immediately dominates that
   entry `++`, GCC constant-folds `0 + 1` -> `li 1`. The target's real cc1
   emitted `addiu` (did NOT fold). This is the EXACT fold the cheat
   `__asm__("")` barrier was suppressing. No pure-C structural lever found that
   keeps the faithful count semantics AND disrupts the fold (see rejected/).

## rejected_forms (measured, do NOT re-derive)
- do-while w/ s1++ at loop top, no entry-++, no s1-- : WRONG COUNT (drops
  slot-0 count; off by one). v6/v7.
- v3 (nextfile in the if-condition, s1++ before branch): 34 insns (+1 nop),
  fold persists, swap persists.
- decl reorder var_s1-first (v4) / var_s0-first: allocation UNCHANGED.
- inlining firstfile into a v0_val temp (v5): 13, unchanged.

## Avenues for next session (change MODALITY)
- **decomp-permuter** from candidate.c — the documented modality for both a
  tied register rename AND a fold-disrupting structural mutation. Not yet run
  (per-fn permuter setup for code6cac_c is the remaining work).
- **cc1 -da greg dump** to confirm the allocno-priority tiebreaker is the s0/s1
  driver, then a live-range lever to raise the pointer's priority.
- Possible **cc1-vs-cc1psx divergence** on the `li 1` fold — calibration check
  (cc1psx may not fold here). If confirmed-divergent + permuter-negative, this
  is an escalation candidate, not a worker close.

## Floor
- HEAD honest distance: 13 (carries 2 pins + 1 opt-barrier cheat)
- candidate.c honest distance: 13 (zero cheats) — floor NOT lowered below HEAD.
