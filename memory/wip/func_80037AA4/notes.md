# func_80037AA4 — WIP/BLOCKED (sum-and-scale over D_80102810 entries)

## TL;DR
HEAD "matches" via THREE `register asm()` pins (a2/a0/v0) PLUS an unused
`s32 sp_dummy[2]` frame-padding array. Two coupled blockers prevent a clean
pure-C close:

1. **8-byte unused stack frame.** The target has `addiu sp,sp,-8` / `+8` with
   NO sw/lw to sp (no spill, nothing stored). A leaf function with no
   used locals gets NO frame in GCC 2.7.2, so the original had a real stack
   local whose content is UNRECOVERABLE from the pure logic. The only way to
   reproduce the frame is a forbidden coercion (`sp_dummy[2]` unused array /
   address-taken / volatile) -- the dead-vars-local-array cheat
   ([[register-alloc-pure-c]] Lever D, FORBIDDEN). candidate.c (no dummy)
   produces NO frame -> -2 insns vs target.
2. **a0<->v1 register rename.** Target: accumulator var_a0 -> a0, pointer
   var_v1 -> v1. Pin-free GCC swaps them (accumulator->v1, pointer->a0). Same
   tied-priority-rename class as func_80037A20 / func_80044098.

## Measured
- HEAD honest distance (pins+dummy stripped): 14.
- candidate.c (no pins, NO dummy): 16 diffs -- WORSE, because the absent frame
  costs the 2 sp-adjust insns the dummy was supplying. Floor NOT lowered.
- decl reorder (v2): 16, swap+frame unchanged.

## Why blocked (policy, not just difficulty)
The 8-byte frame has no non-cheat pure-C reconstruction: the original stack
local's type/content is unknown, and any unused-array/address-of/volatile form
that forces the frame is a forbidden coercion. This needs either (a) recovery of
what the real stack local was (data-flow / sibling-function evidence), or (b) a
user policy call on reconstructing an unused frame. PLUS the a0<->v1 rename.

## Avenues for next session
- Investigate whether a sibling/caller reveals the original 8-byte local's
  purpose (then it's a legitimate named local, not a cheat).
- If the frame is genuinely an artifact with no real local, escalate as a
  policy question (reconstruct-unused-frame) -- not worker-closable.
- decomp-permuter for the a0<->v1 rename once the frame question is resolved.

## Floor
- HEAD: 14 (3 pins + sp_dummy[2] frame cheat). candidate.c: 16 (cheat-free, but
  short the frame). Not lowered; blocked on the frame-reconstruction policy +
  tied register rename.
