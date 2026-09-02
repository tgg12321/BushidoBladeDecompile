# Hypothesis ledger — func_80030580

## Session 1 (2026-09-02, recon) — floor 2

### CONFIRMED
- H1.1 Loop = explicit pointer biv with `for (i = 0; i < 12; i++, obj += 0x64)`; loop.c gives the
  `v1 = obj+0xA` giv; increment order i++ then obj bump. (draft1→draft3 loop hunk: 4 insns → 0.)
- H1.2 Kind dispatch = linear if/else-if chain 1,2,3 with the case-1 and case-3 bodies written
  identically (cross-jumped by jump.c). (switch → chain: 6 insns → 0.)
- H1.3 Whole body byte-identical with draft3 (pairdiff: only the two `addiu sp` insns differ).
- H1.4 Our 8-byte frame = one combine.c REG_DEAD-orphan USE slot (combine.c:10836) for the first
  Judge lookup's address pseudo; target = 24 = two more such slots or one untouched 16-byte
  BLKmode temp. Mechanism read from .rtl/.combine/.lreg dumps + BB2_FRAME_DEBUG census.

### KILLED (all instance kills, chassis = draft3 body, no FAKE constructs present)
- K1.1 `obj = base + i*0x64` in the loop body (derived giv) — measured 15 vs 2.
- K1.2 `switch (tbl[0])` — compare tree, +6 insns.
- K1.3 `obj += 0x64` as last body statement (after the break test) — increment order reversed, +2.
- K1.4 Statement-order variants of the post-loop block change the body (fdiff lines): zxy 170,
  yxz 22, xzy 168, p12x 55, tblfirst 18, tbltop 18, velafterpos 160, idx 22, mulorder 40,
  objidx 19, sincos 185, ang 165, jp 19. None changes vars= from 8.
- K1.5 Body-neutral frame probes all stay at vars=8: sfirst, tblu8, sn32, ptrjudge, bytejudge,
  kind, dy, pv, pv_late, pos_only, srcp, nocopy. A real `Vec3i tmp` gives vars=24 but emits
  6 sp accesses (+6 insns) — not the target.

### FRONTIER (next session: forensics on the frame; body is done)
- F1 Two more orphan-USE slots. Mechanism: combine.c distribute_notes REG_DEAD fallback
  (combine.c:10836) fires when a pseudo dies in a combined insn and its setter is already a
  NOTE; only the FIRST Judge lookup trips it now. Probe: count `(use (reg` lines in the
  .combine dump per variant (dump.ps1 or frame_instr.sh + grep) while sweeping spellings that
  keep the body: struct-typed `obj` (`obj->vel.x`, COMPONENT_REF trees), a struct-typed
  `src`, the sibling func_80032064's hand-spelled `/32` (`if (v1 < 0) v1 += 0x1F; >> 5`),
  `s32 speed = tbl[2]`-style locals for the two multiplies, and reading the angle via a
  `u16` field for the copy at obj+0x56.
- F2 One untouched 16-byte BLKmode temp (assign_stack_temp; Vec3i rounds 12→16). Probe:
  FRAMEDBG `ctx=stack_temp` while sweeping struct-valued expression shapes that expand may
  materialise then bypass (chained struct assignment `a = b = *p`, struct copy through a
  conditional, struct copy where the RHS is a COMPONENT_REF of a larger struct, struct
  copy of `pos` BEFORE the `+=` passes with the += then applied to both). Accept only
  variants with 0 sp accesses in the body.
- F3 Confirm the phantom mechanism on the matched sibling func_80032064 (FRAMEDBG census of
  its committed C) — if its 0x28 frame also carries orphan slots from the same Judge idiom,
  its spelling is a direct template for ours.

## [s1] Loop shape: explicit pointer biv `for (i = 0; i < 12; i++, obj += 0x64)` with the break test in the body reproduces the v1=obj+0xA giv and the i++-then-bump increment order
- mechanism: loop.c strength reduction (giv of the explicit biv), reorg.c delay-slot placement of the obj bump
- probe: sandbox --disable all + tools/pairdiff.py on draft1/draft2/draft3
- result: loop hunk 4 insns -> 0; draft2 (obj derived from i) scored 15
- verdict: CONFIRMED

## [s1] Kind dispatch is a linear if/else-if chain on 1,2,3 with the case-1 and case-3 bodies written identically (cross-jumped), not a switch
- mechanism: jump.c cross-jump merges identical tails; switch expansion emits a compare tree for 3 cases
- probe: sandbox + pairdiff draft1 (switch) vs draft2 (chain)
- result: switch: +6 insns (li 2/beq/slti 3/beqz tree); chain matches
- verdict: CONFIRMED

## [s1] The 8-byte frame of the current form is a reload spill slot for pseudo 110 (first Judge lookup address) whose only reference is a combine-emitted (use (reg 110)) at the block head
- mechanism: combine.c:10836 distribute_notes REG_DEAD fallback emits a USE after the CODE_LABEL when the death note finds no home; lreg gives the pseudo no class; reload1 alter_reg allocates the slot; the USE emits nothing
- probe: BB2_FRAME_DEBUG census (spill_new_p110 size=8) + .rtl/.combine/.lreg dumps in tmp/grind/func_80030580/dumps
- result: confirmed by dump read; target 24 = this + 16 more
- verdict: CONFIRMED

## [s1] Reordering the post-loop statements (vel z/x/y permutations, interleaved pos/vel updates, tbl computed earlier, idx/sin/cos/ang locals, tbl[2]*Judge operand order, obj derived from i) on the draft3 chassis with no FAKE constructs keeps the body identical
- mechanism: sched.c would re-sort the block regardless of source order
- probe: frame.py sweep: fdiff vs var_base.s for zxy/yxz/xzy/p12x/tblfirst/tbltop/velafterpos/idx/mulorder/objidx/sincos/ang/jp
- result: all change the body (18-185 fdiff lines); draft3 order is load-bearing
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis, sandbox 2, no FAKE constructs

## [s1] Body-neutral respellings of the Judge index, tbl address, s16 kind/dy locals, Vec3i* pointer locals into obj/src, and scalar-vs-struct copy, on the draft3 chassis with no FAKE constructs, raise vars= above 8
- mechanism: different expand trees could leave additional combine.c REG_DEAD-orphan pseudos
- probe: frame_instr.sh sweep: sfirst tblu8 sn32 ptrjudge bytejudge kind dy pv pv_late pos_only srcp nocopy
- result: all vars=8 with fdiff=0; jp (s16* alias of Judge) drops to vars=0 and changes the body; a real Vec3i tmp reaches 24 but emits 6 sp accesses
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis, sandbox 2, no FAKE constructs
