# Hypothesis ledger - func_80017FA0

## RESOLVED - MATCHED IN PURE C, NO COERCION CONSTRUCT (s5, permuter modality, 2026-08-20)

- H-S5 [loop notes] The inner loop's three scratchpad stores keep their full
  absolute addresses (target's `lui $at,%hi ; addu $at,$a1,$at ; sw $2,%lo($at)`
  x3) if and only if the inner loop is GOTO-FORMED. Nothing about the address
  expression, the volatile, or the assembler was ever the question: it is the
  loop NOTES.
  mechanism: the C front end emits NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END only
  for for/while/do statements, never for a loop built from a label + goto, and
  loop.c analyses only note-delimited loops. With `do { } while (j < 2)` the cc1
  `.loop` dump shows loop.c forming the three stores' addresses as DEST_ADDR
  givs of the biv `sp_inner` (`dest address src reg 86 ... mult 1 add 528482404
  / 528482408 / 528482412`), combine_givs merging them (`giv at 85 combined with
  giv at 94`, `giv at 76 combined with giv at 94`) and strength_reduce hoisting
  one biased base (`giv at 94 reduced to (reg:SI 102)`) out of the loop - 57
  insns against target's 61. Written goto-formed, loop.c never sees the loop, the
  numeric absolute addresses survive to the assembler, and maspsx expands each
  `sw $2,528482404($5)` (numeric operand > 32767) into the target's operand order
  `addu $at,$a1,$at` (tools/maspsx/maspsx/__init__.py:1183).
  probe: variant tmp/grind/func_80017FA0/s5/vg1.c = the s4 non-volatile numeric
  body (s4/body_nv.c) with the inner do-while rewritten as
  `inner: { ... } if (j < 2) goto inner;`. Checked with
  tmp/grind/func_80017FA0/s4/check.sh, then applied to src/code6cac.c.
  result: 61 insns, objdump IDENTICAL TO TARGET, `.frame $sp,8 # vars= 8`;
  `sandbox func_80017FA0 --disable all` = 0; `build` SHA1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle). Zero rules, zero cheat
  asm, zero volatile, zero FAKE constructs, no dead locals.
  verdict: **CONFIRMED - function closed in pure C.**

- H-S5b [loop.c is unconditional once the giv forms] With a NUMERIC address on a
  single biv, GCC 2.7.2 will ALWAYS strength-reduce; no numeric spelling can
  survive. This retro-explains s4's seven dead numeric variants and is why the
  answer had to be "prevent the loop from being analysed at all".
  mechanism: read out of tools/gcc-2.7.2/loop.c - strength_reduce:3823 skips a
  giv only when `v->lifetime * threshold * benefit < insn_count`, with threshold
  = 2*(3+n_non_fixed_regs) (large) and lifetime 1, so it skips only when benefit
  <= 0. A single DEST_ADDR giv has benefit 2 - add_cost(2)*biv_count(1) = 0 and
  IS left alone (dump: "giv of insn 70 not worth while, 0 vs 15"), but
  combine_givs:5494 merges the three same-biv address givs into one of benefit 6
  first, and combine_givs_p:5457 always accepts them on MIPS because
  ADDRESS_COST(reg + small const) = 1 <= ADDRESS_COST(reg + 0x1F800068) = 2
  (mips.c:mips_address_cost:1653). express_from:5417 is the only escape hatch
  (`GET_CODE (g1->add_val) != CONST_INT` -> return 0), which is exactly why the
  s4 extern-SYMBOL form survived - but a symbol operand is then passed through
  maspsx unexpanded and GNU as emits the OTHER operand order
  (`addu $at,$at,$a1`), which is why that form stalled at 58/61.
  verdict: CONFIRMED.

- KILLED (s5): "The 3-instruction residual is a GNU-as / build-surface question,
  i.e. an integration handoff." Direct measurement of the assembler
  (tmp/grind/func_80017FA0/s5/asmorder.sh) shows GNU as emits `addu at,at,base`
  for BOTH numeric and absolute-symbol address expressions - s4's claim that as
  picks the order from the expression form was wrong. The target's operand order
  comes from maspsx's own expansion of numeric >32767 store offsets
  (tools/maspsx/maspsx/__init__.py:1183), which fires only for a NUMERIC operand.
  So the residual was always a cc1 question, and the answer was in C.


