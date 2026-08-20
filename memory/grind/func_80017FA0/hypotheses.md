# Hypothesis ledger — func_80017FA0

## RESOLVED — the function is MATCHED (s4, permuter, 2026-08-20)

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
