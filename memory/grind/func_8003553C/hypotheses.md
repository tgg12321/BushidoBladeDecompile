# Hypothesis ledger — func_8003553C

## Session 1 (recon, 2026-07-30)

### Resolved
- **H1 CONFIRMED — OT-base load is source-position-pinned by may-alias.**
  Mechanism: GCC 2.7.2 sched1 cannot move the `lw %lo(D_800A374C)` across stores
  through `p` in either direction (true-dep one way, anti-dep the other), because
  `p` derives from the opaque global `D_800A38B4`. Probe: hoist the argument into
  a local `ot` placed between the RGB byte block and the trailing coordinate
  stores, matching where target emits the load. Result: 14 -> 10.
- **H2 CONFIRMED — a 640-valued store must precede the 240-valued stores in
  source to get `li $v1,640` at the top of the block.** Mechanism: the constant
  `li`s float to the head of the scheduling block in first-source-use order, and
  two simultaneously-live constants force the second into `$v1`; target keeps 640
  in `$v1` across the whole body. Probe: move the `+0x10 = 640` store to the head
  of the body, ahead of the two 240 stores. Result: 10 -> 4 (first five insns and
  the entire RGB block now byte-identical to target).
- **H3 KILLED — "natural" PsyQ order (setRGB0..3 then setXY4).** Score 16;
  `li $v0,128` becomes the first constant and drags `sb $v0,6`/`sb $v0,0xE` out
  of field order, which target does not do.
- **H4 KILLED — assigning `D_800A38B4` before the call to pull `addiu $s0,0x24`
  ahead of the `jal`.** The `sw` to the global moves with it (target's `sw` is
  after the jal), so it is a wash — 10 stayed 10. Staging through a separate
  local is also a wash: GCC coalesces the local into `$s0` and still emits the
  `addiu` after the delay slot.

### Live frontier (in priority order)
1. **F1 — materialize 640 early without an early store to +0x10.** The residual
   `sh $v1,0x10` position is the larger of the two remaining diffs. The untried
   lead is the PsyQ `setXYWH(p, x, y, w, h)` expansion, whose `x1 = _x + _w` and
   `y2 = _y + _h` subexpressions create the 640 and 240 values (in that order,
   640 first) at points independent of the store order, with CSE giving each
   value two uses. Probe: write the coordinate block as the setXYWH expansion
   (all eight fields ascending, values via `x + w` / `y + h` sums) while keeping
   the `ot` local from H1 in place, and sweep where the sums are written relative
   to the RGB block. Compare the emitted `li` order and the `sh $v1,0x10`
   position against `tmp/grind/func_8003553C/s1/forms_and_scores.md`.
2. **F2 — `addiu $s0,$s0,0x24` before the `jal`.** One instruction. H4 killed the
   two obvious spellings. Remaining ideas: express the advance as part of the
   pointer used for the call's second argument (e.g. link the primitive through
   an expression that leaves `p` dead afterwards), or make the post-call store
   read a value whose only live definition must precede the call. Check
   `.claude/rules/defer-store-past-later-compute-into-jal-delay.md` and
   `store-before-jal.md` for the established shapes before improvising.
3. **F3 — permuter, seeded from the score-4 form.** The search space left is pure
   statement ordering within one basic block, which is exactly what the permuter
   randomizes well, and the base is now 4 rather than 17. Build the single-
   function `target.o` from `asm/funcs/func_8003553C.s` + prelude.inc (offset 0)
   as `difficult-is-not-impossible` §3 requires, then run a fresh-seed campaign
   via `tools/permuter_campaign.py`.

## [s1] The lw of D_800A374C (ot_Link's first argument) cannot be scheduled across stores through p, so its emitted position tracks its source position; target emits it between the RGB byte block and the last six coordinate stores, therefore the original source computed that argument there.
- mechanism: GCC 2.7.2 sched1 alias analysis: p derives from the opaque global pointer D_800A38B4, so stores through p may-alias the global load. Moving the load later creates a true dependence, moving it earlier violates the anti-dependence; either way it is pinned relative to the stores.
- probe: Hoisted the argument into a local `u32 *ot = (u32 *)(D_800A374C + 0x401C);` placed between the RGB block and the trailing coordinate stores; objdump-diffed the sandbox object against asm/funcs/func_8003553C.s.
- result: Score 14 -> 10; the lui/lw/move-a1 trio moved to exactly target's position and the six trailing sh stores fell in behind it, matching target.
- verdict: CONFIRMED

## [s1] Target's leading li $v1,640 / li $v0,240 (with 640 held in $v1 across the whole body) requires a 640-valued store to precede the 240-valued stores in the source.
- mechanism: Constant li insns float to the head of the scheduling block in first-source-use order; two constants live simultaneously force the second allocation onto $v1 instead of reusing $v0. First-use order is source order, so whichever constant is used first is materialized first and gets the long live range.
- probe: Moved the x1 store (*(s16 *)(p + 0x10) = 640) to the head of the body, ahead of the two 240 stores; re-ran sandbox --disable all and objdump-diffed.
- result: Score 10 -> 4; target's first five instructions and the entire twelve-instruction RGB byte block are now emitted identically, in strict field order.
- verdict: CONFIRMED

## [s1] The natural PsyQ authoring order (setRGB0..3 then setXY4, i.e. RGB byte block first, coordinates ascending after) is the original source shape.
- mechanism: If true, the RGB block being first makes 0x80 the first-used constant, so li $v0,128 heads the block.
- probe: Wrote that exact order as pure C and measured; also measured the mirror form (all eight coordinate stores first, RGB block second).
- result: Score 16 for RGB-first (li $v0,128 leads and hoists sb $v0,6 / sb $v0,0xE out of field order, which target does not do) and 14 for coordinates-first. Both are strictly worse than the score-4 form. Banked as memory/grind/func_8003553C/rejected/rgb-block-first-natural-psyq-order.c.
- verdict: KILLED

## [s1] Assigning D_800A38B4 = p + 0x24 before the ot_Link call (directly, or staged through a separate local) will pull addiu $s0,$s0,0x24 ahead of the jal as target has it.
- mechanism: Moving the pointer advance earlier in source should let sched1 place the addiu in the pre-jal slot.
- probe: Two forms measured: (a) the whole assignment moved before the call; (b) `next = p + 0x24;` before the call with `D_800A38B4 = next;` after it.
- result: Both neutral. In (a) the sw to the global moves before the jal too, but target's sw is after it — one diff traded for another. In (b) GCC coalesces `next` into $s0 and still emits the addiu after the delay slot.
- verdict: KILLED