## SUPERSEDED — s4's claim of a match (the form it matched with is now BANNED)

- H-S4 [live-local phantom frame] The target's zero-store 8-byte leaf frame does
  NOT require a dead local. An ordinary LIVE local that the allocator keeps in a
  register is still counted by get_frame_size(), and ALIGN8(4)=8 on
  MIPS_STACK_ALIGN reserves exactly the target's 8 bytes with zero frame stores.
  mechanism: cc1 assigns the counter `i` a frame slot at expand time; flow/local-alloc
  then keep it in a register, so mips.c:compute_frame_size still emits
  `addiu sp,sp,-8` / `addiu sp,sp,8` while no `sw`/`lw` to $sp is ever emitted.
  Predicted and byte-verified project-wide in
  memory/project/phantom-frame-slots-gcc272.md (witness: tslLineG5Init, COMPLETED-C).
  probe: spell the outer loop's entry guard against the live counter —
  `if (i < ptr[1])` instead of `if (ptr[1] > 0)` (identical test, i==0 there).
  result: `.frame $sp,8 # vars= 8`; 61 insns; objdump diff vs the assembled
  asm/funcs/func_80017FA0.s EMPTY; `sandbox --disable all` = 0; full clean build
  SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle). Zero rules, zero
  cheat-asm, zero FAKE constructs.
  verdict: **CONFIRMED — function closed.**

- H-F1 (s2) and H-F2 (s3) are hereby **UN-KILLED AS FALSELY REASONED**, not
  merely superseded. Both rested on the claim "every zero-store vars=8 shape is a
  dead local". That claim came from probes (F0/F1, g1-g4, framemath) that only ever
  varied DEAD locals and genuinely-USED AGGREGATES, never a live ordinary SCALAR.
  The missing quadrant was the answer. The 2026-07-24 OWNER-ESCALATION and the
  2026-07-27 owner ruling (REFUSED / OWNER-ACCEPTED INCOMPLETE) were filed on that
  false dichotomy and are superseded by the oracle match — see docs/grind/decisions.md.

- STANDING LESSON for other phantom-frame functions (the family s1-s3 named:
  AddTbpOfst_80047EE8, InitHiraRmd_80047FBC, func_8003DBE4, file_LoadSectors /
  func_800165F8, and any other "target reserves frame bytes it never writes" case):
  before concluding a dead local is the only producer, test whether a LIVE local
  the function already owns can be brought into an expression that currently uses
  a literal — a loop guard against a counter, a bound already held in a variable,
  an index already computed. Measure with cc1's own `# vars=` comment, which is a
  direct gradient on the frame.

## CONFIRMED (s1)

- H1 [fold] Fixed-write 3-insn-vs-2-insn gap is the integer-constant-address
  synthesis path (`*(volatile s32*)0xCONST` -> `lui;ori;sw 0(reg)`). Routing the
  store through a POINTER-TYPED LVALUE (`scr[idx]` local-ptr-var, or struct field)
  makes cc1 emit `sw x,<const>` which the assembler folds to `lui;sw disp(reg)`.
  Probe P4/P5 fold, P1 does not. CONFIRMED — pure C, no cheat. (Kills the WIP's
  "no pure-C idiom reliably produces the fold" claim.)

- H2 [cascade] The top v0<->v1 register cascade (5 diffs) is DOWNSTREAM of H1:
  the early delay-slot lui that materialized the B8 address in v0 forced `temp`
  into v1. Fixing H1 removed the early lui; temp stays in v0; cascade vanished.
  CONFIRMED (score 13->2 in one edit closed both fold + cascade).

- H3 [frame-trigger] Target's empty 8-byte leaf frame (zero frame stores) is
  reproduced in pure C ONLY by a dead <=8-byte local ARRAY (F2 vars=8, no sw/lw).
  Dead scalars go to registers (F0/F1 vars=0); written arrays add stores target
  lacks (F3). CONFIRMED the producer is the forbidden dead-vars-local-array;
  WRITTEN-array carve-out inapplicable (zero frame stores in target).

## KILLED (s3, structural — the axis s2 skipped)

