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

### Session-1 frontier — disposition after session 2
F1 (setXYWH sums create 640 before 240) — KILLED, see H5 below.
F2 (`addiu $s0,$s0,0x24` before the `jal`) — **CONFIRMED and CLOSED**, see H7.
F3 (permuter from the score-4 base) — still untried; the base is now 2.

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

## Session 2 (structural, 2026-07-30) — floor 4 -> 2

### Resolved
- **H5 KILLED — a constant-holder local (or a setXYWH-style sum) creates the 640
  pseudo early while its stores stay late.** cse propagates the constant into both
  uses and deletes the standalone set; four spellings (s16 w/h, s16 w only, s32
  w/h, `x + w`/`y + h` sums) all score exactly what the no-lever form scores.
- **H6 KILLED — struct-typed member stores dissolve the may-alias barrier via
  MEM_IN_STRUCT_P** (`tools/gcc-2.7.2/sched.c:834-881`). Measured 12/10/10 against
  a base of 2; strictly worse in every spelling.
- **H7 CONFIRMED — R2 closes by keeping BOTH pointer values live across the call.**
  `q = p; p += 0x24; ot_Link(ot,(u32*)q); D_800A38B4 = p;` gives target's exact
  `move $a1,$s0` / `addiu $s0,$s0,0x24` / `jal` / `sw` tail. 4 -> 2.
- **H8 KILLED — pure statement reordering inside the block can reach 0.** ~45
  orderings measured; minimum 2, reached only by the two forms with exactly one
  640-valued store at the head of the body.

## Session 3 (structural, 2026-07-30) — floor stays 2; residual mechanism identified

### Resolved
- **H9 KILLED — the OT-base-load spelling axis** (s2 frontier F2). Array-indexed
  slot and s32-staged base read both score 2 = the base. Inert.
- **H10 KILLED — MEM_IN_STRUCT_P (`p[i]`/`sp[i]` indexing) frees the coordinate
  stores from the may-alias barrier and lets the 0x10 store sink.** The exemption
  really does apply to HImode indexed stores (QImode is excluded, so the RGB byte
  stores never escape) — and it changes nothing, because the list scheduler is
  movement-minimizing. 2 / 8 / 9 / 8.
- **H11 KILLED — extend another constant's live range to force 640 onto a second
  hard register.** 128-extender 6 (640 still in $v0), 240-extender 11.
- **H12 CONFIRMED (mechanism) — the residual is hard-register reuse of the 640
  constant, created by sched1 sinking its set.** RTL trace + a diagnostic-only
  pin that puts every store in target's slot. See below.

### Live frontier after session 3
1. **Defeat sched1's sink of the 640 constant set, or otherwise get it allocated
   a register no earlier constant used, in pure C.** The pin diagnostic shows
   this is worth 12 of the 13 misplaced insns; the last one (the `li`'s slot
   among the three constants) is a separate, smaller question.
2. **Permuter from the score-2 base** — still untried, and now better motivated:
   the search target is a register-lifetime shape, which random statement/temp
   mutation explores far better than hand enumeration.
3. **Sibling census** — find a matched function in the tree that materializes a
   constant into a SECOND register with only late uses (i.e. target-style
   `li $v1,K` at a block head), and read its C. That is direct evidence of the
   spelling that survives sched1's sink.

