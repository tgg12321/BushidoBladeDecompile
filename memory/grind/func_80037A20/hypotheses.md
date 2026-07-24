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

## Live frontier (untried, mechanism-grounded)
1. Raise POINTER (pseudo 74) ref-weight above the counter WITHOUT touching its
   live range: the swap is decided by n_refs*freq/live_length. Pointer already
   has 2 loop-body refs vs counter's 1, but its long hoisted live range divides
   it down. Try loop restructurings that add a pointer ref inside the loop the
   counter can't mirror (e.g. compute the nextfile arg from the walked pointer in
   a form that keeps an extra pointer use live), OR reduce the counter's loop
   weight. Probe: edit -> re-dump greg, watch for allocation order flipping to
   "74 75". (decomp-permuter is the documented modality for a tied rename.)
2. Fold-disruption (diff #13, independent of swap): find a faithful-count C form
   where cse2 does NOT const-propagate 0 into the entry `var_s1++`. Target emits
   `addiu` (add reading 0); ours folds to `li 1`. Probe the cse2 dump
   (pre.i.cse2) to see WHERE the propagation happens, then a structural lever
   that keeps var_s1=0 reaching the entry ++ across a cse boundary. Not a pin.
3. decomp-permuter campaign from candidate.c with a clean single-function
   target.o (per difficult-is-not-impossible §3): the sanctioned modality for a
   simultaneous tied register-rename + fold-disrupting structural mutation. Not
   yet run for code6cac_c. Bank the base score, run fresh-seed windows.

## [s1] Moving the pointer init (var_s0=&D_80102810) below the func_80079A30 call trims the pointer pseudo's live range, raising its global.c priority above the counter so it grabs s0 and resolves the s0<->s1 swap.
- mechanism: global.c allocno priority ~ n_refs*freq/live_length; pointer(74) has a long live range because its symbol-address load conflicts with a2/a3 (live across the func_80079A30 arg setup). Shortening it should raise priority.
- probe: Edit src to move the pointer init after the call; sandbox --disable all; re-dump pre.i.greg and check allocation order.
- result: sandbox 16 (WORSE than 13). greg IDENTICAL: order still 75 74, pointer 74 still ->s1(17) and still conflicts 6,7. GCC hoists the loop-invariant la D_80102810 to the function top regardless of C statement position, so the live range/a2-a3 conflict is not reorder-controllable; reposition only worsened scheduling.
- verdict: KILLED