- H-F2 [OVERSIZED-LOCALS carve-out sanctions the fully-dead 8-byte pad] The
  2026-07-13 OVERSIZED-LOCALS carve-out (which s2 never evaluated — s2 checked
  only the 2026-07-01 WRITTEN carve-out) might sanction a fully-dead `s32 pad[2]`
  to reserve target's 8-byte zero-store frame via its frame-math proof.
  mechanism: frame(8)−saves(0)−args(0) > written(0) ⇒ target frame "proves"
  dead locals; fully-dead-pad fallback shape.
  probe: decoded frame from asm/funcs/func_80017FA0.s (leaf, 0 saves/args/writes,
  frame=8); ran the carve-out's 5 prerequisites (tmp/grind/func_80017FA0/s3/framemath.md).
  result: KILLED. Prerequisite 1 satisfied only TRIVIALLY (0<N holds for every
  zero-store phantom leaf frame ⇒ non-distinguishing from plain frame coercion);
  no written prefix / no live object ⇒ fully-dead-pad fallback; NO SOTN precedent
  for an unwritten local array as a phantom-frame carrier; sanctioning requires
  engine-detector allowlist wiring (forbidden grind surface) + owner ruling.
  verdict: KILLED. ⇒ all sanctioned axes dead; OWNER-ESCALATION filed, owner-gated.

## KILLED (s2, structural)

- H-F1 [empty-8-byte-frame legitimate shape] Some legitimate small-local shape
  (struct-by-value, union, two address-taken scalars) reserves target's 8-byte
  zero-store frame without being a dead-var cheat.
  mechanism: function.c assign_stack_local reserves vars=8 for an aggregate/
  address-taken local; if all accesses DCE'd only addiu sp,-8/+8 remain.
  probe: frameprobe2.{c,s} — g1 dead struct=vars8 zero-store, g2 dead union=vars8
  zero-store, g3 two &-taken scalars (&x==&y folds)=vars8 zero-store, g4 struct
  read-once=vars8 but WITH sw stores target lacks.
  result: KILLED. Every zero-store vars=8 shape (array/struct/union/addr-taken) is
  a DEAD local = forbidden dead-vars-local-array; WRITTEN carve-out inapplicable
  (target has ZERO frame stores). Every genuinely-used aggregate emits stores
  target lacks. No legitimate structural shape exists. verdict: KILLED.
  => endgame-lock condition met; next step is escalation-modality OWNER-ESCALATION
  (mirror sibling InitHiraRmd_80047FBC / AddTbpOfst_80047EE8), NOT more structural.

## FRONTIER (for the next session — structural/endgame modality)

- F1 [empty-8-byte-frame] The sole residual (distance 2). Mechanism: `.frame
  $sp,8` from a source-level local decl reserved by function.c assign_stack_local,
  with all accesses DCE'd. Measured: only a dead <=8-byte aggregate reproduces it
  (forbidden). canonical-asm NEGATIVE (scan_hand_coded LOW 1/8, ordinary compiled
  C — so a C source DID produce it). Next probes for a deep-dive:
  1. Exhaust remaining legitimate small-local shapes that could reserve 8 bytes
     with zero accesses: struct-by-value local, union, two address-taken scalars
     whose address-use folds away — confirm each is either vars=0 or a cheat.
  2. If all legitimate shapes are dead -> this is endgame-lock-disposition
     (RA/frame-locked, 2 insns short, byte-matchable only via forbidden dead-vars).
     Mirror sibling AddTbpOfst_80047EE8's OWNER-ESCALATION (docs/grind/decisions.md,
     filed 2026-07-20, STILL AWAITING RULING — identical zero-store phantom-frame
     residual). File owner-gated in the escalation modality, not before.
  See [[endgame-lock-disposition]], [[dead-vars-local-array]] (WRITTEN carve-out),
  sibling AddTbpOfst_80047EE8 + InitHiraRmd_80047FBC evidence.