## [s3] The remaining single misplaced instruction is caused by hard-register reuse of the 640 constant, which sched1 creates by sinking the constant's set down to its first use.
- mechanism: `priority()` (tools/gcc-2.7.2/sched.c:1425) makes INSN_PRIORITY the longest dependence path from the block HEAD and `schedule_block` builds the block backward from the tail; `rank_for_schedule` (:2399) breaks the (near-universal) priority ties by class-relative-to-last-scheduled and then by INSN_LUID, explicitly to minimize movement. A constant set whose only uses are late therefore ends up adjacent to those uses after sched1; local-alloc then sees a 2-insn live range and reuses $v0 (dead since the 128 constant's last use); that produces a REG_DEP_OUTPUT against `li $v0,128` in sched2, which pins the `li 640` after `sb $v0,0xE` — block idx 7, mid-RGB-block. Target instead holds 640 in $v1 for the whole block, so its `li` is free to sit at the block head and its two stores are free to sit in the post-load group.
- probe: (a) full `cc1 -da` pass dump of the holder form (tmp/grind/func_8003553C/s3/rtl_B/) — traced insn uid 17 through combine / sched / lreg / greg / sched2; (b) a DIAGNOSTIC-ONLY `register s16 w asm("$3")` variant of the same form, compiled outside the sandbox via tmp/grind/func_8003553C/s3/rtl.sh (the sandbox strips cheat-asm, so it cannot be scored) and read out of rtl_I/out.s.
- result: (a) cse KEEPS the constant set at the top of the RTL (REG_EQUAL note attached); sched1 sinks it; lreg/greg assign $v0; sched2 shows REG_DEP_OUTPUT 36 + anti-deps 38/49. Session 2's "cse deletes the standalone set" explanation is disproved. (b) With 640 pinned to $3, EVERY store lands in target's slot — the RGB block is in strict field order and `sh $3,16` sits in the post-load group between `sh 0,10` and `sh 0,18`, i.e. exactly the instruction our clean score-2 form misplaces. The only residual left is `li 640` emitted after `li 128` instead of ahead of `li 240`.
- verdict: CONFIRMED

## [s3] The OT-base load's spelling (array-indexed slot, or the base read staged into an s32 local with the +0x401C applied later) changes which side of sched.c's memory disambiguation the load lands on, letting the coordinate stores reorder around it.
- mechanism: sched.c's dependence test compares the two MEMs' in-struct flags and whether their addresses vary; changing how the OT base is obtained changes the rtx shape of that MEM.
- probe: two spellings measured against the score-2 base — `ot = &((u32 *)D_800A374C)[0x1007];` and `otb = D_800A374C;` staged before the trailing coordinate stores with `ot = (u32 *)(otb + 0x401C);` after them.
- result: 2 and 2 — bit-identical scores to the base. The load is a `(mem:SI (symbol_ref))` in every spelling, so nothing about the disambiguation changes. This closes session 2's frontier item 2.
- verdict: KILLED

## [s3] Giving the HImode coordinate stores MEM_IN_STRUCT_P (writing them as `sp[i]` through `s16 *sp = (s16 *)p;`) dissolves the may-alias barrier for them, letting the 0x10 store sink into the post-load group while the leading 640 store keeps `li $v1,640` at the block head.
- mechanism: tools/gcc-2.7.2/sched.c:817-866 exempts an in-struct MEM with a varying address from conflicting with a MEM that is neither in-struct nor address-varying (the fixed-address `lw` of D_800A374C) — but the exemption excludes QImode, which is why the twelve RGB byte stores (already `mem/s:QI`, since `p[4]` indexing sets the flag) stay barrier-bound while HImode indexed stores would be freed.
- probe: four forms — all eight coordinate stores as `sp[i]` in the s2 statement order; the same in target's statement order; only the x1 store as `sp[8]` with the rest plain deref; and `sp[i]` plus a 640 holder in target's order.
- result: 2 / 8 / 9 / 8. The freed form scores EXACTLY what the base scores; nothing moved. GCC 2.7.2's list scheduler minimizes movement, so removing a dependence without changing priorities or lifetimes gives the insn no reason to move. Aliasing is not the binding constraint on this function.
- verdict: KILLED

## [s3] The 640 constant can be pushed onto a second hard register by keeping another constant's pseudo live across the point where its set lands ($v0 busy ⇒ 640 gets $v1).
- mechanism: local-alloc reuses $v0 for the 640 set only because both the 240 and the 128 pseudos are dead by then. Moving one of their stores into the post-load group extends that pseudo's range past the 640 set.
- probe: `p[0xE] = 0x80` (rgb1 blue, one of the two 128 users) moved into the post-load group; `sh 0x22 = 240` (one of the two 240 users) moved into the post-load group; the latter also with a 640 holder.
- result: 6 / 11 / 11. The 128-extender's disassembly shows 640 STILL in $v0: sched1 sinks the 640 set next to its use, and the extender's store is scheduled just before that point, so $v0's range still ends immediately before the set. Net effect is one extra displaced insn, not a register change.
- verdict: KILLED

## [s2] The addiu $s0,$s0,0x24 lands in the pre-jal slot when the OLD primitive pointer and the ADVANCED pointer are both live across the ot_Link call.
- mechanism: With `q = p; p += 0x24;` before the call, the argument value (old p) and the allocator value (p+0x24) are two simultaneously-live values, so GCC cannot coalesce them: it emits the argument copy `move $a1,$s0` early, performs the advance in place on $s0 before the call, and keeps the `sw` to D_800A38B4 after the call because the call clobbers memory. s1's two killed spellings each left only ONE of the two values live, so either the sw moved forward with the advance or the extra local was coalesced back into $s0.
- probe: Measured four tail spellings on the score-4 base: (Q) the s1 candidate, (V2) q/p split as above, (V4) `next = p + 0x24;` before the call with `D_800A38B4 = next;` after, (V5) the q/p split hoisted above initPolyG4.
- result: Q 4, V2 **2**, V4 4, V5 15 (46 insns). V2's disassembly reproduces the entire tail of target; the whole-function residual is now a single instruction.
- verdict: CONFIRMED

## [s2] A named constant-holder local (s16 w = 640) — or the PsyQ setXYWH `x + w` / `y + h` sum spelling — creates the 640 pseudo at the top of the RTL stream while its stores stay late, giving target's leading li $v1,640.
- mechanism: If the set survived at the declaration point it would be the first constant insn in the block, so 640 would head the block and take the long live range in $v1 while the x1/x3 stores stayed in the post-load group.
- probe: Four spellings measured on the score-4 base: `s16 w, h`; `s16 w` only with literal 240 stores; `s32 w, h`; and `s32 x,y,w,h` with `x + w` / `y + h` as the stored values (the setXYWH expansion).
- result: All four score 10 — bit-identical in score to the form with no leading 640 store at all. GCC 2.7.2's cse propagates the constant into both use sites and deletes the standalone set, so the declaration leaves no insn behind and the li is re-created at the first USE. Banked as rejected/constant-holder-local-640-folds-away.c. This closes s1's F1.
- verdict: KILLED

## [s2] Writing the primitive through a real POLY_G4 struct pointer frees the coordinate stores from the may-alias barrier against the lw of D_800A374C, letting the 0x10 store sink into the post-load group.
- mechanism: tools/gcc-2.7.2/sched.c:834-881 implements the classic MEM_IN_STRUCT_P disambiguation — an in-struct MEM with a varying (register-based) address is treated as NOT conflicting with a MEM that is neither in-struct nor address-varying, which is exactly the fixed-address load of the scalar global D_800A374C. If member stores set MEM_IN_STRUCT_P, the pre-load/post-load barrier that pins our sh $v1,0x10 would dissolve.
- probe: Declared a POLY_G4 typedef (tag / rgb+code / x,y per vertex) and measured three struct-typed bodies against the score-2 base: leading `poly->x1 = 640`, ascending coordinates after the ot load, and setXY4 order.
- result: 12 / 10 / 10 — every struct spelling is strictly worse than its scalar byte-offset counterpart (the ascending scalar equivalent is 8). The barrier is not attackable this way and the struct spelling costs extra diffs elsewhere. Banked as rejected/struct-typed-polyg4-member-stores.c.
- verdict: KILLED

## [s2] Pure statement reordering within the block can reach 0 from the score-2 base.
- mechanism: The residual is a single instruction position, and the only remaining degrees of freedom in the block are the relative order of the 8 coordinate stores, the 12 RGB byte stores and the ot load.
- probe: Built a generator (tmp/grind/func_8003553C/s2/ordersweep.ps1) taking pipe-separated order specs and scored ~45 distinct orderings, sweeping each 640 store's position, the 240 pair, the ot load's position, and the RGB block including its four 3-store subgroups, under both the s1 and the s2 tail.
- result: Minimum 2, reached only by the two forms with exactly one 640-valued store at the head of the body (x1-leading and its x3-leading mirror). No leading 640 store bottoms out at 8; two leading 640 stores at 5. Full table in tmp/grind/func_8003553C/s2/forms_and_scores.md §C.
- verdict: KILLED

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

## [s2] The `addiu $s0,$s0,0x24` lands in the pre-jal slot (with the `sw` to D_800A38B4 still after the call) when the OLD primitive pointer and the ADVANCED pointer are both live across the ot_Link call: `q = p; p += 0x24; ot_Link(ot,(u32*)q); D_800A38B4 = p;`.
- mechanism: With both values live at the call GCC cannot coalesce them into one pseudo, so it materializes the argument copy `move $a1,$s0` early, performs the advance in place on $s0 before the call, and keeps the global store after the call because a call clobbers memory and the sw cannot be hoisted across it. Session 1's two killed spellings each left only ONE of the two values live, which is exactly why they were a wash: advancing the global directly drags the sw forward with the addiu, and `next = p + 0x24;` staged in a plain extra local is coalesced straight back into $s0.
- probe: Measured four tail spellings on the score-4 base with `sandbox --disable all` and objdump-diffed each: (Q) the s1 candidate `D_800A38B4 = p + 0x24;` after the call; (V2) the q/p split above; (V4) `next = p + 0x24;` before the call with `D_800A38B4 = next;` after (a re-measurement of s1's H4-(b)); (V5) the q/p split hoisted above initPolyG4.
- result: Q 4, V2 = 2, V4 4 (still a wash), V5 15 at 46 insns. V2's disassembly reproduces target's entire tail — move a1,s0 / addiu s0,s0,0x24 / jal ot_Link / addiu a0,a0,0x401C in the delay slot / sw s0 — leaving exactly one misplaced instruction in the whole function.
- verdict: CONFIRMED

## [s2] A named constant-holder local (`s16 w = 640;`), or the PsyQ setXYWH `x + w` / `y + h` sum spelling, creates the 640 pseudo at the top of the RTL stream while its stores stay late — giving target's leading `li $v1,640` without an early store to +0x10. (This was session 1's top frontier item F1.)
- mechanism: If the set survived at the declaration point it would be the first constant insn in the block, so 640 would head the block and take the long live range in $v1 while the x1/x3 stores stayed in the post-load group.
- probe: Four spellings measured on the score-4 base: `s16 w, h`; `s16 w` only with literal 240 stores; `s32 w, h`; and `s32 x,y,w,h` with `x + w` / `y + h` as the stored values (the full setXYWH expansion).
- result: All four score 10 — identical to the form that has no leading 640 store at all. GCC 2.7.2's cse propagates the constant into both use sites and deletes the standalone set, so the declaration leaves no insn behind and the li is re-created at the first USE. There is therefore no source spelling that materializes 640 early without an early USE of 640. Banked as memory/grind/func_8003553C/rejected/constant-holder-local-640-folds-away.c.
- verdict: KILLED

## [s2] Writing the primitive through a real POLY_G4 struct pointer frees the coordinate stores from the may-alias barrier against the lw of D_800A374C, letting the +0x10 store sink into the post-load group where target has it.
- mechanism: tools/gcc-2.7.2/sched.c:834-881 implements the classic MEM_IN_STRUCT_P disambiguation: an in-struct MEM with a varying (register-based) address is treated as NOT conflicting with a MEM that is neither in-struct nor address-varying — which is exactly the fixed-address load of the scalar global D_800A374C. If member stores set MEM_IN_STRUCT_P, the pre-load/post-load store barrier would dissolve and the scheduler could sink the 0x10 store freely.
- probe: Declared a POLY_G4 typedef (tag / rgb+code / x,y per vertex, 0x24 bytes) and measured three struct-typed bodies against the score-2 base: leading `poly->x1 = 640`, ascending coordinates after the ot load, and setXY4 order (RGB block first).
- result: 12 / 10 / 10. Every struct spelling is strictly worse than its scalar byte-offset counterpart (the ascending scalar equivalent scores 8). The barrier is not attackable this way. Banked as memory/grind/func_8003553C/rejected/struct-typed-polyg4-member-stores.c.
- verdict: KILLED

## [s2] Pure statement reordering inside the single scheduling block can reach 0 from the score-2 base.
- mechanism: The residual is one instruction position and the only remaining degrees of freedom in the block are the relative order of the 8 coordinate stores, the 12 RGB byte stores and the ot load.
- probe: Built a spec-driven generator (tmp/grind/func_8003553C/s2/ordersweep.ps1) and scored ~45 distinct orderings under both the s1 and s2 tails, sweeping each 640 store's position, the 240 pair, the ot load's position, and the RGB block including splitting it into its four 3-store subgroups.
- result: Minimum is 2, reached by exactly two forms — the ones with exactly ONE 640-valued store at the head of the body (x1 leading with trailing group 8,A,12,18,20; or the mirror, x3 leading with trailing group 8,A,10,12,18). Every form with no leading 640 store bottoms out at 8; every form with two of them at 5. Full table in tmp/grind/func_8003553C/s2/forms_and_scores.md section C.
- verdict: KILLED

## [s3] The remaining single misplaced instruction (our sh $v1,0x10 at block idx 6 vs target's post-load-group slot) is caused by hard-register reuse of the 640 constant, which sched1 creates by sinking that constant's set down adjacent to its first use.
- mechanism: priority() (tools/gcc-2.7.2/sched.c:1425) walks LOG_LINKS = predecessors, so INSN_PRIORITY is the longest dependence path from the block HEAD, and schedule_block builds the block backward from the tail. rank_for_schedule (:2399) breaks the near-universal priority ties by class-relative-to-last-scheduled (a dependence of insn_cost 1 counts as independent) and then by INSN_LUID, explicitly 'to minimize instruction movement'. A constant whose only uses are late therefore ends up adjacent to them after sched1; local-alloc sees a 2-insn live range and reuses $v0 (dead since the 128 constant's last use); sched2 then sees REG_DEP_OUTPUT against li $v0,128 plus anti-deps on its two uses, so the li 640 can never rise above sb $v0,0xE. Target instead holds 640 in $v1 across the whole block, which frees both its li (block head) and its two stores (post-load group).
- probe: (a) Full cc1 -da pass dump of the constant-holder form (tmp/grind/func_8003553C/s3/rtl_B/) and traced the 640 set (insn uid 17) through base.i.combine, base.i.sched, base.i.lreg, base.i.greg, base.i.sched2. (b) DIAGNOSTIC-ONLY variant with register s16 w asm("$3") on the same form, compiled outside the sandbox via tmp/grind/func_8003553C/s3/rtl.sh (the sandbox strips cheat-asm so a pinned form cannot be scored) and read out of rtl_I/out.s.
- result: (a) cse KEEPS the constant set at the top of the RTL with a REG_EQUAL note — session 2's 'cse propagates and deletes the standalone set' explanation is disproved; sched1 sinks it; lreg/greg assign $v0; sched2 shows REG_DEP_OUTPUT 36 and anti-deps 38/49; final position block idx 7. (b) With 640 in $3 every store lands in target's slot: the RGB block returns to strict field order AND sh $3,16 sits in the post-load group between sh 0,10 and sh 0,18 — precisely the instruction our clean score-2 form misplaces. The sole residual in the pinned build is li 640 emitted after li 128 instead of ahead of li 240.
- verdict: CONFIRMED

## [s3] Varying how the OT base is obtained (array-indexed slot, or staging the base read into an s32 local with the +0x401C applied after the trailing stores) changes the rtx shape of that MEM and lets the coordinate stores reorder around the load. (Session 2's frontier item 2.)
- mechanism: sched.c's dependence test compares in-struct flags and address-varying-ness of the two MEMs; a differently-shaped OT-base MEM could land on the non-conflicting side of the disambiguation.
- probe: Two spellings measured with sandbox --disable all against the score-2 base: ot = &((u32 *)D_800A374C)[0x1007]; and otb = D_800A374C; staged before the trailing coordinate stores with ot = (u32 *)(otb + 0x401C); after them.
- result: 2 and 2 — identical to the base. The load is a (mem:SI (symbol_ref)) in every spelling, so nothing about the disambiguation changes. Session 2's frontier item 2 is closed.
- verdict: KILLED

## [s3] Giving the HImode coordinate stores MEM_IN_STRUCT_P (writing them as sp[i] through s16 *sp = (s16 *)p) dissolves the may-alias barrier against the lw of D_800A374C for them, letting the 0x10 store sink into the post-load group while the leading 640 store keeps li $v1,640 at the block head.
- mechanism: tools/gcc-2.7.2/sched.c:817-866 exempts an in-struct MEM with a varying address from conflicting with a MEM that is neither in-struct nor address-varying (the fixed-address lw of the scalar global) — but the exemption excludes QImode, which is why the twelve RGB byte stores (already mem/s:QI, since p[4] indexing sets the flag) stay barrier-bound while HImode indexed stores are genuinely freed.
- probe: Four forms scored: all eight coordinate stores as sp[i] in the s2 statement order; the same in target's statement order; only the x1 store as sp[8]; and sp[i] plus a 640 holder in target's order.
- result: 2 / 8 / 9 / 8. The freed form scores EXACTLY the base. Removing a dependence without changing priorities or lifetimes gives the insn no reason to move — GCC 2.7.2's list scheduler minimizes movement. Aliasing is not the binding constraint on this function.
- verdict: KILLED

## [s3] The 640 constant can be pushed onto a second hard register by keeping another constant's pseudo live across the point where its set lands ($v0 busy => 640 gets $v1).
- mechanism: local-alloc reuses $v0 only because both the 240 and the 128 pseudos are dead by the time the 640 set is scheduled; moving one of their stores into the post-load group extends that pseudo's live range past it.
- probe: p[0xE] = 0x80 (one of the two 128 users) moved into the post-load group; sh 0x22 = 240 (one of the two 240 users) moved into the post-load group; the latter also with a 640 holder. All scored with sandbox --disable all, the first also objdump-diffed.
- result: 6 / 11 / 11. The 128-extender's disassembly shows 640 STILL in $v0: sched1 re-collapses the 640 set next to its use and the extender's store is scheduled just before that point, so $v0's range still ends immediately before the set. Net effect is one extra displaced insn, not a register change.
- verdict: KILLED

## [s3] A constant-holder local (s16/s32 w = 640) materializes the 640 pseudo early enough to reach target's leading li, when combined with session 2's new tail.
- mechanism: If the holder's set survives at the declaration point it is the first constant insn in the block, so 640 heads the block and takes the long live range while both its stores stay in the post-load group.
- probe: Three holder spellings (s16 w; s32 w; s16 w + s16 h) re-measured on the score-2 s2 tail with target's statement order, plus the no-holder control.
- result: 8 / 8 / 8, identical to the no-holder control (8). Score-inert — but the RTL trace shows the set DOES survive cse and is sunk by sched1, which is the corrected mechanism recorded above and the reason this axis is dead in its current spelling.
- verdict: KILLED
