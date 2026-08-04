# Hypothesis ledger — func_80021280

## Frontier after s1 (recon, 2026-08-03) — floor 2, diff = adjacent swap of `move t1,v1` / `move a1,zero`

Start from form C in candidate.c (no-init decls, assignments `t1=val; a1=0; t4; t3; t2; mode; t0;`
— emitted order exactly textual). The ONLY remaining diff is that target wants
`move a1,zero` BEFORE `move t1,v1`. Constraint (measured s1): writing `a1=0;`
textually first adds an a1<->t1 RA conflict and regresses to 19. So the flip must
come from something that does not change pre-RA liveness.

1. **Forensics: identify the pass that lifted a1 in the OLD form.** In the old
   floor-2 form (a1=0 textually last), a pass lifted `move a1,zero` past the two
   load pairs (emitted 5th, textual 7th) but in form C nothing moves. Build a
   standalone repro of BOTH forms, run cc1 with `-da` (see
   [[instrumented-cc1-location]]: tools/gcc-2.7.2/cc1 for the BB2_*_DEBUG hooks),
   and diff .greg/.sched/.sched2 dumps to find which pass moved it and under what
   priority/tie-break rule. If it is sched2 (post-RA), a C-visible dependence
   tweak that raises `move a1,zero`'s priority (its in-block consumer is the
   rotated loop head `sll v0,a1,2` at block end) or lowers `move t1,v1`'s could
   flip the adjacent pair WITHOUT touching RA. Probe grid follows from the rule.

2. **t1-set spellings that tolerate the a1 conflict.** The 19-regression comes
   from the specific conflict a1<->t1 when a1=0 precedes t1=val. Sweep spellings
   of t1's set/type/provenance (keeping `a1=0;` first) hunting one where RA still
   assigns $t1/$a1 correctly despite the conflict. Known dead: t1 as s32 (4),
   t1 reloaded from memory (23), fresh inner counter (19), decls-with-init
   orderings (2..6, never better). Untried: t1 initialized from a second local
   copy of val; t1 assigned after the whole li group but before mode/t0
   (a1=0; t4; t3; t2; t1=val; mode; t0 — order of the middle group vs target
   TBD); u8/s8/s16 typings of t4/t3/t2 that change which pseudo wins the tie.

3. **Directed permuter from form C.** Seed a campaign from candidate.c mutating
   ONLY the preamble statement order/spellings (PERM_* on the seven assignments +
   decl styles). The search space is small and the target is a single adjacent
   transposition; permuter may find the legal C89 spelling the manual grid
   misses. Follow permuter-fresh-seed discipline; stop the campaign in-turn.

Dead families (do NOT revisit): any note-carrying loop for loop2 (li-5 hoist,
target has in-loop 5); a1=0 textually first in any decl arrangement measured so
far (19); function-scope no-init hoisting (19); split `a1<<2` (neutral);
everything in the WIP rejected_forms list.

## [s1] Target's loop2 preamble order (a1=0 first, then t1,t4,t3,t2,mode,t0) came from loop.c hoisting body-declared invariants into the preheader after a textually-prior a1=0
- mechanism: NOTE_INSN_LOOP_BEG-carrying loops run loop.c scan_loop/move_movables; each movable is emit_insn_before(loop_start), landing after pre-existing preheader statements
- probe: do-while with all six invariant inits inside the body; second variant with decls outside and only the goto converted to do-while
- result: 35 and 34 (build 73 vs target 72). Counter-init DID land first and cheap inits hoisted, but mode/t0 global loads did NOT hoist (invariant_p rejects MEMs vs the sh-through-a2 loop stores), RA scrambled, and loop.c hoisted the bare constant 5 (li 5) which target materializes INSIDE the loop (addiu v0,zero,5 at 0x8002134C)
- verdict: KILLED

## [s1] A legal-C89 form with a1=0 textually FIRST (block-scope no-init decls + assignments in target order) makes the emitted preamble match target
- mechanism: RTL statement order is textual for assignment statements; scheduler was observed to respect source order in this block
- probe: u16 t1; s32 t4,t3,t2,mode; u8 t0; then a1=0; t1=val; t4=4; ... in the if-block
- result: 19 (72/72) - the $a1/$a2 register swap returns. a1 live across t1's birth adds an a1<->t1 pseudo conflict that flips the allocator low-reg tie. Moving a1=0 to SECOND (after t1=val) restores floor 2 with emitted order exactly textual: t1,a1,t4,t3,t2,mode,t0
- verdict: KILLED

## [s1] Splitting a1<<2 into its own named local shifts the scheduling of the preamble (WIP next_avenues item 3)
- mechanism: Separate statement changes RTL grouping of the rotated loop head
- probe: s32 sh2 = a1 << 2; u16 nibble = (t1 >> sh2) & 0xF;
- result: floor 2 unchanged, identical bytes - neutral
- verdict: KILLED