## [s1] The fixed-write 3-insn (lui;ori;sw 0) vs target 2-insn (lui;sw disp) gap is the integer-constant-address synthesis path; routing the store through a pointer-typed lvalue (scr[idx] local-ptr-var or struct field) makes cc1 emit sw x,<const> which the assembler folds.
- mechanism: *(volatile s32*)0xCONST force_regs the CONST_INT (lui+ori) then sw 0(reg); a pointer-offset lvalue yields (mem (const_int)) that as expands to lui at,0x1f80 + sw x,disp(at).
- probe: cc1 -O2 -G0 -funsigned-char -mcpu=3000 on foldprobe.c, then maspsx 2.34 + as + objdump: P1 int-cast=3insn no fold; P4 local-ptr-var and P5 struct-cast = 2-insn folded = target bytes (foldprobe.o).
- result: P4/P5 folded to lui at,0x1f80; sw a0,0x60(at); P1 stayed 3-insn. Applied scr[0x2E]/scr[0x18] to the real fn -> sandbox --disable all dropped 13->2.
- verdict: CONFIRMED

## [s1] The top v0<->v1 register cascade (5 of 13 diffs) is downstream of the B8-store fold, not an independent RA wall.
- mechanism: The early delay-slot lui materializing the 0x1F8000B8 address in v0 forced temp/ptr[0] into v1; removing the early lui (via the fold) frees v0 for temp.
- probe: Same edit as H1; re-disassembled build (build_disasm.txt): top now lw v0,0xC(a0)/move t1,v0/lw v0,0(t1)/sll v0 — matches target register-for-register.
- result: Cascade vanished; the single fold edit closed both the fold and the cascade (11 of 13 diffs).
- verdict: CONFIRMED

## [s1] Target's empty 8-byte leaf frame (zero frame stores) is reproducible in pure C ONLY by a dead <=8-byte local array (the forbidden dead-vars-local-array); no legitimate scalar/written-array shape yields it.
- mechanism: function.c assign_stack_local reserves vars=8 for an aggregate local decl; when its uses are DCE'd only addiu sp,-8/+8 remain (zero sw/lw). Dead scalars go to registers (vars=0); written arrays add sw stores target lacks.
- probe: frameprobe.c compiled with cc1: F0 no-local=vars0, F1 dead-scalar=vars0, F2 dead s32 buf[2]=vars8 with ZERO sw/lw to $sp (matches target), F3 written buf=vars8 + extra sw.
- result: Only F2 (dead unused 8-byte array) reproduces target's zero-store 8-byte frame. WRITTEN-array carve-out inapplicable (target has zero frame stores). canonical-asm NEGATIVE (scan_hand_coded LOW 1/8).
- verdict: CONFIRMED

## [s2] Some legitimate small-local shape (struct-by-value, union, two address-taken scalars) reproduces target's 8-byte zero-store frame without being a dead-var cheat.
- mechanism: function.c assign_stack_local reserves vars=8 for an aggregate/address-taken local decl; if all accesses DCE'd, only addiu sp,-8/+8 remain (zero frame stores).
- probe: frameprobe2.{c,s} via cc1 -O2 -G0 -funsigned-char -mcpu=3000: g0 no-local=vars0 control; g1 dead struct=vars8 zero-store; g2 dead union=vars8 zero-store; g3 two &-taken scalars (&x==&y folds)=vars8 zero-store; g4 struct read-once=vars8 WITH sw $2,0($sp)/sw $3,4($sp)+reloads.
- result: Every zero-store vars=8 shape (array/struct/union/addr-taken) is a DEAD local = forbidden dead-vars-local-array; WRITTEN carve-out inapplicable since target has ZERO frame stores. Every genuinely-used aggregate (g4) emits frame stores the target lacks. No legitimate structural shape produces target's zero-store 8-byte frame.
- verdict: KILLED

## [s3] The 2026-07-13 OVERSIZED-LOCALS carve-out (unchecked by s2, which only evaluated the 2026-07-01 WRITTEN carve-out) sanctions a fully-dead s32 pad[2] to reserve target's 8-byte zero-store leaf frame via its frame-math proof.
- mechanism: cc1 assign_stack_local reserves vars=8 for a source-level local decl with all accesses DCE'd, leaving bare addiu sp,-8/+8 with zero frame stores; carve-out prerequisite 1 = frame(8)-saves(0)-args(0)=8 > written(0), a fully-written form yields 0 locals => 0 frame < 8, so target frame 'proves' 8 dead bytes.
- probe: Reproduced floor 2 (candidate.c applied, sandbox --disable all = 2, 60/61). Decoded frame directly from asm/funcs/func_80017FA0.s (leaf: 0 sw$ra/sw$s?, 0 jal => saves=0 args=0; zero sw/lw ($sp) => 0 written). Ran the carve-out's 5 prerequisites (tmp/grind/func_80017FA0/s3/framemath.md).
- result: Prerequisite 1 satisfied only TRIVIALLY (0<N holds for ANY zero-store phantom leaf frame => does not distinguish a genuine oversized-locals object like the granted func_80037540 [24B written prefix + live callee buffer, ALIGN8(24)+16+24=0x40!=0x48] from plain frame coercion). No written prefix / no live locals object => fully-dead-pad fallback; no SOTN precedent for an unwritten local array as a phantom-frame carrier; sanctioning requires wiring a prerequisite-aware engine allowlist (forbidden grind surface) + owner ruling.
- verdict: KILLED

