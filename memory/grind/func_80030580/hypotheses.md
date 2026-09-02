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

## Session 2 (2026-09-02, structural) — floor 2 (flat); residual typed as "need 2 more orphan spill slots"

### CONFIRMED
- H2.1 The target's 24 locals bytes are three 8-byte combine-orphan spill slots, not a
  BLKmode temp. Evidence: corpus census over 20 src files — every leaf (args=0, regs=0,
  0 sp accesses) with vars>=16 is composed only of `ctx=spill_new` 8-byte slots
  (config:func_8003FECC 16, display:get_cs 16, display:get_ce 16, main:SpuSetCommonAttr 16);
  `ctx=stack_temp` occurs only in functions with calls and always carries sp traffic.
- H2.2 The orphan recipe (read off the matched witnesses get_cs/get_ce, src/display.c:556/574):
  one DISTINCT global whose `symbol_ref` pseudo combine folds into a `lh %lo(SYM)(at)` mem,
  with a CODE_LABEL between the (deleted) setter and the fold point ⇒ one 8-byte slot.
  Two distinct globals ⇒ 16. func_80030580 has only three globals and two of them
  (D_8008E194, D_80106A78) are live base registers in the target bytes, so `Judge` is the
  only orphan source available without changing bytes.
- H2.3 The one slot we do get comes from the SECOND Judge lookup's combination, not the
  first: respelling only the second lookup (`jmix`) drops vars 8 -> 0.

### KILLED (all instance kills; chassis = draft3 body at sandbox 2, no FAKE constructs)
- K2.1 44 structural respellings (loop shape, declaration order, block scoping, type
  narrowing, statement grouping/re-association, pointer-vs-array-vs-COMPONENT_REF access
  forms, sibling func_80032064's hand-spelled `/32` and `(s32)*(&Judge + i)` idiom,
  global-indexed table access, moving the `/32` statement between the velocity stores) all
  measure vars=8 with exactly one spill slot. 15 of them are body-neutral (fdiff 0) and
  are banked as free-to-compose; the rest change the body.
- K2.2 Struct-valued-expression spellings of the pos->old copy do not allocate a BLKmode
  stack temp on this chassis: ARRAY_REF (`((Vec3i*)(obj+0x2C))[1] = [0]`), function-scope
  `struct PB { Vec3i pos, old, vel; }` COMPONENT_REF (`pb->old = pb->pos`), function-scope
  union member copy, and pointer-mediated variants all stay at vars=8, sp_acc=0, fdiff=0.
- K2.3 Making the two Judge lookups structurally symmetric (both u16-indexed, both
  s16-indexed, an extra mask on the second, a `(u16)` cast on the second) does not produce
  a second orphan; each still yields exactly one slot and changes the body by 2-3 lines.

### FRONTIER (highest value first)
- F2.1 Resolve the Judge asymmetry. Both lookups have identical pre-combine RTL
  (s2/base.rtl insns 131/136/138 vs 166/171/173) and combine folds both, yet reg 130's
  REG_DEAD note never reaches the label-walk. Mechanism to confirm: the `elim_i2` early
  `break` at combine.c:10761 (`if (XEXP (note, 0) == elim_i2 || ... ) break;`) drops the
  note when the dying register is exactly the one the i2->i3 fold eliminated, whereas
  reg 110's note arrived via a different (3->2) combination path. Probe: add a
  BB2_COMBINE_DEBUG print in distribute_notes recording (note reg, from_insn, i2, i3,
  elim_i2, chosen place) and diff the two lookups; then look for a body-neutral spelling
  that pushes the second lookup onto the first's combination path. Each success = +8 bytes.
- F2.2 Where does a THIRD slot come from? get_cs gets its two from two DISTINCT globals.
  Ours has one foldable global. Probe: check whether a single global folded at N call
  sites can orphan more than once (census the corpus for a leaf with vars=24 and grep its
  C for how many distinct globals it reads), and check func_8003FECC / SpuSetCommonAttr
  for whether their two slots come from two globals or from one global plus another
  pseudo class.
