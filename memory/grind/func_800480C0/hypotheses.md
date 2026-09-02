# Hypothesis ledger — func_800480C0

## s1 — recon (2026-09-02)

- **H1 CONFIRMED — sibling transplant.** Porting the COMPLETED-C `func_80047FBC` body
  (two constructs: `volatile u32 pre_pad[8]` phantom-frame pad + `arg0 = 0;` dead store)
  extended to 6 args reproduces the target byte-for-byte: full build SHA1 == oracle.
  Measured: sandbox 20 (pad stripped) / build MATCH (pad honoured). Artifacts:
  `tmp/grind/func_800480C0/s1/{ladder.txt,build.log,cand1_build.txt}`.
- **H2 CONFIRMED — the 20-insn honest residual is 100% frame-offset.** Diff of sandbox
  build vs target: every mismatching insn is an sp-relative offset differing by 0x20; the
  instruction order and all non-sp operands match. No RA / scheduler / cse residual
  exists once the frame is 0x58.
- **H3 KILLED (instance) — removing the `arg0 = 0` dead store measures 32 on this chassis.**
  cse2 canonical-reg substitution rebinds the base copies and cascades. measured_on:
  HEAD dd2808d5, pre_pad present-but-stripped, single arm. The store is load-bearing.
- **Not attempted (deliberately):** honest frame producers for the 32-byte region. The
  region has zero stores in the target, so the WRITTEN-array carve-out is inapplicable
  by rule text (`.claude/rules/dead-vars-local-array.md:5`), and the identical-layout
  siblings spent 12+ sessions / ~26.5k permuter iters measuring honest producers inert
  (`docs/grind/decisions.md:7401-7660`, `src/text1b.c:20-36` header). Re-activation
  trigger: a frame-forensics finding that the original declared a LIVE 8-word local here.

## Frontier (for the operator / next session)
1. INTEGRATION HANDOFF filed (docs/grind/decisions.md 2026-09-02 entry): add
   `"func_800480C0": frozenset({("pre_pad", 8)})` to `_SANCTIONED_UNWRITTEN_PADS`, re-run
   sandbox (expect 0), verify-oracle, layer-2 cheat-reviewer, `queue done`.
2. Nothing else remains: the instruction stream is already byte-identical.

## [s1] Transplanting the COMPLETED-C func_80047FBC body (volatile pre_pad[8] + arg0=0 dead store) extended to 6 args reproduces func_800480C0 byte-for-byte
- mechanism: identical prologue/loop idiom; phantom 32-byte vars region reserved by function.c assign_stack_local; cse2 canonical-reg fold defeated by the param dead store
- probe: apply body to src/text1b.c; canonical; sandbox --disable all; full build
- result: canonical C/20; sandbox 20 (74/74, all 20 residual = sp-offset deltas, pad stripped); build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH
- verdict: CONFIRMED

## [s1] With the pad honoured the honest residual is purely frame-offset: no RA/scheduler/cse residual remains
- mechanism: frame 0x38 vs 0x58 shifts every sp-relative operand by 0x20
- probe: objdump diff of sandbox build vs target
- result: 20 mismatching insns, each an sp-offset delta; instruction order and all other operands identical
- verdict: CONFIRMED

## [s1] Removing the arg0 = 0 dead store (pad still stripped) measures 32 on this chassis, so the store is load-bearing for this body
- mechanism: cse2 canonical-register substitution over {arg0,p,base_addr} rebinds addu $s2 / addu $s0 to $a0 and cascades
- probe: delete the store; sandbox --disable all
- result: 32 vs 20 with the store
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD dd2808d5, candidate body with volatile pre_pad[8] present but detector-stripped, arg0=0 removed

## s2 — structural (2026-09-02)

## [s2] The 20 recorded as func_800480C0's honest floor is produced by the sandbox stripper leaving a dangling `volatile` that binds to `u32 *p`, not by any body a session wrote
- mechanism: engine/cheats.py removes the pad declarator text only; the orphaned qualifier attaches to the following declaration, so the scored TU compiles a volatile-pointer body
- probe: read tmp/sandbox/func_800480C0/src/text1b.c:160-161 after `sandbox --disable all` on the s1 candidate; then measure the same body with the pad line deleted from source
- result: the stripped source literally reads `volatile` / `u32 *p;`; the pad-deleted body measures 32 (73 insns) vs 20 (74 insns) for the stripped-pad body; a plain non-volatile pad also measures 32
- verdict: CONFIRMED

