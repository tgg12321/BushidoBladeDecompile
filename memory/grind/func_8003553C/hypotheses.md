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

## Session 4 (permuter, 2026-07-30) — floor stays 2; permuter axis KILLED

### Resolved
- **H13 KILLED — decomp-permuter random search from the score-2 base finds the
  register-lifetime shape hand enumeration cannot express** (session 3 frontier
  item 2). Two fresh-seed chassis, ~85 outputs, none better than difflines 2 at
  43 insns. Root cause measured: the permuter's weighted objective is
  ANTI-CORRELATED with the honest distance on this function.
- **H14 CONFIRMED (negative result, mechanism) — the permuter's scorer
  mis-ranks this function's basin.**
- **H15 KILLED — the matched sibling's store spelling
  (`*(u8 *)((s32)p + N)`, no MEM_IN_STRUCT_P) transplants the sibling's
  block-head-constant behaviour.** Inert: difflines 10 = the control's 10.
- **Sibling census EXECUTED** (session 3 frontier item 3): 164 matched
  functions carry a block-head `addiu $vN,$zero,K` with a >= 4-insn gap to
  first use; `func_80072BC4`'s else-arm is the structural twin (POLY_G4 RGB
  block, three block-head constants, plain ascending literal stores).

## [s4] decomp-permuter, seeded from the banked score-2 candidate and (separately) from target's own statement order, can find the register-lifetime shape that hand enumeration cannot express.
- mechanism: the residual is no longer statement order (saturated at 2 over ~45 hand orderings in s2) but a register-lifetime shape — exactly what the permuter's temp introduction, expression re-association and variable-reuse mutations explore. A base at sandbox 2 should give a clean gradient.
- probe: Built a validated single-function workspace per .claude/rules/difficult-is-not-impossible.md section 3 (tmp/grind/func_8003553C/s4/mkws.sh; target.o from prelude.inc minus `.set gp=64` plus asm/funcs/func_8003553C.s at offset 0; compile.sh runs the real cc1/prologue_fix/maspsx-2.34/multu_pad pipeline and extracts only this function's region). Ran two fresh-seed campaigns via tools/permuter_campaign.py: chassis A (wsA) seeded from the banked candidate, base score 225, ~1.5k iterations, stopped on an 8-minute no-novel-improvement window at best 100; chassis B (wsB) seeded from target's statement order, base 270, best 50. Both harvested with --stop before the session ended. All ~85 output forms from both workspaces were then re-scored offline against target.o with tmp/grind/func_8003553C/s4/score.sh.
- result: NO form beats the incumbent (difflines 2 at 43 instructions). Distribution: 1(44) x1, 2(43) x6, 3 x3, 4(43) x14, 5 x1, 6(43) x13, 8(43) x17, 9-18 x30. The only difflines-1 form emits 44 instructions — chassis A with the leading `*(s16 *)(p + 0x10) = 640;` store DUPLICATED into the post-load group, i.e. all 43 target instructions in target's order plus one extra `sh`; a +1-insn redundant dead store, not a match.
- verdict: KILLED

## [s4] The permuter's weighted objective tracks the honest sandbox distance closely enough to be a usable search gradient for this function.
- mechanism: the permuter scores register mismatches x5, reorderings x60 and insertions/deletions x100; if a single displaced instruction is charged as one reordering, the incumbent should sit near the bottom of the permuter's scale and improvements should be visible.
- probe: compare permuter base/best scores against offline difflines for the same forms.
- result: ANTI-CORRELATED. The incumbent (one instruction displaced by 17 slots, sandbox 2, difflines 2) is charged permuter score 225, while chassis-B forms that are objectively 5x worse (difflines 10-12, sandbox ~8) score 50-65. A long-range displacement is charged as a large pile of reorderings, while register-name agreement is rewarded, so the search gradient points AWAY from the true optimum. Extra sampling or more reseeding cannot fix a mis-specified objective; the permuter axis is closed for this function, not merely unproductive.
- verdict: CONFIRMED

## [s4] Transplanting the matched sibling func_80072BC4's store spelling — `*(u8 *)((s32)p + N) = v` instead of `p[N] = v`, which does NOT set MEM_IN_STRUCT_P — reproduces its block-head constant materialisation.
- mechanism: func_80072BC4's else-arm is the structural twin of our body (a POLY_G4 colour block written through a register-held primitive pointer) and GCC gives it three block-head `addiu $vN,$zero,K` constants with late first uses — precisely the shape our 640 needs. If the sibling's syntax is what earns that, respelling our stores the same way should reproduce it.
- probe: chassis B respelled with all twenty stores as `*(s16|u8 *)((s32)p + N)` (tmp/grind/func_8003553C/s4/C2_sibling_s32cast_spelling.c) versus the identical chassis-B control with `p[N]` / `*(s16 *)(p + N)` (C3_chassisB_control.c); both scored with score.sh.
- result: difflines 10 and 10 — bit-identical. The spelling is inert. What makes the sibling's constants behave is its dataflow (all uses inside one uninterrupted store run), not its syntax. Banked as rejected/sibling-s32cast-store-spelling-inert.c.
- verdict: KILLED

## [s4] The permuter's best chassis-B reordering (hoisting p[6] = 0x80 between the two 240 stores) improves on the chassis-B control.
- mechanism: moving an early use of the 128 constant between the two 240 stores changes which constant is materialised first and could lift 640 off $v0.
- probe: C1_rgb6_between_240s.c (the permuter output-65 form with its inert pointer temps stripped) scored with score.sh against the chassis-B control.
- result: difflines 12 vs the control's 10 — strictly worse. The permuter's apparent gain (permuter score 65 vs base 270) is entirely an artefact of the mis-specified objective.
- verdict: KILLED

## [s4] decomp-permuter random search, seeded from the banked score-2 candidate and separately from target's own statement order, can find the register-lifetime shape hand enumeration cannot express (session 3 frontier item 2).
- mechanism: The residual is no longer statement order (saturated at 2 over ~45 hand orderings in s2) but a register-lifetime shape - exactly what the permuter's temp-introduction, expression re-association and variable-reuse mutations explore. A base at sandbox 2 should give a clean gradient.
- probe: Built a validated single-function workspace per .claude/rules/difficult-is-not-impossible.md section 3 (tmp/grind/func_8003553C/s4/mkws.sh: target.o from prelude.inc minus '.set gp=64' plus asm/funcs/func_8003553C.s at offset 0; compile.sh runs the real cc1 / prologue_fix / maspsx-2.34 / multu_pad pipeline and extracts only this function's region). Ran two fresh-seed campaigns via tools/permuter_campaign.py: chassis A (wsA, label chassisA-scalar-offsets) seeded from the banked candidate, permuter base 225, stopped on an 8-minute no-improvement window at best 100; chassis B (wsB, label chassisB-target-stmt-order) seeded from target's statement order, base 270, best 50, 16117 iterations. Both harvested with --stop in-session (permuter_campaign.py status now reports 0 live campaigns). All ~85 output forms from both workspaces were then re-scored offline against target.o with tmp/grind/func_8003553C/s4/score.sh.
- result: NO form beats the incumbent (objdump difflines 2 at 43 instructions). Distribution of difflines(insns): 1(44) x1, 2(43) x6, 3 x3, 4(43) x14, 5 x1, 6(43) x13, 8(43) x17, 9-18 x30. The only difflines-1 form emits 44 instructions: chassis A with the leading *(s16 *)(p + 0x10) = 640; store DUPLICATED into the post-load group, i.e. all 43 target instructions in target's exact order plus one extra sh - a +1-instruction redundant dead store, not a match.
- verdict: KILLED

## [s4] The permuter's weighted objective tracks the honest sandbox distance closely enough to be a usable search gradient for this function.
- mechanism: The permuter scores register mismatches x5, reorderings x60 and insertions/deletions x100; if a single displaced instruction were charged as one reordering, the incumbent should sit near the bottom of the permuter scale and real improvements would be visible as score drops.
- probe: Cross-compared permuter base/best scores with offline objdump difflines for the same forms across both chassis.
- result: ANTI-CORRELATED. The incumbent (one instruction displaced by 17 slots, sandbox 2, difflines 2) is charged permuter score 225, while chassis-B forms that are objectively ~5x worse (difflines 10-12, sandbox ~8) score 50-65. A long-range displacement is charged as a large pile of reorderings while register-name agreement is rewarded, so the search gradient points AWAY from the true optimum. Extra sampling or further reseeding cannot repair a mis-specified objective - the permuter axis is closed for this function, not merely unproductive.
- verdict: CONFIRMED

## [s4] Transplanting matched sibling func_80072BC4's store spelling - *(u8 *)((s32)p + N) = v instead of p[N] = v, which does NOT set MEM_IN_STRUCT_P - reproduces its block-head constant materialisation.
- mechanism: func_80072BC4's arg0>=4 else-arm (src/text1b.c:16669-16680) is the structural twin of our body (a POLY_G4 colour block written through a register-held primitive pointer) and GCC gives it three block-head addiu $vN,$zero,K constants with late first uses - precisely the shape our 640 needs. If the sibling's syntax earns that, respelling our stores identically should reproduce it.
- probe: Chassis B respelled with all twenty stores as *(s16|u8 *)((s32)p + N) (tmp/grind/func_8003553C/s4/C2_sibling_s32cast_spelling.c) versus the identical chassis-B control using p[N] / *(s16 *)(p + N) (C3_chassisB_control.c); both scored with score.sh.
- result: difflines 10 and 10 - bit-identical. The spelling is inert; what makes the sibling's constants behave is its dataflow (all uses inside one uninterrupted store run), not its syntax. Banked as rejected/sibling-s32cast-store-spelling-inert.c.
- verdict: KILLED

## [s4] The permuter's best chassis-B reordering (hoisting p[6] = 0x80 between the two 240 stores) improves on the chassis-B control.
- mechanism: Moving an early use of the 128 constant between the two 240 stores changes which constant is materialised first and could lift 640 off $v0.
- probe: C1_rgb6_between_240s.c (the permuter output-65 form with its inert pointer temps stripped) scored with score.sh against the chassis-B control.
- result: difflines 12 versus the control's 10 - strictly worse. The permuter's apparent gain (score 65 from base 270) is entirely an artefact of the mis-specified objective.
- verdict: KILLED

## Session 5 (permuter, 2026-07-30) — floor stays 2; the order/spelling/decl space is exhausted, helper-parameter axis killed

### Resolved
- **H16 KILLED — some combination of statement order, declaration order and
  store spelling puts `li 640` at the block head with BOTH of its stores in the
  post-load group** (session 4 frontier items 1 and 3). 840 forms measured on the
  honest metric; the combination does not exist in the space.
- **H17 KILLED — a helper function taking the 640 as a parameter gives the
  constant an early def (at the call site) with late uses.** `static` is not
  inlined at all by this toolchain; `static inline` inlines and the constant is
  propagated then sunk exactly like a holder local.
- **H18 CONFIRMED (negative, and it corrects the s3/s4 framing) — the banked
  body already reproduces target's constant signature.** Target's own target.o
  disassembles to `li 640 @7 in $v1 / li 240 @8 / li 128 @11`, and so does the
  incumbent. The remaining objective is ONLY the slot of `sh v1,16(s0)`.

## [s5] Some combination of statement order, declaration order, and store spelling materialises 640 at the block head in a register no earlier constant used WHILE both of its stores stay in the post-load group (session 4 frontier items 1 and 3).
- mechanism: s3 proved with a diagnostic-only `register s16 w asm("$3")` pin that if 640 occupies a fresh register, every store — including the one our clean form misplaces — lands in target's slot. s4's sibling census suggested the discriminator is whether the constant's uses lie inside one uninterrupted store run, so repositioning the may-alias-pinned OT load relative to the store run (and varying the store spelling and the declaration order, which changes pseudo numbering and hence local-alloc's allocno order) should be able to buy the register without an early 640 store.
- probe: Built a corrected-objective search (s4 measured decomp-permuter's own objective to be anti-correlated here, so its search cannot be reused): tmp/grind/func_8003553C/s5/search_lib.py generates a standalone form, compiles it through the real pipeline via s4/wsA/compile.sh, and scores it as the objdump instruction-sequence diff against target.o while extracting a constant-materialisation signature `<idx li 640>/<idx li 240>/<idx li 128>@<640's register>`. hill.py then ran an exhaustive 1-token-move hill climb (160 neighbours per iteration, 8-way parallel) from three chassis — the banked candidate, its x3-leading mirror, and TARGET's own statement order — over the cross product with 6 declaration orders, 3 coordinate-store spellings, 2 RGB spellings and 2 OT-base spellings; search.py added 200 uniform-random permutations as a control. 840 forms total.
- result: Minimum difflines over all 840 forms is 2 — the incumbent (50 ties). The climb from TARGET's own statement order (difflines 10, sig 13/7/10@v0) converges in two iterations onto the exact banked order, so the basin has a single attractor. Decisively: 428 forms reach target's constant shape (640 at index 7 in $v1) and EVERY one contains a 640-valued store before the OT load; every form whose two 640 stores are both post-load puts 640 in $v0 at index 11/13/15/17/23 and scores 8 or worse. The early `li` is caused BY the early store; the two roles are inseparable in this space. Uniform-random ordering bottoms out at 18, confirming the hand-picked chassis were the right instrument.
- verdict: KILLED

## [s5] A helper function taking 640 (and 240) as parameters gives the constant an early definition — the argument copy is emitted at the CALL SITE, ahead of the helper's inlined stores — so the constant survives with late uses, which is the shape target has and no in-body spelling reaches.
- mechanism: parameter pseudos are created by the caller at the call point, before any of the callee's body insns, so after inlining the 640 pseudo's def sits at the top of the post-call block while its uses are the inlined stores 12+ insns later. This is also how a human would naturally factor a full-screen-quad builder (PsyQ setXY4/setXYWH style), so it is a legitimate pure-C shape rather than a coercion.
- probe: Four partitionings, each scored with s5/scoreforms.py, first as `static void` and then as `static inline void`: (A) the whole body incl. the OT load and ot_Link in the helper; (B) the same with the q/p tail split in the caller; (C) coordinates + OT + link in the helper, RGB block in the caller; (D) only the post-load group + link in the helper, with the leading 640 store removed.
- result: `static void` is NOT inlined by GCC 2.7.2 at -O2 (the `inline` keyword is required; no -finline-functions) — a real `jal` is emitted and the extracted region is 19/19/32/37 insns, difflines 32/30/17/16. `static inline void` does inline (43 insns in all four) and the parameter constant is propagated into its uses and then sunk exactly as a local holder is: 640 lands in $v0 at index 13/27/27/27, difflines 12/46/28/18. Parameter dataflow is no more resistant to sched1's sink than a holder local. Banked as rejected/inline-static-helper-param-const-still-sunk.c and rejected/noninline-static-helper-real-call-19-insns.c.
- verdict: KILLED

## [s5] The primary objective inherited from s3/s4 — "get li 640 emitted ahead of li 240" — is already satisfied by the banked body, so it cannot be the residual's lever.
- mechanism: If the incumbent's constant order or register differed from target's, that difference would be part of the 2-line diff; it is not.
- probe: Disassembled s4/wsA/target.o and the incumbent's object with the same extractor and compared the constant-materialisation signatures.
- result: Both are `li 640` at block index 7 holding $v1, `li 240` at 8, `li 128` at 11. Identical. The whole residual is the slot of `sh v1,16(s0)` (our index 13 vs target's 30). Any future probe must be scored on the STORE's slot, not on the constant's — and, per H16, no form in the ordering/spelling/declaration space moves the store without losing the constant shape.
- verdict: CONFIRMED

## [s5] Some combination of statement order, declaration order and store spelling materialises 640 at the block head in a register no earlier constant used WHILE both of its stores stay in the post-load group (session 4 frontier items 1 and 3).
- mechanism: s3's diagnostic-only register pin proved that if 640 occupies a fresh register every store lands in target's slot, and s4's sibling census suggested the discriminator is whether the constant's uses lie inside one uninterrupted store run. Repositioning the may-alias-pinned OT load relative to the store run, and varying store spelling and declaration order (which changes pseudo numbering and hence local-alloc's allocno order), should therefore be able to buy the register without an early 640 store. decomp-permuter cannot search for this because s4 MEASURED its weighted objective to be anti-correlated with the honest distance here, so this session rebuilt the same instrument with the objective fixed.
- probe: tmp/grind/func_8003553C/s5/search_lib.py generates a standalone form (s4 proved the TU context is not load-bearing), compiles it through the real pipeline via s4/wsA/compile.sh, and scores it as the objdump instruction-sequence diff against target.o while extracting a constant-materialisation signature '<idx li 640>/<idx li 240>/<idx li 128>@<640's register>'. hill.py ran an exhaustive 1-token-move hill climb (160 neighbours/iteration, 8-way parallel, ~1 min/iteration) from three chassis - the banked candidate, its x3-leading mirror, and TARGET's own statement order - crossed with 6 declaration orders, 3 coordinate-store spellings, 2 RGB spellings and 2 OT-base spellings. search.py added 200 uniform-random permutations as a control. 840 forms measured in total.
- result: Minimum difflines over all 840 forms is 2 - the incumbent (50 ties). The climb from TARGET's own statement order (difflines 10, sig 13/7/10@v0) converges in two iterations onto the EXACT banked order 10,1A,22,R0,R1,R2,R3,OT,08,0A,12,18,20, so the basin has a single attractor and the incumbent is its global optimum. Decisively: 428 forms reach target's constant shape (640 at index 7 in $v1) and EVERY one of them contains a 640-valued store before the OT load; every form whose two 640 stores are both post-load puts 640 in $v0 at index 11/13/15/17/23 and bottoms out at difflines 8. The early li is CAUSED BY the early store - the two roles are inseparable in this space. Uniform-random ordering bottoms out at 18, confirming the hand-picked chassis were the right instrument.
- verdict: KILLED

## [s5] A helper function taking the 640 (and 240) as parameters gives the constant an early definition - the argument copy is emitted at the CALL SITE, ahead of the helper's inlined stores - producing target's 'early def, late uses' shape that no in-body spelling reaches.
- mechanism: Parameter pseudos are created by the caller at the call point, before any callee body insn, so after inlining the 640 pseudo's def sits at the top of the post-call block while its uses are the inlined stores 12+ insns later. It is also how a human would naturally factor a full-screen-quad builder (PsyQ setXY4/setXYWH style), so it is a legitimate pure-C shape rather than a coercion.
- probe: Four partitionings scored with s5/scoreforms.py, each first as 'static void' then as 'static inline void': (A) whole body incl. OT load and ot_Link in the helper; (B) same with the q/p tail split kept in the caller; (C) coordinates + OT + link in the helper, RGB block in the caller; (D) only the post-load group + link in the helper, leading 640 store removed.
- result: 'static void' is NOT inlined by GCC 2.7.2 at -O2 (the inline keyword is required; no -finline-functions): a real jal is emitted and the extracted region is 19/19/32/37 insns, difflines 32/30/17/16. 'static inline void' does inline (43 insns in all four) but the parameter constant is propagated into its uses and then sunk exactly as a local holder is - 640 lands in $v0 at index 13/27/27/27, difflines 12/46/28/18. Parameter dataflow is no more resistant to sched1's sink than a holder local. Banked as rejected/inline-static-helper-param-const-still-sunk.c and rejected/noninline-static-helper-real-call-19-insns.c.
- verdict: KILLED

## [s5] The objective inherited from sessions 3 and 4 - 'get li 640 emitted ahead of li 240 while both 640 stores stay late' - is already half-satisfied: the banked body reproduces target's constant order and register exactly, so the constant is not the residual's lever.
- mechanism: If the incumbent's constant order or register differed from target's, that difference would appear in the 2-line diff. It does not.
- probe: Disassembled s4/wsA/target.o and the incumbent's object with the same extractor and compared the constant-materialisation signatures.
- result: Both are li 640 at block index 7 holding $v1, li 240 at 8, li 128 at 11 - identical. The entire residual is the slot of sh v1,16(s0): our index 13 vs target's 30. Future probes must be scored on the STORE's slot, not the constant's - and per the killed hypothesis above, nothing in the ordering/spelling/declaration space moves the store without losing the constant shape.
- verdict: CONFIRMED