## s5 (2026-08-20)

- **KILLED** — "The s4 candidate only needs a citation/annotation fix." The
  02:54 Judge FAIL was on the scratchpad `volatile`, a load-bearing construct
  now on the driver's BANNED list, not on wording. Restoring the candidate would
  auto-reject. (Citation corrected in self_vet.md regardless; the frame lever's
  real exhibit is func_8003DBE4 / phantom-slot-frame-lever.md:37-41 and
  src/code6cac_c2.c:1325, not tslLineG5Init.)
- **KILLED** — "A numeric-constant scratchpad address can reach target without
  volatile." Seven spellings measured (vNV, va1, va2, vb1, vb2, vc1, vc4); all
  are strength-reduced by loop.c into one biased base register. Best 57 insns,
  worst 63. Mechanism named and measured, not guessed.
- **CONFIRMED** — "A symbol_ref scratchpad address defeats loop.c strength
  reduction without any volatile." vb4: 61 insns, 58/61 identical, vars= 8.
- **CONFIRMED** — "The s4 frame lever is independent of the volatile." Every
  non-volatile variant still reports `# vars= 8`.
- **CONFIRMED** — "The remaining 3-instruction residual is a GNU as expansion
  choice, not a cc1 choice." maspsx emits `sw $2,D_1F800000+108($5)` unexpanded;
  as picks `addu at,at,base` for symbol expressions and `addu at,base,at` for
  numeric ones.
- **KILLED** — "A two-prong volatile grant could be cited for scratchpad."
  sotn-construct-index.md has zero 0x1F800000-range entries: negative census.

## [s4] The s4 candidate needs only a citation/annotation fix and can be restored and resubmitted this session.
- mechanism: The fix-up notice quoted the 02:40 layer-1 note ('citation only'). The LATER 02:54 Judge call FAILed the same candidate on the scratchpad volatile - a load-bearing construct the driver has since added to the BANNED list for this function.
- probe: Read docs/grind/decisions.md:7742-7748 and the brief's BANNED CONSTRUCTS block; compared them against memory/grind/func_80017FA0/candidate.c, which declares volatile s32 *scr / *ac_base and three *(volatile s32*)(0x1F8000xx + off) stores.
- result: Restoring candidate.c would produce a diff whose self-vet must declare a banned construct, which the driver rejects before the Judge is spawned. src/code6cac.c left untouched (INCLUDE_ASM). The citation itself was corrected in self_vet.md anyway: the s4 vet cited tslLineG5Init, which is producer #2 (combine orphan-USE); the rotated guard's real exhibit is func_8003DBE4 at .claude/rules/phantom-slot-frame-lever.md:37-41 plus the in-tree instance src/code6cac_c2.c:1325.
- verdict: KILLED

## [s4] The s4 frame lever (rotated guard on the live counter) depends on the volatile and dies with it.
- mechanism: If the 8-byte phantom frame came from the volatile scratchpad accesses rather than from `i` surviving to frame layout, banning volatile would also cost the frame.
- probe: Compiled seven non-volatile variants (vNV, va1, va2, vb1, vb2, vb4, vc1) through the project cc1 and read the .frame directive via tmp/grind/func_80017FA0/s4/check.sh.
- result: Every non-volatile variant reports '.frame $sp,8,$31 # vars= 8, regs= 0/0, args= 0, extra= 0' - identical to target. The frame lever is fully independent of the volatile and survives the ban.
- verdict: KILLED