## [s2] `volatile` is not the codegen agent in the pad family: a plain unused u32 pre_pad[8] reserves the same 32 frame bytes
- mechanism: assign_stack_local reserves the slot from the DECL at RTL-expand regardless of the qualifier
- probe: tmp/frame_probe.sh func_800480C0 text1b on both spellings
- result: both give `.frame $sp,88 # vars= 32`; the non-volatile form is the forbidden dead-vars-local-array family, so this is not an escape from the pad ban
- verdict: CONFIRMED

## [s2] Honest phantom-slot structural producers reach vars=8 on this body, one quarter of the 32 bytes the target reserves
- mechanism: phantom-slot-frame-lever producers 1-3 (folded loop-guard compare, combine orphan-USE, live named locals on multi-read fields); one unallocated pseudo takes an 8-byte-aligned alter_reg slot and nothing further strands
- probe: 13 spellings through tmp/frame_probe.sh - guard-variable (func_8003D9A0 shape), rotated-while (func_8003DBE4 shape), 4 stacked independent guards, per-stack-arg guards, nested guards, hoisted inner temporaries, HImode narrow accumulators, named pointer intermediates, unsigned count, added register pressure, guard on the offset chain, guard on the inner loop exit, sx_ locals hoisted to function scope
- result: every spelling measured vars= 0 or vars= 8; none reached 16 or 32 (tmp/grind/func_800480C0/s2/frame_ladder.txt). The vars=8 slot is zero-cost (no sw/lw in 0x18-0x1F, tmp/grind/func_800480C0/s2/v9.s) but the best spelling carrying it scores 36, worse than the vars=0 body's 32
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c8e81f83, pad-free bodies (only FAKE construct present is the banked `arg0 = 0;` dead store), 13 spellings, cc1 `# vars=` gradient plus sandbox --disable all

## [s2] The callee-saved seat rotation and the frame size are one coupled residual, not two
- mechanism: get_frame_size feeds reload/alter_reg and the allocno ordering global.c walks, so adding the 32 vars bytes re-seats the callee-saved bindings
- probe: objdump diff of the honest build against asm/funcs/func_800480C0.s, compared with the s1 pad-carrying build (full build SHA1 == oracle)
- result: the honest build differs only in sp offsets plus base_addr in $s6 (target $s2) and sx_arg2..5 in $s5/$s4/$s3/$s2 (target $s6/$s5/$s4/$s3); the pad build has BOTH correct. Seat-only and frame-only attacks are therefore mis-decompositions
- verdict: CONFIRMED

## Frontier (for the next session)
1. RA-solver classification of the seat rotation: `tools/ra_solver/inverse_compose.py classify`
   on the honest build's allocno ordering, asking which C-level lever moves base_addr from
   $s6 to $s2. If the solver returns REACHABLE for the seat, re-measure whether the frame
   follows (the coupling observed in s2 runs frame -> seats; the inverse is untested).
2. sched_solver / emission-order check on the two `addu ...,$zero` base copies at insns 3 and 5,
   which is where the seat divergence first appears in the stream.
3. The pad remains banned (Judge 2026-09-02 04:28, docs/grind/decisions.md:20349). The
   bytes-proven body is preserved at rejected/pad-judge-banned-2026-09-02.c should the owner
   ever grant the engine/volatile_cheats.py allowlist row.

