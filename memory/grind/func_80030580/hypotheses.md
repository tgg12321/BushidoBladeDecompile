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

## Session 3 (2026-09-02, structural) - floor 2 (flat); the frame residual has a generative law

### CONFIRMED
- H3.1 slots = max(0, indexed-Judge lookup sites - 1), one 8-byte `ctx=spill_new` each.
  Measured 0/1/2/3/4 sites -> vars 0/0/8/16/24 (j0read, j1only, base, j3read, j4read).
  The target's vars=24 needs FOUR source-level sites while emitting only the two
  `lh %lo(Judge)` loads it actually has.
- H3.2 The generator is the array-INDEXED global fold `(mem (plus (reg idx) (symbol_ref)))`.
  Constant-address global reads do not orphan (otherglob/otherglob2 stay at vars=8).
- H3.3 The target's exact frame IS reachable on this chassis: `dupXZ` (both velocity
  statements duplicated) measures vars=24 with three spill_new slots - at a cost of 199
  fdiff body lines. The frame is not the obstacle; a byte-neutral 4-site spelling is.

### KILLED (all instance kills; chassis = candidate.c/draft3 body at sandbox 2, no FAKE constructs)
- K3.1 s2's recipe reading ("one slot per DISTINCT global folded across a CODE_LABEL").
  Refuted by the corpus census: func_80041E10 has three orphan slots with no branch or
  label in its body, and func_80042874 has six from six indexed lookups of ONE global.
- K3.2 26 further structural respellings on new axes (ternary-shaped `/32` and `/2`,
  block-scope `extern` redeclaration, integer-cast address arithmetic, `%` and `~` mask
  spellings, HImode/QImode narrow intermediates, redundant global-address expressions
  that CSE merges, TU-wide array declaration of Judge) all measure vars=8.
- K3.3 Cross-jump-mediated duplication of a Judge-dependent statement into the kind
  chain's arms does not come out byte-neutral (arms2 +17, armsvx_keep +73, armsvx +84).

### FRONTIER (highest value first)
- F3.1 A byte-neutral 3rd and 4th indexed-Judge site. Only jump2 cross-jumping removes
  bytes after combine, so the duplicate's merged code must coincide with insns the target
  already emits. Probe: enumerate the target's own cross-jump merge points
  (asm/funcs/func_80030580.s) and try placing a duplicate lookup so its emitted insns are
  exactly the ones already at the merge tail. Cheapest measured duplicate so far is
  arms2 at +17 fdiff.
- F3.2 Why "sites - 1" and not "sites"? One site's address pseudo always escapes and gets
  a hard register. If that site can be made to orphan too, THREE sites suffice (and a 3rd
  site costs less body than a 4th: j3read +18 vs j4read +25). Probe: read the .lreg/.greg
  dumps for j3read and diff the surviving pseudo's class against the two that orphan.
- F3.3 Other `ctx=spill_new` generators that cost no bytes. func_80041E10's three orphans
  come from constant-offset STORES into a global array (`sh $x,g_anim_select+k`), a shape
  our function does not have; func_8001B478 / SetDrawEnv / func_800460E4 / func_80041AC8
  are further 3-slot witnesses whose RTL shapes have not been read. Probe: classify each
  witness's orphan source, then look for one our function could carry byte-neutrally.

## [s3] The number of 8-byte combine-orphan spill slots in func_80030580 equals the number of source-level indexed `(&Judge)[...]` lookup sites minus one
- mechanism: each indexed global read creates its own address pseudo `idx*2 + &Judge`; combine folds the read into `(mem (plus (reg idx) (symbol_ref "Judge")))`, the pseudo dies with no home for its REG_DEAD note, combine emits a bare `(use (reg N))`, lreg gives the pseudo no register class and reload1 alter_reg hands it an 8-byte stack slot that nothing ever touches. Exactly one site per function escapes this and keeps a hard register.
- probe: tmp/grind/func_80030580/s3/frame3.py variants j0read (both lookups replaced by 0), j1only (second lookup replaced by 0), base (the candidate, 2 sites), j3read (a third lookup added to the vel-y store), j4read (a third and fourth) - each measured for vars=, the FRAMEDBG census, the ($sp) count and fdiff vs s1/var_base.s
- result: vars = 0, 0, 8, 16, 24 for 0, 1, 2, 3, 4 sites, with 0/0/1/2/3 ctx=spill_new slots respectively. The target's 24 bytes therefore correspond to FOUR source-level lookup sites, while asm/funcs/func_80030580.s contains exactly two lui %hi(Judge) / lh %lo(Judge)($at) pairs (lines 59/61 and 76/78) in straight-line code.
- verdict: CONFIRMED