- F2.3 If F2.1/F2.2 show only two slots are reachable from `Judge`, re-examine whether the
  target's third slot comes from a pseudo class we have not seen yet — enumerate every
  `ctx=` label the instrumented cc1 can emit (grep BB2_FRAME_DEBUG in tools/gcc-2.7.2/)
  and census the corpus for a leaf carrying that ctx with 0 sp accesses.

## [s2] The target's 24 locals bytes are three 8-byte combine-orphan spill slots rather than one 16-byte BLKmode stack temp plus our 8
- mechanism: reload1 alter_reg gives a stack slot to a class-less pseudo whose only reference is the combine.c:10836 (use (reg)) emitted after a CODE_LABEL; assign_stack_local aligns each such slot to 8
- probe: tmp/grind/func_80030580/s2/census2.py over 20 src/*.c — group every FRAMEDBG allocation by function, filter to leaves (args=0, regs=0, 0 sp accesses) with vars>=16
- result: all four such leaves (config:func_8003FECC, display:get_cs, display:get_ce, main:SpuSetCommonAttr) are exactly two spill_new slots; no leaf reaches vars>0 via ctx=stack_temp
- verdict: CONFIRMED

## [s2] Struct-valued expression spellings of the pos->old copy on the draft3 chassis with no FAKE constructs leave vars at 8 and allocate no BLKmode stack temp
- mechanism: expand_expr would allocate a BLKmode temp via assign_stack_temp for a struct rvalue that needs materialising, then bypass it when the destination is a known MEM
- probe: frame2.py variants arridx, arridx2, compref, comprefp, unioncpy, copylast — FRAMEDBG census + ($sp) count + fdiff vs var_base.s
- result: every spelling vars=8 with the single spill_new slot, sp_acc=0; the four body-neutral ones (arridx, arridx2, compref, comprefp, unioncpy) confirm the copy shape is frame-inert
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis, sandbox 2, no FAKE constructs

## [s2] Structural respellings of the loop, declaration order, block scoping, type narrowing, access forms and statement placement on the draft3 chassis with no FAKE constructs leave vars at 8
- mechanism: a different expand/cse tree could leave additional combine.c REG_DEAD-orphan pseudos, each worth an 8-byte reload spill slot
- probe: 44 variants through tmp/grind/func_80030580/s2/frame2.py + extra2.py (hd32, castjudge, spd, sibfull, hd32cast, veccopy, vely, dord1-3, blk1, i16, iu, lp_nest, lp_while, lp_hi, c_255, z16, acopy, spin, slot8, a0idx, tbl_late, ang32, hd32top, arridx, arridx2, jmix, jmix2, copylast, s32idx, bothu16, u16cast, secondzx, firstsx, compref, comprefp, unioncpy, tblidx, tblidx_arm, posy_mid, posy_last, base)
- result: all 44 at vars=8 with exactly one spill_new slot; 15 are body-neutral (fdiff 0) and banked as composable; jmix alone moved the frame, and downward (vars=0, fdiff 4)
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis, sandbox 2, no FAKE constructs

## [s2] Making the two Judge lookups structurally symmetric produces a second combine orphan
- mechanism: identical pre-combine RTL shapes should take the same distribute_notes path and each orphan its folded symbol pseudo
- probe: frame2.py variants bothu16, u16cast, secondzx, firstsx, sfirst(s1), castjudge, jmix2
- result: every symmetric spelling still yields exactly one slot (vars=8) and changes the body by 2-3 fdiff lines; the asymmetry is inside combine, not in the source shape
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis, sandbox 2, no FAKE constructs

## [s2] The target's 24 locals bytes are three 8-byte combine REG_DEAD-orphan spill slots rather than one 16-byte BLKmode stack temp plus our existing 8
- mechanism: reload1 alter_reg hands a stack slot to a class-less pseudo whose only reference is the (use (reg)) that combine.c:10836 emits after a CODE_LABEL; assign_stack_local aligns each such slot to 8 bytes
- probe: tmp/grind/func_80030580/s2/census2.py compiled 20 src/*.c with the instrumented cc1 (BB2_FRAME_DEBUG=1), grouped every FRAMEDBG allocation by function and filtered to leaves carrying the target's frame signature (args=0, regs=0, zero ($sp) accesses) with vars>=16
- result: All four such leaves in the corpus (config:func_8003FECC, display:get_cs, display:get_ce, main:SpuSetCommonAttr) are composed of exactly two ctx=spill_new slots of 8 bytes each; no leaf anywhere reaches vars>0 through a BLKmode ctx=stack_temp. In this TU ctx=stack_temp appears only in functions that have calls (func_800300B4 16/8/32, func_8002FF20 32, func_80032064's real s16 sp_area[2]) and always carries sp traffic. get_cs/get_ce (src/display.c:556/574, both COMPLETED-C leaves) give the generative recipe: one 8-byte slot per DISTINCT global whose symbol_ref pseudo combine folds into a lh %lo(SYM)(at) mem across a CODE_LABEL.
- verdict: CONFIRMED

## [s2] Struct-valued expression spellings of the pos->old copy on the draft3 chassis with no FAKE constructs allocate a BLKmode stack temp and raise vars above 8
- mechanism: expand_expr allocates a BLKmode temp via assign_stack_temp for a struct rvalue that must be materialised, then bypasses it when the destination turns out to be a known MEM, leaving the slot unreferenced
- probe: frame2.py variants arridx (((Vec3i*)(obj+0x2C))[1] = [0]), arridx2, compref (function-scope struct PB { Vec3i pos, old, vel; } with pb->old = pb->pos), comprefp, unioncpy (function-scope union member copy) and copylast - each measured for vars=, the FRAMEDBG census, the ($sp) count and fdiff vs s1/var_base.s
- result: Every spelling stayed at vars=8 with the same single ctx=spill_new slot and sp_acc=0; five of the six are byte-neutral (fdiff 0), so the copy shape is frame-inert on this chassis. No ctx=stack_temp line was emitted for func_80030580 by any variant.
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s2] Structural respellings of the loop, declaration order, block scoping, type narrowing, statement placement, and pointer-vs-array-vs-COMPONENT_REF access shapes on the draft3 chassis with no FAKE constructs raise vars above 8
- mechanism: a different expand/cse tree could leave additional combine.c REG_DEAD-orphan pseudos, each worth an 8-byte reload spill slot
- probe: 44 variants through tmp/grind/func_80030580/s2/frame2.py + extra2.py: base, hd32, castjudge, spd, sibfull, hd32cast, veccopy, vely, dord1, dord2, dord3, blk1, i16, iu, lp_nest, lp_while, lp_hi, c_255, z16, acopy, spin, slot8, a0idx, tbl_late, ang32, hd32top, arridx, arridx2, jmix, jmix2, copylast, s32idx, bothu16, u16cast, secondzx, firstsx, compref, comprefp, unioncpy, tblidx, tblidx_arm, posy_mid, posy_last
- result: All 44 measured vars=8 with exactly one spill_new slot. 15 are body-neutral (fdiff 0) and are banked in evidence.md as free to compose with future levers: hd32 (sibling func_80032064's hand-spelled /32), castjudge, hd32cast, vely, dord1-3, lp_nest, lp_while, c_255, acopy, slot8, a0idx, s32idx, ang32, arridx/arridx2/compref/comprefp/unioncpy, jmix2. The only variant that moved the frame at all moved it DOWN: jmix (second Judge lookup as a byte-offset address) gives vars=0, which proves the single slot we have is produced by the SECOND Judge lookup's combination.
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s2] Making the two Judge lookups structurally symmetric in the source produces a second combine orphan on the draft3 chassis with no FAKE constructs
- mechanism: the two lookups have identical pre-combine RTL (s2/base.rtl insns 131/136/138 and 166/171/173) and combine folds both symbol_ref pseudos into lh %lo(Judge)(at) mems, so a symmetric source shape should take the same distribute_notes path twice
- probe: frame2.py variants bothu16 (both indices u16-based), firstsx (both s16-based), u16cast ((u16) cast on the second), secondzx (extra mask on the second), plus castjudge and jmix2 (pointer-add spellings)
- result: Every symmetric spelling still yields exactly one 8-byte slot (vars=8) and costs 2-3 fdiff lines of body. The asymmetry lives inside combine, not in the source shape: reg 110's death note reaches the label-walk and orphans after code_label 99, while reg 130's note is dropped - most likely at the elim_i2 early break, combine.c:10761.
- verdict: KILLED
- kill_scope: instance
- measured_on: draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present