## [s2] The 20 recorded as func_800480C0's honest floor is produced by the sandbox stripper leaving a dangling `volatile` qualifier that binds to the following `u32 *p;` declaration, not by any body a session wrote; the honest pad-free floor of this body is 32.
- mechanism: engine/cheats.py removes only the declarator text of `volatile u32 pre_pad[8];`, so the orphaned `volatile` attaches to the next declaration and the scored TU compiles a volatile-pointer body (74 insns) instead of the plain body (73 insns).
- probe: Read tmp/sandbox/func_800480C0/src/text1b.c:160-161 after `sandbox func_800480C0 --disable all` on the s1 candidate; then re-measure the identical body with the pad line deleted from source, and again with a plain non-volatile pad.
- result: The stripped source literally reads `volatile` on one line and `u32 *p;` on the next. Pad present: score 20, 74 build insns. Pad deleted: score 32, 73 insns. Plain non-volatile pad: score 32, 73 insns (stripped cleanly, no dangling qualifier). The 20 was therefore never a measurement of a written body.
- verdict: CONFIRMED

## [s2] `volatile` is not the codegen agent in the pad family: a plain non-volatile unused `u32 pre_pad[8]` reserves the same 32 frame bytes as the volatile spelling.
- mechanism: GCC 2.7.2 function.c assign_stack_local reserves the aggregate's slot from the DECL at RTL-expand regardless of the type qualifier; get_frame_size counts it either way.
- probe: tmp/frame_probe.sh func_800480C0 text1b (cpp | cc1 -mel, reading cc1's own `# vars=`) on both spellings.
- result: Both compile to `.frame $sp,88 # vars= 32, regs= 8/0, args= 24`. This closes the obvious 'respell it without volatile' route: the plain form is the forbidden dead-vars-local-array family, so it is not an escape from the Judge's 2026-09-02 pad ban.
- verdict: CONFIRMED

## [s2] Honest phantom-slot structural producers reach vars=8 on this body across 13 measured spellings, one quarter of the 32 vars bytes the target reserves, and the best spelling carrying the 8 bytes scores 36 against the vars=0 body's 32.
- mechanism: phantom-slot-frame-lever producers 1-3 (folded loop-guard compare, combine orphan-USE, live named locals on multi-read fields): one unallocated pseudo takes an 8-byte-aligned alter_reg slot in reload and nothing further strands.
- probe: 13 spellings through the cc1 `# vars=` gradient: guard-variable (func_8003D9A0 shape), rotated-while (func_8003DBE4 shape), 4 stacked independent guards, per-stack-arg guards, nested guards, hoisted inner temporaries, HImode narrow accumulators, named pointer intermediates, unsigned count, added register pressure, guard on the offset chain, guard on the inner loop exit, sx_ locals hoisted to function scope; plus sandbox --disable all on the two extremes.
- result: Every spelling measured vars= 0 or vars= 8; none reached 16 or 32 (tmp/grind/func_800480C0/s2/frame_ladder.txt). The vars=8 slot is genuinely zero-cost - tmp/grind/func_800480C0/s2/v9.s shows no sw/lw anywhere in 0x18-0x1F, saves start at 0x20 - so the lever is real but saturates well short of the target. Its sandbox score is 36 vs 32 for the vars=0 baseline.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c8e81f83, pad-free bodies, only FAKE construct present is the banked `arg0 = 0;` dead store; 13 spellings measured with the cc1 `# vars=` gradient plus sandbox --disable all

## [s2] The callee-saved seat rotation and the frame size are one coupled residual on this body rather than two independent ones.
- mechanism: get_frame_size feeds reload/alter_reg and the allocno ordering global.c walks, so adding the 32 vars bytes re-seats the callee-saved bindings.
- probe: objdump of the honest build (tmp/grind/func_800480C0/s2/v0_dis.txt) diffed against asm/funcs/func_800480C0.s, compared with the s1 pad-carrying build whose full-build SHA1 == oracle.
- result: The honest build matches the target instruction-for-instruction except (a) every sp-relative offset (frame 0x38 vs 0x58) and (b) the callee-saved bindings: base_addr sits in $s6 where the target uses $s2, and sx_arg2..5 sit in $s5/$s4/$s3/$s2 where the target uses $s6/$s5/$s4/$s3. The pad-carrying build has both correct. So seat-only and frame-only attacks are mis-decompositions, and s1's reading of the gap as 'sp-offset deltas only' was an artifact of the accidental volatile-pointer body, which happens to seat correctly.
- verdict: CONFIRMED
