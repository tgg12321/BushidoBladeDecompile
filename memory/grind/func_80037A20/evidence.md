# Evidence bank — func_80037A20

- s1 (recon) GREG-CONFIRMED mechanism (artifact tmp/grind/func_80037A20/s1/dump/pre.i.greg).
  2 regs to allocate; allocation order "75 74". pseudo 74 (POINTER, `la D_80102810`)
  -> s1(17); pseudo 75 (COUNTER) -> s0(16). Target wants pointer->s0, counter->s1
  (SWAPPED). Order 75-then-74 => counter has HIGHER global.c priority
  (n_refs*freq/live_length) and grabs s0 first. pseudo 74 conflicts list INCLUDES
  6,7 (a2/a3): the pointer's symbol-address load is hoisted to the function top by
  GCC (loop-invariant), so it is live across the func_80079A30 arg setup -> long
  live range -> lower priority. Counter is set AFTER the call -> no 6/7 conflict,
  shorter range -> higher priority. BOTH factors favor the counter for s0.

- s1 KILLED: pointer-init-after-call (move `var_s0=&D_80102810;` below the
  func_80079A30 call to trim its live range). sandbox 16 (WORSE). greg IDENTICAL
  (still 75 74, 74 still conflicts 6,7): GCC hoists the address load to the top
  regardless of C statement position, so the pointer live range / a2-a3 conflict
  are NOT reorder-controllable. Reposition only worsened scheduling (+3). This is
  the [[register-alloc-pure-c]] "confirmed limit" priority-wall class.
  rejected/pointer-init-after-call-live-range-trim.c

- s1 m2c reference shape (tmp/grind/func_80037A20/s1/m2c_target.c): m2c splits the
  counter into two SSA vars — `var_s1=0; if(firstfile){ var_s1_2=1; do{ptr+=0x28;
  var_s1_2+=1;} while(nextfile(ptr)); var_s1=var_s1_2-1;}`. Confirms the faithful
  count++/count-- do-while structure (candidate.c). NOTE m2c renders the entry as
  `var_s1_2 = 1` but the TARGET instruction is `addiu s1,s1,1` (an ADD reading the
  prior 0) — our build folds it to `li s0,1` (diff #13). The fold is a cse2
  (pre-allocation) transform, so it is INDEPENDENT of the final register (not a
  downstream symptom of the swap). Target's cse2 did not const-propagate 0 into
  the entry ++; ours does. Faithful-semantics fold-disruption lever still open.

- s1 near-dup lead obj_InitTaskCamera (0.606) is SPURIOUS — it is only an extern
  decl in this TU; a camera-task init, no firstfile/nextfile file-count loop. Dead.

- WIP rejected_form: do-while no-entry-increment (wrong count, off-by-one)

- WIP rejected_form: nextfile-in-if-condition (34 insns, +nop)

- WIP rejected_form: decl reorder var_s1-first / var_s0-first (allocation unchanged)

- WIP rejected_form: firstfile inlined to v0_val temp (13, unchanged)

- == imported from memory/wip notes.md ==
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


- [s1] canonical: verdict C, pure-C distance 13, 0 rules. sandbox --disable all = 13 (33=33 insns, 34 cheat-asm instances stripped). HEAD 'matches' only via 2 register-asm pins + 1 __asm__ opt-barrier (cheats).

- [s1] greg (pre.i.greg): 2 regs to allocate, order '75 74'. pseudo 74 = POINTER (la D_80102810) -> s1(17); pseudo 75 = COUNTER -> s0(16). Target wants pointer->s0, counter->s1 (SWAPPED). Counter allocated first => higher priority => grabs s0.

- [s1] pseudo 74 conflicts include 6,7 (a2/a3); pseudo 75 does not. Pointer's symbol-address load is hoisted to the top (loop-invariant) so it is live across the func_80079A30 arg setup -> long live range -> lower priority. Counter is set after the call -> shorter range, no 6/7 conflict -> higher priority. Both factors favor counter for s0.

- [s1] Emitted honest asm (out.s): entry increment = 'li $16,1' (folded 0+1) vs target 'addiu $s1,$s1,1' (add reading prior 0). The fold is a cse2 (pre-allocation) transform => INDEPENDENT of the final register, NOT a downstream symptom of the swap.

- [s1] m2c reference (m2c_target.c) confirms the faithful count++/count-- do-while structure (candidate.c); m2c renders the entry as 'var_s1_2=1' but the real target instruction is addiu.

- [s1] Near-dup lead obj_InitTaskCamera (0.606) is SPURIOUS: only an extern decl in this TU, a camera-task init, no firstfile/nextfile loop. Dead lead.