## [s3] The orphan generator is any read of a global, including constant-address scalar reads
- mechanism: combine folds a global's symbol_ref pseudo into the mem operand of the access, so any global read should leave a dying address pseudo
- probe: frame3.py variants otherglob (+ D_8008EBA0 added to the vel-y store) and otherglob2 (+ D_8008EBA0 + D_80101E02)
- result: both stay at vars=8 with the single spill_new slot (bodydiff 6 and 10). A constant-address global read folds to lh %lo(SYM)($at) and needs no index register, so no `plus` pseudo exists to orphan. Only the ARRAY-INDEXED form `(mem (plus (reg idx) (symbol_ref)))` generates a slot.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s3] Session-2's recipe reading - one slot per DISTINCT global folded across a CODE_LABEL - describes the generator
- mechanism: s2 read the single orphan off the .combine dump as (use (reg 110)) emitted after code_label 99 (the /32 bgez target) and generalised from display.c's get_cs/get_ce, whose two globals are folded inside a ternary
- probe: tmp/grind/func_80030580/s3/census3.py over every src/*.c, reporting any function with >=3 ctx=spill_new slots or a leaf (args=0, regs=0, sp_acc=0) with vars>=24
- result: text1a_post:func_80041E10 (src/text1a_post.c:465) carries THREE orphan slots with vars=24, regs=0, args=0, sp_acc=0 - our target's exact frame signature - and its emitted body contains no branch and no label at all (three divide-by-255 mult/mfhi sequences and three sh $x,g_anim_select+k stores). text1a_c:func_80042874 (src/text1a_c.c:193) carries SIX slots from roughly six indexed lookups of the SINGLE global Judge. The CODE_LABEL is incidental and the "distinct global" count is not the quantity that matters.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-corpus census with the instrumented cc1 (BB2_FRAME_DEBUG=1) over every src/*.c at HEAD, no FAKE constructs present

## [s3] Structural respellings on the ternary, block-scope-extern, integer-cast-address, mask-spelling, narrow-mode and redundant-global-address axes raise vars above 8 on this chassis
- mechanism: a different expand/cse tree could leave additional folded address pseudos to orphan, each worth an 8-byte reload spill slot
- probe: 26 variants through tmp/grind/func_80030580/s3/frame3.py + extra3.py: d32tern, div2tern, bothtern, d32tern_x, jlocalext, alllocalext, jlocalext_late, s32addr, s32addr1, s32addr2, modmask, judgearr, hi1, hi2, hi12, qi6, hitest, hiarg, notmask, notmask2, notmaskboth, hloc, hloc2, tblglob0, tblglob2, tblglob4, tbl2decl, objglob, dupread
- result: every one measured vars=8 with exactly one ctx=spill_new slot. 17 are body-neutral and are banked in evidence.md as composable, notably judgearr (TU-wide `extern s16 Judge[];` with Judge[i] indexing, TUdiff=0 so the rest of code6cac_b.c is unaffected), the three block-scope extern redeclarations, the integer-cast address spellings, and dupread. The ternary spelling of the /32 that mirrors display.c's get_cs/get_ce costs 41 fdiff lines, so the witness's ternary shape does not transfer.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s3] A byte-neutral third and fourth indexed-Judge site can be obtained by duplicating a Judge-dependent statement so that CSE or jump2 cross-jumping removes the extra emitted code
- mechanism: CSE before combine, or cross-jumping after it, could delete the duplicate's insns while the duplicate's own address pseudo still orphans during combine
- probe: frame3.py variants dupread (same lookup assigned twice to one local), dupX / dupZ / dupXZ (the velocity statements duplicated in place), arms2 (vel.x duplicated into the two already-cross-jumped kind-chain arms), armsvx_keep (into all four arms, original kept), armsvx (moved into all four arms)
- result: dupread is byte-neutral but CSE deletes the pseudo too, leaving vars=8. Every duplicate that survives to combine adds emitted insns: dupX +15 fdiff (vars=16), dupZ +16 (vars=16), dupXZ +199 (vars=24 - the target's exact frame with three slots), arms2 +17 (vars=16), armsvx_keep +73 (vars=16), armsvx +84 (vars=8). The frame is reachable; on this chassis no measured duplication of a Judge-dependent statement is byte-neutral.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s3] The number of 8-byte combine-orphan spill slots in func_80030580 equals the number of source-level indexed (&Judge)[...] lookup sites minus one
- mechanism: Each indexed global read creates its own address pseudo (idx*2 + &Judge); combine folds the read into (mem (plus (reg idx) (symbol_ref "Judge"))), the pseudo dies with no home for its REG_DEAD note, combine emits a bare (use (reg N)), lreg gives the pseudo no register class, and reload1 alter_reg hands it an 8-byte stack slot that no instruction ever touches. Exactly one site per function escapes this and keeps a hard register.
- probe: tmp/grind/func_80030580/s3/frame3.py variants j0read (both lookups replaced by 0), j1only (second lookup replaced by 0), base (the candidate, 2 sites), j3read (a third lookup added to the vel-y store) and j4read (a third and a fourth), each measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) for vars=, the FRAMEDBG slot census, the ($sp) access count and fdiff vs s1/var_base.s
- result: vars = 0, 0, 8, 16, 24 for 0, 1, 2, 3, 4 sites, with 0/0/1/2/3 ctx=spill_new slots respectively. The target's 24 locals bytes therefore correspond to FOUR source-level lookup sites, while asm/funcs/func_80030580.s carries exactly two lui %hi(Judge) / lh %lo(Judge)($at) pairs (lines 59/61 and 76/78) in straight-line code with no nearby labels.
- verdict: CONFIRMED

## [s3] The orphan generator is any read of a global, including a constant-address scalar read
- mechanism: combine folds a global's symbol_ref pseudo into the mem operand of the access, so any global read should leave a dying address pseudo behind
- probe: frame3.py variants otherglob (a read of D_8008EBA0 added to the vel-y store) and otherglob2 (D_8008EBA0 + D_80101E02)
- result: Both stayed at vars=8 with the single ctx=spill_new slot (bodydiff 6 and 10). A constant-address global read folds to lh %lo(SYM)($at) and needs no index register, so there is no plus-pseudo to orphan. Only the array-indexed form (mem (plus (reg idx) (symbol_ref))) generates a slot.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s3] Session 2's recipe reading - one slot per DISTINCT global folded into lh %lo(SYM)(at) across a CODE_LABEL - describes the generator for this frame
- mechanism: s2 read the single orphan off the .combine dump as (use (reg 110)) emitted after code_label 99 (the /32 bgez target) and generalised from display.c's get_cs/get_ce, whose two distinct globals are folded inside a ternary
- probe: tmp/grind/func_80030580/s3/census3.py compiled every src/*.c with the instrumented cc1 and reported any function with >=3 ctx=spill_new slots, plus any leaf (args=0, regs=0, sp_acc=0) with vars>=24
- result: text1a_post:func_80041E10 (src/text1a_post.c:465) carries THREE orphan slots at vars=24, regs=0, args=0, sp_acc=0 - our target's exact frame signature - and its emitted body contains no branch and no label at all (three divide-by-255 mult/mfhi sequences and three sh $x,g_anim_select+k stores). text1a_c:func_80042874 (src/text1a_c.c:193) carries SIX slots from roughly six indexed lookups of the SINGLE global Judge. The CODE_LABEL is incidental and the distinct-global count is not the governing quantity.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-corpus census with the instrumented cc1 (BB2_FRAME_DEBUG=1) over every src/*.c at HEAD, no FAKE constructs present

## [s3] Structural respellings on the ternary, block-scope-extern, integer-cast-address, mask, narrow-mode and redundant-global-address axes raise vars above 8 on this chassis
- mechanism: a different expand/cse tree could leave additional folded address pseudos to orphan, each worth an 8-byte reload spill slot
- probe: 26 variants through tmp/grind/func_80030580/s3/frame3.py + extra3.py: d32tern, div2tern, bothtern, d32tern_x, jlocalext, alllocalext, jlocalext_late, s32addr, s32addr1, s32addr2, modmask, judgearr, hi1, hi2, hi12, qi6, hitest, hiarg, notmask, notmask2, notmaskboth, hloc, hloc2, tblglob0, tblglob2, tblglob4, tbl2decl, objglob, dupread
- result: Every one measured vars=8 with exactly one ctx=spill_new slot. 17 are body-neutral and are banked in evidence.md as free to compose - notably judgearr (TU-wide extern s16 Judge[]; with Judge[i] indexing, TUdiff=0 so the rest of code6cac_b.c stays byte-identical), the three block-scope extern redeclarations, the integer-cast address spellings and dupread. The ternary spelling of the /32 that mirrors get_cs/get_ce costs 41 fdiff lines, so the witness's ternary shape does not transfer.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s3] A byte-neutral third and fourth indexed-Judge site can be obtained by duplicating a Judge-dependent statement so that CSE or jump2 cross-jumping removes the duplicate's emitted code
- mechanism: CSE before combine, or cross-jumping after it, could delete the duplicate's insns while the duplicate's own address pseudo still dies during combine and orphans into a spill slot
- probe: frame3.py variants dupread (the same lookup assigned twice to one local), dupX / dupZ / dupXZ (the velocity statements duplicated in place), arms2 (vel.x duplicated into the two already-cross-jumped kind-chain arms), armsvx_keep (into all four arms with the original kept) and armsvx (moved into all four arms)
- result: dupread is byte-neutral but CSE deletes the pseudo along with the read, leaving vars=8. Every duplicate that survives to combine adds emitted insns: dupX +15 fdiff (vars=16), dupZ +16 (vars=16), dupXZ +199 (vars=24 - the target's exact frame, three slots), arms2 +17 (vars=16), armsvx_keep +73 (vars=16), armsvx +84 (vars=8). The frame is reachable; no measured duplication of a Judge-dependent statement is byte-neutral on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## Session 4 (2026-09-02, permuter) - floor 2 (flat); two campaigns, 84k iterations, 0 improvement

### KILL RE-AUDIT (mandated; floor flat 3 sessions)
Re-measured on the CURRENT chassis this session: `sandbox --disable all` on the applied
candidate = 2 (unchanged). `tools/fake_ablate.py --func func_80030580 --file code6cac_b
--candidate memory/grind/func_80030580/candidate.c` reports "no FAKE-annotated constructs
found; nothing to ablate" - the candidate and every banked variant are FAKE-free, so
condition (b) of the re-audit (ablate every FAKE carrier) is vacuous for this function and
every s1-s3 instance kill stands as measured. The closest banked forms reproduce exactly:
arms2 vars=16/bodydiff=17, j3read vars=16/18, dupXZ vars=24/199, j4read vars=24/25.

### CONFIRMED
- H4.1 jump2 cross-jumping DOES re-merge a statement duplicated into all FOUR arms of the
  kind chain at essentially zero byte cost. `armjoin1` (the join statement `*(s16 *)obj = 0;`
  duplicated into each of the four arms and deleted from the join) measures bodydiff=1 -
  and the single differing line is a REMOVED insn (139 insns vs base's 140), i.e. the merge
  is not merely free, it absorbs a delay-slot filler. `armjoin_keep` = 13, `armjoin` (all
  three join statements) = 17, `armjoin1_keep` = 3. The re-merge mechanism that F3.1 needed
  is real and cheap; what it cannot do here is carry a Judge lookup, because the kind-chain
  join is the target's ONLY cross-jump merge point and none of its three statements
  (`*(s32 *)(obj+0x50) = 1;`, `*(u8 *)(obj+5) = 0;`, `*(s16 *)obj = 0;`) depends on Judge.
- H4.2 The s3 law is narrower than "indexed global site". `tblall` respells all four
  D_8008E194 accesses as direct indexed reads of the global and deletes the `tbl` pointer
  local - four indexed sites on a second global - and still measures vars=8 with one slot.
  Reading the target asm confirms why: D_8008E194 and D_80106A78 are MATERIALISED
  (`lui/addiu` into $a0 / $a3) in both ours and the target, so their symbol pseudos stay
  live and never orphan; only Judge is folded to `lui $at,%hi(Judge); addu $at,$at,$v0;
  lh %lo(Judge)($at)`. The generator is specifically a symbol_ref that combine folds INTO
  the mem with a register index, and in this function only Judge takes that form.

## [s4] A statement duplicated into all four arms of the kind chain is re-merged byte-neutrally by jump2 cross-jumping, and such a merge can therefore carry a third/fourth indexed-Judge site for free
- mechanism: jump2 cross-jumping is the only byte-removing pass after combine; four identical arm tails feeding one join are merged back to a single copy, while each source-level copy still creates its own address pseudo during combine and orphans into an 8-byte reload spill slot
- probe: tmp/grind/func_80030580/s4/frame4.py variants armjoin (all three join statements into all four arms, deleted from the join), armjoin_keep, armjoin1 (only `*(s16 *)obj = 0;`), armjoin1_keep, armp2z (the pos.z += vel.z/2 pass into the arms), armjx; each measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) for vars=, the FRAMEDBG slot census, the ($sp) count and fdiff vs s1/var_base.s
- result: the FIRST half is confirmed and the second is refuted. armjoin1 costs bodydiff=1 (139 insns vs base 140 - the merge removes an insn), armjoin1_keep 3, armjoin_keep 13, armjoin 17, armp2z 157. Every one stays at vars=8 with the single ctx=spill_new_p110 slot, because the only statements available at that merge point (obj+0x50 = 1, obj+5 = 0, obj+0 = 0) contain no indexed-Judge read, and the two statements that do contain one (the vel.x / vel.z stores) must execute BEFORE the += passes and so cannot be moved past the chain. The kind-chain join is the target's only cross-jump merge point (asm/funcs/func_80030580.s .L800307B0), so on this chassis the cross-jump route does not deliver an extra lookup site.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present (fake_ablate reports none)

## [s4] Enriching the ADDRESS AST of the two existing Judge lookups splits the folded address pseudo into two and adds an orphan slot
- mechanism: a two-level address expression (2-D array subscript, struct member then subscript, an intermediate pointer cast, a byte-offset add, a pointer local, a named index local) could leave combine with a second dying pseudo per site
- probe: frame4.py + extra4.py variants judge2d (`extern s16 Judge[][0x1000]`, `Judge[0][i]`), judgestr (`extern struct { s16 t[0x1000]; } Judge`, `Judge.t[i]`), jcastb (`((s16 *)(u8 *)&Judge)[i]`), jbyteoff / jbyteoff1 (`*(s16 *)((u8 *)&Judge + i*2)`), jptr2 (two `s16 *` locals aliasing &Judge, one per site), jidx2 (named `s32` index locals ax/az), plus i16 / uidx (HImode and unsigned index casts), objalias (a second pointer local aliasing obj), srcs16 (`src` retyped `s16 *` with all offsets halved), regall / regi (`register` storage class on the locals)
- result: all thirteen are body-neutral (bodydiff=0) and all measure vars=8 with exactly one ctx=spill_new slot. judge2d/judgestr also leave the rest of the TU untouched. Address-AST depth, index mode, storage class and pointer aliasing do not change how many pseudos combine leaves dying; they are banked as further free-to-compose spellings, not as frame levers.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s4] The s3 orphan law generalises to any array-indexed global, so adding indexed sites on D_8008E194 supplies the missing slots
- mechanism: s3 established slots = indexed-Judge sites - 1 and attributed it to the fold (mem (plus (reg idx) (symbol_ref))); a second global indexed the same way should feed the same distribute_notes path
- probe: frame4.py variant tblall (all four D_8008E194 accesses respelled `(&D_8008E194)[arg1 * 7 + k]`, the `tbl` pointer local removed - four indexed sites on the second global), plus a re-measure of s3's tblglob0 / tblglob2 / tblglob4 with bodydiff reported
- result: tblall stays at vars=8 with one slot at bodydiff=64; tblglob0 = 0 bodydiff, tblglob2 = 46, tblglob4 = 152, all vars=8. Reading asm/funcs/func_80030580.s lines 4-5 and 46-47 explains it: D_80106A78 and D_8008E194 are MATERIALISED into hard registers by `lui %hi / addiu %lo` in the target exactly as in ours, so their symbol pseudos stay live and never die into an orphan note; only Judge is folded into the mem (`lui $at,%hi(Judge); addu $at,$at,$v0; lh %lo(Judge)($at)`, lines 59/61 and 76/78). The generator is a symbol_ref that combine folds INTO the mem with a register index, and Judge is the only symbol in this function that takes that form.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s4] A decomp-permuter campaign on the score-2 chassis finds a spelling that closes the 16-byte frame gap
- mechanism: the permuter's randomizers (perm_temp_for_expr, perm_duplicate_assignment, perm_expand_expr, perm_reorder_stmts, perm_refer_to_var, perm_struct_ref, perm_pad_var_decl, ...) explore source shapes hand-enumeration does not reach, and with --stack-diffs the scorer SEES the `addiu sp,-8` vs `-24` prologue difference, so the frame gap is inside the objective
- probe: two campaigns via tools/permuter_campaign.py with --stack-diffs (default) and --stop-on-zero, workspaces built by tmp/grind/func_80030580/s4/mk_ws.sh (single-function preprocessed TU, prelude.inc + asm/funcs/func_80030580.s assembled at offset 0, cheat stages omitted from compile.sh so the search space is the honest pure-C one). Campaign s4a = the draft3/candidate chassis, base score 10, 38,802 iterations over 16 min. Campaign s4b = the structurally different named-index-local chassis (var_jidx2.c: `s32 ax, az` index locals, body-neutral), base score 10, 45,262 iterations over 18 min.
- result: 84,064 iterations across the two chassis produced ZERO novel finds - the only output directory in each workspace is a score-10 TIE with the base (output-10-1, at 2.0 s and 22.8 s after launch respectively), i.e. a re-spelling that reproduces the same two-instruction gap. Best new score 10 = base score 10 in both. Both campaigns were harvested with --stop and `permuter_campaign.py status` reports 0 live campaigns. The permuter's local search does not reach the frame lever from either chassis: the one randomizer that moves the frame (perm_pad_var_decl) does so by adding an emitted store, which the byte scorer immediately penalises.
- verdict: KILLED
- kill_scope: instance
- measured_on: two permuter chassis (draft3 candidate and jidx2 named-index-local), both at sandbox --disable all = 2, no FAKE constructs present

### FRONTIER after s4 (highest value first)
- F4.1 The orphan generator is a symbol_ref folded INTO a mem with a register index, and
  Judge is the only such symbol here. The remaining question is therefore whether a THIRD
  and FOURTH Judge fold can exist without emitting bytes. Every byte-removing route after
  combine is now measured: CSE (deletes the pseudo too, dupread), and jump2 cross-jumping
  (works, ~free, but the only merge point carries no Judge statement). Next probe: check
  whether the loop's back-edge / the `/32` bgez join at code_label 99 is a second usable
  merge point - place a duplicated Judge-dependent statement at the two `bgez` arms of the
  `/32` and at the loop exit, and measure bodydiff / vars.
- F4.2 Why "sites - 1"? One site's address pseudo keeps a hard register. Unread since s3.
  Probe: `pwsh tools/grinder/dump.ps1 func_80030580` on var_j3read and diff the .lreg/.greg
  entries for p110 / p130 / the survivor to see what class assignment distinguishes them.
  If the survivor can be made classless, THREE sites give vars=24 and j3read's +18 body
  becomes the whole remaining gap.
- F4.3 Re-open the "24 = 8 + 16" reading. Every session so far has assumed three 8-byte
  orphans, from an s2 corpus census of LEAF functions. func_80030580 is a leaf, but the
  alternative decomposition (one orphan + one untouched 16-byte slot, the
  [[phantom-frame-slots-gcc272]] mechanism) has only been probed through BLKmode struct
  temps (all of which emit sp accesses). Probe: sweep constructs that make cc1 call
  assign_stack_local for a 16-byte object that reload later never touches - a DImode or
  larger-mode intermediate that combine folds away, or a struct-valued conditional - and
  accept only variants with 0 ($sp) accesses.

## [s4] A decomp-permuter campaign on the score-2 chassis finds a spelling that closes the 16-byte frame gap
- mechanism: the permuter's randomizers (perm_temp_for_expr, perm_duplicate_assignment, perm_expand_expr, perm_reorder_stmts, perm_refer_to_var, perm_struct_ref, perm_pad_var_decl, ...) explore source shapes hand-enumeration does not reach, and with --stack-diffs the scorer SEES the `addiu sp,-8` vs `-24` prologue difference, so the frame gap is inside the objective
- probe: two campaigns via tools/permuter_campaign.py with --stack-diffs (default) and --stop-on-zero; workspaces built by tmp/grind/func_80030580/s4/mk_ws.sh (single-function preprocessed TU, prelude.inc + asm/funcs/func_80030580.s assembled at offset 0, cheat stages omitted from compile.sh so the search space is the honest pure-C one). s4a = the draft3/candidate chassis (base score 10, 38,802 iterations / 973 s); s4b = the structurally different named-index-local chassis var_jidx2.c (base score 10, 45,262 iterations / ~1090 s).
- result: 84,064 iterations across the two chassis produced ZERO novel finds. Each workspace's only output directory is a score-10 TIE with the base (output-10-1 at 2.0 s and 22.8 s after launch), i.e. a respelling reproducing the same two-instruction gap; best_new_score == base_score == 10 in both. Both harvested with --stop; permuter_campaign.py status reports 0 live campaigns. Reusable fact: tools/decomp-permuter/src/randomizer.py contains exactly one frame-moving randomizer, perm_pad_var_decl, and it moves the frame by adding an emitted store that the byte scorer immediately penalises, so the permuter's local search cannot reach this residual from a body-exact chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: two permuter chassis (draft3 candidate and jidx2 named-index-local), both at sandbox --disable all = 2, no FAKE constructs present (fake_ablate reports none in candidate.c)

## [s4] A statement duplicated into all four arms of the kind chain is re-merged byte-neutrally by jump2 cross-jumping, and such a merge can therefore carry a third and fourth indexed-Judge site for free
- mechanism: jump2 cross-jumping is the only byte-removing pass after combine; four identical arm tails feeding one join are merged back to a single copy, while each source-level copy still creates its own address pseudo during combine and orphans into an 8-byte reload spill slot
- probe: tmp/grind/func_80030580/s4/frame4.py variants armjoin (all three post-chain join statements duplicated into all four arms and deleted from the join), armjoin_keep, armjoin1 (only `*(s16 *)obj = 0;`), armjoin1_keep, armp2z (the pos.z += vel.z/2 pass into the arms) and armjx, each measured with the instrumented cc1 (BB2_FRAME_DEBUG=1) for vars=, the FRAMEDBG slot census, the ($sp) count and fdiff vs s1/var_base.s
- result: The mechanism half is CONFIRMED and the payload half is refuted. armjoin1 costs bodydiff=1 and the single differing line is a REMOVED insn (139 vs base's 140) - the four-arm merge is not merely free, it absorbs a delay-slot filler. armjoin1_keep 3, armjoin_keep 13, armjoin 17, armp2z 157. All stay at vars=8 with the single ctx=spill_new_p110 slot, because the only statements available at that merge point (obj+0x50 = 1, obj+5 = 0, obj+0 = 0) contain no indexed-Judge read, while the two statements that do (the obj+0x44 and obj+0x4C velocity stores) must execute before the += passes and so cannot be moved past the chain. The four-arm join at .L800307B0 is the target's only cross-jump merge point.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s4] The s3 orphan law generalises to any array-indexed global, so adding indexed sites on D_8008E194 supplies the missing slots
- mechanism: s3 established slots = indexed-Judge sites - 1 and attributed it to the fold (mem (plus (reg idx) (symbol_ref))); a second global indexed the same way should feed the same combine distribute_notes path
- probe: frame4.py variant tblall (all four D_8008E194 accesses respelled `(&D_8008E194)[arg1 * 7 + k]` with the tbl pointer local removed - four indexed sites on the second global), plus a re-measure of s3's tblglob0 / tblglob2 / tblglob4 with bodydiff reported, cross-read against the lui/addiu vs folded-mem patterns in asm/funcs/func_80030580.s
- result: tblall stays at vars=8 with one slot at bodydiff=64; tblglob0 = 0 bodydiff, tblglob2 = 46, tblglob4 = 152, all vars=8. asm/funcs/func_80030580.s lines 4-5 and 46-47 show D_80106A78 and D_8008E194 MATERIALISED into hard registers by lui %hi / addiu %lo in the target exactly as in ours, so their symbol pseudos stay live and never die into an orphaned REG_DEAD note; only Judge is folded into the mem (lines 59/61 and 76/78). The law narrows to: a symbol_ref that combine folds INTO the mem with a register index, and Judge is the only symbol in this function taking that form.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s4] Enriching the address AST of the two existing Judge lookups splits the folded address pseudo into two and adds an orphan slot
- mechanism: a two-level address expression (2-D array subscript, struct member then subscript, an intermediate pointer cast, a byte-offset add, a pointer local, a named index local) could leave combine with a second dying pseudo per site
- probe: frame4.py + extra4.py variants judge2d (extern s16 Judge[][0x1000], Judge[0][i]), judgestr (extern struct { s16 t[0x1000]; } Judge, Judge.t[i]), jcastb (((s16 *)(u8 *)&Judge)[i]), jbyteoff / jbyteoff1 (*(s16 *)((u8 *)&Judge + i*2)), jptr2 (two s16 * locals aliasing &Judge), jidx2 (named s32 index locals), i16 / uidx (index casts), objalias (a second u8 * alias of obj), srcs16 (src retyped s16 * with halved offsets), regall / regi (register storage class)
- result: All thirteen are body-neutral (bodydiff=0) and all measure vars=8 with exactly one ctx=spill_new slot; judge2d and judgestr also leave the rest of the TU byte-identical. Address-AST depth, index mode, storage class and pointer aliasing do not change how many pseudos combine leaves dying. They are banked in evidence.md as further free-to-compose spellings for any future frame lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s4] The s1-s3 instance kills were measured on a chassis or FAKE state that no longer holds, so the closest banked forms need re-measuring before new probes
- mechanism: an instance kill is only as good as its chassis and FAKE state; a lever measured inert while a FAKE carrier occupied its target pseudo is not a kill
- probe: mandated kill re-audit: engine sandbox func_80030580 --disable all on the applied candidate; tools/fake_ablate.py --func func_80030580 --file code6cac_b --candidate memory/grind/func_80030580/candidate.c; and a re-run of the four closest s3 forms (arms2, j3read, dupXZ, j4read) plus base through frame3.py with the instrumented cc1
- result: The chassis is unchanged: sandbox --disable all prints score 2. fake_ablate reports 'no FAKE-annotated constructs found; nothing to ablate' - the candidate and every banked variant are FAKE-free, so the ablation half of the re-audit is vacuous for this function. The closest forms reproduce their s3 numbers exactly: base vars=8/bodydiff=0, arms2 vars=16/17, j3read vars=16/18, dupXZ vars=24/199, j4read vars=24/25. The s1-s3 instance kills stand as recorded.
- verdict: CONFIRMED

## Session 5 (2026-09-02, synthesis)

### Re-audited (mandated kill re-audit) — all three re-measured on the CURRENT chassis
**H-s5-0. The s3/s4 indexed-site kills were measured on a chassis or FAKE state that no
longer holds.** KILLED (instance). Probe: `frame3.py base j3read j4read dupXZ dupX` with
the instrumented cc1 on the tree as it stands, candidate.c applied, zero FAKE constructs
(fake_ablate found none at s4 and candidate.c is unchanged since). Result: byte-for-byte
the same numbers as s3/s4 — base 8/0, j3read 16/18, j4read 24/25, dupX 16/15, dupXZ
24/199. Nothing to reopen on that axis; **j4read** (exact target frame at only +25 body
lines) is hereby promoted as the closest-to-target banked form, superseding dupXZ (+199)
as the thing the next session should try to make byte-neutral.

### H-s5-1. The phantom-frame HImode-bitwise generator supplies the missing 16 bytes
KILLED (instance). Mechanism tested: `phantom-frame-slots-gcc272` records that two HImode
locals feeding an HImode bitwise expression `(a & ~b) & 1` make GCC 2.7.2 allocate a stack
slot for a computation it then register-allocates away, with no store emitted — a second,
Judge-independent generator that this function's ledger had never tested.
Probe: five in-function forms via `s5/frame5.py` (`himin` two s16 locals, `himin32` the
s32 control, `hi1` a single s16 local, `hiu` unsigned, `qimin` QImode), plus a standalone
repro `gen_probe.c:p_hi` through the new `s5/genprobe.py`.
Result: every in-function form stays **vars=8 with the single spill_new_p110 slot**
(bodydiff 7-10); the standalone repro measures **vars=0**. The generator does not fire in
this function's context. Measured on: candidate.c/draft3 chassis (sandbox --disable all
= 2 re-measured this session), no FAKE constructs present.
Banked `rejected/himode-bitwise-phantom-generator-inert.c`.

### H-s5-2. A generator that scales as SITES (not sites-1) exists and can be hosted here
CONFIRMED in part, KILLED (instance) as a lever for this function. Mechanism: the s3
census witness `text1a_post:func_80041E10` (vars=24, three orphans, no branch and no label)
was never explained. Probe: reduced it to a minimal standalone repro and bisected the shape
across 33 constructs in `s5/gen_probe.c`, `s5/gen2.c`, `s5/gen3.c` under `genprobe.py`.
Result — the generator is `(s16)(<masked-or-shifted dividend> / <constant>)`, i.e. a signed
magic-multiply division whose dividend carries a mask/shift and whose quotient is truncated
to HImode at the store. It yields **one ctx=spill_new slot PER SITE with no -1**
(p_div255_1/2/3 -> vars 8/16/24). Each half is load-bearing: `q1` `(s16)(x/255)` = 0,
`r2` (s32 destination) = 0, `r7` (`>>3`) = 0, `r17` (`%`) = 0, `r13`/`r15` (arithmetic
after the cast) = 0; `r6` (`/3`), `r8`, `r11`, `r14` all fire.
But func_80030580's only divisions are `/32` and `/2`, which expand to shift sequences and
never enter the magic-multiply path: `t32a`/`t32b`/`m32a`/`m32b`/`m32c` are byte-neutral
AND frame-inert (vars=8), `t32c`/`t32d` cost 15 body lines, `h2x`/`h2all` cost 29/51.
Measured on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this
session), no FAKE constructs present.
Banked `rejected/truncated-division-generator-not-hostable.c`.

### H-s5-3. The original signature was wider, and a dead incoming argument reserves the bytes
KILLED (instance). Mechanism: an unused parameter gets an incoming-argument pseudo in the
prologue; if that pseudo survived to reload unallocated it would take an alter_reg stack
slot exactly like the combine orphan, and the callers (func_800307D0 / func_80030900 /
cpu_set_move_command_and_dir) pass two arguments under K&R no-prototype rules, so a wider
signature is ordinary C and costs nothing at the call sites.
Probe: `arg3u`, `arg4u`, `arg5u` (3, 4 and 5 parameters with the extras unused), `arg3p`
(pointer-typed extra), `arg3s` (s16-typed extra) via `s5/frame5.py`.
Result: all five are **bodydiff=0 and vars=8**. The dead argument pseudo is deleted before
reload and never orphans. Measured on: candidate.c/draft3 chassis (sandbox --disable all
= 2 re-measured this session), no FAKE constructs present.
Banked `rejected/unused-trailing-params-frame-inert.c`.

### H-s5-4. Two folds in one basic block can be made to yield two orphans by block geometry
KILLED (instance). Mechanism: s2 read the orphan as "the REG_DEAD walk crosses a
CODE_LABEL", which predicts one orphan per fold once each fold sits at the head of its own
block. Probe: regenerated `tmp/grind/func_80030580/dumps/` on the current chassis and read
`.lreg` and `.combine` directly (pass attribution, not inference).
Result: `.lreg` names exactly one pseudo — `Register 110 used 2 times across 67 insns in
block 6; ST_REGS or none; pointer` — and pseudos 107/127/130 do not appear at all (combine
deleted them outright). `.combine` holds exactly one orphan USE, `(insn 393 99 100 (use
(reg:SI 110)))`, right after `code_label 99`. Both emitted Judge loads live in the SAME
basic block in the target as well (0x80030644-0x800306D0, no label between 0x80030618 and
0x80030790), so the target's three orphans cannot come from two folds however the blocks
are arranged: the target's own geometry is the geometry we already reproduce.
Measured on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this
session), no FAKE constructs present.

### Frontier after s5 (reset to the strongest three)
1. **Make j4read byte-neutral.** j4read reaches the target's exact vars=24 for +25 body
   lines — the cheapest known form by a factor of eight over dupXZ. Nobody has yet
   itemised WHICH 25 lines those are. Next probe: `tools/fdiff.py s1/var_base.s
   s3/var_j4read.s func_80030580` and classify every added insn; if the surplus is
   dominated by the two extra `lui/addu/lh` triples, search placements for the two extra
   sites where jump2 cross-jumping (measured at NEGATIVE cost by s4's armjoin1) removes
   them, i.e. sites duplicated into every arm of a join rather than added in line.
2. **Mine the corpus for generator 4 with the new standalone probe.** Two generators are
   now characterised (indexed-global fold, sites-1; truncated magic-multiply division,
   sites) and one documented generator is inert here. `s5/genprobe.py` reduces "does
   construct X orphan?" to one cc1 run. Next probe: re-run `s3/census3.py` to list every
   leaf with vars>0, then for each witness whose C contains neither an indexed global nor
   a truncated constant division, reduce it to a minimal standalone repro in the
   `gen2.c`/`gen3.c` style and bisect it. Only a generator that fires on a construct
   func_80030580 already contains (a pointer-biv loop, a struct copy, a 32x32 multiply
   with `>>12`, a shift-based signed divide, a four-arm equality chain) can close this.
3. **Re-open the 16-vs-24 decomposition with a direct measurement, not a census.** Every
   session since s2 has assumed vars=24 means three 8-byte slots because a corpus census
   of OUR compiled C never produced a non-spill_new leaf slot. That is an argument about
   the sample, not about the target. `MIPS_STACK_ALIGN` means the target's vars is only
   pinned to 17..24, so 8+8+4 (two orphans plus a 4-byte object) and 8+16 (one orphan
   plus a 16-byte object) both round to 24. Next probe: use `genprobe.py` to enumerate
   which constructs produce a non-spill_new `ctx=` (stack_temp / stack_local) that emits
   ZERO `($sp)` accesses in a LEAF — s1 only ever tested `Vec3i tmp` (which does touch
   sp); untested are unions, casts between struct types, a struct-typed conditional
   expression, and a struct compared or passed by value to nothing.

## [s5] The s3/s4 indexed-Judge-site kills were measured on a chassis or FAKE state that no longer holds, so the site law needs re-deriving
- mechanism: Kills are chassis-relative; a lever measured inert while a FAKE carrier occupied its target pseudo is not a kill. candidate.c carries no FAKE constructs (fake_ablate, s4) and is unchanged, so the re-measurement is a zero-FAKE control.
- probe: Re-ran tmp/grind/func_80030580/s3/frame3.py base j3read j4read dupXZ dupX with the instrumented cc1 (BB2_FRAME_DEBUG=1) on the current tree with candidate.c applied, after re-measuring the chassis with sandbox --disable all.
- result: Byte-for-byte the s3/s4 numbers: base vars=8/bodydiff 0 (spill_new_p110), j3read 16/18, j4read 24/25, dupX 16/15, dupXZ 24/199. Chassis re-measured at score 2. The kills stand; j4read is promoted over dupXZ as the closest-to-target banked form (exact vars=24 for +25 body lines instead of +199). fdiff itemises j4read's surplus as exactly the two extra Judge lookups (lh angle / addu const / andi 0xFFF / sll 1 / lh Judge / addu, twice) plus the prologue-epilogue pair flipping from -8 to -24.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s5] The phantom-frame HImode-bitwise generator (two s16 locals feeding (a & ~b) & 1, project memory phantom-frame-slots-gcc272) supplies the missing 16 frame bytes in this function
- mechanism: That memory records GCC 2.7.2 allocating a stack slot for a computation it later register-allocates away, with no store emitted - a generator independent of the indexed-global fold that this function's ledger had never tested.
- probe: Five in-function forms through the new tmp/grind/func_80030580/s5/frame5.py harness (himin = two s16 locals, himin32 = the s32 control, hi1 = one s16 local, hiu = unsigned, qimin = QImode), plus a standalone repro gen_probe.c:p_hi through the new s5/genprobe.py catalog probe.
- result: Every in-function form stays vars=8 with the single ctx=spill_new_p110 slot (bodydiff 7-10); the standalone repro measures vars=0. The generator does not fire in this function's context. Banked rejected/himode-bitwise-phantom-generator-inert.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s5] A phantom-slot generator that scales as SITES rather than sites-1 exists in this compiler and can be hosted by func_80030580's divisions
- mechanism: The s3 census witness text1a_post:func_80041E10 has vars=24 with three orphan slots and no branch or label at all, so a generator other than the indexed-global fold must exist; if it counts sites rather than sites-1, three source constructs would buy the target's three slots.
- probe: Reduced func_80041E10 to a minimal standalone repro and bisected the shape across 33 constructs in s5/gen_probe.c, s5/gen2.c and s5/gen3.c under s5/genprobe.py; then tried every truncation and mask/shift spelling of this function's own /32 and /2 divisions in-function (t32a-t32d, m32a-m32c, h2x, h2all).
- result: The generator is (s16)(<masked-or-shifted dividend> / <constant>) - a signed magic-multiply division whose dividend carries a mask/shift and whose quotient is truncated to HImode at the store - and it yields ONE ctx=spill_new slot PER SITE with no -1 (p_div255_1/2/3 -> vars 8/16/24). Each half is load-bearing: q1 (s16)(x/255)=0, r2 (s32 destination)=0, r7 (>>3)=0, r17 (%)=0, r13/r15 (arithmetic after the cast)=0, while r3/r4/r6/r8/r11/r14 all fire. But func_80030580 divides only by 32 and by 2, which expand to shift sequences and never enter the magic-multiply path: t32a/t32b/m32a/m32b/m32c are byte-neutral AND frame-inert (vars=8), t32c/t32d cost 15 body lines, h2x/h2all cost 29/51. Confirmed as a generator, killed as a lever for this function. Banked rejected/truncated-division-generator-not-hostable.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s5] The original function had a wider signature and a dead incoming-argument pseudo reserves the missing frame bytes
- mechanism: An unused parameter gets an incoming-argument pseudo in the prologue; if it survived to reload unallocated it would take an alter_reg stack slot exactly like the combine orphan. The three callers pass two arguments under K&R no-prototype rules, so a wider signature is ordinary C and costs nothing at the call sites.
- probe: arg3u, arg4u, arg5u (3, 4 and 5 parameters with the extras unused), arg3p (pointer-typed extra) and arg3s (s16-typed extra) through s5/frame5.py.
- result: All five measure bodydiff=0 and vars=8 - body-neutral and frame-inert. The dead argument pseudo is deleted before reload and never becomes an orphan. Banked rejected/unused-trailing-params-frame-inert.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s5] Two folded Judge addresses in one basic block can be made to yield two orphan slots by rearranging block geometry, per session 2's CODE_LABEL reading
- mechanism: s2 read the orphan as combine.c:10836 distribute_notes walking back from i3, hitting a CODE_LABEL and emitting (use (reg N)) there, which predicts one orphan per fold once each fold sits at the head of its own block.
- probe: Regenerated tmp/grind/func_80030580/dumps/ on the current chassis (pass attribution, per contract) and read .lreg and .combine directly instead of inferring, then compared against the target's block boundaries in asm/funcs/func_80030580.s.
- result: .lreg names exactly one pseudo - 'Register 110 used 2 times across 67 insns in block 6; ST_REGS or none; pointer' - and pseudos 107/127/130 do not appear at all, combine having deleted them outright. .combine holds exactly one orphan USE, (insn 393 99 100 (use (reg:SI 110))), immediately after code_label 99. Both emitted Judge loads sit in ONE basic block in the target too (0x80030644-0x800306D0, no label between 0x80030618 and 0x80030790), so the geometry we already reproduce is the target's own geometry and two folds in it yield one orphan.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c/draft3 chassis (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s6] The s3/s4/s5 indexed-Judge site-law kills still hold on the current chassis, and the law is a straight-line-block law rather than a global one
- mechanism: An instance kill is only as good as the chassis and FAKE state it was measured under, so the mandated re-audit re-ran the two closest-to-target forms (j4read, the cheapest form reaching the exact vars=24, and dupX) plus the arm-duplication forms on the tree as it stands today.
- probe: INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s3/frame3.py base j3read j4read dupX armsvx armsvx_keep arms2, with tools/fake_ablate.py first confirming candidate.c carries no FAKE-annotated construct to ablate.
- result: base 8/0, j3read 16/18, j4read 24/25, dupX 16/15, arms2 16/17, armsvx 8/84, armsvx_keep 16/73 - byte-for-byte the s3/s4/s5 numbers. New refinement: armsvx MOVES one site into all four arms (four sites) and yields ONE slot, while armsvx_keep (five sites) yields two, so slots = sites - 1 counts sites within a straight-line block; sites duplicated into sibling arms collapse to one.
- verdict: CONFIRMED

## [s6] An existing Judge lookup can be respelled with a constant symbol bias that the index cancels, giving combine a second address pseudo to fold and orphan without emitting any instruction
- mechanism: (&Judge)[i] and (&Judge + K)[i - K] denote the same address, but the front end lowers the second as t = &Judge + 2K then u = t + 2*(i-K) then mem(u) - one extra intermediate pseudo per site whose constant contribution is zero, so the emitted address arithmetic should be unchanged after constant folding while combine gets a second REG_DEAD note to orphan.
- probe: Seven spellings through the new tmp/grind/func_80030580/s6/frame6.py sweep - bias1 (first site, K=0x400), bias2 (second site), biasboth, biasneg (negative K), bias1s (K=1), biases (a different K per site) and biasp (a biased s16 * pointer local shared by both sites) - plus biasbyte, the byte-offset spelling of the same identity.
- result: bias1, bias2, biasboth, biasneg, bias1s, biases and biasp are ALL bodydiff=0 with vars=8 and the single ctx=spill_new_p110 slot - body-neutral and completely frame-inert; biasbyte collapses the existing orphan to vars=0 at bodydiff 4. cse folds the constant into the symbol_ref before the address chain ever splits into two pseudos, so no second note exists to orphan. Banked rejected/symbol-address-bias-frame-inert.c; the seven neutral spellings join the composable bank.
- verdict: KILLED
- kill_scope: instance
- measured_on: s1/draft3.c body applied to src/code6cac_b.c (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s6] The target's vars=24 need not be three 8-byte orphans - some construct allocates a non-spill_new stack object that a leaf function never touches, so 8+8+4 or 8+16 would also round to 24
- mechanism: Every session since s2 assumed three orphans on the strength of a census of OUR compiled corpus, which is an argument about the sample rather than about the target; s1 only ever tested one non-spill_new shape (a Vec3i temp local, which does touch sp), leaving the space of untouched stack_temp / stack_local constructs essentially unexplored.
- probe: Eleven standalone shapes in tmp/grind/func_80030580/s6/gen4.c through s5/genprobe.py, chosen to cover the untested quadrant: a union word/half pun (e1), three struct-temp shapes (e2/e8/e11), a struct copy through a local (e3), a pointer-cast struct copy (e4), a two-halfword struct read as a word (e5), long long arithmetic (e6), a 64-bit (x*y)>>12 (e7), and a struct-typed conditional expression (e9).
- result: Every shape that allocated anything at all got a ctx=stack_temp WITH real sp traffic - e2 vars=16/sp=6, e3 vars=16/sp=9, e5 vars=8/sp=3, e8 vars=16/sp=9, e11 vars=16/sp=7 - and every shape with sp=0 measured vars=0, except e10, which turned out to be generator 4 and is ctx=spill_new after all. Across these eleven shapes no construct produced an untouched non-spill_new slot, so the three-orphan reading of the target's frame survives this probe.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone probe corpus compiled with the project cpp/cc1 flags under BB2_FRAME_DEBUG=1, no FAKE constructs present

## [s6] A fourth phantom-slot generator exists that fires on a construct func_80030580 can host byte-neutrally, supplying the missing two slots without adding a Judge lookup site
- mechanism: Three generators were known (the indexed-global fold at sites-1, the truncated magic-multiply division at one per site, and the inert HImode-bitwise form). The s5 genprobe instrument reduces "does construct X generate a phantom slot?" to one cc1 run, so the remaining catalog could be swept directly rather than reasoned about.
- probe: 29 standalone shapes in tmp/grind/func_80030580/s6/gen5.c and gen6.c, then the surviving shape hosted in-function at eleven different sites through tmp/grind/func_80030580/s6/frame6.py + extra6.py, each measured for vars=, ($sp) count, FRAMEDBG context list and fdiff against the s1 base; the two cheapest were then confirmed with engine sandbox func_80030580 --disable all.
- result: CONFIRMED. A 4-byte union local holding an SImode member and two HImode members, where both halfword members are written and at least one is read back, is register-allocated (sp=0) and yields TWO ctx=spill_new orphan slots - gen6.c b1 and d2, gen5.c a10 and a11 all measure vars=16 with sp=0. The generator is byte-free: gen5.c a1 (union) and a7 (the plain (x & 0xFFFF) | (x << 16) spelling) emit the identical instruction sequence, a1 reserving 8 phantom bytes and a7 none. Preconditions measured: the union wrapper with a word member is required (two plain u16 locals c1/c2 give vars=0; a bare two-halfword struct d1 and a u16[2] array become a real stack_temp with sp traffic), both halves must be written (a3/a4 give 0), the members must be HImode (a four-u8 union a6 gives 0), and a union written as a word then split gives nothing (b3 = 0). Hosted on the function's last two zero stores - union { s32 w; u16 h[2]; } z; z.h[0]=0; z.h[1]=0; *(u8 *)(obj+5)=z.h[0]; *(s16 *)obj=z.h[1]; - it measures vars=24, sp=0, three ctx=spill_new slots and bodydiff=4, where all four diff lines are the wanted subu $sp,$sp,8 -> subu $sp,$sp,24 prologue/epilogue pair, and engine sandbox func_80030580 --disable all printed "score": 0. Equivalent byte-neutral placements: z05n, z78, z05b, z05i, z05u, u0, u2, u0b.
- verdict: CONFIRMED

## [s6] The union generator can be spelled with every member carrying a distinct real value, making it ordinary C rather than a coercion construct
- mechanism: If the two union members held two DIFFERENT values that the target actually stores, the union would be a genuine packing intermediate - each member written once from a real value and read once into a store the target emits - and the construct would sit inside the named-intermediate prongs rather than outside every frozen family.
- probe: Five in-function placements where the two halves carry different real values - q54 and q54n (0 and the angle, into obj+0x54 / obj+0x56), q56 (the angle and 0, into obj+0x56 / obj+0x58), q0a (the slot index and arg1, into obj+0xA / obj+2) and q5c (0 and the spin value, inside kind-chain arm 1) - measured with frame6.py.
- result: All five change the body substantially: q54 and q54n bodydiff=28, q56 bodydiff=26, q0a bodydiff=31 (all vars=16), q5c bodydiff=74 (vars=24). Reading two different values back out of the union forces shift/mask composition that the target does not emit. Byte-neutrality is only reached when both members carry the SAME value, which on this function means zero - so the only byte-neutral placements are the function's zero stores, where the union has no packing role. Banked rejected/live-union-pair-halfword-store-body-28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s1/draft3.c body applied to src/code6cac_b.c (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## [s6] The union wrapper is incidental and a plain two-halfword struct or u16[2] array would give the same frame bytes
- mechanism: If the phantom slots came from the pair of HImode objects rather than from the union type, the construct could be spelled without a union - and a written-then-read local array would fall under a rule family with an existing (if narrow) grant path.
- probe: z05s (struct { u16 a, b; }) and z05a (u16 z[2]) hosted at the same site as z05, plus the standalone controls gen6.c d1 (struct) and gen5.c a14 (struct read through an s32 cast).
- result: Both in-function forms measure vars=16 with THREE ($sp) accesses and bodydiff=11 - the object becomes a real ctx=stack_temp that emits sh/sh/lw against the frame, which is neither byte-neutral nor the target's untouched-frame signature. The standalone controls agree (d1 vars=8 sp=2 stack_temp; a14 vars=8 sp=3 stack_temp). The union wrapper with a word-sized member is what keeps the object in registers. Banked rejected/bare-struct-halfword-pair-spills-to-stack.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s1/draft3.c body applied to src/code6cac_b.c (sandbox --disable all = 2 re-measured this session), no FAKE constructs present

## s6b (driver session 6, synthesis) — the Judge-mandated pad route

**H-PAD (CONFIRMED, bytes proven).** *Statement:* the target's 16 missing frame bytes are
reachable by the Phantom-frame-slot volatile pad local family in its exact sanctioned
form — `volatile u32 pre_pad[4];` as the first declaration of the s1-s5 pure-C body,
`// !FAKE` annotated. *Mechanism:* reload's `alter_reg` / `get_frame_size()` count a
never-accessed volatile local, so it costs frame bytes and zero instructions.
*Probe:* `INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s6/padsweep.py` (N=1..6)
plus a full clean build. *Result:* pad4 -> vars=24, sp=0, bodydiff=4 (exactly the two
`subu/addu $sp` lines, 140/140 insns); full build SHA1 == oracle, verified twice. The
honest sandbox still prints 2 because the stripper removes the pad absent the
`_SANCTIONED_UNWRITTEN_PADS` row — engine/ is not a grind-session surface, so the
residual is an INTEGRATION HANDOFF, not a codegen problem.

**H-UNION (dead by ruling).** The s6a union local is FAILed (2026-09-02 17:07): the union
type carries no semantic content, and no frozen family covers an aggregate carrier in
that role. Do not respell it.

**H-DETECT (finding, not a lever).** `find_unused_local_arrays` treats a pad as
referenced when its identifier appears anywhere in the body text, comments included, so
a self-naming annotation yields a false `sandbox = 0`. Observed and reverted this
session; reported for the operator. It is not to be used as a route to a 0.

## Session 7 (2026-09-02, synthesis) — frontier reset

### H-s7-1 — CONFIRMED (re-measurement, not inference)
**Statement.** The s6 candidate (`pure-c-floor2-body.c` + `volatile u32 pre_pad[4]; // !FAKE`
in first-decl position) makes `src/code6cac_b.c` build to the oracle SHA1, and the
pure-C chassis without the pad still measures `sandbox --disable all` = 2 on the
current chassis.
**Probe.** Applied each body with `tmp/grind/func_80030580/s1/apply.py`; ran
`sandbox --disable all` on HEAD (148), on the floor body (2) and on the candidate (2);
ran the full clean driver `build` on the candidate.
**Result.** `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want ... MATCH`. The
candidate's sandbox 2 is the cheat-stripper deleting the pad
(`engine/volatile_cheats.py:249`), not a body divergence. **Verdict: CONFIRMED.**

### H-s7-2 — the residual is a DECLARED untouched frame object, not a spill slot (CONFIRMED)
**Statement.** The 16 frame bytes separating our `vars=8` from the target's `vars=24`
are contributed by a declared frame object of roughly 9..16 bytes that no instruction
references; they are not additional combine-orphan spill slots.
**Mechanism.** With all 140 body instructions matching, a spill slot carrying any
traffic would appear as body divergence; `tools/fdiff.py` shows the entire diff is
`subu/addu $sp,8 -> 24`. The pad sweep confirms the size→frame mapping empirically:
12 bytes (`pad3`) and 16 bytes (`pad4`) both round to `vars=24`, 8 bytes to 16, 20 to 32.
**Probe.** `INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s6/padsweep.py 2 3 4 5`
with the instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_FRAME_DEBUG=1`); every variant
reported `sp=0`.
**Result.** `padbase vars=8 bodydiff=0 sp=0 ctx=spill_new_p110` ·
`pad2 vars=16` · `pad3 vars=24` · `pad4 vars=24 bodydiff=4 sp=0` · `pad5 vars=32`.
**Verdict: CONFIRMED.** Consequence: the s2/s3 "three 8-byte orphan spill slots" model
is retired, and with it the premise under live-frontier items 1 and 2 (both search for
extra indexed-`Judge` sites to manufacture orphan spills under the sites-1 law). The
honest search space is now "ordinary C that declares a 9..16-byte object GCC 2.7.2
keeps in the frame but never touches".

### H-s7-3 — KILLED (instance): reaching sandbox 0 by a form the stripper does not detect
**Statement.** On this chassis, substituting a non-volatile dead scalar local for the
volatile pad array is not an available route to a submittable candidate, because the
2026-09-02 17:07 ruling reads `named-local-fake-exception.md:5,72` as excluding frame
coercion from the dead-scalar family.
**Mechanism.** `find_volatile_unused_locals` (`engine/volatile_cheats.py:388`) matches
only `volatile`-qualified scalars, and `find_orphaned_local_decls`
(`engine/volatile_cheats.py:659`) explicitly skips zero-reference declarations, so a
plain `u32` dead local survives stripping and would score 0 — which is precisely the
cheat-checklist T4/T5 shape (passing a detector only because the detector does not
catch this spelling).
**Probe.** Source read of the three detectors plus the ruling text in
`docs/grind/decisions.md` (2026-09-02 17:07); no measurement spent, deliberately.
**Result.** Recorded so that a later session does not re-derive this as a discovery.
**Verdict: KILLED.** `kill_scope: instance` — measured/argued on the
`pure-c-floor2-body.c` chassis (sandbox = 2 re-measured this session) with the pad as
the only FAKE construct; a future owner ruling that extends the dead-scalar family to
frame coercion would revive it.

### FRONTIER (reset — strongest first)
1. **An ordinary-C construct that reserves a 9..16-byte frame object GCC 2.7.2 never
   touches.** This is now the ONLY honest axis, and H-s7-2 makes it precise for the
   first time. Probe: extend `tmp/grind/func_80030580/s5/genprobe.py` to sweep
   *semantically real* candidates whose stores DCE away or whose value is folded —
   a `Vec3i` local assigned from `*(Vec3i *)(obj + 0x2C)` and then never read, a struct
   copy whose destination is immediately overwritten, a small union used for one
   half-word read, a struct-typed conditional expression — and accept only
   `vars=24 bodydiff<=4 sp=0`. Note that the sanctioned-family bar still applies to
   anything whose stores vanish; the target is a construct that reads truthfully.
2. **Operator integration.** `docs/grind/decisions.md` 2026-09-02
   `func_80030580 — OWNER-ESCALATION: INTEGRATION HANDOFF` — apply `candidate.c`,
   layer-2 cheat-reviewer, add `"func_80030580": frozenset({("pre_pad", 4)}),` to
   `_SANCTIONED_UNWRITTEN_PADS`, then sandbox reads 0.
3. **Deprioritised:** the j4read / jump2 cross-jump route and the "fourth generator"
   hunt. Both were built on the retired orphan-spill model (H-s7-2); re-rank them only
   if that model is somehow reinstated.

## [s6] The s6 candidate (pure-c-floor2-body.c plus `volatile u32 pre_pad[4]; // !FAKE` in first-decl position) builds src/code6cac_b.c to the oracle SHA1, and the same body without the pad measures sandbox --disable all = 2 on the current chassis.
- mechanism: The pad is a declared, never-accessed volatile object: reload's alter_reg / get_frame_size count it into the frame, so it costs 16 frame bytes and zero instructions. The sandbox cheat-stripper deletes the declaration (engine/volatile_cheats.py:249 find_unused_local_arrays), which is why the honest score reads 2 while the real build matches.
- probe: Applied each body with tmp/grind/func_80030580/s1/apply.py; ran `sandbox func_80030580 --disable all` on HEAD, on pure-c-floor2-body.c and on candidate.c; ran the full clean driver build on candidate.c.
- result: HEAD = 148 (HEAD carries INCLUDE_ASM). Floor body = 2. Candidate = 2 (pad stripped). Full build: sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want -> MATCH. Nothing was taken on trust from the discarded s6 session; every number was re-run this session.
- verdict: CONFIRMED

## [s6] The 16 frame bytes separating our vars=8 from the target's vars=24 are contributed by a declared frame object of roughly 9 to 16 bytes that no instruction references, not by additional combine-orphan spill slots.
- mechanism: All 140 body instructions already match the target, so a spill slot carrying any traffic would appear as body divergence; tools/fdiff.py shows the entire diff is subu/addu $sp,8 -> 24. The pad sweep gives the size-to-frame mapping empirically: 8 bytes -> vars=16, 12 and 16 bytes -> vars=24, 20 bytes -> vars=32, every variant with sp=0.
- probe: INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s6/padsweep.py 2 3 4 5, using the instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_FRAME_DEBUG=1.
- result: padbase vars=8 bodydiff=0 sp=0 ctx=spill_new_p110 | pad2 vars=16 | pad3 vars=24 | pad4 vars=24 bodydiff=4 sp=0 | pad5 vars=32. This retires the s2/s3 model that the target's vars=24 is three 8-byte combine-orphan spill slots - the model that shaped sessions 2 through 5 and underpins live-frontier items 1 and 2, both of which hunt extra indexed-Judge sites under the sites-1 orphan law. Live-frontier item 3 is vindicated and promoted to first.
- verdict: CONFIRMED

## [s6] On this chassis, substituting a non-volatile dead scalar local for the volatile pad array is not an available route to a submittable candidate, because the 2026-09-02 17:07 ruling reads named-local-fake-exception.md:5,72 as excluding frame coercion from the dead-scalar family.
- mechanism: engine/volatile_cheats.py:388 find_volatile_unused_locals matches only volatile-qualified scalars, and engine/volatile_cheats.py:659 find_orphaned_local_decls explicitly skips zero-reference declarations, so a plain u32 dead local survives stripping and the sandbox would print 0 without any engine row. That is exactly cheat-checklist T4/T5: passing the detectors only because the detectors do not catch this spelling.
- probe: Source read of the three detectors plus the ruling text at docs/grind/decisions.md 2026-09-02 17:07; deliberately no measurement spent on a form that cannot be submitted.
- result: Recorded in hypotheses.md as H-s7-3 so a later session does not re-derive the detector gap as a discovery and burn a Judge cycle on it.
- verdict: KILLED
- kill_scope: instance
- measured_on: pure-c-floor2-body.c chassis, sandbox --disable all = 2 re-measured this session; the only FAKE construct present in the candidate is `volatile u32 pre_pad[4]`

## Session 8 (2026-09-02, solver) — floor 2; the orphan-spill model is REINSTATED

### H-s8-1 — KILLED (class): the solver suite cannot address this residual
**Statement.** `goal_from_tgt.py classify` types func_80030580's residual as PRE-RA with
"next tool: none", so neither `tools/ra_solver` nor `tools/sched_solver` has a model that
covers it: the two streams already agree on register disposition and emission order and
differ only in the prologue/epilogue `addiu $sp` immediate.
**Mechanism.** The frame size is fixed by `assign_stack_local` calls made from
`alter_reg` (`tools/gcc-2.7.2/reload1.c:2403`) for pseudos that reach reload with refs and
no hard register. The solver models represent seats (which hard register a pseudo gets) and
clocks (which order insns issue); a slot handed to a pseudo that got NO seat and issues NO
insn is outside both representations.
**Probe.** `python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_80030580`
(refused, zero-rule guard) then `python3 tools/ra_solver/goal_from_tgt.py classify
code6cac_b func_80030580`.
**Result.** `FIRST DIVERGENCE: PRE-RA / next tool: none — the residual is upstream of every
model`; ours-only `addiu #,#,-8` / `addiu #,#,8`, target-only `addiu #,#,-24` / `addiu
#,#,24`, 148 vs 148 insns.
**Verdict: KILLED.** `kill_scope: class` — the tool's own typed verdict, not a spelling
result. `predicate_cite: tools/gcc-2.7.2/reload1.c:2403`. `measured_on:
pure-c-floor2-body.c chassis, sandbox --disable all = 2, zero FAKE constructs
(fake_ablate reports none).`

### H-s8-2 — CONFIRMED: the residual is THREE 8-byte alter_reg slots (H-s7-2 corrected)
**Statement.** The target's `vars=24` is three 8-byte `alter_reg` stack slots and our
`vars=8` is one of them; the s6 reading that the residual is "a declared untouched frame
object of 9..16 bytes, not additional combine-orphan spill slots" is retracted.
**Mechanism.** An unallocated pseudo's `alter_reg` slot costs zero instructions by
construction, so "every body instruction matches" is not evidence against a spill slot — it
is what an orphan slot looks like. Slot size is 8 rather than 4 because `alter_reg` calls
`assign_stack_local (mode, total_size, -1)` and `align = -1` rounds to `alignment = 8`.
**Probe.** `tmp/grind/func_80030580/s7/fdbg.py` on the base chassis (raw FRAMEDBG lines),
plus the mandated re-audit `tmp/grind/func_80030580/s3/frame3.py base j3read j4read dupX`.
**Result.** base: `ctx=spill_new_p110 mode=4 size=8 align=-1 alignment=8 frame_offset=8`.
j3read: p110@8 + p130@16 (vars=16). j4read: p110@8 + p130@16 + p143@24 (vars=24, the exact
target frame) at bodydiff=25. dupX: p110@8 + p125@16 (vars=16) at bodydiff=15. Every slot
is `mode=4` (SImode), `size=8`, `align=-1`.
**Verdict: CONFIRMED.** Consequence: the s7 frontier item 1 ("an ordinary-C declared frame
object of 13..16 bytes that no instruction touches") and the Judge constraint phrased on top
of it are aimed at a premise this session retracts; the honest search space is again "two
more pseudos that reach reload with refs and no hard register, at zero instruction cost".

### H-s8-3 — CONFIRMED: the target's two extra slots cannot come from its Judge folds
**Statement.** Because our body is byte-identical to the target's 148 instructions and
contains exactly two indexed `Judge` loads, and because the re-audited law gives
`slots = max(0, indexed-Judge sites - 1)`, the target's Judge folds account for exactly ONE
of its three slots; the other two are produced by a generator that emits no instructions at
all.
**Mechanism.** The Judge orphan is a `combine`/`distribute_notes` REG_DEAD-orphan `(use
(reg N))` on a folded `(symbol_ref)` address pseudo; its count is fixed by the number of
folded sites, which is fixed by the body, which matches. Any further slot must come from a
pseudo that is never emitted, i.e. a byte-free generator.
**Probe.** The re-audit above (site law) combined with the s5/s6 generator census
(`tmp/grind/func_80030580/s5/gen_probe.c`, `gen2.c`, `gen3.c`, `s6/gen4.c`, `gen5.c`,
`gen6.c`) and this session's twelve in-function shapes.
**Result.** The only byte-free multi-slot generator in the census is generator 4 (the
register-allocated union with a word member and two written HImode members: gen5.c `a1` and
`a7` emit the identical instruction stream, `a1` reserving 8 phantom bytes and `a7` none;
`a10`/`a11`/`b1`/`d2` reserve two slots). 1 + 2 = 3 = vars 24.
**Verdict: CONFIRMED** as a structural constraint on the search, NOT as a proposal: the
aggregate carrier was FAILed by the Judge on 2026-09-02 17:07 and the standing constraint
forbids respelling it.

### H-s8-4 — KILLED (instance): pointer-alias-to-`Judge` as a slot source
**Statement.** Declaring `s16 *jt = &Judge;` and indexing through it at both lookup sites
moves the frame from vars=8 to vars=0 on this chassis, i.e. away from the target's 24.
**Mechanism.** Materialising the symbol in a live pointer register stops combine folding
`(symbol_ref)` into the two `lh` insns, so no address pseudo is left to be orphaned by
`distribute_notes`; the existing p110 slot disappears with it.
**Probe.** `tmp/grind/func_80030580/s7/var_jptr.c` through `s7/runfiles.py` (instrumented
cc1, `BB2_FRAME_DEBUG=1`).
**Result.** `vars=0 bodydiff=19 sp=0`, only `ctx=round_frame`. Banked
`rejected/judge-pointer-alias-destroys-orphan.c`.
**Verdict: KILLED.** `kill_scope: instance` — `measured_on: pure-c-floor2-body.c chassis,
sandbox --disable all = 2, zero FAKE constructs present`.

### H-s8-5 — KILLED (instance): a scalar sub-word-pun spelling of generator 4
**Statement.** Substituting an `s32` local written through sub-word lvalues
(`*(u16 *)&t = 0; *((u16 *)&t + 1) = 0;`) for the s6 union does not reproduce generator 4's
byte-free slots on this chassis: it measures vars=16 with three `($sp)` accesses and 11
body-diff lines.
**Mechanism.** Taking `&t` runs the pseudo through `put_reg_into_stack`, turning it into a
REAL addressable frame object that the body then loads and stores; generator 4's
byte-freeness depends on the union staying register-allocated with only phantom
`ctx=spill_new` slots attached.
**Probe.** `tmp/grind/func_80030580/s7/var_punword.c` (obj+5 / obj+0 placement) and
`var_punpair7.c` (obj+7 / obj+8 placement) through `s7/runfiles.py`.
**Result.** punword `vars=16 bodydiff=11 sp=3  ctx=put_reg_into_stack | ctx=spill_new_p111`;
punpair7 `vars=16 bodydiff=12 sp=3`. Banked
`rejected/subword-pun-scalar-forces-frame-address.c`.
**Verdict: KILLED.** `kill_scope: instance` — `measured_on: pure-c-floor2-body.c chassis,
sandbox --disable all = 2, zero FAKE constructs present`.

### H-s8-6 — KILLED (instance): a CODE_LABEL placed between the two Judge folds
**Statement.** Relocating the `/32` term so its `bgez` join sits between the two indexed
`Judge` lookups does not strand the second REG_DEAD note: the frame stays at vars=8 (one
slot) and the body costs 30 diff lines.
**Mechanism.** `distribute_notes` emits the bare `(use (reg N))` only when the backward
scan from i3 finds no home before a CODE_LABEL/JUMP_INSN; the second fold's note still finds
a home regardless of the intervening label, so block geometry is not the controlling
variable for the second slot.
**Probe.** `tmp/grind/func_80030580/s7/var_divmid.c` through `s7/runfiles.py`.
**Result.** `vars=8 bodydiff=30 sp=0  ctx=spill_new_p102`. Banked
`rejected/div32-between-judge-sites-no-second-orphan.c`. Re-confirms the s5/s6
block-geometry kill on a new spelling and on the current chassis.
**Verdict: KILLED.** `kill_scope: instance` — `measured_on: pure-c-floor2-body.c chassis,
sandbox --disable all = 2, zero FAKE constructs present`.

### H-s8-7 — KILLED (instance): phantom-slot-frame-lever producers 1 and 3 in this function
**Statement.** The two ordinary-C producers named by `.claude/rules/phantom-slot-frame-lever.md`
— the folded loop-guard compare (producer 1) and live named locals on multi-read fields
(producer 3) — do not buy a byte-free slot in func_80030580 in the eight spellings measured
this session; every named-local shape that reaches vars=24 does so through a
`ctx=stack_temp` with real `($sp)` traffic.
**Mechanism.** The loop guard here is already a folded pointer-biv compare whose pseudo is
allocated; a fresh named local for a multi-read field is either coalesced into the existing
value (bodydiff 0, vars unchanged) or materialised as an addressable temp (stack_temp with
`($sp)` accesses), never left unallocated-with-refs.
**Probe.** `tmp/grind/func_80030580/s7/sweep.py` (11 shapes) + `runfiles.py` (posptr,
srcptr).
**Result.** whilerot `vars=8 bodydiff=0`; kindlocal `vars=8 bodydiff=0`; anglocal
`vars=8 bodydiff=0`; posptr `vars=8 bodydiff=0`; neguard `vars=8 bodydiff=20`; limitvar
`vars=8 bodydiff=11`; velnamed `vars=8 bodydiff=100`; srcptr `vars=8 bodydiff=23`; vecnamed
`vars=24 bodydiff=10 sp=6`; vechalf `vars=24 bodydiff=183 sp=5`; srcvec `vars=24
bodydiff=173 sp=5`; tblstruct `vars=16 bodydiff=154 sp=3`.
**Verdict: KILLED.** `kill_scope: instance` — `measured_on: pure-c-floor2-body.c chassis,
sandbox --disable all = 2, zero FAKE constructs present`. Four of the shapes (whilerot,
kindlocal, anglocal, posptr) are byte-neutral and are banked as free composables.

### FRONTIER (rewritten on the corrected model)
1. **A byte-free generator, other than the s6 union, that yields two `alter_reg` slots.**
   H-s8-3 proves the target needs exactly that. The census so far covers Judge folds
   (site-law), the HImode-bitwise generator (inert here), the truncated-division generator
   (not hostable), and generator 4 (the union, Judge-FAILed). Probe: run
   `tmp/grind/func_80030580/s5/genprobe.py` over a NEW standalone corpus aimed at
   register-allocated aggregates that are not unions — a small `struct` passed BY VALUE to
   nothing, a bitfield struct, an `enum`-typed local, a `char[2]` inside a struct read as a
   half, `short` bitfields — accepting only sp=0 with `ctx=spill_new`, then host the hits
   in-function with `s7/runfiles.py` and require vars=24 bodydiff<=4.
2. **A 4-site Judge form whose two extra lookups vanish after combine.** j4read reaches the
   exact target frame at bodydiff=25, itemised by s5 as exactly the two extra lookups
   (lh angle / addu const / andi 0xFFF / sll 1 / lh Judge / addu, twice). Probe: spellings
   in which the extra index expressions are provably equal to existing ones so cse deletes
   the LOADS while combine still folds and orphans the ADDRESSES — measure whether cse
   deleting the load also deletes the address pseudo (it is assumed to, but never measured).
3. **The disposition question.** Both known routes to vars=24 are closed by standing Judge
   constraints (the `volatile u32 pre_pad[4]` phantom-pad and the generator-4 union), and
   H-s8-1 closes the solver axis by tool verdict. If frontier items 1 and 2 also measure
   dead, the ladder for this function is genuinely spent and the next escalation-modality
   session has the full evidence chain to file a foreclosure record.

## [s7] goal_from_tgt.py classify types func_80030580's residual as PRE-RA with 'next tool: none', so neither tools/ra_solver nor tools/sched_solver has a model that covers it: the two 148-instruction streams already agree on register disposition and emission order and differ only in the prologue/epilogue addiu $sp immediate.
- mechanism: The frame size is fixed by assign_stack_local calls made from alter_reg for pseudos that reach reload with refs and no hard register. The solver models represent seats (which hard register a pseudo gets) and clocks (which order insns issue); a slot handed to a pseudo that got no seat and issues no insn is outside both representations.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_80030580 (refused by the zero-rule guard, which routes to the object path) then python3 tools/ra_solver/goal_from_tgt.py classify code6cac_b func_80030580.
- result: FIRST DIVERGENCE: PRE-RA / next tool: none - the residual is upstream of every model. ours 148 insns, target 148 insns, object-level replace_with_asmfile-safe; ours-only addiu #,#,-8 and addiu #,#,8, target-only addiu #,#,-24 and addiu #,#,24. The solver modality is mechanically closed for this function.
- verdict: KILLED
- kill_scope: class
- measured_on: pure-c-floor2-body.c chassis, sandbox --disable all = 2 re-measured this session; tools/fake_ablate.py reports no FAKE-annotated constructs in the chassis
- predicate_cite: tools/gcc-2.7.2/reload1.c:2403

## [s7] The target's vars=24 is three 8-byte alter_reg stack slots and our vars=8 is one of them; the s7 reading that the residual is a declared untouched frame object of 9 to 16 bytes rather than additional combine-orphan spill slots is retracted.
- mechanism: An unallocated pseudo's alter_reg slot costs zero instructions by construction, so 'every body instruction matches' is not evidence against a spill slot - it is exactly what an orphan slot looks like. Slot size is 8 rather than 4 because alter_reg calls assign_stack_local (mode, total_size, -1) and align = -1 rounds to alignment = 8.
- probe: tmp/grind/func_80030580/s7/fdbg.py on the base chassis (raw FRAMEDBG lines from the instrumented cc1), plus the mandated kill re-audit INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s3/frame3.py base j3read j4read dupX.
- result: base: ctx=spill_new_p110 mode=4 size=8 align=-1 alignment=8 frame_offset=8. j3read: p110@8 + p130@16 (vars=16, bodydiff=18). j4read: p110@8 + p130@16 + p143@24 (vars=24 - the exact target frame - bodydiff=25). dupX: p110@8 + p125@16 (vars=16, bodydiff=15). Every slot is mode=4 (SImode), size=8, align=-1. The s2/s3 orphan-spill model is reinstated and the site law slots = max(0, indexed-Judge sites - 1) reproduces exactly.
- verdict: CONFIRMED

## [s7] Because our body is byte-identical to the target's 148 instructions and contains exactly two indexed Judge loads, and because the re-audited law gives slots = max(0, indexed-Judge sites - 1), the target's Judge folds account for exactly one of its three slots; its other two slots are produced by a generator that emits no instructions.
- mechanism: The Judge orphan is a combine/distribute_notes REG_DEAD-orphan (use (reg N)) on a folded (symbol_ref) address pseudo; its count is fixed by the number of folded sites, which is fixed by the body, which matches. Any further slot must come from a pseudo that is never emitted, i.e. a byte-free generator.
- probe: The kill re-audit above combined with the s5/s6 standalone generator census (s5/gen_probe.c, gen2.c, gen3.c; s6/gen4.c, gen5.c, gen6.c) and this session's twelve in-function shapes measured with s7/sweep.py and s7/runfiles.py.
- result: The only byte-free multi-slot generator anywhere in the census is generator 4 (the register-allocated union with a word member and two written HImode members: gen5.c a1 and a7 emit the identical instruction stream, a1 reserving 8 phantom bytes and a7 none; a10/a11/b1/d2 reserve two slots). 1 Judge orphan + 2 generator-4 slots = 24. Recorded as the SHAPE of the answer and a constraint on the search, NOT as a proposal - the aggregate carrier was Judge-FAILed on 2026-09-02 17:07 and the standing constraint forbids respelling it.
- verdict: CONFIRMED

## [s7] Declaring s16 *jt = &Judge; and indexing through it at both lookup sites moves the frame from vars=8 to vars=0 on this chassis, away from the target's 24, at a cost of 19 body-diff lines.
- mechanism: Materialising the symbol in a live pointer register stops combine folding (symbol_ref) into the two lh insns, so no address pseudo is left for distribute_notes to orphan and the existing p110 slot disappears with it. Same failure direction as s2's jmix byte-offset spelling.
- probe: tmp/grind/func_80030580/s7/var_jptr.c compiled through s7/runfiles.py with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_FRAME_DEBUG=1).
- result: vars=0 bodydiff=19 sp=0, only ctx=round_frame reported. Banked memory/grind/func_80030580/rejected/judge-pointer-alias-destroys-orphan.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: pure-c-floor2-body.c chassis, sandbox --disable all = 2 re-measured this session, zero FAKE constructs present (fake_ablate reports none)

## [s7] Substituting an s32 local written through sub-word lvalues (*(u16 *)&t = 0; *((u16 *)&t + 1) = 0;) for the s6 union does not reproduce generator 4's byte-free slots on this chassis: it measures vars=16 with three ($sp) accesses and 11 body-diff lines.
- mechanism: Taking &t runs the pseudo through put_reg_into_stack, turning it into a real addressable frame object that the body then loads and stores; generator 4's byte-freeness depends on the union staying register-allocated with only phantom ctx=spill_new slots attached.
- probe: tmp/grind/func_80030580/s7/var_punword.c (obj+5 / obj+0 zero-store placement) and var_punpair7.c (obj+7 / obj+8 placement) through s7/runfiles.py.
- result: punword vars=16 bodydiff=11 sp=3 with contexts put_reg_into_stack + spill_new_p111; punpair7 vars=16 bodydiff=12 sp=3. The frozen sub-word-access family therefore does not buy generator 4's slots without an aggregate carrier. Banked memory/grind/func_80030580/rejected/subword-pun-scalar-forces-frame-address.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: pure-c-floor2-body.c chassis, sandbox --disable all = 2 re-measured this session, zero FAKE constructs present

## [s7] Relocating the /32 term so its bgez join sits between the two indexed Judge lookups does not strand the second REG_DEAD note on this chassis: the frame stays at vars=8 (one slot, now p102) and the body costs 30 diff lines.
- mechanism: distribute_notes emits the bare (use (reg N)) only when the backward scan from i3 finds no home before a CODE_LABEL/JUMP_INSN; the second fold's note still finds a home regardless of the intervening label, so block geometry is not the controlling variable for a second slot.
- probe: tmp/grind/func_80030580/s7/var_divmid.c through s7/runfiles.py with the instrumented cc1.
- result: vars=8 bodydiff=30 sp=0, ctx=spill_new_p102. Re-confirms the s5/s6 block-geometry kill on a new spelling and on the current chassis. Banked memory/grind/func_80030580/rejected/div32-between-judge-sites-no-second-orphan.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: pure-c-floor2-body.c chassis, sandbox --disable all = 2 re-measured this session, zero FAKE constructs present

## [s7] The two ordinary-C producers named by .claude/rules/phantom-slot-frame-lever.md - the folded loop-guard compare (producer 1) and live named locals on multi-read fields (producer 3) - do not buy a byte-free slot in func_80030580 in the eight spellings measured this session; every named-local shape that reached vars=24 did so through a ctx=stack_temp with real ($sp) traffic.
- mechanism: The loop guard here is already a folded pointer-biv compare whose pseudo is allocated; a fresh named local for a multi-read field is either coalesced into the existing value (bodydiff 0, vars unchanged) or materialised as an addressable temp with ($sp) accesses, never left unallocated-with-refs.
- probe: tmp/grind/func_80030580/s7/sweep.py (11 shapes: base, vecnamed, vechalf, velnamed, whilerot, neguard, limitvar, tblstruct, kindlocal, anglocal, srcvec) plus s7/runfiles.py (posptr, srcptr).
- result: whilerot vars=8 bodydiff=0; kindlocal vars=8 bodydiff=0; anglocal vars=8 bodydiff=0; posptr vars=8 bodydiff=0; neguard vars=8 bodydiff=20; limitvar vars=8 bodydiff=11; velnamed vars=8 bodydiff=100; srcptr vars=8 bodydiff=23; vecnamed vars=24 bodydiff=10 sp=6; vechalf vars=24 bodydiff=183 sp=5; srcvec vars=24 bodydiff=173 sp=5; tblstruct vars=16 bodydiff=154 sp=3. Four shapes (whilerot, kindlocal, anglocal, posptr) are byte-neutral and are banked as free composables; posptr is saved as memory/grind/func_80030580/composable-posptr-byte-neutral.c and srcptr as rejected/src-vec3i-pointer-costly.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: pure-c-floor2-body.c chassis, sandbox --disable all = 2 re-measured this session, zero FAKE constructs present