## [s4] Some pure-C NUMERIC-constant address expression makes cc1 re-materialize the 0x1F80 base at every inner-loop store (the Judge's binding constraint).
- mechanism: With a numeric address, cc1 loop.c sees (mem (plus (reg sp_inner) (const_int 0x1F800064))) as a general induction variable, combines the three address givs and hoists one biased base (lui;ori;addu a1,t2,v0) out of the inner loop, collapsing the stores to sw v0,-8(a1)/-4(a1)/0(a1).
- probe: Seven spellings built and objdump-diffed against asm/funcs/func_80017FA0.s: vNV plain 57 insns; vb1 u32 sp_inner 57; vb2 (u8*)0x1F800064 + sp_inner 57; va2 operand order sp_inner + 0x1F800064 57; va1 sp_off = sp_inner (unbiased value consumed after the inner loop) 57; vc1 named intermediate s32 ad = 0x1F800064 + sp_inner with ad/ad+4/ad+8 60; vc4 no sp_inner, 0x1F800064 + i*0x18 + j*0xC 63 (three separate hoisted bases).
- result: All seven strength-reduced; target is 61 insns. The numeric-constant address family is dead for this residual.
- verdict: KILLED

## [s4] An extern-SYMBOL scratchpad address blocks the giv and reproduces target's per-store address materialization with no volatile anywhere.
- mechanism: A symbol_ref address is not a general induction variable in GCC 2.7.2 loop.c, so the three address givs are never formed or combined; cc1 emits 'sw $2,D_1F800000+100($5)' per store.
- probe: Variant vb4 = the non-volatile body with the three inner stores written *(s32 *)((u8 *)D_1F800000 + 0x64 + sp_inner) and extern s32 D_1F800000[]; assembled with D_1F800000 = 0x1F800000 and diffed instruction-for-instruction against target.
- result: 61 insns (target 61), 58 of 61 byte-identical, vars= 8, zero volatile in the function. Banked as the new candidate.c.
- verdict: CONFIRMED

## [s4] The remaining 3-instruction residual is a cc1 codegen choice reachable from C.
- mechanism: If cc1 chose the addu operand order, some C spelling could flip it.
- probe: Ran the cc1 -> prologue_fix -> maspsx pipeline alone (tmp/grind/func_80017FA0/s4/pipe.sh) and inspected the pre-assembler text.
- result: maspsx passes the store through unexpanded ('sw $2,D_1F800000+108($5)'), so GNU as performs the expansion; as emits 'addu at,base,at' for a NUMERIC address expression and 'addu at,at,base' for a SYMBOL expression. The operand order is an assembler expansion-path artifact, not a cc1 choice.
- verdict: KILLED

## [s4] A two-prong legitimate-volatile grant could be cited for scratchpad RAM 0x1F800000-0x1F8003FF, reinstating the s4 form.
- mechanism: legitimate-volatile-interrupt-touched requires an IRQ/DMA writer plus a sanctioned use-site shape; mmio-volatile-type-level.md:44-46 excludes the scratchpad range by name.
- probe: Case-insensitive grep of docs/reference/sotn-construct-index.md for 0x1F800, 1f8000 and 'scratchpad'.
- result: Zero matches - a negative census, i.e. a FAILED gate rather than an open question. No SOTN-master precedent exists for volatile on the scratchpad range; the ban should be treated as final.
- verdict: KILLED


## s6 (2026-08-20, synthesis) - frontier RESET

The pre-s6 frontier (H12-H14: a numeric-but-non-giv address spelling, the
GNU-as expansion-order integration handoff, a declared scratchpad object) is
**RETIRED WHOLESALE**, and not because it was exhausted - because it answers a
question that is no longer open. All three were attempts to close a
3-instruction residual that s5 already closed, and s5's reading of
`tools/gcc-2.7.2/loop.c` shows the whole family was never viable:
`combine_givs:5494` merges the three same-biv DEST_ADDR address givs into one
worth benefit 6 before `strength_reduce:3823` ever applies its leave-alone test,
and `combine_givs_p:5457`'s MIPS gate always passes
(`ADDRESS_COST(reg+small)=1 <= ADDRESS_COST(reg+0x1F800068)=2`). H13 in
particular is affirmatively DEAD: s5 measured
(`tmp/grind/func_80017FA0/s5/asmorder.sh`) that GNU as emits `addu at,at,base`
for BOTH numeric and absolute-symbol address expressions, so no symbol
registration in named_syms.txt / symbol_addrs.txt can ever produce the target's
`addu at,base,at` - that operand order comes from maspsx's own expander
(`tools/maspsx/maspsx/__init__.py:1183`), which fires only on a NUMERIC operand.
There is no integration handoff here.

