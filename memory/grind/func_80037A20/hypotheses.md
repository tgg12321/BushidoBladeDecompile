# Hypothesis ledger — func_80037A20

Floor = 13 (honest, cheat-free). HEAD "matches" only via 2 register-asm pins +
1 __asm__ opt-barrier (all cheats). candidate.c = pin-free faithful body @ 13.

The 13 diffs = ~12 from the s0<->s1 register SWAP + 1 from the entry-increment
`li s0,1` vs target `addiu s1,s1,1` FOLD. GREG-confirmed (s1): the swap is a
global.c priority wall (counter pseudo-75 out-prioritizes pointer pseudo-74 for
s0; pointer's live range is forced long by GCC hoisting the loop-invariant
`la D_80102810` to the top -> conflicts a2/a3 -> lower priority). The fold is a
separate cse2 const-prop (pre-allocation).

## KILLED (measured)
- H1 pointer-init-after-call (trim pointer live range): sandbox 16, greg
  identical -> address load hoisted regardless of statement order. [s1]
- (WIP) do-while no-entry-increment: wrong count. decl reorder: alloc unchanged.
  nextfile-in-if-condition: 34 insns +nop. firstfile->v0_temp: 13 unchanged.
- [s2] counter-init-before-call (EXTEND counter live range across the call to
  DROP its priority below the ptr, AND put a call boundary before the entry ++):
  sandbox 15. greg: both 74,75 now conflict 6,7 (equal ranges) but order still
  75 74, dispositions still 74->s1/75->s0; fold persists. Equalizing live length
  does NOT flip the ref-count tiebreak. KILLED.
- [s2] do-while0-entry-increment (BB boundary to block cse2 0+1 fold): sandbox 13
  unchanged; do-while(0) collapses; fold+swap persist. KILLED.
- [s2] KEY: target loads the ptr `la $s0` BEFORE func_80079A30 too => target's
  ptr live-range is IDENTICAL to ours yet target allocates ptr->s0 and does NOT
  fold. => both diffs are cc1-internal divergences (global.c allocno-order
  tiebreak + cse2 const-prop) under the MANDATORY target structure, not
  pure-C-structural. Structural axis EXHAUSTED (s1+s2).

## Live frontier (untried, mechanism-grounded)
1. [s2 KILLED — structural levers exhausted] Frontier items 1 (raise ptr
   ref-weight / equalize live range) and 2 (structural fold-disruption via BB
   boundary) are both measured dead. Ref-count tiebreak (counter ~6 vs ptr ~4)
   and cse2 fold both persist under every faithful statement rearrangement,
   because byte-match forces target's exact do-while+entry-++ structure. Target
   proves identical live-ranges yet opposite allocation => cc1-internal.
2. decomp-permuter campaign from candidate.c with a clean single-function
   target.o (per difficult-is-not-impossible §3): the sanctioned modality for a
   simultaneous tied register-rename + fold-disrupting structural mutation. Not
   yet run for code6cac_c. Bank the base score, run fresh-seed windows.
   THIS IS THE ONLY REMAINING SANCTIONED AXIS not yet measured dead.
3. cc1-vs-cc1psx calibration check on BOTH the allocno-order tiebreak (swap) and
   the 0+1 cse2 fold: if cc1psx diverges from our decompals port on the SAME C,
   this is an escalation candidate (compiler divergence, not a worker close),
   pending after permuter comes back negative.

## [s1] Moving the pointer init (var_s0=&D_80102810) below the func_80079A30 call trims the pointer pseudo's live range, raising its global.c priority above the counter so it grabs s0 and resolves the s0<->s1 swap.
- mechanism: global.c allocno priority ~ n_refs*freq/live_length; pointer(74) has a long live range because its symbol-address load conflicts with a2/a3 (live across the func_80079A30 arg setup). Shortening it should raise priority.
- probe: Edit src to move the pointer init after the call; sandbox --disable all; re-dump pre.i.greg and check allocation order.
- result: sandbox 16 (WORSE than 13). greg IDENTICAL: order still 75 74, pointer 74 still ->s1(17) and still conflicts 6,7. GCC hoists the loop-invariant la D_80102810 to the function top regardless of C statement position, so the live range/a2-a3 conflict is not reorder-controllable; reposition only worsened scheduling.
- verdict: KILLED

## [s2] Moving `var_s1 = 0;` above the func_80079A30 call extends the counter pseudo's live range across the call to drop its priority below the pointer (fixing the s0<->s1 swap) AND places a call boundary before the entry ++ that blocks cse2 from const-propagating 0+1 (fixing fold diff #13).
- mechanism: global.c allocno priority ~ n_refs*freq/live_length; equalizing the counter's live length to the pointer's should remove the pointer's live-range disadvantage. A call insn between the 0-init and the ++ was expected to break cse2's extended-basic-block const propagation.
- probe: Edit src (var_s1=0 before call); sandbox --disable all; re-dump pre.i.greg (allocation order/dispositions) and out.s (entry increment).
- result: sandbox 15 (WORSE). greg: both pseudo 74(ptr) and 75(counter) now conflict a2/a3 (equal live ranges) but order STILL '75 74', dispositions STILL 74->s1(17)/75->s0(16). Fold persists (li $16,1) across the call boundary; +2 sched damage from a redundant a0 recompute. Equalizing live length does not flip the ref-count tiebreak (counter ~6 refs vs ptr ~4).
- verdict: KILLED

## [s2] Wrapping the entry increment in a sanctioned do{var_s1++;}while(0) introduces a basic-block/loop-note boundary that cse2 will not const-propagate the dominating var_s1=0 across, so the entry ++ emits addiu (reading 0) like the target instead of our folded li 1.
- mechanism: cse2 folds 0+1->1 because var_s1=0 dominates var_s1++ in the same EBB; a real BB boundary would stop the propagation. do-while(0) is a sanctioned codegen-effect technique.
- probe: Edit src (do{var_s1++;}while(0)); sandbox --disable all; re-dump out.s entry increment + greg dispositions.
- result: sandbox 13 UNCHANGED; the do-while(0) COLLAPSED (documented behavior) - out.s entry still li $16,1 (fold persists), dispositions still 74->s1/75->s0 (swap persists). No boundary survived.
- verdict: KILLED

## [s2] The s0/s1 swap and the li/addiu fold are pure-C-structural (reorderable), not compiler-internal divergences.
- mechanism: If the diffs were structural, some faithful statement arrangement would flip the allocation or block the fold.
- probe: Read target asm (asm/funcs/func_80037A20.s) and compare its pointer live-range/init positions and fold behavior to ours under multiple structural rearrangements.
- result: DISPROVEN. Target loads the pointer `la $s0` at the TOP, BEFORE the func_80079A30 jal - IDENTICAL live range to ours - yet target allocates pointer->s0 and emits `addu $s1,$0,$0`+`addiu $s1,$s1,1` (no fold). Given identical C live-ranges/refs our GCC port allocates the opposite way and folds 0+1. Byte-match forces target's exact do-while+entry-++ +post-- structure, so no faithful reshape can change it. Both diffs are cc1-internal (global.c allocno-order tiebreak + cse2 const-prop).
- verdict: CONFIRMED