**The function is SOLVED in pure C. The remaining frontier is procedural, not
technical**, and is deliberately narrow:

- **H15 (the live one).** The only thing between this ledger and COMPLETED-C is
  `state.json.banned_constructs[1]` - a mechanical tripwire the driver banked
  from a layer-1 review whose own next-action field says "Do not treat this as a
  construct ban". Mechanism: `grindlib.check_banned_constructs` / `_ban_trips`
  scans the vet's CONSTRUCTS: block for >=50% of the ban phrase's content words,
  so an HONEST declaration of the goto-formed inner loop auto-discards the
  session before any reviewer sees it, while a declaration worded to slip past it
  would be detector evasion (cheat-checklist T4) and is not on the table. Probe:
  s6 emitted `ruling-request`; the driver's ruling path (`grind.ps1:531-538`)
  clears the tripwire via `grindlib.py unban . func_80017FA0 <needle>` when the
  ruling narrows or supersedes the ban. Next session, IF the ban is cleared:
  apply candidate.c (already annotated), re-measure sandbox (expect 0), return
  candidate-ready with the staged self_vet.md verbatim. IF the ruling instead
  upholds the ban as a real construct ban, the next attack is H16.
- **H16 (fallback attack, only if H15's ruling upholds the ban).** A
  note-delimited (`do`/`while`/`for`) inner loop whose three store addresses are
  never FORMED as DEST_ADDR givs, without volatile and without goto. Mechanism:
  `loop.c:find_mem_givs` requires the address to reduce to `(plus (reg-that-is-a-
  biv) (const_int))` under `simplify_giv_expr`; semantically-true spellings that
  break that are (a) an inner-loop address whose variable part is loaded from
  MEMORY inside the loop rather than carried in a biv, (b) a base whose increment
  is not a compile-time constant, (c) a shape where the store base also escapes.
  Probe: build each as a variant against `tmp/perm_17fa0/base.c` with the s4
  harness (`tmp/grind/func_80017FA0/s4/{mkvar.py,check.sh,batch.sh}`) and read
  the `.loop` dump each time (`pwsh tools/grinder/dump.ps1 func_80017FA0`) to
  confirm giv formation is actually ABSENT rather than merely unprofitable. Note
  in advance: every such spelling is MORE contrived than the goto loop, i.e.
  worse under the human-programmer test - H16 is a fallback, not an improvement.
- **H17 (dormant, only if both above fail).** The inner loop is not a loop in the
  original at all but a two-iteration macro/inline expansion whose second copy is
  cross-jumped back by jump.c tail-merging, producing a back edge with no loop
  note and no goto in the source. Mechanism: `jump.c` cross-jumping two identical
  statement copies creates the same note-free back edge the goto form does.
  Probe: write the body twice (fully unrolled, 6 stores, identical statement
  text) and check whether `jump2` merges them into the target's single copy plus
  back edge; read the `.jump2` dump to confirm. If it does, the match is
  reachable with NO goto and NO annotation at all.

## s5 (synthesis, 2026-08-20) � frontier resolved, function CLOSED

H15 (goto-formed inner loop defeats loop.c giv formation and closes the last 3
instructions) is CONFIRMED and now shipped: score 0 / 61 of 61 / oracle SHA1
re-measured this session. H16 (a numeric-address spelling that is not a
recognised giv) and H17 (a declared scratchpad object / an assembler-surface
expansion-order fix) are both MOOT � they were alternative routes to the same
three instructions that H15 already closes; the Judge ruled H16 "NOT required and
would be strictly more contrived" (docs/grind/decisions.md:7835). They are not
killed by measurement, they are simply unnecessary, and should NOT be revived
unless the H15 form is ever rejected on grounds other than paperwork.

No live hypotheses remain. The frontier for this function is empty; the only
outstanding step is acceptance of the resubmitted candidate (layer-1 + Judge).

