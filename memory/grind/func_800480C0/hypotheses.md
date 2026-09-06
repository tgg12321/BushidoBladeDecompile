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

## s3 — structural (2026-09-02)

## [s3] The 32 recorded by s2 as func_800480C0's honest pad-free floor is produced by the sandbox deleting an UN-ANNOTATED `arg0 = 0;` dead param store; the same body with the annotation the dead-store family already requires measures 20.
- mechanism: engine/volatile_cheats.py::find_dead_param_assigns strips un-annotated dead assignments to parameters out of the scored TU; an annotated one is bypassed. The contract is pinned by engine/test_engine.py:1094-1116 ("annotated dead param assign bypassed" / "un-annotated dead param assign still flagged").
- probe: take s2's best form (tmp/grind/func_800480C0/s2/body_v0.c) unchanged, add only `/* FAKE: ..., mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */` to the `arg0 = 0;` line, and re-run `sandbox func_800480C0 --disable all`.
- result: bare store -> score 32, build_insns 73. Annotated store -> score 20, build_insns 74. Nothing else changed. The ledger floor is corrected to 20.
- verdict: CONFIRMED

## [s3] The callee-saved seat rotation s2 recorded (base_addr in $s6 against the target's $s2) exists only in the stripped sandbox build and is the downstream effect of the deleted dead store, not an RA divergence.
- mechanism: with `arg0 = 0;` removed, cse2 canonicalises $a0 and base_addr into one value, which changes the allocno set global.c walks; with the store present the allocnos and their seats are the target's.
- probe: run the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1) with BB2_ALLOC_DEBUG=1 over the honest body and read both the emitted asm and the allocation order (tmp/grind/func_800480C0/s3/{probe.s,alloc.err}); compare against the sandbox object's objdump.
- result: cc1 emits `move $18,$16` ($s2 = $s0) and seats sx_arg2..5 in $s6/$s5/$s4/$s3 — the target's exact bindings. BB2_ALLOC_DEBUG: pseudo 83 (base_addr, nrefs=4, livelen=52) is the third allocno and takes hardreg 18 = $s2. The sandbox object, built from the stripped source, is the one that shows `move $s6,$s0` and `addu $s0,$a0,$v0`. The residual at 20 is purely the frame size: the instruction stream is byte-identical and the 20 differing insns are exactly the 20 sp-relative operands. s2 frontier items 1 (ra_solver seat classify) and 2 (sched_solver base-copy order) are closed — there is no seat divergence to classify.
- verdict: CONFIRMED

## [s3] Phantom-slot production on this body is one ST_REGS-classed compare residue worth a 4-byte alter_reg slot (vars=8); eight further structural spellings each produced exactly zero or one such unallocated pseudo, never two
- mechanism: reload1.c alter_reg gives a stack slot to every allocno left with reg_renumber < 0. The only allocno that global.c fails to seat here is the folded loop-guard's compare residue, whose preferred class is ST_REGS (tmp/grind/func_800480C0/dumps/text1b.lreg: "Register 92 used 2 times across 2 insns in block 0; dies in 0 places; ST_REGS or none"), so find_reg cannot place it in GR_REGS; its 4-byte slot rounds get_frame_size up to vars=8.
- probe: new instrument tmp/grind/func_800480C0/s3/probe_body.sh — installs a body, runs the instrumented cc1 with BB2_ALLOC_DEBUG=1, prints the .frame line AND the count of allocnos with hardreg=-1. Eight new spellings measured: for-loop explicit index, guard + explicit remainder counter, guard + pointer guard, guard with function-scope sign-extends, dual independent folded guards (count and base_addr), named loop-tail intermediate, break-form infinite loop, chained guard-of-guard.
- result: every spelling reported unalloc_pseudos = 0 or 1 and .frame vars = 0 or 8; none reached 2 phantoms or 16 bytes. With s2's 13 spellings that is 21 measured structural forms against the 32 vars bytes the target reserves.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f3ba5f78, pad-free bodies, only FAKE construct present is the annotated `arg0 = 0;` dead param store; 8 new spellings via the instrumented-cc1 BB2_ALLOC_DEBUG phantom census plus sandbox --disable all on the baseline

## [s3] The (args=56, vars=0) frame decomposition is layout-equivalent to (args=24, vars=32) on this body but is not reachable from any call this function makes
- mechanism: only args+vars = 0x38 is observable in the shipped bytes (the register saves start at 0x38), so a 32-byte-wider outgoing-args partition would place every sp-relative operand exactly where the target has it. current_function_outgoing_args_size is MIPS_STACK_ALIGN of the widest call's argument area.
- probe: read the `args=` term of the cc1 .frame line across all 21 measured spellings; enumerate the call sites in the body.
- result: args reads 24 in every spelling (MIPS_STACK_ALIGN(20) for the single 5-word call to func_800482C8). Reaching 56 requires a 14-word call, i.e. fabricating a wider or never-executed call site — the "fabricated dead call site (reconstructed compiled-out call site)" family REFUSED by owner ruling 2026-08-17, .claude/rules/no-new-park-categories.md:357-366, which states it "may not be re-proposed in any spelling". s2 frontier item 3 is closed.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f3ba5f78, all 21 s2+s3 spellings, annotated dead-store present in the s3 set

## Frontier (for the next session)
1. The residual is now known to be a pure +32 `vars` delta with an already byte-identical
   instruction stream, and the honest producer caps at a single 4-byte ST_REGS compare
   residue. The untested question is SLOT WIDTH rather than slot count: reload1.c alter_reg
   sizes the slot `MAX (inherent_size, reg_max_ref_width[i])`, so a phantom allocno in a
   wider mode (DImode) or one referenced through a wider subreg would take 8+ bytes.
   Probe: force the folded loop-guard's compare residue into DImode (`long long` guard) and
   read `unalloc_pseudos` + `.frame vars=` with tmp/grind/func_800480C0/s3/probe_body.sh.
   Note the DImode-chain family is banned for SCHEDULING coercion; a width question is not
   automatically that family, but check before spelling it.
2. All three s2 frontier items are closed (see the s3 hypotheses): there is no seat
   rotation to classify with ra_solver, no base-copy emission-order divergence for
   sched_solver, and the args-side widening is a REFUSED family. Do not respend them.
3. The pad remains the only known producer of the exact 32 bytes and remains banned here
   (Judge 2026-09-02 04:28, docs/grind/decisions.md:20349). The bytes-proven body is at
   rejected/pad-judge-banned-2026-09-02.c; a fresh owner ruling adding the
   engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS row integrates it unchanged.
   With the s3 correction the handoff packet's numbers should be read as floor 20 (not 20
   "because the pad is stripped") — the pad-free annotated body is genuinely 20.

## [s3] The 32 recorded by s2 as func_800480C0's honest pad-free floor is produced by the sandbox deleting an UN-ANNOTATED `arg0 = 0;` dead param store; the same body carrying the annotation the dead-store family already requires measures 20.
- mechanism: engine/volatile_cheats.py::find_dead_param_assigns strips un-annotated dead assignments to parameters out of the scored TU while bypassing annotated ones; the contract is pinned by engine/test_engine.py:1094-1116.
- probe: Take s2's best form (tmp/grind/func_800480C0/s2/body_v0.c) unchanged, add only a /* FAKE: ..., mechanism: ..., lever-exhaustion: ... */ comment to the `arg0 = 0;` line, re-run sandbox func_800480C0 --disable all.
- result: Bare store: score 32, build_insns 73. Annotated store: score 20, build_insns 74, target_insns 74. Nothing else changed. Ledger floor corrected to 20.
- verdict: CONFIRMED

## [s3] The callee-saved seat rotation s2 recorded (base_addr in $s6 against the target's $s2) exists only in the stripped sandbox build and is the downstream effect of the deleted dead store, not an RA divergence.
- mechanism: With `arg0 = 0;` removed, cse2 canonicalises $a0 and base_addr into one value and changes the allocno set global.c walks; with the store present the allocnos and their seats are the target's.
- probe: Run the instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_ALLOC_DEBUG=1 over the honest body; read the emitted asm and the allocation order (tmp/grind/func_800480C0/s3/probe.s, alloc.err); compare with objdump of the sandbox object.
- result: cc1 emits `move $18,$16` ($s2 = $s0) and seats sx_arg2..5 in $s6/$s5/$s4/$s3 - the target's exact bindings. BB2_ALLOC_DEBUG: pseudo 83 (base_addr, nrefs=4, livelen=52) is the third allocno and takes hardreg 18 = $s2. Only the sandbox object, built from the stripped source, shows the rotation. The residual at 20 is purely frame size: the instruction stream is byte-identical and the 20 differing insns are exactly the 20 sp-relative operands (1 addiu $sp, 8 sw, 2 incoming stack-arg lw, 8 lw, 1 addiu $sp). s2 frontier items 1 and 2 are closed - there is no seat divergence to classify.
- verdict: CONFIRMED

## [s3] Phantom-slot production on this body is one ST_REGS-classed compare residue worth a 4-byte alter_reg slot (vars=8), and eight further structural spellings each produced exactly zero or one such unallocated pseudo, never two.
- mechanism: reload1.c alter_reg gives a stack slot to every allocno left with reg_renumber < 0. The only allocno global.c fails to seat here is the folded loop-guard's compare residue, whose preferred class is ST_REGS (tmp/grind/func_800480C0/dumps/text1b.lreg: 'Register 92 used 2 times across 2 insns in block 0; dies in 0 places; ST_REGS or none'), so find_reg cannot place it in GR_REGS; its 4-byte slot rounds get_frame_size up to vars=8.
- probe: New instrument tmp/grind/func_800480C0/s3/probe_body.sh installs a body, runs the instrumented cc1 with BB2_ALLOC_DEBUG=1 and prints the .frame line plus the count of allocnos with hardreg=-1. Eight new spellings: for-loop explicit index, guard + explicit remainder counter, guard + pointer guard, guard with function-scope sign-extends, dual independent folded guards, named loop-tail intermediate, break-form infinite loop, chained guard-of-guard.
- result: Every spelling reported unalloc_pseudos = 0 or 1 and .frame vars = 0 or 8; none reached 2 phantoms or 16 bytes. With s2's 13 spellings that is 21 measured structural forms against the 32 vars bytes the target reserves. Representative rejects banked at memory/grind/func_800480C0/rejected/phantom-multiplicity-capped-at-one-vars8.c and dual-fold-guard-still-one-phantom.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f3ba5f78, pad-free bodies, only FAKE construct present is the annotated `arg0 = 0;` dead param store; 8 new spellings via the instrumented-cc1 BB2_ALLOC_DEBUG phantom census plus sandbox --disable all on the baseline

## [s3] The (args=56, vars=0) frame decomposition is layout-equivalent to (args=24, vars=32) on this body but is not reachable from any call this function makes.
- mechanism: Only args+vars = 0x38 is observable in the shipped bytes (register saves start at 0x38), so a 32-byte-wider outgoing-args partition would place every sp-relative operand exactly where the target has it. current_function_outgoing_args_size is MIPS_STACK_ALIGN of the widest call's argument area.
- probe: Read the `args=` term of the cc1 .frame line across all 21 measured spellings; enumerate the call sites in the body.
- result: args reads 24 in every spelling (MIPS_STACK_ALIGN(20) for the single 5-word call to func_800482C8). Reaching 56 needs a 14-word call, i.e. fabricating a wider or never-executed call site - the 'fabricated dead call site' family REFUSED by owner ruling 2026-08-17 (.claude/rules/no-new-park-categories.md:357-366), which states it may not be re-proposed in any spelling. s2 frontier item 3 is closed.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f3ba5f78, all 21 s2+s3 spellings, annotated dead-store present in the s3 set

## s4 — permuter (2026-09-02)

Chassis check at dispatch: `sandbox func_800480C0 --disable all` on the s3 candidate
re-measured **20** this session (74/74 insns), so the s3 floor correction holds and every
s4 measurement below is on that chassis.

## [s4] Widening the phantom slot by giving the folded loop-guard a DImode (`long long`) carrier does not enlarge `vars`; it removes the phantom entirely
- mechanism: reload1.c alter_reg sizes a spill slot MAX (inherent_size, reg_max_ref_width[i]), so a DImode allocno left unallocated would take 8 bytes instead of the 4-byte ST_REGS compare residue the s3 census found. The premise fails upstream: with an `s64` guard the compare is expanded as a DImode comparison whose residue is allocated, so global.c leaves nothing unseated.
- probe: four `s64` spellings built from the banked vars=8 form (rejected/phantom-guard-vars8-ceiling.c) — single DImode guard, two DImode guards, DImode guard + s32 guard, DImode guard kept live through the loop by an xor accumulation — each through tmp/grind/func_800480C0/s4/probe_body.sh (instrumented cc1, BB2_ALLOC_DEBUG=1, `.frame` + count of allocnos with hardreg=-1).
- result: all four report `.frame $sp,56 # vars= 0` and `unalloc_pseudos=0`, i.e. WORSE than the s32 guard's vars=8. The instrument was sanity-checked in the same batch: the banked s3 forms still report vars=8 / unalloc_pseudos=1 (phantom-guard-vars8-ceiling.c, dual-fold-guard-still-one-phantom.c). Banked at rejected/dimode-guard-erases-phantom.c. s3 frontier item 1 is closed.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8395e8f4, pad-free bodies, dead param store `arg0 = 0;` present (bare in the probe bodies — cc1 sees it either way; only the sandbox stripper cares about the annotation), instrumented cc1 tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG census

## [s4] Placing folded guards in DISTINCT basic blocks (including inside the loop body) does not raise phantom multiplicity above one
- mechanism: reload1.c alter_reg gives each unallocated allocno its own assign_stack_local slot, so two guards whose live ranges never overlap should strand two pseudos and reserve 16 bytes.
- probe: six further spellings through the same instrument — guard inside the loop body, two guards in two distinct pre-loop blocks, three guards in three distinct blocks, two guards folded against DIFFERENT constants (-1 and -7), pointer-typed guard beside the scalar guard, 64-bit multiply residue guard.
- result: every spelling reported unalloc_pseudos = 0 or 1 and `vars` = 0 or 8; none reached 2 phantoms or 16 bytes. One spelling (guard inside the loop) reached `.frame $sp,72` but by taking a NINTH callee-saved register (regs= 9/0), which moves the register-save block and therefore every sp offset the wrong way. With s2's 13 and s3's 8, that is 27 measured structural forms against the target's 32 `vars` bytes. Banked at rejected/multi-bb-guards-still-one-phantom.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8395e8f4, pad-free bodies, bare `arg0 = 0;` dead param store present, instrumented-cc1 BB2_ALLOC_DEBUG census (tmp/grind/func_800480C0/s4/body_[EFGHIJ]*.c)

## [s4] Two permuter campaigns (~50k iterations, two structurally distinct chassis) produce no form below the 20-objdump-diff baseline and never exceed vars=8
- mechanism: random C mutation over the candidate body; campaigns launched with the default --stack-diffs so the permuter's scorer does NOT normalise the sp-relative offsets away (without it this function false-matches at 0).
- probe: tmp/perm_480C0_s4a (candidate chassis, base_score 586, 39.4k iters, 6 finds) and tmp/perm_480C0_s4b (vars=8 guard chassis, base_score 857, 10.6k iters, 39 finds), both `-j` parallel, harvested with --stop. Each campaign's best eight finds were recompiled and objdump-diffed against target.o (tmp/grind/func_800480C0/s4/{s4a_finds.txt,s4b_finds.txt}).
- result: campaign A best 442 — 74 insns, 20 objdump diffs, i.e. EQUAL to the baseline residual, frame 64. Campaign B best 583 — 25 objdump diffs, frame 72. Across both, the largest frame any find reached is 72 and every 72-byte frame carries MORE differing insns (23-29) than the 20 of the vars=0 baseline; no find reached vars=16 and none scored below 20. Best find banked at rejected/permuter-s4a-best-442-frame64.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8395e8f4, candidate chassis (annotated dead param store present) and vars=8 guard chassis, decomp-permuter with --stack-diffs, tmp/perm_480C0_s4a + tmp/perm_480C0_s4b

## [s4] No frame term other than `var_size` can supply the target's +32 bytes on a mips1 -mno-abicalls build, so every remaining candidate form must produce 32 bytes of get_frame_size() locals
- mechanism: GCC 2.7.2 mips.c compute_frame_size: `extra_size = MIPS_STACK_ALIGN (((TARGET_ABICALLS) ? UNITS_PER_WORD : 0))` is 0 for -mno-abicalls (mips.c:4464); `args_size = MIPS_STACK_ALIGN (current_function_outgoing_args_size)` is pinned at 24 by the single 5-word call (mips.c:4466, and the widening route is the REFUSED fabricated-dead-call-site family); `current_function_pretend_args_size` is added to total_size ONLY under `ABI_64BIT && mips_isa >= 3` (mips.c:4530-4531), which this build is not; and the register-save offset is computed as `args_size + extra_size + var_size + gp_reg_size - UNITS_PER_WORD` (mips.c:4548-4550), so growing the save area instead of `vars` moves the saves to the wrong offsets (measured: the s4 in-loop-guard spelling reaches frame 72 via a 9th callee-saved register and its sp offsets diverge further, not less).
- probe: read mips.c:4448-4575 (the only producer of the `.frame` numbers) and cross-check against the `.frame` term readings of all 27 measured spellings.
- result: total_size = var_size + args_size + gp_reg_rounded exactly on this configuration. The residual is definitionally a get_frame_size() question: 32 bytes of stack locals, whose only measured producers are (a) the banned unwritten pad aggregate and (b) alter_reg phantom slots, which cap at 8 bytes here.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 8395e8f4, all 27 s2/s3/s4 spellings plus the compiler source predicate
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4464

## Frontier (for the next session)
1. PRECEDENT CENSUS (cheap, no compile): does any COMPLETED-C function already on main
   reproduce an allocated-but-never-touched `vars` region from ordinary C — i.e. without a
   pad aggregate and without a spill? Grep the matched bodies for `.frame` vars regions with
   zero sw/lw traffic. A hit names a producer nobody here has spelled; a clean negative is
   itself the strongest evidence for the pad-allowlist route.
2. FORENSICS (unchanged from s3 item 2): the three text1b siblings carrying the identical
   32-byte untouched region (func_80047EE8 / func_80047FBC / func_800481E8). Look for a call,
   struct or buffer shape in their neighbourhood that implies the same 8-word local in the
   original TU — that would make an ordinary-C LIVE local the producer instead of a pad.
3. The pad remains the only known producer of exactly 32 bytes and is banned here only by the
   closed per-function enumeration (Judge 2026-09-02 04:28, docs/grind/decisions.md:20349);
   the bytes-proven body is preserved at rejected/pad-judge-banned-2026-09-02.c and integrates
   unchanged if an owner ruling adds the engine/volatile_cheats.py row. Sibling func_80047EE8
   ships exactly this construct on main (src/text1b.c:20-36).

## [s4] Widening the phantom slot by giving the folded loop-guard a DImode (long long) carrier enlarges vars beyond 8 on this body
- mechanism: reload1.c alter_reg sizes a spill slot MAX (inherent_size, reg_max_ref_width[i]), so a DImode allocno left unallocated would take 8 bytes instead of the 4-byte ST_REGS compare residue the s3 census found
- probe: four s64 spellings built from rejected/phantom-guard-vars8-ceiling.c (single DImode guard, two DImode guards, DImode + s32 guard, DImode guard kept live through the loop) through tmp/grind/func_800480C0/s4/probe_body.sh (instrumented cc1, BB2_ALLOC_DEBUG=1, .frame + hardreg=-1 count)
- result: all four report .frame $sp,56 # vars= 0 and unalloc_pseudos=0 - the DImode compare residue is allocated, so the phantom disappears instead of widening. Same-batch sanity check: the banked s3 forms still report vars=8 / unalloc_pseudos=1. s3 frontier item 1 closed. Banked at rejected/dimode-guard-erases-phantom.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8395e8f4, pad-free bodies, bare `arg0 = 0;` dead param store present (cc1 sees it with or without the FAKE comment), instrumented cc1 tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG

## [s4] Placing folded guards in distinct basic blocks, including inside the loop body, raises phantom multiplicity above one and reserves 16 bytes
- mechanism: reload1.c alter_reg gives each unallocated allocno its own assign_stack_local slot, so two guards with disjoint live ranges should strand two pseudos
- probe: six spellings through the same instrument: guard inside the loop body, two guards in two distinct pre-loop blocks, three guards in three blocks, two guards folded against different constants (-1 / -7), pointer-typed guard beside the scalar guard, 64-bit multiply residue guard (tmp/grind/func_800480C0/s4/body_[EFGHIJ]*.c)
- result: every spelling reported unalloc_pseudos = 0 or 1 and vars = 0 or 8; none reached 2 phantoms or 16 bytes. The one spelling reaching .frame $sp,72 did so by taking a NINTH callee-saved register (regs= 9/0), which moves the register-save block and every sp offset the wrong way. Running total with s2 (13) and s3 (8): 27 measured structural forms. Banked at rejected/multi-bb-guards-still-one-phantom.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8395e8f4, pad-free bodies, bare `arg0 = 0;` dead param store present, instrumented-cc1 BB2_ALLOC_DEBUG census

## [s4] A permuter campaign on this function finds a form scoring below the 20-objdump-diff baseline
- mechanism: random C mutation over the candidate body; campaigns launched with the default --stack-diffs so the scorer does not normalise away the sp-relative offsets that ARE the residual here
- probe: tmp/perm_480C0_s4a (candidate chassis, base_score 586, 39.4k iterations, 6 finds) and tmp/perm_480C0_s4b (vars=8 guard chassis, base_score 857, 10.6k iterations, 39 finds); each campaign best-eight finds recompiled and objdump-diffed against target.o (s4a_finds.txt / s4b_finds.txt); both harvested with --stop, status reports 0 live campaigns
- result: campaign A best 442 = 74 insns / 20 objdump diffs / frame 64 - EQUAL to the baseline, not better. Campaign B best 583 = 25 diffs / frame 72. Largest frame reached across both is 72 and every frame-72 form carries 23-29 differing insns against the baseline 20; no find reached vars=16. Best find banked at rejected/permuter-s4a-best-442-frame64.c
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8395e8f4, candidate chassis (annotated dead param store present) and the vars=8 guard chassis, decomp-permuter with --stack-diffs, ~50k total iterations

## [s4] A frame term other than var_size (extra_size, pretend_args_size, or a larger register-save area) can supply the target's +32 bytes on this mips1 -mno-abicalls build
- mechanism: GCC 2.7.2 mips.c compute_frame_size is the sole producer of the .frame numbers: extra_size = MIPS_STACK_ALIGN((TARGET_ABICALLS ? UNITS_PER_WORD : 0)) is 0 without abicalls (mips.c:4464); args_size is MIPS_STACK_ALIGN(current_function_outgoing_args_size), pinned at 24 by the single 5-word call (mips.c:4466); current_function_pretend_args_size is added to total_size only under ABI_64BIT && mips_isa >= 3 (mips.c:4530-4531); the register saves sit at args_size + extra_size + var_size + gp_reg_size - 4 (mips.c:4548-4550)
- probe: read mips.c:4448-4575 and cross-check the args=/vars=/regs= terms of all 27 measured spellings, including the frame-72 spelling that grows the save area instead of vars
- result: total_size = var_size + args_size + gp_reg_rounded exactly on this configuration; growing the save area (regs= 9/0, frame 72) moves every sp offset further from the target rather than closer. The residual is definitionally a get_frame_size() question - 32 bytes of stack LOCALS - whose only measured producers are the banned unwritten pad aggregate and alter_reg phantom slots (capped at 8 bytes here)
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 8395e8f4, all 27 s2/s3/s4 spellings plus the compiler source predicate
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4464

## s5 - synthesis (2026-09-02)

Chassis check at dispatch: HEAD carries `INCLUDE_ASM` and reads 74 / `no_c_body`;
with `memory/grind/func_800480C0/candidate.c` installed, `sandbox --disable all`
re-measured **20** (74/74) this session. All s5 measurements are on that chassis.

## [s5] One unallocated pseudo reserves 4 bytes of `vars` (s3's model), so the target's 32 untouched bytes need eight phantom slots
- mechanism: s3 read the single ST_REGS compare residue as a 4-byte alter_reg slot that `assign_stack_local` rounds the frame up to 8 for. If that were right, the 32-byte target region would be eight 4-byte residues.
- probe: tree-wide correlation of cc1's own `# vars=` term against the instrumented cc1's `BB2_ALLOC_DEBUG` hardreg=-1 count, per function, over all 32 `src/*.c` TUs (tmp/grind/func_800480C0/s5/census.sh -> s5/asm/*.s, s5/alloc_tu.sh -> s5/*.alloc.err, correlated by s5/census3.py and the per-function greps).
- result: the relation is exactly `untouched vars = 8 * unallocated pseudos`, with no rounding term: func_80042874 6 phantoms / 48 bytes, func_80042A88 6 / 48, func_80041E10 3 / 24, get_cs / get_ce / func_8003FECC / func_80038170 / func_80040594 / SsSeqCalledTbyT / _SsSeqPlay / SpuSetCommonAttr 2 / 16, ~30 further functions 1 / 8. s3's 4-byte unit is corrected. The target needs FOUR phantoms, not eight, and the ledger's 27-form ceiling of one phantom covers 8 of the 32 bytes rather than a quarter of a 4-byte-unit budget.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, all 32 src/*.c TUs compiled with the project cc1 (build/cc1) and the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1); no FAKE construct involved (the census reads shipped COMPLETED-C bodies as they stand on main)

## [s5] Some COMPLETED-C function already on main reproduces an allocated-but-never-touched `vars` region from ordinary C, without a pad aggregate and without a spill (s4 frontier item 1)
- mechanism: get_frame_size() counts every assign_stack_local slot, and reload1.c alter_reg pays off every allocno left with reg_renumber < 0; a residue that never materialises a load or store still costs frame bytes. The project has ~1200 matched bodies to census.
- probe: tmp/grind/func_800480C0/s5/census.sh compiles all 32 `src/*.c` to `s5/asm/*.s`; census2.py parses every `.frame $sp,N # vars=, regs=, args=` line, computes the vars window `[args, args+vars)`, and reports functions with zero `N($sp)` traffic in that window AND zero uses of `$sp` as a source operand (the second filter is load-bearing: without it the census false-positives on 21 address-taken-local functions such as func_8006C168 vars=88).
- result: CONFIRMED and it is a HIT, not the clean negative the frontier anticipated. 184 functions have vars>0; 80 have an untouched vars window; 59 of those never take a frame address; only 5 of the 59 carry a sanctioned pad row. Fifty-four ordinary-C COMPLETED-C bodies on main reserve untouched frame bytes, and EIGHT of them reach 16 bytes (two phantoms): get_cs, get_ce, func_8003FECC, func_80038170, func_80040594, SsSeqCalledTbyT, _SsSeqPlay, SpuSetCommonAttr. The producer is the same ST_REGS-classed compare residue this function already makes one of - display.lreg for get_cs names both: `Register 85 used 2 times across 2 insns in block 1; ST_REGS or none` and `Register 99 ... block 6`, from the two ordinary clamp ternaries at src/display.c:556. The pad is NOT the only producer of untouched vars; what this function lacks is multiplicity, not mechanism.
- verdict: CONFIRMED

## [s5] The DImode HILO scratch of `mult`/`div` is a second, independent 8-byte phantom producer usable on func_800480C0
- mechanism: on mips1 the `mulsi3`/`divsi3` patterns carry a DImode HILO scratch; when global.c leaves it unallocated, alter_reg gives it an 8-byte stack slot that no instruction touches. func_80041E10 (src/text1a_post.c:465) is a five-statement leaf whose three `/255` magic-number divisions emit three `mult`s and reserve `vars=24` with zero traffic; func_80042874/func_80042A88 reach 6 phantoms / 48 bytes the same way.
- probe: grep asm/funcs/func_800480C0.s for mult/multu/div/divu; cross-check the mult/div count against untouched vars for all 59 census candidates (s5/census3.py prints the `muldiv` and `8*md` columns).
- result: the producer is real and confirmed elsewhere, but `asm/funcs/func_800480C0.s` contains ZERO mult/multu/div/divu instructions. Since this function's honest build is already byte-identical to the target in its 74-instruction stream, any spelling that introduces a multiply or divide materialises instructions the target does not have. The producer is inapplicable to this body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, target listing asm/funcs/func_800480C0.s and the s5 tree census; no FAKE construct involved

## [s5] Folding a guard onto BOTH of this body's conditional branches (entry test and do-while backedge) raises phantom multiplicity to two
- mechanism: reload1.c alter_reg pays off each unallocated allocno separately, and get_cs demonstrates two ST_REGS residues surviving in two different basic blocks; func_800480C0 has exactly two conditional branches, so one residue per branch would give vars=16.
- probe: three new spellings built from candidate.c and measured with tmp/grind/func_800480C0/s3/probe_body.sh (instrumented cc1, BB2_ALLOC_DEBUG=1, `.frame` + hardreg=-1 count): P = a folded guard on the entry test plus a separate folded guard variable driving the do-while backedge; Q = the same with an unsigned entry guard against a signed backedge guard; R = a get_cs-style `(u32)g < K` range test on the entry plus the backedge guard (tmp/grind/func_800480C0/s5/bodies/).
- result: P and Q both report `.frame $sp,72 # vars= 8, regs= 9/0, args= 24` and `unalloc_pseudos=1`; R reports `vars= 0`, `unalloc_pseudos=0` - the range compare gets allocated and erases the residue. All three additionally take a NINTH callee-saved register, which moves the register-save block and therefore every sp offset further from the target rather than closer. Running total 30 measured structural forms on this body, none above one phantom. Banked at rejected/two-branch-guards-still-one-phantom.c and rejected/range-outer-guard-erases-phantom.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, pad-free bodies, bare `arg0 = 0;` dead param store present (cc1 sees it with or without the FAKE comment), instrumented cc1 tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG census

## [s5] Among functions in this tree containing no mult/div, some reach more than two phantoms
- mechanism: if the folded-compare residue can stack to four on a mult-free body anywhere in the tree, the same shape can in principle be spelled onto func_800480C0.
- probe: s5/census3.py restricted to the 45 census candidates whose bodies contain no mult/multu/div/divu; read the distinct untouched-vars values, then check the sanctioned-pad functions' phantom counts with BB2_ALLOC_DEBUG.
- result: the untouched-vars values over that subset are exactly {8, 16, 32}, and every 32 belongs to one of the five sanctioned-pad functions (func_80041688, func_80047EE8, func_80047FBC, func_800481E8 at 32; func_80049A2C at 8). BB2_ALLOC_DEBUG reports ZERO unallocated pseudos for func_80047EE8 and func_80047FBC - their 32 bytes are the declared `volatile u32 pre_pad[8]` array, not phantoms. So the maximum folded-compare multiplicity observed anywhere in this tree on a mult-free body is TWO, and four has no in-tree precedent.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, all 32 src/*.c TUs as they stand on main, project cc1 + instrumented cc1 BB2_ALLOC_DEBUG

## Frontier (for the next session)
1. IDENTIFY THE RTL SHAPE THAT ORPHANS A SECOND RESIDUE. get_cs is the cheapest live
   exhibit of multiplicity 2 (`src/display.c:556`, dumps already generated at
   tmp/grind/func_800480C0/s5/dumps/display.{combine,lreg,greg}). Probe: read the
   `.combine` and `.lreg` entries for get_cs blocks 1 and 6, identify precisely which
   insn pair leaves Registers 85 and 99 ref'd-but-unallocated, then ask whether that
   exact shape can be spelled onto func_800480C0's two conditional branches. Thirty
   guard-shaped spellings have failed by analogy; the dump names the shape in one read.
   This is the modality the pass-attribution rule exists for.
2. FOUR PHANTOMS OR ONE AGGREGATE. The s5 census bounds the folded-compare producer at
   two on a mult-free body tree-wide, and this body's target has no mult/div. If
   frontier 1 yields at most two, the residual is 16 bytes short and the remaining
   honest hypothesis is that the original declared a 32-byte AGGREGATE (a PsyQ MATRIX
   is exactly 32 bytes) whose uses this decompilation folded away - i.e. the forensics
   item below is the live one, not a spelling question.
3. FORENSICS (carried from s3/s4, still unspent): the three text1b siblings with the
   identical 32-byte untouched region (func_80047EE8 / func_80047FBC / func_800481E8)
   all ship the pad, and BB2_ALLOC_DEBUG confirms they have zero phantoms - so all four
   siblings' 32 bytes are one shared source construct, not four coincidental residues.
   Look at func_800482C8 and the callers for a 32-byte record (MATRIX / 8-word buffer)
   that the original built on the stack and passed by address, whose address-taking this
   decompilation folded into pointer arithmetic. A LIVE such local is ordinary C.

## [s5] One unallocated pseudo reserves 4 bytes of vars (s3's model), so the target's 32 untouched bytes need eight phantom slots
- mechanism: s3 read the single ST_REGS-classed compare residue as a 4-byte alter_reg slot that assign_stack_local rounds the frame up to 8 for; on that model the 32-byte region is eight 4-byte residues
- probe: tree-wide correlation of cc1's own `# vars=` term against the instrumented cc1's BB2_ALLOC_DEBUG hardreg=-1 count, per function, over all 32 src/*.c TUs (tmp/grind/func_800480C0/s5/census.sh -> s5/asm/*.s; s5/alloc_tu.sh -> s5/*.alloc.err; correlated by s5/census3.py)
- result: The relation is exactly vars = 8 * unallocated_pseudos with no rounding term: func_80042874 6 phantoms/48 bytes, func_80042A88 6/48, func_80041E10 3/24, get_cs, get_ce, func_8003FECC, func_80038170, func_80040594, SsSeqCalledTbyT, _SsSeqPlay and SpuSetCommonAttr 2/16, and ~30 further functions 1/8. s3's 4-byte unit is corrected: the target needs FOUR phantoms, and the ledger's 30-spelling ceiling of one phantom supplies 8 of the 32 bytes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, all 32 src/*.c TUs compiled with the project cc1 (tools/gcc-2.7.2/build/cc1) and the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1); no FAKE construct involved - the census reads shipped COMPLETED-C bodies as they stand on main

## [s5] Some COMPLETED-C function already on main reproduces an allocated-but-never-touched vars region from ordinary C, without a pad aggregate and without a spill (s4 frontier item 1)
- mechanism: get_frame_size() counts every assign_stack_local slot and reload1.c alter_reg pays off every allocno left with reg_renumber < 0, so a residue that never materialises a load or store still costs frame bytes
- probe: s5/census.sh compiles all 32 src/*.c with the project cc1; s5/census2.py parses each `.frame $sp,N # vars=, regs=, args=` line, computes the vars window [args, args+vars), and reports functions with zero N($sp) traffic in that window AND zero uses of $sp as a source operand (the second filter removes address-taken locals; without it the census false-positives on 21 functions such as func_8006C168 vars=88)
- result: CONFIRMED, and it is a HIT rather than the clean negative the frontier anticipated. 184 functions have vars>0; 80 have an untouched vars window; 59 of those never take a frame address; only 5 of the 59 carry a sanctioned pad row. Eight ordinary-C bodies reach 16 untouched bytes (two phantoms): get_cs, get_ce, func_8003FECC, func_80038170, func_80040594, SsSeqCalledTbyT, _SsSeqPlay, SpuSetCommonAttr. The producer is the same ST_REGS-classed compare residue this function already makes one of - tmp/grind/func_800480C0/s5/dumps/display.lreg names both of get_cs's: 'Register 85 used 2 times across 2 insns in block 1; ST_REGS or none' and 'Register 99 ... block 6', from the two ordinary clamp ternaries at src/display.c:556. The pad is not the only producer of untouched vars; this body lacks multiplicity, not mechanism.
- verdict: CONFIRMED

## [s5] The DImode HILO scratch of mult/div is a second, independent 8-byte phantom producer usable on func_800480C0
- mechanism: the mips1 mulsi3/divsi3 patterns carry a DImode HILO scratch; left unallocated by global.c, alter_reg gives it an 8-byte stack slot no instruction touches. func_80041E10 (src/text1a_post.c:465) is a five-statement leaf whose three /255 magic-number divisions emit three mults and reserve vars=24 with zero traffic; func_80042874/func_80042A88 reach 6 phantoms / 48 bytes the same way
- probe: grep asm/funcs/func_800480C0.s for mult/multu/div/divu, and cross-check the mult/div count against untouched vars for all 59 census candidates (s5/census3.py prints the muldiv and 8*md columns)
- result: The producer is real and confirmed elsewhere in the tree, but asm/funcs/func_800480C0.s contains ZERO mult/multu/div/divu. This function's honest build is already byte-identical to the target across its 74-instruction stream, so any spelling introducing a multiply or divide materialises instructions the target does not have. Inapplicable to this body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, target listing asm/funcs/func_800480C0.s plus the s5 tree census; no FAKE construct involved

## [s5] Folding a guard onto BOTH of this body's conditional branches (the entry test and the do-while backedge) raises phantom multiplicity to two
- mechanism: reload1.c alter_reg pays off each unallocated allocno separately, and get_cs demonstrates two ST_REGS residues surviving in two different basic blocks; func_800480C0 has exactly two conditional branches, so one residue per branch would give vars=16
- probe: three new spellings built from candidate.c and measured with tmp/grind/func_800480C0/s3/probe_body.sh (instrumented cc1, BB2_ALLOC_DEBUG=1, .frame plus hardreg=-1 count): P = folded guard on the entry test plus a separate folded guard variable driving the do-while backedge; Q = the same with an unsigned entry guard against a signed backedge guard; R = a get_cs-style (u32)g < K range test on the entry plus the backedge guard (tmp/grind/func_800480C0/s5/bodies/)
- result: P and Q both report `.frame $sp,72 # vars= 8, regs= 9/0, args= 24` with unalloc_pseudos=1; R reports vars=0 / unalloc_pseudos=0 because the range compare gets allocated and the residue disappears. All three additionally take a NINTH callee-saved register, which moves the register-save block and therefore every sp offset further from the target. Running total 30 measured structural forms on this body, none above one phantom. Banked at rejected/two-branch-guards-still-one-phantom.c and rejected/range-outer-guard-erases-phantom.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, pad-free bodies, bare `arg0 = 0;` dead param store present (cc1 sees it with or without the FAKE comment), instrumented cc1 tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG census

## [s5] Among functions in this tree containing no mult/div, some reach more than two phantoms
- mechanism: if the folded-compare residue stacks to four on a mult-free body anywhere in the tree, the same shape could in principle be spelled onto func_800480C0
- probe: s5/census3.py restricted to the 45 census candidates whose bodies contain no mult/multu/div/divu; read the distinct untouched-vars values, then check the sanctioned-pad functions' phantom counts with BB2_ALLOC_DEBUG
- result: Over that subset untouched vars takes exactly the values {8, 16, 32}, and every 32 belongs to a sanctioned-pad function (func_80041688, func_80047EE8, func_80047FBC, func_800481E8). BB2_ALLOC_DEBUG reports ZERO unallocated pseudos for func_80047EE8 and func_80047FBC, so their 32 bytes are the declared volatile u32 pre_pad[8] array rather than phantoms. The maximum folded-compare multiplicity observed on a mult-free body anywhere in this tree is TWO.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 118e147d, all 32 src/*.c TUs as they stand on main, project cc1 plus instrumented cc1 BB2_ALLOC_DEBUG

## s6 - synthesis (2026-09-02, chassis HEAD ba593529)

## [s6] The phantom slot is an orphan `(insn (use (reg P)))` planted by combine.c's distribute_notes, and the orphan-USE count in a .combine dump predicts a function's phantom count exactly
- mechanism: combine rewrites or deletes the insn that DEFINED intermediate pseudo P; P's REG_DEAD note then has no home, and the backward scan at tools/gcc-2.7.2/combine.c:10757-10762 walks prev_nonnote_insn only while the predecessor is an INSN/CALL_INSN, so it stops at the block's leading JUMP_INSN or CODE_LABEL. With place == 0 && tem != 0 combine emits `(use P)` after that jump/label (combine.c:10832-10841). P now has no set and no constraint-bearing reference, so regclass records nothing and .lreg prints the default `ST_REGS or none`; find_reg cannot seat it, alter_reg pays a stack slot, assign_stack_local bumps frame_offset by 8, and the `use` emits zero instructions.
- probe: read display.flow / display.combine for get_cs (insns 22/23/24 -> insn 143 `(use (reg:SI 85))` planted after jump_insn 18); then tmp/grind/func_800480C0/s6/count_uses.py over six TUs' .combine dumps, correlated against s5's BB2_ALLOC_DEBUG hardreg=-1 counts.
- result: exact 1:1 agreement wherever the phantoms are combine residues - SetDrawEnv 3/3, SetDrawEnv2 3/3, get_cs 2/2, get_ce 2/2, func_80040594 2/2, func_80038170 2/2, SpuSetCommonAttr 2/2, SsSeqCalledTbyT 2/2, six functions at 1/1. The only disagreements are the mult/div DImode-HILO producer, alone (func_80042874 0 USEs / 6 phantoms) or mixed (func_8003FECC, SpuSetReverbModeParam, _SsSeqPlay: 1 USE / 2 phantoms). Two independent producer classes are therefore established: class A = combine orphan USE, class B = everything else.
- verdict: CONFIRMED

## [s6] s5's tree-wide bound - that among functions containing no mult/div none reserves more than two phantom slots - holds
- mechanism: s5 restricted census3.py to the 45 mult-free entries of its untouched-vars-window census and read the distinct untouched-vars values {8, 16, 32}, attributing every 32 to a declared pad array. If that bound were real, four folded-compare residues would have no in-tree precedent and the phantom route would be worth abandoning for the aggregate-forensics route.
- probe: s6's orphan-USE census over six TUs, cross-checked against the raw `.frame` lines in tmp/grind/func_800480C0/s5/asm/display.s rather than against the filtered census.
- result: REFUTED by counterexample. SetDrawEnv (src/display.c:360) and SetDrawEnv2 (src/display.c:436) contain no mult/multu/div/divu, are ordinary C, are COMPLETED-C on main, and each carry THREE orphan USEs and THREE phantoms; display.s:1243 reads `.frame $sp,64,$31 # vars= 32, regs= 3/0, args= 16` = 24 phantom bytes plus 8 bytes for the live `u16 buf[4]`. s5's bound was an artifact of its own census FILTER, which admits only functions with zero N($sp) traffic in the vars window and therefore drops every function that also owns a live stack local. Three phantoms from ordinary C on a mult-free body has an in-tree precedent; the ceiling is not two. The three SetDrawEnv orphans are pseudos 128/137/140, all from the `s16 loc; ... if (SHORT_GLOBAL - 1 < loc) loc2 = SHORT_GLOBAL - 1;` clamp idiom (src/display.c:392-400) that also produces get_cs's two.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ba593529, six -da TU dumps (display, text1a_c, text1a_pre, config, code6cac_c_mid, main) from the project cc1, correlated with s5's BB2_ALLOC_DEBUG captures; no FAKE construct involved (the measurement is on other functions' shipped bodies)

## [s6] The single phantom the banked vars=8 forms produce is the same class-A combine orphan the multiplicity-2 and multiplicity-3 exhibits use
- mechanism: if this body's known phantom were class A, the 30-form ceiling of s2-s5 would be a ceiling on the same producer that reaches 3 in SetDrawEnv, and raising multiplicity would be a matter of finding more sites. If it is class B, every spelling tried so far has been attacking a producer that has no multiplicity-3 exhibit anywhere in this tree.
- probe: re-measure rejected/phantom-guard-vars8-ceiling.c and rejected/two-branch-guards-still-one-phantom.c on HEAD ba593529 with tmp/grind/func_800480C0/s6/probe.sh, which reports frame, BB2_ALLOC_DEBUG unalloc count AND the new orphan-USE count in one line.
- result: phantom-guard-vars8-ceiling.c = `vars= 8, regs= 8/0, unalloc=1, orphanUSE=0`; two-branch-guards-still-one-phantom.c = `vars= 8, regs= 9/0, unalloc=1, orphanUSE=0`. Both s4/s5 instance kills reproduce unchanged on the current chassis, and both phantoms are class B. Class A has NEVER fired on this body in any of the 34 measured forms. This reframes the entire s2-s5 ceiling: it bounds class B, not the producer that reaches 3 in this tree.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ba593529, the two banked score-36/frame-64 guard bodies as stored in memory/grind/func_800480C0/rejected/, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump; the annotated `arg0 = 0;` dead param store present in both

## [s6] A class-A orphan can be induced on this body by re-spelling the four param sign-extends or by leaving a foldable intermediate at the loop-head block boundary
- mechanism: class A needs an intermediate pseudo whose defining insn combine rewrites or deletes and which is not referenced again before the head of its basic block. This body has two non-entry blocks whose heads are candidates - the fallthrough block after `beqz $s1` (which holds all four `sll/sra` param sign-extends) and the do-while body at .L80048144 - so a spelling that routes a foldable intermediate through either head should plant a USE.
- probe: four bodies measured with tmp/grind/func_800480C0/s6/probe.sh - B_s32params.c (arg2..arg5 declared s32 with explicit `(s16)` casts, the shape the target's `lw 0x68($sp)` + `sll/sra` suggests), C_hi_intermediates.c (four block-scope s16 intermediates feeding sx_arg2..5), D_fnscope_hi.c (the same four hoisted to function scope), G_splitshift_loop.c (`(((u32)word >> 1) >> 1) << 2` at the loop head).
- result: all four measure `vars= 0, unalloc=0, orphanUSE=0`. The sign-extend chains never present combine with a deletable intermediate because mips1 has no register sign-extend pattern, so `ashift`/`ashiftrt` both survive as real insns and their intermediate is genuinely live. The split shift never creates two insns at all: GCC folds constant-shift-of-constant-shift during RTL generation, long before combine.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ba593529, the four s6 bodies in tmp/grind/func_800480C0/s6/bodies/, annotated `arg0 = 0;` dead param store present in all four, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump

## Frontier (for the next session)
1. EXTRACT THE THREE SetDrawEnv ORPHAN SITES AND TEST THEM VERBATIM. SetDrawEnv is the
   tree's only mult-free multiplicity-3 exhibit and it is ordinary C on main. Probe:
   generate `-da` dumps for display (tmp/grind/func_800480C0/s6/dumpall.sh already does
   this), locate pseudos 128, 137 and 140 in display.flow and display.combine, and write
   down for each one the exact three-insn chain combine collapsed and which jump/label
   the USE was planted after. Then ask, per site, whether the same chain can be routed
   through func_800480C0's two block heads WITHOUT adding instructions. This is the
   dump-named version of the question s2-s5 answered thirty times by analogy - and note
   that the class A / class B distinction means none of those thirty spellings actually
   tested it.
2. INSTRUMENT THE PLANT SITE INSTEAD OF GUESSING SOURCE SHAPES. tools/gcc-2.7.2/cc1 is
   already the instrumented compiler (BB2_*_DEBUG hooks). Add a hook at
   combine.c:10836-10841 that logs, for every planted USE, the function, the pseudo, the
   insn UID it was planted after, and the pattern of the insn combine had just rewritten;
   run it over all 32 TUs. That converts "which C shapes orphan an intermediate at a
   block head" from analogy into an exhaustive catalogue, and it is reusable by every
   future frame-residual grind. (Editing tools/ is outside a grind session's surface -
   the catalogue can instead be built read-only by diffing each function's .flow and
   .combine dumps for pseudos that lose their set, which the s6 instruments already do
   most of.)
3. FOUR PHANTOMS OR ONE AGGREGATE (carried from s5, still unspent and now better posed).
   With the bound of two refuted, 3 is demonstrated and 4 is no longer obviously out of
   reach, so the aggregate-forensics item is no longer forced - but it is still the only
   hypothesis that explains why all FOUR text1b siblings reserve exactly 32 bytes.
   Probe: func_800482C8 and the callers of the four siblings, looking for a 32-byte
   record (PsyQ MATRIX, 8-word buffer) that the original built on the stack and passed by
   address, whose address-taking this decompilation folded into pointer arithmetic. A
   LIVE such local is ordinary C and needs no allowlist row.

## [s6] The phantom slot is an orphan `(insn (use (reg P)))` planted by combine.c's distribute_notes, and the orphan-USE count in a .combine dump predicts a function's phantom count exactly
- mechanism: combine rewrites or deletes the insn that DEFINED intermediate pseudo P; P's REG_DEAD note then has no home, and the backward scan at tools/gcc-2.7.2/combine.c:10757-10762 walks prev_nonnote_insn only while the predecessor is an INSN/CALL_INSN, so it stops at the block's leading JUMP_INSN or CODE_LABEL. With place == 0 && tem != 0 combine emits `(use P)` after that jump/label (combine.c:10832-10841). P now has no set and no constraint-bearing reference, so regclass records nothing and .lreg prints the default 'ST_REGS or none'; find_reg cannot seat it, alter_reg pays a stack slot, assign_stack_local bumps frame_offset by 8, and the `use` emits zero instructions.
- probe: Read display.flow / display.combine for get_cs (insns 22/23/24 collapse; insn 143 `(use (reg:SI 85))` planted after jump_insn 18), then ran tmp/grind/func_800480C0/s6/count_uses.py over six TUs' .combine dumps and correlated with s5's BB2_ALLOC_DEBUG hardreg=-1 counts.
- result: Exact 1:1 agreement wherever the phantoms are combine residues: SetDrawEnv 3/3, SetDrawEnv2 3/3, get_cs 2/2, get_ce 2/2, func_80040594 2/2, func_80038170 2/2, SpuSetCommonAttr 2/2, SsSeqCalledTbyT 2/2, and six functions at 1/1. The only disagreements are the mult/div DImode-HILO producer, alone (func_80042874 and func_80042A88: 0 USEs, 6 phantoms) or mixed (func_8003FECC, SpuSetReverbModeParam, _SsSeqPlay: 1 USE, 2 phantoms). Two independent producer classes are now established: class A = combine orphan USE, class B = everything else.
- verdict: CONFIRMED

## [s6] s5's tree-wide bound - that among functions containing no mult/div none reserves more than two phantom slots - holds
- mechanism: s5 restricted census3.py to the 45 mult-free entries of its untouched-vars-window census, read the distinct untouched-vars values {8, 16, 32}, and attributed every 32 to a declared pad array. If that bound were real, four folded-compare residues would have no in-tree precedent and the phantom route would be worth abandoning for the aggregate-forensics route.
- probe: s6's orphan-USE census over six TUs, cross-checked against the raw `.frame` lines in tmp/grind/func_800480C0/s5/asm/display.s rather than against the filtered census output.
- result: Refuted by counterexample. SetDrawEnv (src/display.c:360) and SetDrawEnv2 (src/display.c:436) contain no mult/multu/div/divu, are ordinary C, are COMPLETED-C on main, and each carry three orphan USEs and three phantoms; display.s:1243 reads `.frame $sp,64,$31 # vars= 32, regs= 3/0, args= 16` = 24 phantom bytes plus 8 bytes for the live `u16 buf[4]`. s5's bound was an artifact of its own census FILTER, which admits only functions with zero N($sp) traffic in the vars window and therefore drops every function that also owns a live stack local. The three SetDrawEnv orphans are pseudos 128/137/140, all from the `s16 loc; ... if (SHORT_GLOBAL - 1 < loc) loc2 = SHORT_GLOBAL - 1;` clamp idiom at src/display.c:392-400 that also produces get_cs's two. Three phantoms from ordinary C on a mult-free body has an in-tree precedent.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ba593529, six -da TU dumps (display, text1a_c, text1a_pre, config, code6cac_c_mid, main) from the project cc1, correlated with s5's BB2_ALLOC_DEBUG captures; no FAKE construct involved (the measurement is on other functions' shipped bodies)

## [s6] The single phantom the banked vars=8 forms produce is the same class-A combine orphan that the multiplicity-2 and multiplicity-3 exhibits use
- mechanism: If this body's known phantom were class A, the 30-form ceiling of s2-s5 would bound the same producer that reaches 3 in SetDrawEnv and raising multiplicity would be a matter of finding more sites. If it is class B, every spelling tried so far has been attacking a producer with no multiplicity-3 exhibit anywhere in this tree.
- probe: Re-measured rejected/phantom-guard-vars8-ceiling.c and rejected/two-branch-guards-still-one-phantom.c on the current chassis with tmp/grind/func_800480C0/s6/probe.sh, which reports frame, BB2_ALLOC_DEBUG unalloc count and the new orphan-USE count in one line.
- result: phantom-guard-vars8-ceiling.c = vars= 8, regs= 8/0, unalloc=1, orphanUSE=0; two-branch-guards-still-one-phantom.c = vars= 8, regs= 9/0, unalloc=1, orphanUSE=0. Both s4/s5 instance kills reproduce unchanged on HEAD ba593529, and both phantoms are class B. Class A has not fired on this body in any of the 34 measured forms. This reframes the whole s2-s5 ceiling: it bounds class B, not the producer that reaches 3 in this tree.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ba593529, the two banked score-36/frame-64 guard bodies as stored in memory/grind/func_800480C0/rejected/, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump; the annotated `arg0 = 0;` dead param store present in both

## [s6] A class-A orphan can be induced on this body by re-spelling the four param sign-extends or by leaving a foldable intermediate at the loop-head block boundary
- mechanism: Class A needs an intermediate pseudo whose defining insn combine rewrites or deletes and which is not referenced again before the head of its basic block. This body has two non-entry blocks whose heads are candidates: the fallthrough block after `beqz $s1` (which holds all four sll/sra param sign-extends) and the do-while body at .L80048144.
- probe: Four bodies measured with tmp/grind/func_800480C0/s6/probe.sh: B_s32params.c (arg2..arg5 declared s32 with explicit (s16) casts, the shape the target's `lw 0x68($sp)` + sll/sra suggests), C_hi_intermediates.c (four block-scope s16 intermediates feeding sx_arg2..5), D_fnscope_hi.c (the same four hoisted to function scope), G_splitshift_loop.c (`(((u32)word >> 1) >> 1) << 2` at the loop head).
- result: All four measure vars= 0, unalloc=0, orphanUSE=0. The sign-extend chains never present combine with a deletable intermediate because mips1 has no register sign-extend pattern, so ashift and ashiftrt both survive as real insns and the intermediate between them is genuinely live. The split shift never creates two insns at all: GCC folds constant-shift-of-constant-shift during RTL generation, long before combine runs. Bodies banked in memory/grind/func_800480C0/rejected/s6-*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ba593529, the four s6 bodies in tmp/grind/func_800480C0/s6/bodies/, annotated `arg0 = 0;` dead param store present in all four, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump

## s7 - solver (2026-09-02, chassis HEAD 7e18adc2, floor re-measured 20 / 74-of-74 insns)

## [s7] The 20-diff residual is reachable by one of the RA or scheduler solver models
- mechanism: solver modality's mandated first step. tools/ra_solver models global.c / local-alloc / reload and tools/sched_solver models both scheduler passes; inverse_compose.py classify triages a residual PRE-RA / RA / SCHED / IDENTICAL so a session does not search the wrong layer.
- probe: `inverse_compose.py classify text1b func_800480C0` refuses on an INCLUDE_ASM-represented function (it would report a fictitious PRE-RA verdict from the missing .tgt.s), so the object-level classifier was used instead: `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_800480C0` run under WSL with memory/grind/func_800480C0/candidate.c installed in src/text1b.c and tmp/sandbox/func_800480C0/text1b.o freshly produced by `sandbox --disable all` (score 20).
- result: `FIRST DIVERGENCE: PRE-RA / next tool: none - the residual is upstream of every model`. Ours 74 insns, target 74 insns, and the entire shape delta is the 24 sp-relative operands (`addiu #,#,-56` vs `-88`, eight `sw` and three `lw` shifted by 32, the two incoming-stack-arg `lw` at 104/108 vs 44/48). Neither solver has any purchase: there is no seat to re-assign and no emission order to permute, because the register disposition and the instruction order already match exactly. The residual is decided at RTL-expand / combine time via get_frame_size, upstream of global.c, reload and both scheduler passes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, memory/grind/func_800480C0/candidate.c installed in src/text1b.c (sandbox --disable all = 20, 74/74 insns), annotated `arg0 = 0;` dead param store present; object-level classify against build/src/text1b.o

## [s7] A combine class-A orphan USE is planted at most once per plant site, so this body's control-flow shape caps class A at two
- mechanism: distribute_notes plants the orphan `(use (reg P))` after the insn the backward scan stops at (combine.c:10757-10762 terminating on a JUMP_INSN/CODE_LABEL, plant at combine.c:10836-10841). If exactly one note could strand per stopping point, the 74-insn target CFG - entry block, the fallthrough block after `beqz $s1`, the do-while body at .L80048144, the epilogue at .L800481BC - would offer at most two or three usable plant sites and could never reach the four phantoms the 32-byte residual needs.
- probe: tmp/grind/func_800480C0/s7/plantsites.py and s7/plantkind.py over every .combine dump in tmp/grind/func_800480C0/s5/dumps and s6/dumps (six TUs: display, text1a_pre, code6cac_c_mid, config, main, plus the s6 scratch t.combine). They extract every `(insn UID PREV NEXT (use (reg:M N)))`, resolve PREV to its insn kind, and group by function and plant site.
- result: REFUTED by counterexample. 31 orphan USEs across 21 functions. 30 of the 31 sit immediately after a JUMP_INSN or CODE_LABEL, one per site - but func_80040594 (src/text1a_pre.c) carries TWO at the SAME site: insn 565 `(use (reg:SI 198))` is planted after `code_label 457 ("done_cases")`, and insn 564 `(use (reg:SI 187))` is then planted after insn 565, chained onto the first orphan at the same block head. Orphans therefore STACK at one plant site, and the number of block heads is not the ceiling on class-A multiplicity. The census also fixes the plant-site kind exactly: JUMP_INSN or CODE_LABEL only (the single apparent `planted-after insn 565` entry is the chained case, not an ordinary insn).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, the six -da .combine dumps banked under tmp/grind/func_800480C0/s5/dumps and s6/dumps; measurement is over other functions' shipped COMPLETED-C bodies, no FAKE construct involved

## [s7] The class-A producer that stacks is the HImode sign-extend fold, so re-spelling func_800480C0's four narrow loads as signed loads will orphan four HImode intermediates
- mechanism: s6 named the producer but not the RTL shape. checkRECT (text1b sibling file display.c) and SetDrawEnv both show `(set (reg:HI P) (mem:HI ...))` + `ashift` + `ashiftrt` collapsed by combine into one `extendhisi2_internal` (`lh`), deleting P's def and stranding its REG_DEAD note. func_800480C0's loop performs exactly four 16-bit loads that are then sign-extended, so four such folds would give four orphans - an exact fit for the 32-byte residual.
- probe: tmp/grind/func_800480C0/s7/bodies/A_s16ptr_loads.c - the four loop loads respelled `a1v = *((s16 *)p);` instead of `a1v = (s16)(*((u16 *)p));` - measured with tmp/grind/func_800480C0/s6/probe.sh (instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump). Cross-checked against the target listing.
- result: `vars= 0, regs= 8/0, unalloc=0, orphanUSE=0`. Two independent reasons, both measured. (a) A `*(s16 *)p` deref expands straight to `(set (reg:SI) (sign_extend:SI (mem:HI)))` - one insn, no HImode intermediate for combine to delete - so the fold never happens. (b) More decisively, the TARGET does not contain the fold at all: asm/funcs/func_800480C0.s at .L80048144 emits `lhu` + `sll 16` + `sra 16` for each of the four loads, never `lh`. The target's own narrow loads are the UNSIGNED-load-then-shift shape candidate.c already spells, so the HImode sign-extend producer cannot fire on this body without changing the 74-instruction stream. The rule's own boundary predicts this: .claude/rules/phantom-slot-frame-lever.md:47-53 requires the widened `reg:HI` to have a second use AS AN HIMODE VALUE, and this algorithm consumes all four values as s32 addends.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, tmp/grind/func_800480C0/s7/bodies/A_s16ptr_loads.c, annotated `arg0 = 0;` dead param store present, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump; banked at rejected/s7-s16ptr-direct-loads-fold-clean.c

## [s7] Single-use pointer copy temps at the loop head present combine with deletable defs and orphan one pseudo per temp
- mechanism: the func_80040594 double-orphan is a `(set (reg T) (plus A B))` + `(set X (mem (reg T)))` pair that combine merges into `(set X (mem (plus A B)))`, deleting T's def; T's death note then finds no home and strands at the block head. If a plain single-use pointer temp were enough, four temps in the loop body would give four orphans at .L80048144.
- probe: tmp/grind/func_800480C0/s7/bodies/C_ptrtemps4.c (`{ u16 *q1 = (u16 *)p; a1v = (s16)(*q1); }` for each of the four narrow loads) and D_ptrtemps5.c (the same plus `{ u32 *q0 = p; word = *q0; }` for the word load), both via s6/probe.sh.
- result: both measure `vars= 0, regs= 8/0, unalloc=0, orphanUSE=0`. A plain register COPY is propagated away by cse/cse2 long before combine runs, so combine never rewrites a death note for it and no note strands. The func_80040594 shape is strictly narrower than "single-use temp": the deleted def must be a real ARITHMETIC insn (there, `index*4 + base`, dump insns 480/482/488 in text1a_pre.flow) whose result is dereferenced exactly once and dies at that deref. func_800480C0 has exactly one site of that shape - the entry-block `p = (u32 *)(base_addr + (((*p) >> 2) << 2)); count = *(p++);` - and its address pseudo does NOT die at the deref, because `p` is carried live into the loop. Producing a second, dying copy of that address would be a semantically redundant recomputation (extra insns, no observable effect), i.e. the forbidden no-semantic-purpose class rather than a lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, tmp/grind/func_800480C0/s7/bodies/{C_ptrtemps4.c,D_ptrtemps5.c}, annotated `arg0 = 0;` dead param store present, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump; banked at rejected/s7-single-use-ptr-copy-temps-no-orphan.c and rejected/s7-five-ptr-copy-temps-no-orphan.c

## Frontier (for the next session)
1. THE ARITHMETIC-ADDRESS ORPHAN IS THE ONLY CLASS-A SHAPE MEASURED TO STACK, AND IT
   NEEDS A DYING ADDRESS PSEUDO. s7 pinned the reproducible shape: `(set T (plus A B))`
   + `(set X (mem T))`, combine merges to `(mem (plus A B))`, T's def is deleted, the
   note strands, and TWO can chain at one block head (func_80040594, text1a_pre.combine
   insns 564/565 after code_label 457). Probe: enumerate every mem reference in
   func_800480C0's target stream whose address is a register that could legitimately be
   a dying single-use sum - the entry `count = *(p++)` after
   `p = base_addr + ((*p>>2)<<2)` is the only candidate - and ask whether the ALGORITHM
   (not a fabricated copy) can be spelled so that address dies at the deref while `p`'s
   subsequent advance comes from a different live value. If it cannot, class A is capped
   at 0-1 here and the 32-byte residual is not a phantom problem at all.
2. FOUR PHANTOMS OR ONE AGGREGATE - now the leading hypothesis, not the fallback. s7
   removed the two producer shapes that could plausibly have scaled to four on this body
   (HImode sign-extend fold: absent from the target's own `lhu`+`sll`+`sra` stream;
   pointer copy temps: propagated before combine). All four text1b siblings reserve
   exactly 32 bytes, three of them via a sanctioned `volatile u32 pre_pad[8]` row whose
   BB2_ALLOC_DEBUG phantom count is ZERO. Probe: forensics on func_800482C8 and on the
   callers of the four siblings for a 32-byte record (PsyQ MATRIX, 8-word buffer) the
   original built on the stack and passed by address, whose address-taking this
   decompilation folded into pointer arithmetic. A LIVE such local is ordinary C and
   needs no allowlist row. This is a FORENSICS modality item, not a spelling item.
3. DO NOT SPEND ANOTHER SESSION ON RA OR SCHEDULER MODELS. `goal_from_tgt.py classify`
   returns PRE-RA with "next tool: none - the residual is upstream of every model", and
   the two streams agree on all 74 instructions and on every register. Any future solver
   work on this function measures a layer the divergence does not live in. Note also
   that `inverse_compose.py classify` self-refuses on INCLUDE_ASM-represented functions;
   `goal_from_tgt.py classify` under WSL is the working entry point.

## [s7] The 20-diff residual on func_800480C0 is reachable by one of the RA or scheduler solver models
- mechanism: tools/ra_solver models global.c / local-alloc / reload and tools/sched_solver models both scheduler passes; inverse_compose.py classify triages a residual PRE-RA / RA / SCHED / IDENTICAL so a session does not search the wrong layer. This is the solver modality's mandated first step.
- probe: inverse_compose.py classify text1b func_800480C0 self-refuses on an INCLUDE_ASM-represented function (it would report a fictitious PRE-RA verdict from the missing text1b.tgt.s), so the object-level classifier was used: `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_800480C0` under WSL, with memory/grind/func_800480C0/candidate.c installed in src/text1b.c and tmp/sandbox/func_800480C0/text1b.o freshly produced by `sandbox func_800480C0 --disable all` (score 20, 74/74 insns).
- result: The classifier returns `FIRST DIVERGENCE: PRE-RA` with `next tool: none - the residual is upstream of every model`. Ours 74 insns, target 74 insns; the whole shape delta is 24 sp-relative operands (addiu -56 vs -88; eight sw and three lw at offsets shifted by exactly 32; the two incoming stack-argument lw at 44/48 vs the target's 104/108). Every register assignment and every instruction position already agrees, so there is no seat to re-assign and no emission order to permute. The divergence is decided by get_frame_size at RTL-expand/combine time, upstream of global.c, reload and both scheduler passes. Recorded so no future session spends a modality on this layer.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, memory/grind/func_800480C0/candidate.c installed over the INCLUDE_ASM line in src/text1b.c (sandbox --disable all = 20, 74/74 insns), annotated `arg0 = 0;` dead param store present; object-level classify against build/src/text1b.o and tmp/sandbox/func_800480C0/text1b.o

## [s7] A combine class-A orphan USE is planted at most once per plant site, so this body's two usable block heads cap class-A phantom multiplicity at two
- mechanism: distribute_notes plants the orphan `(use (reg P))` after the insn its backward scan stops at (tools/gcc-2.7.2/combine.c:10757-10762 terminating on a JUMP_INSN/CODE_LABEL; plant at combine.c:10836-10841). If exactly one note could strand per stopping point, the target CFG - entry block, the fallthrough block after `beqz $s1`, the do-while body at .L80048144, the epilogue at .L800481BC - could never reach the four phantoms the 32-byte residual needs.
- probe: tmp/grind/func_800480C0/s7/plantsites.py and s7/plantkind.py over every .combine dump banked in tmp/grind/func_800480C0/s5/dumps and s6/dumps (six TUs: display, text1a_pre, code6cac_c_mid, config, main, plus the s6 scratch t.combine). They extract every `(insn UID PREV NEXT (use (reg:M N)))`, resolve PREV to its insn kind, and group by function and plant site.
- result: Refuted by counterexample. 31 orphan USEs across 21 functions; all 31 are planted immediately after a JUMP_INSN or a CODE_LABEL, and 30 sit one per site - but func_80040594 (src/text1a_pre.c) carries TWO at the SAME head: insn 565 `(use (reg:SI 198))` after `code_label 457 ("done_cases")`, and insn 564 `(use (reg:SI 187))` chained after insn 565. Orphans stack at one plant site, so the block-head count is not the ceiling on class-A multiplicity and the natural bound this session set out to establish does not exist. The census does firmly fix the plant-site KIND: JUMP_INSN or CODE_LABEL only.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, the six -da .combine dumps banked under tmp/grind/func_800480C0/s5/dumps and s6/dumps; the measurement reads other functions' shipped COMPLETED-C bodies, no FAKE construct involved

## [s7] Re-spelling func_800480C0's four narrow loop loads as signed s16 loads orphans four HImode intermediates through the combine sign-extend fold
- mechanism: s6 named the class-A producer but not its RTL shape. checkRECT and SetDrawEnv both show `(set (reg:HI P) (mem:HI ...))` + `ashift` + `ashiftrt` collapsed by combine into one `extendhisi2_internal` (`lh`), which deletes P's def and strands its REG_DEAD note. func_800480C0's loop performs exactly four 16-bit loads that are then sign-extended, an exact numerical fit for the four phantoms the 32-byte residual needs.
- probe: tmp/grind/func_800480C0/s7/bodies/A_s16ptr_loads.c - the four loop loads respelled `a1v = *((s16 *)p);` instead of `a1v = (s16)(*((u16 *)p));` - measured with tmp/grind/func_800480C0/s6/probe.sh (instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1, plus a -da combine dump and the s6 orphan-USE counter). Cross-checked against asm/funcs/func_800480C0.s.
- result: `vars= 0, regs= 8/0, unalloc=0, orphanUSE=0`. Two independent reasons, both measured. (a) A `*(s16 *)p` deref expands straight to `(set (reg:SI) (sign_extend:SI (mem:HI)))` - one insn, no HImode intermediate for combine to delete. (b) More decisively, the TARGET does not contain the fold at all: asm/funcs/func_800480C0.s at .L80048144 emits `lhu` + `sll 16` + `sra 16` four times and never `lh`, i.e. the target's own narrow loads are the unsigned-load-then-shift shape candidate.c already spells. Inducing the fold would change the 74-instruction stream. The rule's own boundary predicts this: .claude/rules/phantom-slot-frame-lever.md:47-53 requires the widened `reg:HI` to have a second use AS AN HIMODE VALUE, and this algorithm consumes all four values as s32 addends.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, tmp/grind/func_800480C0/s7/bodies/A_s16ptr_loads.c, annotated `arg0 = 0;` dead param store present, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump; banked at memory/grind/func_800480C0/rejected/s7-s16ptr-direct-loads-fold-clean.c

## [s7] Single-use pointer copy temps at the loop head present combine with deletable defs and orphan one pseudo per temp
- mechanism: func_80040594's double orphan comes from `(set (reg T) (plus A B))` followed by `(set X (mem (reg T)))`, which combine merges into `(set X (mem (plus A B)))`, deleting T's def and stranding its death note at the block head. If a plain single-use pointer temp were sufficient, four temps in the loop body would give four orphans at .L80048144.
- probe: tmp/grind/func_800480C0/s7/bodies/C_ptrtemps4.c (`{ u16 *q1 = (u16 *)p; a1v = (s16)(*q1); }` for each of the four narrow loads) and D_ptrtemps5.c (the same plus `{ u32 *q0 = p; word = *q0; }` for the word load), both measured via tmp/grind/func_800480C0/s6/probe.sh.
- result: Both measure `vars= 0, regs= 8/0, unalloc=0, orphanUSE=0`. A plain register COPY is propagated away by cse/cse2 long before combine runs, so combine never rewrites a death note for it and nothing strands. The func_80040594 shape is strictly narrower than 'single-use temp': the deleted def must be a real ARITHMETIC insn (there `index*4 + base`, text1a_pre.flow insns 480/482/488 and 509/511/513) whose result is dereferenced exactly once and dies at that deref. func_800480C0 owns exactly one site of that shape - the entry-block `p = (u32 *)(base_addr + (((*p) >> 2) << 2)); count = *(p++);` - and its address pseudo does not die at the deref because `p` is carried live into the do-while. Making a second, dying copy of that address would be a semantically redundant recomputation with no observable effect, i.e. the forbidden no-semantic-purpose class rather than a lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7e18adc2, tmp/grind/func_800480C0/s7/bodies/{C_ptrtemps4.c,D_ptrtemps5.c}, annotated `arg0 = 0;` dead param store present, instrumented cc1 BB2_ALLOC_DEBUG plus -da combine dump; banked at memory/grind/func_800480C0/rejected/s7-single-use-ptr-copy-temps-no-orphan.c and rejected/s7-five-ptr-copy-temps-no-orphan.c

## s8 — forensics (2026-09-02, chassis HEAD 28583e8e, candidate.c + its one FAKE dead param store)

### H-s8-1 (KILL RE-AUDIT, mandated) — "the banked kills were measured while a FAKE carrier occupied the residual's pseudo"
- **Mechanism tested:** func_8002EA24-s8 hazard — a `/* FAKE */` construct sitting on the
  register a lever targets makes the lever read inert.
- **Probe:** `tools/fake_ablate.py --func func_800480C0 --file text1b --candidate
  memory/grind/func_800480C0/candidate.c` (full subset grid; one FAKE unit ⇒ 2 variants).
- **Result:** keep-all = 20 / 74 insns; drop-1 = 32 / 73 insns. The dead param store is
  load-bearing and its pseudo ($a0/arg0) is not the residual's carrier — the residual is the
  frame size and the two streams already agree on all 74 instructions.
- **Verdict:** KILLED (the masking hypothesis). kill_scope: instance.

### H-s8-2 — "the 32 untouched bytes are a caller-built 32-byte record passed by address, folded into pointer arithmetic by this decompilation" (s7 frontier item 2, first half)
- **Probe:** every call site of the four siblings, in C and in asm
  (src/text1a_post.c:277-286, src/text1a_pre.c:268-278, src/text1a_c2.c:161,
  asm/funcs/func_80040594.s:125/144/152/172, func_80041988.s:47/61/68,
  func_80045B68.s:149, func_800460E4.s:219), plus the callee func_800482C8 (src/text1b.c:209).
- **Result:** every caller passes scalars; the callee takes a `u8 *` into an image blob and
  owns its own `s16 rect[4]` / `s16 buf[512]`. No caller-provided 32-byte stack area exists
  anywhere in the cluster.
- **Verdict:** KILLED. kill_scope: instance (measured on the shipped call sites of these
  four functions on HEAD 28583e8e).

### H-s8-3 — "the target's 32-byte untouched region is allocation residue (phantom slots), so an ordinary-C spelling that produces four phantoms exists" (the axis every session s2–s7 has attacked)
- **Mechanism:** alter_reg pays 8 bytes of `vars` per unallocated pseudo; four phantoms = 32 bytes.
- **Probe A (frame invariance):** tmp/grind/func_800480C0/s8/framecensus.py over the four
  siblings' shipped listings. All four reserve the SAME 0x18–0x37 untouched window with
  callee-saved base 0x38 and args 0x18, despite having 2/4/6/2 parameters and different loop
  bodies. Phantom counts track expression shape; a shape-invariant 32 bytes does not.
- **Probe B (filter-free tree census):** all 32 TUs recompiled with the instrumented cc1
  (s8/allcensus.sh), 1096 functions, phantom = `hardreg=-1 AND livelen<=2`
  (s8/phantom_census_v2.py; the livelen discriminator separates phantoms from genuine spills —
  v1 mis-read func_80060E38's nine spills as phantoms). Distribution mult-free
  {0:980, 1:30, 2:7, 3:3}; only mult/div bodies exceed 3 (func_80042874 / func_80042A88 at 6).
- **Result:** the highest phantom count measured on any mult-free body in the tree is 3
  (SetDrawEnv, SetDrawEnv2, func_80041AC8). func_800480C0's target stream contains no
  mult/div, so the 6-phantom class-B producer cannot be spelled here byte-neutrally.
- **Verdict:** KILLED. kill_scope: instance — this is a census result over the 1096 functions
  this tree compiles on HEAD 28583e8e, not a proof from a gate predicate. Re-open it if a
  mult-free 4-phantom body is ever compiled anywhere in the tree.

### H-s8-4 — "spelling the power-of-two scales as ordinary unsigned division/multiplication leaves an expand_divmod intermediate for combine to delete, planting phantoms"
- **Probe:** three bodies (both `>>2 <<2` sites as `/4 *4`; plus the `(arg1<<16)>>14` scale as
  `((arg1<<16)>>16) * 4`; loop site alone) through s6/probe.sh + a sandbox score on A.
- **Result:** all three give `vars= 0, regs= 8/0, args= 24, unalloc=0, orphanUSE=0`, and A
  scores 20 / 74 / 74 — codegen-identical to the shift spelling. GCC 2.7.2 folds the constant
  power-of-two divide at expand, so no intermediate insn ever exists to orphan.
- **Verdict:** KILLED. kill_scope: instance (measured on HEAD 28583e8e with the annotated dead
  param store present; bodies at tmp/grind/func_800480C0/s8/bodies/, banked in rejected/).

### H-s8-5 — "class A can be reached four times by reading the four halfwords at constant offsets from one base pointer"
- **Mechanism:** class A is `T = base + CONST; X = *T` with T dying; combine folds CONST into
  the mem displacement, deletes T's def, strands the REG_DEAD note at the block head.
- **Probe:** read the target loop rather than compile — asm/funcs/func_800480C0.s `.L80048144`.
- **Result:** the target emits four `lhu $aN, 0x0($s0)` with a separate `addiu $s0,$s0,0x2`
  between each. Every narrow load is at displacement ZERO off a pointer that stays live across
  the loop; there is no dying constant-offset address anywhere in the target stream. The
  respelling would change the emitted displacements and the addiu count.
- **Verdict:** KILLED. kill_scope: instance (measured against the shipped listing for this
  function; the shape is byte-visible, so no compile was needed).

### s8 frontier
1. The residual is a DECLARED 32-byte local in the shipped source, and the only C form of it
   is the unwritten pad the Judge banned here for want of an engine allowlist row. The four
   siblings' frame invariance (H-s8-3 probe A) plus the tree-wide mult-free ceiling of 3
   phantoms (probe B) plus the caller/callee census (H-s8-2) now close every honest producer
   this ledger has been able to name. What is NOT yet done: a targeted survey of what a
   32-byte PsyQ-era local in a sprite-batch routine would BE (MATRIX, a 8-word DMA/packet
   staging buffer) and whether any of the four siblings' asm shows an addressing pattern that
   would make it LIVE in a variant of the routine elsewhere in the game (e.g. an unsplit
   duplicate at another address). A live 32-byte local is ordinary C and needs no row.
2. func_80041AC8 (src/text1a_post.c) is the one mult-free body in the tree with 3 phantoms AND
   vars=32 that is NOT a display.c clamp idiom. Its producing shape has never been read.
   Reading its .combine orphans (s6/count_uses.py) may name a third phantom producer class,
   which is the only way the mult-free ceiling of 3 could move.
3. Class B multiplicity on this body remains 1 across 41 measured forms; class A is capped at
   ≤1 by the target listing. Neither axis can reach 4 — further spellings on either are
   re-measuring dead ground.

## [s8] The s2-s7 kills on func_800480C0 were measured while the annotated `arg0 = 0;` FAKE dead param store occupied the pseudo those levers targeted, so at least one of them is a masking artifact rather than a real kill.
- mechanism: func_8002EA24-s8 hazard: a /* FAKE */ construct sitting on the register a lever moves makes the lever read inert. tools/fake_ablate.py scores the candidate across every subset of its FAKE units.
- probe: python3 tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c (one FAKE unit found; full 2-variant grid scored).
- result: keep-all = score 20 at 74 build insns; drop-1 (store removed) = score 32 at 73 build insns. The store is strictly load-bearing, and its pseudo ($a0/arg0) is not the residual's carrier - the residual is the frame size and the two streams already agree on all 74 instructions. No banked kill is a masking artifact.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 28583e8e, memory/grind/func_800480C0/candidate.c installed over the INCLUDE_ASM line in src/text1b.c; FAKE state = the single annotated dead param store, ablated in the drop-1 arm

## [s8] The target's 32 untouched vars bytes are a 32-byte record the caller built on the stack and passed by address, whose address-taking this decompilation folded into pointer arithmetic (s7 frontier item 2, first half).
- mechanism: function.c assign_stack_local reserves an aggregate slot at RTL-expand from the source DECL; a caller-owned record passed by address would explain a live 32-byte object invisible in the callee's own stream.
- probe: Read every call site of the four siblings in C and asm (src/text1a_post.c:277-286, src/text1a_pre.c:268-278, src/text1a_c2.c:161, asm/funcs/func_80040594.s:125/144/152/172, func_80041988.s:47/61/68, func_80045B68.s:149, func_800460E4.s:219) and the callee func_800482C8 (src/text1b.c:209).
- result: Every caller passes scalars only - e.g. func_800480C0(sec, 0, 0x80, 0, -0x140, 0xF0). The callee takes a u8* into a TIM/image blob and owns its own s16 rect[4] / s16 buf[512]; it never reads a caller-supplied 32-byte stack area. If the original declared a 32-byte object it was declared LOCAL and never referenced.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 28583e8e, shipped call sites of the four siblings; no candidate installed (read-only forensics)

## [s8] func_800480C0's 32-byte untouched vars region is allocation residue, so an ordinary-C spelling of this body that produces four phantom slots exists.
- mechanism: alter_reg pays 8 bytes of vars per unallocated pseudo (class A = combine.c distribute_notes orphan USE, class B = mult/div DImode HILO scratch); four phantoms would be 32 bytes.
- probe: (a) tmp/grind/func_800480C0/s8/framecensus.py over the four siblings' shipped listings; (b) filter-free tree census - all 32 src/*.c TUs recompiled with the instrumented cc1 (s8/allcensus.sh), 1096 functions, phantom = hardreg=-1 AND livelen<=2 (s8/phantom_census_v2.py; the livelen term separates phantoms from genuine spills, which v1 conflated - func_80060E38's nine hardreg=-1 pseudos are spills at livelen 57-65, not phantoms).
- result: (a) All four siblings reserve the SAME 0x18-0x37 untouched window with args=0x18 and callee-saved base 0x38, differing only in frame total (0x48/0x50/0x58/0x48) as their register counts differ - despite having 2/4/6/2 parameters and structurally different bodies. Phantom residue tracks expression shape; a shape-invariant 32 bytes does not. (b) Mult-free phantom distribution over the tree is {0:980, 1:30, 2:7, 3:3}; the only bodies above 3 are func_80042874/func_80042A88 (both mult/div, 6 phantoms). The target stream for func_800480C0 has no mult/div, so the 6-phantom producer cannot be spelled here byte-neutrally, and the measured mult-free ceiling is 3 - one short of four.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 28583e8e, census over the 1096 functions the 32 src/*.c TUs compile with the instrumented cc1; candidate not installed during the census (HEAD sources), FAKE state irrelevant to the census

## [s8] Spelling the power-of-two scales as ordinary unsigned division/multiplication leaves an expand_divmod intermediate for combine to delete, planting phantom slots on this body.
- mechanism: expand_divmod's constant-power-of-two path could emit a separate scale insn that combine folds into the consumer, deleting its def and stranding a REG_DEAD note at the block head (the class-A shape).
- probe: Three bodies through tmp/grind/func_800480C0/s8/run.sh (s6 probe: instrumented cc1 .frame + BB2_ALLOC_DEBUG unalloc + orphan-USE count on a -da .combine dump): both `((x)>>2)<<2` sites as `/4*4`; the same plus `(arg1<<16)>>14` as `((arg1<<16)>>16)*4`; the loop site alone. Plus a sandbox score on the first.
- result: All three: .frame $sp,56 # vars= 0, regs= 8/0, args= 24, unalloc=0, orphanUSE=0. sandbox --disable all on A_divmul_both prints score 20 / target 74 / build 74 - the division spelling is codegen-IDENTICAL to the shift spelling, because GCC 2.7.2 folds the constant power-of-two divide at expand before combine runs. Running total of measured structural forms on this body: 41.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 28583e8e, bodies at tmp/grind/func_800480C0/s8/bodies/ installed one at a time over the INCLUDE_ASM line, annotated dead param store present in each

## [s8] Class A can be reached four times on this body by reading the four halfwords at constant offsets from one base pointer and advancing the pointer once.
- mechanism: Class A is `T = base + CONST; X = *T` with T dying; combine folds CONST into the mem displacement, deletes T's def, and distribute_notes plants the stranded REG_DEAD as an orphan USE at the block head - insn count unchanged, which is what a zero-byte-delta lever needs.
- probe: Read the shipped loop rather than compile: asm/funcs/func_800480C0.s at .L80048144.
- result: The target emits four `lhu $aN, 0x0($s0)` each followed by a separate `addiu $s0,$s0,0x2`. Every narrow load sits at displacement ZERO off a pointer that is live across the whole loop; there is no dying constant-offset address in the target stream at all. A constant-offset respelling changes the emitted displacements and the addiu count, so it cannot be byte-neutral. With s7's finding that the body owns exactly one arithmetic-address-then-deref site, class A is capped at <=1 here by the shipped listing itself.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 28583e8e, shipped listing asm/funcs/func_800480C0.s; byte-visible shape, no compile required

## s9 — forensics (2026-09-02, chassis HEAD 0c7f30e4)

### H-s9-1 — KILLED (instance)
**Statement.** A fifth or sixth member of the func_800482C8 batch-loader family exists in
the shipped binary, and one of them writes the sp+0x18..0x37 window, which would name the
32-byte object and give an honest row-free C form for all four known siblings.
**Mechanism.** function.c assign_stack_local reserves a slot from a source DECL at
RTL-expand; a live sibling would show the object's stores in its shipped listing.
**Probe.** `grep -l 'func_800482C8' asm/funcs/*.s src/*.c`.
**Result.** Exactly five files match: asm/funcs/{func_80047EE8,func_80047FBC,func_800480C0,
func_800481E8}.s, func_800482C8's own listing, and src/text1b.c. The family has four
members and they are all already characterised; there is no further body to inspect.
**measured_on.** HEAD 0c7f30e4, read-only grep over asm/funcs and src; no candidate installed.

### H-s9-2 — KILLED (instance)
**Statement.** func_80041AC8's three phantoms come from a producer whose RTL provenance
differs from SetDrawEnv's, which would mean the mult-free ceiling of three measured in s8
is not a real ceiling.
**Mechanism.** s8 established func_80041AC8 (src/text1a_post.c:298) is mult-free with 3
phantoms and vars=32 and is not the display.c clamp idiom, so its orphans might have a
different plant site.
**Probe.** Dumped text1a_post.i and display.i with the instrumented cc1 -da
(tmp/grind/func_800480C0/s9/{dump.sh,dump2.sh}); located each hardreg=-1 pseudo in the .cse
and .combine dumps.
**Result.** Both bodies use the identical producer. func_80041AC8's orphans 115/105/85 are
defined at .cse insns 171/106/26 by `(set (reg:SI P) (ashift:SI (subreg:SI (reg:HI Q) 0)
(const_int 16)))`, each followed by an `ashiftrt ... 16` carrying `REG_EQUAL (sign_extend:SI
(reg:HI Q))`; SetDrawEnv's orphans 140/137/128 are defined at .cse insns 217/209/166 by the
same pattern. In .combine each survives only as `(insn (use (reg:SI P)))`. Same class, no
third producer.
**measured_on.** HEAD 0c7f30e4, instrumented cc1 tools/gcc-2.7.2/cc1 on src/text1a_post.c
and src/display.c as they stand at HEAD; no candidate installed.

### H-s9-3 — KILLED (class)
**Statement.** The class-A combine ashift-deletion producer can be made to fire on
func_800480C0's four halfword sites while the shipped 74-insn stream is preserved.
**Mechanism.** combine deletes the `ashift` half of the shift-pair sign extension only when
it can substitute a MEMORY equivalent for the HImode pseudo and re-form the extension as a
signed narrow load; that substitution is gated by can_combine_p's
`use_crosses_set_p (src, INSN_CUID (insn))` test, which refuses whenever a register in the
MEM's address is set between the two insns.
**Probe.** (a) Read the emitted asm of both attested producers: func_80041AC8 ships
`lh $2,0($16)` beside `lhu $3,0($16)`, SetDrawEnv ships `lh $5,22($sp)` and
`lh $2,D_8009BE78` against its own stack local — every orphan is paid for with an emitted
signed narrow load. (b) Read asm/funcs/func_800480C0.s: zero `lh`, and every one of the
four `lhu $aN,0x0($s0)` loads has `addiu $s0,$s0,0x2` on its base register between the load
and the `sll/sra` pair, which is why those pairs survive into the shipped bytes at all.
(c) Measured the immediate-sign-extend spelling (`s32 a1v = (s16)*(u16*)p;`) that would be
the fold's best chance: `vars= 0, regs= 8/0, args= 24`, unalloc=0, orphanUSE=0 — identical
to the control.
**Result.** Firing the producer requires giving a halfword a memory home or removing the
intervening `addiu`; both change the emitted stream (the constant-offset variant was killed
in s8 on displacement and addiu-count grounds), and the spelling that keeps the stream
keeps orphanUSE at 0.
**predicate_cite.** tools/gcc-2.7.2/combine.c:917
**measured_on.** HEAD 0c7f30e4, s6/probe.sh on tmp/grind/func_800480C0/s9/bodies/
{CTRL_candidate,A_s32_immediate_sx}.c installed over the INCLUDE_ASM line; annotated
`arg0 = 0;` FAKE present in every body (its ablation grid was scored in s8: keep-all 20,
drop-1 32).

### H-s9-4 — KILLED (instance)
**Statement.** Extending the arg1 scale into a longer combine-foldable arithmetic chain
plants extra distribute_notes orphan USEs, one per fold.
**Mechanism.** The shipped `sll $a1,16; sra $a1,14` is itself a combine merge of a
sign-extension pair with a `* 4`; if each additional merge orphaned an intermediate, chain
length would be a free phantom multiplier.
**Probe.** Three bodies measured with s6/probe.sh: B_arg1_chain_2x2 (`((s16)arg1*2)*2`),
C_arg1_chain_2x2x1 (`((s16)arg1*2)*2*1`), D_arg1_mul4_after_sx (`(s16)arg1*4`).
**Result.** All three print `vars= 0, regs= 8/0, args= 24`, unalloc=0, orphanUSE=0 —
identical to the control. combine's shift-merge rewrites i3 in place, so the intermediate's
REG_DEAD note always finds a home and distribute_notes never has to plant a USE. Chain
length is not the lever; only the MEM-substitution shape of H-s9-3 orphans anything.
**measured_on.** HEAD 0c7f30e4, bodies at tmp/grind/func_800480C0/s9/bodies/ installed one
at a time over the INCLUDE_ASM line; annotated `arg0 = 0;` FAKE present in every body.

## [s9] A fifth or sixth member of the func_800482C8 batch-loader family exists in the shipped binary, and one of them writes the sp+0x18..0x37 window, which would name the 32-byte object and give a row-free C form for all four known siblings.
- mechanism: function.c assign_stack_local reserves a slot from a source DECL at RTL-expand and never reclaims frame_offset; a sibling in which the object is LIVE would show its stores in the shipped listing.
- probe: grep -l 'func_800482C8' asm/funcs/*.s src/*.c
- result: Exactly five files match: asm/funcs/{func_80047EE8,func_80047FBC,func_800480C0,func_800481E8}.s, func_800482C8's own listing, and src/text1b.c. The family has four members, all already characterised in s8's frame census; there is no further body to inspect for a live instance of the object.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 0c7f30e4, read-only grep over asm/funcs/*.s and src/*.c; no candidate installed, no FAKE construct in play

## [s9] func_80041AC8's three phantoms come from a producer whose RTL provenance differs from SetDrawEnv's, which would mean s8's mult-free ceiling of three is not a real ceiling.
- mechanism: s8 established func_80041AC8 (src/text1a_post.c:298) is mult-free with 3 phantoms and vars=32 and is not the display.c clamp idiom, so its orphan plant sites might have a different RTL shape that stacks differently.
- probe: Dumped src/text1a_post.c and src/display.c with the instrumented cc1 -da (tmp/grind/func_800480C0/s9/dump.sh, dump2.sh) and located every BB2_ALLOC_DEBUG hardreg=-1 pseudo in the .cse and .combine dumps.
- result: Identical producer in both bodies. func_80041AC8's orphans 115/105/85 are defined at .cse insns 171/106/26 by `(set (reg:SI P) (ashift:SI (subreg:SI (reg:HI Q) 0) (const_int 16)))`, each followed by an `ashiftrt ... 16` carrying REG_EQUAL (sign_extend:SI (reg:HI Q)); SetDrawEnv's orphans 140/137/128 are defined at .cse insns 217/209/166 by the same pattern; in .combine each survives only as `(insn (use (reg:SI P)))`. Same class, no distinct third producer.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 0c7f30e4, instrumented cc1 tools/gcc-2.7.2/cc1 on src/text1a_post.c and src/display.c as they stand at HEAD; no candidate installed

## [s9] The class-A combine ashift-deletion producer can be made to fire on func_800480C0's four halfword sites while the shipped 74-insn stream is preserved.
- mechanism: combine deletes the ashift half of a shift-pair sign extension only when it can substitute a MEMORY equivalent for the HImode pseudo and re-form the extension as a signed narrow load; that substitution is gated by can_combine_p's use_crosses_set_p test, which refuses whenever a register in the MEM's address is set between the two insns.
- probe: (a) Read the emitted asm of both attested producers - func_80041AC8 ships `lh $2,0($16)` beside `lhu $3,0($16)`, SetDrawEnv ships `lh $5,22($sp)` and `lh $2,D_8009BE78` - so every orphan is paid for with an emitted signed narrow load. (b) Read asm/funcs/func_800480C0.s: zero `lh`, and each of the four `lhu $aN,0x0($s0)` loads has `addiu $s0,$s0,0x2` on its base register between the load and its sll/sra pair. (c) Measured the immediate-sign-extend spelling (s32 a1v = (s16)*(u16*)p), the fold's best chance, with s6/probe.sh.
- result: The spelling that preserves the stream keeps orphanUSE at 0: A_s32_immediate_sx prints `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`, unalloc=0, orphanUSE=0, identical to the CTRL_candidate control. Firing the producer requires giving a halfword a memory home or removing the intervening addiu, both of which change the emitted stream (the constant-offset variant was already killed in s8 on displacement and addiu-count grounds). The surviving sll/sra pairs in the shipped bytes are themselves the evidence that the fold did not fire in the original.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 0c7f30e4, tmp/grind/func_800480C0/s9/bodies/{CTRL_candidate,A_s32_immediate_sx}.c installed one at a time over the INCLUDE_ASM line in src/text1b.c; annotated `arg0 = 0;` FAKE dead param store present in every body (s8 scored its ablation grid: keep-all 20, drop-1 32)
- predicate_cite: tools/gcc-2.7.2/combine.c:917

## [s9] Extending the arg1 scale into a longer combine-foldable arithmetic chain plants extra distribute_notes orphan USEs, one per fold.
- mechanism: The shipped `sll $a1,16; sra $a1,14` is itself a combine merge of a sign-extension shift pair with a `* 4`; if each additional merge orphaned its intermediate, chain length would be a free phantom multiplier on a body that needs four phantoms.
- probe: Three bodies measured with tmp/grind/func_800480C0/s6/probe.sh via s9/run.sh: B_arg1_chain_2x2 (((s16)arg1*2)*2), C_arg1_chain_2x2x1 (((s16)arg1*2)*2*1), D_arg1_mul4_after_sx ((s16)arg1*4).
- result: All three print `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`, unalloc=0, orphanUSE=0 - identical to the control. combine's shift-merge rewrites i3 in place, so the intermediate's REG_DEAD note always finds a home and distribute_notes never has to plant a USE. Chain length is not the lever on this body; only the MEM-substitution shape orphans anything.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 0c7f30e4, bodies at tmp/grind/func_800480C0/s9/bodies/ installed one at a time over the INCLUDE_ASM line; annotated `arg0 = 0;` FAKE present in every body

## s10 hypotheses (rederive, 2026-09-02, chassis HEAD a0198d09)

**H-s10-1 (KILLED, instance).** A `long long` intermediate in this body leaves an
unallocated DImode pseudo that alter_reg pays 8 bytes of `vars` for, giving the
class-B-style phantoms the target's 32-byte window needs without an emitted
mult/div. *Mechanism:* s8's census found the only bodies above three phantoms are
func_80042874/func_80042A88 at six, both mult/div, whose phantoms are DImode HILO
residue; the hope was that the DImode-ness, not the mult, was the producer.
*Probe:* the two places a long long can enter without inventing a value - b3
(`long long count` loop counter) and b4 (`unsigned long long acc` on both
`(word >> 2) << 2` scale sites, high word provably zero and never read), measured
with tmp/grind/func_800480C0/s10/probe.sh. *Result:* b3 emits real DImode
compare/decrement code (insns 72 -> 86) and grows the callee-saved set to
`regs= 10/0` - the DImode pseudo is allocated and pays registers; vars=0,
unalloc=0. b4 is lowered to SImode by cc1 (insns 72 -> 76); vars=0, unalloc=0.
*measured_on:* HEAD a0198d09, bodies at tmp/grind/func_800480C0/s10/bodies/,
installed one at a time over the INCLUDE_ASM line; the annotated `arg0 = 0;` FAKE
present in b3/b4 and no other FAKE construct.

**H-s10-2 (KILLED, instance).** The two STACK-passed s16 parameters are a class-A
substitution site the four `lhu` sites are not - their incoming slots at
0x68/0x6C($sp) are never written, so `use_crosses_set_p` cannot block - and firing
the substitution there plants the orphan USE that reserves `vars`. *Mechanism:*
s9's predicate (tools/gcc-2.7.2/combine.c:917) closes the four halfword sites
because an `addiu $s0,$s0,0x2` sits between each load and its sll/sra pair; the
stack args have no such intervening set. *Probe:* b7 (arg4/arg5 declared s32, read
`*(s16 *)&argN`), b8 (all four params read sub-word), b9 (s16 round-trip
temporaries that keep the emitted triple). *Result:* the substitution fires and it
costs bytes - b7 and b8 both drop insns 72 -> 68 because each
`lw + sll + sra` triple folds into one signed halfword load - while unalloc stays
0, because the fold rewrites i3 in place and leaves no homeless REG_DEAD note for
distribute_notes to convert into an orphan USE. b9 preserves the 72-insn stream
and also plants nothing. Class A is now measured in both directions on this body:
blocked at the four `lhu` sites, and a four-instruction deletion where it is not
blocked. *measured_on:* HEAD a0198d09, bodies at
tmp/grind/func_800480C0/s10/bodies2/; annotated `arg0 = 0;` FAKE present, no other
FAKE construct.

**H-s10-3 (KILLED, instance).** A structurally different chassis for the same
program - a different cursor type, a record struct, or index addressing instead of
a moving pointer - changes which pseudos combine sees and reserves the target's
32-byte `vars` window. *Mechanism:* the rederive premise, that the shape banked
since s1 is one of several and a different one allocates differently. *Probe:* b1
fresh-m2c chassis, b2 u8* byte cursor, b5 12-byte struct-record with member reads
and `r++`, b6 `for` index chassis over a u16 array with h[2..5] indexing.
*Result:* all four measure `vars= 0` with `unalloc=0`, and the two that abandon
the moving cursor diverge from the target stream in the wrong direction - b5 emits
base+offset addressing (63 insns) and b6 collapses the four addiu increments (62
insns) against the target's 74. The interleaved `lhu`/`addiu` cursor is not a
stylistic choice; it is what the target bytes are. *measured_on:* HEAD a0198d09,
bodies at tmp/grind/func_800480C0/s10/bodies/; FAKE state as banked per body (b1
carries none, b2/b5/b6 carry the annotated `arg0 = 0;`).

**H-s10-4 (CONFIRMED).** The annotated `arg0 = 0;` dead param store is load-bearing
for the register match and masks no phantom lever. *Probe:* `tools/fake_ablate.py`
returned ERR for both variants on this chassis, so the ablation was done by hand -
candidate.c with the line deleted, through s10/probe.sh. *Result:* insns stays 72
and vars/unalloc stay 0, but the base carrier moves from $18/$s2 to $22, the second
base use binds $4 instead of $18, and the prologue store order shifts. s8's
keep-all-20 / drop-1-32 verdict re-confirmed on HEAD a0198d09.
Banked: rejected/s10-candidate-minus-fake-reseats-registers.c.

**H-s10-5 (CONFIRMED).** The shape that reaches the target for this function is
already on main three times, and the only thing this candidate lacks is a
declaration it is not permitted to make. *Probe:* read func_80047FBC
(src/text1b.c:82) and func_80047EE8 (src/text1b.c:35) against candidate.c, and the
grant rows at engine/volatile_cheats.py:757-767. *Result:* func_80047FBC is this
routine with four parameters instead of six and is COMPLETED-C on main; its body
is line-for-line candidate.c - same init chain, same annotated `arg0 = 0;` dead
param store with the same cse2 justification, same in-loop cursor arithmetic, same
`while ((count--) != 0)` tail - plus a leading `volatile u32 pre_pad[8];` granted
by the 2026-08-20 owner ruling. func_800481E8 holds the same grant from the
2026-08-22 ruling. func_800480C0 is the fourth member of the family in the same
translation unit with the same 0x18-0x37 untouched window and is not in the
enumeration (Judge FAIL 2026-09-02 04:28, docs/grind/decisions.md:20349).

### Frontier after s10

The rederive axis is spent: fresh m2c reproduces the banked control flow, the
sibling transplant produces the banked body verbatim, and ten structural
respellings all measure vars=0/unalloc=0. Both phantom producer classes are now
measured closed in both directions (class A blocked at the `lhu` sites on a cited
predicate, and a four-insn deletion where it is not blocked; class B capped at 1
across 55 forms), and the DImode route that s9 left open is dead. What remains is
not a spelling question.

## [s10] A long long intermediate in this body leaves an unallocated DImode pseudo that alter_reg pays 8 bytes of vars for, giving class-B-style phantoms without an emitted mult/div (s9 frontier item 1).
- mechanism: s8's census found the only bodies above three phantoms are func_80042874/func_80042A88 at six, both mult/div, whose phantoms are DImode HILO residue; the open question was whether the DImode-ness rather than the mult was the producer. There are exactly two places a long long can enter this body without inventing a value: the loop counter and the two (word >> 2) << 2 scale expressions.
- probe: b3 (long long count loop counter) and b4 (unsigned long long acc on both scale sites, high word provably zero and never read), each installed over the INCLUDE_ASM line and measured with tmp/grind/func_800480C0/s10/probe.sh (frame line + BB2_ALLOC_DEBUG hardreg=-1 count).
- result: b3 emits genuine DImode compare/decrement code (insns 72 -> 86) and grows the callee-saved set to regs= 10/0 - the DImode pseudo is ALLOCATED and pays registers - with vars=0 and unalloc=0. b4 is lowered to SImode by cc1, adding four insns (72 -> 76), also vars=0/unalloc=0: the provably dead high word does not survive to allocation. Banked as rejected/s10-dimode-count-no-phantom.c and rejected/s10-dimode-scale-folds-to-simode.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a0198d09, bodies at tmp/grind/func_800480C0/s10/bodies/b3_dimode_count.c and b4_dimode_scale.c installed one at a time over the INCLUDE_ASM line in src/text1b.c; the annotated `arg0 = 0;` dead-param-store FAKE present in both, no other FAKE construct.

## [s10] The two stack-passed s16 parameters are a class-A substitution site that the four lhu sites are not - their incoming slots at 0x68/0x6C($sp) are never written, so use_crosses_set_p cannot block - and firing the substitution there plants the orphan USE that reserves vars (s9 frontier item 3).
- mechanism: s9's predicate at tools/gcc-2.7.2/combine.c:917 closes the four halfword sites because an addiu $s0,$s0,0x2 sits between each load and its sll/sra pair. The two incoming stack arguments have no such intervening set, so combine should be free to substitute the stack MEM and delete the intermediate pseudo, leaving a homeless REG_DEAD note for distribute_notes.
- probe: b7 (arg4/arg5 declared s32 and read *(s16 *)&argN, the narrow-stack-param-subword-offset family), b8 (all four s16 params read sub-word), b9 (s16 round-trip temporaries that keep the emitted lw+sll+sra triple intact), measured with tmp/grind/func_800480C0/s10/probe.sh.
- result: The substitution fires and it costs bytes rather than producing frame. b7 and b8 both drop insns 72 -> 68 because each lw + sll + sra triple collapses into a single signed halfword load, and unalloc stays 0 - the fold rewrites i3 in place, so no REG_DEAD note is left homeless and distribute_notes plants no orphan USE. b9 preserves the 72-insn stream and plants nothing either. Class A is now measured in both directions on this body: blocked at the four lhu sites, and a four-instruction deletion where it is not blocked. Banked as rejected/s10-subword-stackargs-folds-lh-deletes-insns.c, s10-subword-all-params-folds-lh.c, s10-himode-roundtrip-no-orphan.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a0198d09, bodies at tmp/grind/func_800480C0/s10/bodies2/ installed one at a time over the INCLUDE_ASM line; the annotated `arg0 = 0;` FAKE present, no other FAKE construct.

## [s10] A structurally different chassis for the same program - a different cursor type, a 12-byte record struct, or index addressing instead of a moving pointer - changes which pseudos combine sees and reserves the target's 32-byte vars window.
- mechanism: The rederive premise: the shape banked since s1 is one of several possible spellings of this routine, and a different one may allocate differently. Tested with the mandated rederive sources - a fresh m2c decompile of asm/funcs/func_800480C0.s and three hand-built alternative chassis.
- probe: b1 fresh-m2c chassis (arg0 used directly at both base sites, s32 arg4/arg5, no FAKE), b2 u8* byte-cursor chassis, b5 12-byte struct-record chassis with member reads and r++, b6 for-index chassis over a u16 array with h[2..5] indexing; all measured with tmp/grind/func_800480C0/s10/probe.sh.
- result: All four measure vars= 0 with unalloc=0. b1 is 71 insns - dropping the second base carrier removes the move $18,$16 the target ships at 0x800480D0, so the m2c shape is one insn short rather than a new lever. b2 is 72 insns (cursor type is not a lever). b5 emits base+offset addressing (63 insns, regs= 9/0) and b6 collapses the four addiu increments (62 insns, regs= 9/0), both against the target's 74: the interleaved lhu/addiu cursor is not a stylistic choice, it is what the target bytes are. Banked as rejected/s10-m2c-direct-chassis-no-phantom.c, s10-bytewalker-chassis-no-phantom.c, s10-struct-record-chassis-stream-diverges.c, s10-index-for-chassis-stream-diverges.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a0198d09, bodies at tmp/grind/func_800480C0/s10/bodies/ installed one at a time over the INCLUDE_ASM line; FAKE state per body - b1 carries no FAKE at all, b2/b5/b6 carry the annotated `arg0 = 0;` dead param store and nothing else.

## [s10] The annotated `arg0 = 0;` dead param store in candidate.c is load-bearing for the register match and masks no phantom lever (mandated FAKE re-audit on the current chassis).
- mechanism: A lever measured inert while a FAKE carrier occupies its target pseudo is not a kill, so the FAKE has to be ablated before the 55-form vars=0 record can be trusted.
- probe: tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c returned ERR for both keep-all and drop-1 on this chassis (a tool-side failure, not a scoring result), so the ablation was done by hand: candidate.c with the `arg0 = 0;` line physically deleted, run through tmp/grind/func_800480C0/s10/probe.sh and diffed against the baseline listing.
- result: With the store removed the body still emits 72 insns with vars=0 and unalloc=0, so the FAKE hides nothing; but the seating changes - sw $22,48($sp) / move $22,$16 replaces sw $18,32($sp) / move $18,$16, the second base use binds $4 instead of $18 (addu $16,$4,$2 vs addu $16,$18,$2), and the prologue store order shifts. s8's keep-all-20 / drop-1-32 verdict re-confirmed on HEAD a0198d09. Banked as rejected/s10-candidate-minus-fake-reseats-registers.c.
- verdict: CONFIRMED

## [s10] The C shape that reaches this function's target bytes is already on main three times over, and the only structural difference between the accepted siblings and this candidate is one declaration this function is not permitted to make.
- mechanism: The rederive sibling-transplant axis. func_800482C8's caller family has exactly four members (s9), all in src/text1b.c, all reserving the identical untouched 0x18-0x37 window.
- probe: Read func_80047FBC (src/text1b.c:82) and func_80047EE8 (src/text1b.c:35) side by side with memory/grind/func_800480C0/candidate.c, and the grant rows at engine/volatile_cheats.py:757-767.
- result: func_80047FBC is this routine with four parameters instead of six and is COMPLETED-C on main. Its body is line-for-line candidate.c: same base_addr/p init chain, the same annotated `arg0 = 0;` dead param store with the same cse2 canonical-register justification, the same (((s32)(arg1 << 16)) >> 14) offset, the same (((*p) >> 2) << 2) re-base, the same in-loop word + four halfword cursor reads, the same while ((count--) != 0) tail. The one difference is its leading `volatile u32 pre_pad[8];`, granted by the 2026-08-20 owner ruling; func_800481E8 holds the same grant from the 2026-08-22 parked-but-proven audit. func_800480C0 is the fourth member of that family, in the same TU, with the same window, and is not in the enumeration - which is exactly what the 2026-09-02 04:28 Judge FAIL turned on (docs/grind/decisions.md:20349).
- verdict: CONFIRMED

## s11 (escalation, 2026-09-02, chassis HEAD 37f9ecdb)

H-s11-1 (KILLED, instance). *Statement:* On the current chassis (HEAD 37f9ecdb)
the banked candidate body still measures a floor of 20, and the single annotated
FAKE it carries (`arg0 = 0;`) is not masking a phantom-slot lever on the pseudo
it occupies. *Mechanism:* a banked instance kill is only valid on the chassis and
FAKE state it was measured under; the driver mandates re-measuring the closest
form with FAKEs ablated before spending a session elsewhere. *Probe:* installed
`memory/grind/func_800480C0/candidate.c` over the INCLUDE_ASM line, ran
`sandbox func_800480C0 --disable all`, then
`tools/fake_ablate.py --func func_800480C0 --file text1b --candidate
memory/grind/func_800480C0/candidate.c`. *Result:* sandbox = 20 (74/74,
rules_dropped 0), identical to s3-s10. Ablator finds exactly one FAKE unit;
keep-all = 20 / 74 insns, drop-1 = 32 / 73 insns. Removing the FAKE raises the
floor and deletes an instruction, so it cannot be hiding a lever that would
lower it. All 29 banked instance kills stand on this chassis.
*measured_on:* HEAD 37f9ecdb, candidate.c installed over the INCLUDE_ASM line,
one FAKE present (`arg0 = 0;`) and separately ablated; volatile pre_pad stripped
by the sandbox in both variants. *kill_scope:* instance.

H-s11-2 (KILLED, instance). *Statement:* func_800480C0 shows STRONG hand-coded-asm
signals, which would open the canonical-asm grant path as the endgame disposition.
*Mechanism:* endgame-lock AND-gate (a) — a canonical-asm grant requires STRONG
`scan_hand_coded` signals S1 (multu pacing), S2 (empty-body branch) or S6 (BIOS
jumptable). *Probe:* `python3 tools/scan_hand_coded.py --single func_800480C0`.
*Result:* `tier=LOW score=1/8`, reason "no strong hand-coded indicators". The
only hit is S4 (front loads: 4 loads in an 8-insn window @ insn 32), which the
tier rule does not count as strong; S1 finds 0 multu/mflo pairs, S3 counts 9
spills across 13 registers (compiler behaviour, not hand allocation), S5 finds no
sibling above jaccard 0.5, S7 finds every callee-save properly saved, S2/S6/S8
find nothing. Gate (a) FAILS; no canonical-asm path.
*measured_on:* HEAD 37f9ecdb, read-only scan over asm/funcs/func_800480C0.s; no
candidate installed, no FAKE construct in play. *kill_scope:* instance.

H-s11-3 (CONFIRMED). *Statement:* An in-hand SOTN-master (PSX / GCC 2.7.2)
precedent EXISTS for the volatile-leading-pad construct that closes this
function, but it evidences the declaration shape only and does not extend BB2's
closed per-function enumeration. *Mechanism:* endgame-lock AND-gate (b) requires
a citable file+line SOTN exhibit, not "same spirit"; separately,
`.claude/rules/no-new-park-categories.md:340-342` and
`engine/volatile_cheats.py:744` scope the BB2 carve-out as an enumeration whose
every row was added by a dated owner ruling. *Probe:* grep
`docs/reference/sotn-construct-index.md` for pad-array declarations and read the
`_SANCTIONED_UNWRITTEN_PADS` table's provenance comments. *Result:* three PSX
hits — `:103` src/st/sel/stream.c:80 `volatile u32 pad[4]; // FAKE`;
`:84` src/st/e_background_bushes_trees.h:160 `volatile char pad[8]; //! FAKE`;
`:101` src/st/sel/2C048.c:564 `volatile u32 pad; // !FAKE:`. The index records
declaration sites only, so the *unwritten* sub-case is not itself established,
and the index header disclaims that a hit is a BB2 authorization. All five
post-original rows in `_SANCTIONED_UNWRITTEN_PADS`
(engine/volatile_cheats.py:746-769) carry inline 2026-08-20 / 2026-08-22 OWNER
RULING citations. So the precedent is real and is recorded in the foreclosure
entry for owner batch review, but it does not authorize the row, and the
disposition remains the silent foreclosure.

**Session outcome:** disposition filed at docs/grind/decisions.md:20353
(RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED), returned `owner-gated`.
No further probe should be spent re-spelling class A / class B / DImode routes —
s4 (~50k permuter iterations), s6-s9 (producer forensics) and s10 (55 structural
forms, all vars=0/unalloc=0) closed them, and s10's sibling transplant showed the
residual is a source DECLARATION carried by three COMPLETED-C siblings under
owner grants, not an unfound spelling.

## [s11] On the current chassis (HEAD 37f9ecdb) the banked candidate body still measures a floor of 20, and the single annotated FAKE it carries (arg0 = 0;) is not masking a phantom-slot lever on the pseudo it occupies.
- mechanism: A banked instance kill is only valid on the chassis and FAKE state it was measured under; the driver's KILL RE-AUDIT mandate requires re-measuring the closest banked form with every FAKE ablated before spending the session elsewhere.
- probe: Installed memory/grind/func_800480C0/candidate.c over the INCLUDE_ASM line via tmp/grind/func_800480C0/s3/install.py, ran `sandbox func_800480C0 --disable all`, then `tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c`; restored src/text1b.c to HEAD afterwards.
- result: sandbox = {"score": 20, "target_insns": 74, "build_insns": 74, "rules_dropped": 0} - identical to s3 through s10, so the flat floor is real on this chassis and not a stale ledger number. The ablator finds exactly one FAKE unit (arg0 = 0; at candidate.c L206): keep-all = 20 / 74 insns, drop-1 = 32 / 73 insns. Removing the FAKE raises the floor and deletes an instruction, so it cannot be concealing a lever that would lower it. The volatile pre_pad[8] is not counted as a FAKE unit because the volatile-cheat stripper removes it before scoring - which is precisely why the honest floor reads 20 and not 0. All 29 banked instance kills stand unchanged on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 37f9ecdb, candidate.c installed over the INCLUDE_ASM line; one FAKE present (arg0 = 0;) and separately ablated; volatile pre_pad stripped by the sandbox in both variants.

## [s11] func_800480C0 shows STRONG hand-coded-asm signals, which would open the canonical-asm grant path as this function's endgame disposition.
- mechanism: Endgame-lock AND-gate (a): a canonical-asm grant requires STRONG scan_hand_coded signals - S1 multu pacing, S2 empty-body branch, or S6 BIOS jumptable. Weak signals (S4 front loads, S5 cluster) do not qualify.
- probe: python3 tools/scan_hand_coded.py --single func_800480C0 (output banked at tmp/grind/func_800480C0/s11/scan_hand_coded.txt).
- result: HAND_CODED: tier=LOW score=1/8 (74 insns), reason 'no strong hand-coded indicators'. Only S4 fires (4 loads in an 8-insn window @ insn 32). S1 finds 0 multu/mflo pairs; S3 counts 9 spills across 13 distinct registers (compiler allocation behaviour, not hand allocation); S5 finds no sibling above jaccard 0.5; S7 finds every callee-save properly saved; S2, S6 and S8 find nothing. Gate (a) FAILS - no canonical-asm grant path. Consistent with the rest of the ledger: m2c rederives this control flow directly (s10 axis 1), and a COMPLETED-C sibling with a line-for-line identical body (func_80047FBC, src/text1b.c:82) already ships on main.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 37f9ecdb, read-only scanner pass over asm/funcs/func_800480C0.s; no candidate installed, no FAKE construct in play.

## [s11] An in-hand SOTN-master (PSX / GCC 2.7.2) precedent exists for the volatile-leading-pad construct that closes this function, but it evidences the declaration shape only and does not extend BB2's closed per-function enumeration.
- mechanism: Endgame-lock AND-gate (b) requires a citable file+line SOTN-master exhibit ('same spirit' does not count). Separately, .claude/rules/no-new-park-categories.md:340-342 and engine/volatile_cheats.py:744 scope BB2's carve-out as an enumeration, not a shape predicate, and every row in it was added by a dated owner ruling.
- probe: Grepped docs/reference/sotn-construct-index.md (machine-generated at sotn-decomp master aa53500226ee84be763f3e8702b27de06456b3a7, 1911 files) for volatile pad declarations, and read the provenance comments on every row of engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS.
- result: Three PSX (untagged, i.e. GCC 2.7.2, not the PSP/mwcc or Saturn ports) hits: sotn-construct-index.md:103 -> src/st/sel/stream.c:80 `volatile u32 pad[4]; // FAKE`; :84 (and :620) -> src/st/e_background_bushes_trees.h:160 `volatile char pad[8]; //! FAKE`; :101 -> src/st/sel/2C048.c:564 `volatile u32 pad; // !FAKE:`. Honest limit: the index records declaration sites only, so it does not resolve whether those pads are subsequently written - it evidences the volatile-leading-pad construct, not specifically the UNWRITTEN sub-case this function needs - and the index header states a hit 'is not, by itself, a BB2 authorization'. Meanwhile all five post-original rows in _SANCTIONED_UNWRITTEN_PADS (engine/volatile_cheats.py:746-769: func_80047EE8, func_80047FBC, func_800481E8, func_80049A2C, func_80041688) carry inline dated 2026-08-20 / 2026-08-22 OWNER RULING citations; none was added by a Judge, a driver scope grant, or a session. So a sixth row is a family extension, which the 2026-08-31 ordinary-c-judge-decidable ruling makes FAIL(CONSTRUCT) - exactly what the 2026-09-02 04:28 Judge ruled at decisions.md:20349. The precedent is recorded in the foreclosure entry so the driver borderline-logs it for owner batch review; the disposition is still the silent foreclosure.
- verdict: CONFIRMED

## s12 — structural (2026-09-05, chassis HEAD 2dba1d48)

- **H-s12-1 (KILLED, instance).** The banked candidate body still measures a floor of 20
  on chassis 2dba1d48, and its single FAKE unit is load-bearing.
  *Probe:* install candidate.c over the INCLUDE_ASM line, `sandbox --disable all`;
  then `tools/fake_ablate.py`.
  *Result:* score 20 (74/74, rules_dropped 0); ablation keep-all 20 / drop-1 32.
  *Measured on:* HEAD 2dba1d48, candidate.c installed, one FAKE (`arg0 = 0;`) present and
  separately ablated. Artifacts: `tmp/grind/func_800480C0/s12/fake_ablate.txt`.

- **H-s12-2 (KILLED, instance).** Twelve structural respellings of this body — declaration
  order and block-scope depth, loop shape and exit form, cursor chassis, statement
  re-association, counter signedness, and sign-extend placement — each emit `vars= 0` with
  zero unallocated pseudos.
  *Probe:* `tmp/grind/func_800480C0/s12/{gen.py,runall.sh}`; per-body `.frame`, insn count
  and `BB2_ALLOC_DEBUG` hardreg=-1 count.
  *Result:* all twelve `vars= 0 / unalloc=0`; b5 and b8 buy a ninth callee-saved register
  (frame 64, wrong direction), b4 deletes the guard insns the target ships (67 insns).
  *Measured on:* HEAD 2dba1d48, bodies at `tmp/grind/func_800480C0/s12/bodies/` installed
  one at a time over the INCLUDE_ASM line, the annotated `arg0 = 0;` FAKE present in every
  body. Artifacts: `tmp/grind/func_800480C0/s12/runall.txt`.

- **H-s12-3 (KILLED, instance).** Giving the four narrow fields a second use AS AN HIMODE
  VALUE — the precondition `.claude/rules/phantom-slot-frame-lever.md:47-56` names for the
  combine orphan-USE producer — does not plant an orphan on this body.
  *Probe:* d1 accumulates all four fields in HImode before widening, d2 accumulates one;
  measured with the same instrument.
  *Result:* d1 `unalloc=0`, 66 insns; d2 `unalloc=0`, 71 insns. Combine re-forms the loads
  as `lh` and folds the chain instead of stranding a REG_DEAD note at the loop label.
  *Measured on:* HEAD 2dba1d48, bodies at `tmp/grind/func_800480C0/s12/diag/`, FAKE
  present. Artifacts: `tmp/grind/func_800480C0/s12/rundiag.txt`.

- **H-s12-4 (CONFIRMED).** The probe is live on this chassis: the banked
  `phantom-guard-vars8-ceiling.c` form still reads `vars= 8 / unalloc=1`, so this
  session's fourteen zeros are real negatives.
  *Artifacts:* `tmp/grind/func_800480C0/s12/runctrl.txt`.

## [s12] The banked candidate body still measures a floor of 20 on the current chassis, and its single FAKE unit (the annotated `arg0 = 0;`) is load-bearing rather than masking a lever.
- mechanism: Mandated kill re-audit: an instance kill is only as good as the chassis and FAKE state it was taken under, and the chassis moved from 37f9ecdb (s11) to 2dba1d48. fake_ablate.py rebuilds the body with each annotated FAKE unit removed in turn and re-scores.
- probe: python3 tmp/grind/func_800480C0/s3/install.py memory/grind/func_800480C0/candidate.c; `sandbox func_800480C0 --disable all`; src/text1b.c restored from HEAD; then `python3 tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c`.
- result: sandbox printed {"score": 20, "target_insns": 74, "build_insns": 74, "rules_dropped": 0}. Ablation: one FAKE unit found; keep-all 20 (74 insns) / drop-1 32 (73 insns). The store is load-bearing for the stream and masks no phantom lever, re-confirming the s8/s10/s11 verdicts on a new chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2dba1d48, candidate.c installed over the INCLUDE_ASM line; one FAKE (`arg0 = 0;`) present in the keep-all run and separately ablated in the drop-1 run.

## [s12] Twelve structural respellings of this body - declaration order and block-scope depth, loop shape and exit form, cursor chassis, statement re-association, counter signedness, and sign-extend placement - each emit `vars= 0` with zero unallocated pseudos.
- mechanism: The residual is 32 bytes of `vars` that no instruction touches, and s5 measured `vars = 8 * phantoms` exactly across this tree, so the target needs FOUR unallocated pseudos. The structural modality's levers (block-local splits, declaration order, type narrowing, statement re-association) were spelled out as twelve bodies and screened on the `.frame`/BB2_ALLOC_DEBUG gradient rather than the sandbox score, which separates 'wrong frame' from 'wrong codegen'.
- probe: tmp/grind/func_800480C0/s12/gen.py generated b1..b12; tmp/grind/func_800480C0/s12/runall.sh installed each over the INCLUDE_ASM line, compiled the TU with the instrumented cc1 (BB2_ALLOC_DEBUG=1), and reported `.frame`, insn count and the hardreg=-1 pseudo count.
- result: All twelve read `vars= 0 / unalloc=0`. b1 new_var in the loop block, b2 all locals at function scope, b3 for(;;)+inline break, b7 new_var computed first, b9 sign-extends inside the loop, b10 half hoisted, b11 an extra nested block, b12 swapped sum operand order and b6 an integer offset cursor all sit at 72 insns / frame 56. Two move the frame the wrong way by buying a ninth callee-saved register (b5 split u32*/u16* cursors and b8 unsigned counter both read `regs= 9`, frame 64), and b4 (pre-test `while ((count--) != 0)` with no leading guard) drops to 67 insns by deleting guard instructions the target ships.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2dba1d48, bodies at tmp/grind/func_800480C0/s12/bodies/ installed one at a time over the INCLUDE_ASM line, with the annotated `arg0 = 0;` FAKE present in every body.

## [s12] Giving the four narrow fields a second use AS AN HIMODE VALUE - the precondition the phantom-slot rule names for the combine orphan-USE producer - plants no orphan pseudo on this body.
- mechanism: .claude/rules/phantom-slot-frame-lever.md:47-56 records that combine's distribute_notes orphan-USE (combine.c:10832-10841) fires when the widened `reg:HI` carries a second use as an HImode value (func_8007CE0C r8, tslLineG5Init pseudo 92), and that an s32-only consumer 'takes the widened value and the site folds clean instead'. s7 and s9 had only established that this body's fields are consumed once as s32; this probe supplies the missing precondition directly and asks whether the producer then fires.
- probe: tmp/grind/func_800480C0/s12/diag/d1_himode_accumulate.c accumulates all four fields in HImode (`a1v = (s16)(a1v + arg2);` then `(s32)a1v`); d2 does it for one field. Both measured with the same .frame/BB2_ALLOC_DEBUG instrument via tmp/grind/func_800480C0/s12/rundiag.sh.
- result: d1 `vars= 0 / unalloc=0`, 66 insns; d2 `vars= 0 / unalloc=0`, 71 insns. Combine re-forms the narrow loads as `lh` and folds the whole chain rather than stranding a REG_DEAD note at the loop-top label, so the note always finds a home. Class A therefore fails on this body even with its documented precondition present - its absence was never the binding constraint. (Both forms are diagnostics, not candidates: they change the program's semantics and delete bytes the target ships.)
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2dba1d48, bodies at tmp/grind/func_800480C0/s12/diag/ installed one at a time over the INCLUDE_ASM line, annotated `arg0 = 0;` FAKE present.

## [s12] The .frame/unalloc probe is live on this chassis, so this session's fourteen zeros are genuine negatives rather than a dead instrument.
- mechanism: A negative-only batch is worthless without a positive control: the banked rejected/phantom-guard-vars8-ceiling.c form is known to produce exactly one class-B phantom, so re-running it on the current chassis proves the instrument still detects unallocated pseudos.
- probe: tmp/grind/func_800480C0/s12/runctrl.sh over memory/grind/func_800480C0/rejected/phantom-guard-vars8-ceiling.c.
- result: Reads `.frame $sp,64 # vars= 8, regs= 8/0, args= 24`, insns=73, unalloc=1 - unchanged from s6. Control passes.
- verdict: CONFIRMED

## [s13] The banked candidate body has been UNCOMPILABLE since the s12 ledger commit: its header comment contained the literal u32-star-slash-u16-star, whose comment terminator closed the block early
- mechanism: install.py splices candidate.c VERBATIM (header comments included) over the INCLUDE_ASM line, so the ledger header is compiled C. s12's header line `b5 split u32*<slash>u16* cursors (regs=9, frame 64)` terminates the comment at the terminator embedded in `u32*<slash>`, after which the remaining header prose is parsed as code. cc1 reported `src/text1b.c:145: parse error before '='` and stopped emitting after func_80047FBC, so text1b.o contained no func_800480C0 at all.
- probe: installed memory/grind/func_800480C0/candidate.c unchanged on HEAD 3c8d48d6 and ran `sandbox func_800480C0 --disable all`; it returned `score: null, scorable: false, error: func_800480C0 not found in tmp/sandbox/func_800480C0/text1b.o` twice. Compiling the same tree under the s13 probe (instrumented cc1) printed the parse errors into tmp/grind/func_800480C0/s13/alloc.err. Replaced the offending text with `u32-ptr / u16-ptr` (BODY UNCHANGED) and re-ran both.
- result: CONFIRMED. After the comment fix the sandbox reads `score 20, target_insns 74, build_insns 74, rules_dropped 0` and the probe reads `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`, insns=72, unalloc=0. This is the direct cause of the dispatch-time CHASSIS CHECK reading "measurement unavailable" for this function: the floor was never lost, only unmeasurable. Standing lesson for every grind ledger: candidate.c header comments are COMPILED, so no header prose may contain a comment terminator.
- verdict: CONFIRMED

## [s13] Mandated kill re-audit: the banked candidate still measures floor 20 on chassis 3c8d48d6 and its single FAKE unit is still load-bearing rather than masking a lever
- mechanism: an instance kill is only valid on the chassis and FAKE state it was taken under; the s12 kills were measured on 2dba1d48 and (unknowingly) through the broken header.
- probe: `sandbox func_800480C0 --disable all` with the fixed candidate installed, then `python3 tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c` (tmp/grind/func_800480C0/s13/fake_ablate.txt).
- result: sandbox 20 (74/74, rules_dropped 0). fake_ablate finds exactly one FAKE unit (`arg0 = 0;`) and scores keep-all 20 / drop-1 32 - identical to s8/s11/s12. The FAKE carries the stream and hides no phantom-slot lever.
- verdict: CONFIRMED

## [s13] Expressing this body's arithmetic through `static __inline__` helpers (integrate.c inline expansion) changes nothing about frame or allocation - a producer class never previously exercised on this body
- mechanism: GCC 2.7.2 expands `__inline__` callees via integrate.c, which copies the callee's RTL and can leave the caller carrying pseudos and stack slots the flat spelling never creates; if inline expansion planted extra degenerate pseudos it would be a phantom producer independent of combine (class A) and of the folded compare (class B).
- probe: three bodies through tmp/grind/func_800480C0/s13/probe.sh (instrumented cc1, BB2_ALLOC_DEBUG): i1 routes the four call arguments through `static __inline__ s32 sx_sum(s16, s32)` (4 expansions), i2 routes both `base + ((w>>2)<<2)` scalings through `static __inline__ s32 scaled(s32, u32)` (2 expansions), i3 uses both helpers (6 expansions). Bodies banked at rejected/s13-inline-*.c.
- result: KILLED. All three emit `.frame $sp,56 # vars= 0, regs= 8/0, args= 24` with insns=72 and unalloc=0 - identical to the flat candidate's codegen. GCC 2.7.2 integrates these single-expression helpers before any allocation-visible pseudo survives, so inline expansion is codegen-transparent on this body and adds no phantom.
- verdict: KILLED

## [s13] The class-B phantom is one SImode CONSTANT pseudo feeding a branch_equality compare whose whole live range sits in the entry block, and on this body the second (backedge) branch does not host a second one because a loop-carried compare constant is allocated instead
- mechanism: dump-grounded (tmp/grind/func_800480C0/s13/dumps/i5.flow insns 64/65): `(set (reg:SI 93) (const_int -1))` feeding `(if_then_else (eq (reg/v:SI 92) (reg:SI 93)) ...)`. i5.lreg prints `Register 93 used 2 times across 2 insns in block 0; dies in 0 places; ST_REGS or none`, so global.c never seats it and reload1.c alter_reg pays 8 bytes of vars for it. A second instance needs another entry-block compare against a NON-ZERO constant with a degenerate live range: a compare against 0 uses `$0` and makes no pseudo at all, and a compare constant used on the loop backedge is loop-invariant, hence live across the loop and allocated.
- probe: five new bodies. i5 puts the guard subtraction in a `static __inline__` helper expanded at BOTH branch sites; g2 gives the two branches DISTINCT constants (entry `!= -1`, backedge `!= 1`); g3 shifts the entry guard to `count - 2 != -2`; r1 duplicates the entry test on an equal-valued second pseudo (`guard2 = count - 1; if (guard2 != -1)`) nested inside the first; r2 duplicates it on a shifted-but-equivalent pseudo (`guard2 = count - 2; if (guard2 != -2)`). All through s13/probe.sh; banked in rejected/ as s13-*.
- result: KILLED for multiplicity. i5 = vars 8 / regs 8 / unalloc 1 / 73 insns (notable: it reaches the 8-byte slot WITHOUT the ninth callee-saved register that s5's two-branch form needed - but still one phantom). g2 = vars 0 / regs 9 / unalloc 0: the backedge constant is allocated and the entry phantom disappears, the direct measurement of the loop-carried-constant mechanism. g3 = vars 0 / regs 8 / unalloc 0 / 71 insns. r1 = vars 8 / unalloc 1 / 73 insns - cse2/jump deletes the redundant compare AND its constant together, so no second phantom. r2 = vars 8 / unalloc 1 / 76 insns - the shifted duplicate survives as three extra emitted instructions and still yields one phantom. Class B stays at one phantom (8 of the 32 bytes) across every spelling measured in s2-s5 and s13.
- verdict: KILLED

## [s13] The banked candidate body has been uncompilable since the s12 ledger commit because its header comment contained a C comment terminator embedded in a pointer-type description, so the sandbox could not score this function at dispatch
- mechanism: install.py splices candidate.c verbatim (header comments included) over the INCLUDE_ASM line, so ledger prose is compiled C. The s12 line describing the split-cursor form wrote the two pointer types with a slash directly after a star, which closes the comment block; the remaining header prose then parses as code. cc1 reported 'src/text1b.c:145: parse error before =' and stopped emitting after func_80047FBC, leaving text1b.o without func_800480C0.
- probe: Installed memory/grind/func_800480C0/candidate.c unchanged on HEAD 3c8d48d6 and ran `sandbox func_800480C0 --disable all` twice: both returned score null / scorable false / 'func_800480C0 not found in tmp/sandbox/func_800480C0/text1b.o'. Recompiled the same tree with the instrumented cc1 under tmp/grind/func_800480C0/s13/probe.sh, which printed the parse errors into s13/alloc.err. Replaced the offending text with 'u32-ptr / u16-ptr' (BODY UNCHANGED) and re-ran both.
- result: CONFIRMED. After the comment fix the sandbox reads score 20, target_insns 74, build_insns 74, rules_dropped 0, and the probe reads .frame $sp,56 # vars= 0, regs= 8/0, args= 24 with 72 insns and unalloc=0. The honest floor was never lost, only unmeasurable. Re-verified a second time after this session's new header was prepended: probe identical, sandbox 20.
- verdict: CONFIRMED

## [s13] Mandated kill re-audit: the banked candidate still measures floor 20 on chassis 3c8d48d6 and its single FAKE unit is load-bearing rather than masking a phantom-slot lever
- mechanism: An instance kill is only valid on the chassis and FAKE state it was taken under; the s12 kills were taken on 2dba1d48 and (unknowingly) through the broken header.
- probe: `sandbox func_800480C0 --disable all` with the fixed candidate installed, then `python3 tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c` (tmp/grind/func_800480C0/s13/fake_ablate.txt).
- result: Sandbox 20 (74/74, rules_dropped 0). fake_ablate finds exactly one FAKE unit (`arg0 = 0;`) and scores keep-all 20 / drop-1 32, identical to s8/s11/s12. The FAKE carries the instruction stream and hides no lever.
- verdict: CONFIRMED

## [s13] Routing this body's arithmetic through static __inline__ helpers, so that GCC 2.7.2's integrate.c expands them into the caller, produces frame and allocation results identical to the flat candidate on this chassis
- mechanism: integrate.c copies a callee's RTL into the caller and can leave behind pseudos and stack slots the flat spelling never creates; if that happened here it would be a phantom producer independent of combine (class A) and of the folded compare (class B), which are the only two producers this ledger has ever exercised.
- probe: Three bodies through tmp/grind/func_800480C0/s13/probe.sh (instrumented cc1, BB2_ALLOC_DEBUG): i1 routes the four call arguments through `static __inline__ s32 sx_sum(s16, s32)` (4 expansions); i2 routes both `base + ((w>>2)<<2)` scalings through `static __inline__ s32 scaled(s32, u32)` (2 expansions); i3 uses both helpers (6 expansions). Results in tmp/grind/func_800480C0/s13/runall.txt; bodies banked at memory/grind/func_800480C0/rejected/s13-inline-sum-helper-codegen-transparent.c and s13-inline-both-helpers-codegen-transparent.c.
- result: KILLED. All three emit .frame $sp,56 # vars= 0, regs= 8/0, args= 24 with insns=72 and unalloc=0, i.e. the flat candidate's codegen exactly. GCC 2.7.2 integrates these single-expression helpers before any allocation-visible pseudo survives, so inline expansion adds no phantom here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 3c8d48d6, bodies at tmp/grind/func_800480C0/s13/bodies/i{1,2,3}*.c installed one at a time over the INCLUDE_ASM line; the annotated FAKE `arg0 = 0;` present in each

## [s13] The class-B phantom on this body is one SImode constant pseudo feeding a branch_equality compare with a degenerate entry-block live range, and the four spellings tried this session that aimed to add a second such pseudo each measured one phantom or none
- mechanism: Dump-grounded: tmp/grind/func_800480C0/s13/dumps/i5.flow insns 64-65 show `(set (reg:SI 93) (const_int -1))` feeding `(if_then_else (eq (reg/v:SI 92) (reg:SI 93)) ...)`, and i5.lreg prints 'Register 93 used 2 times across 2 insns in block 0; dies in 0 places; ST_REGS or none' - global.c never seats it, so reload1.c alter_reg pays 8 bytes of vars for it. A second instance would need another entry-block compare against a non-zero constant with an equally degenerate live range: a compare against 0 uses $0 and creates no pseudo, and a compare constant used on the loop backedge is loop-invariant, hence live across the loop and allocated.
- probe: Five bodies through s13/probe.sh: i5 puts the guard subtraction in a `static __inline__ s32 pre_dec(s32)` expanded at BOTH branch sites; g2 gives the two branches distinct constants (entry != -1, backedge != 1); g3 shifts the entry guard to count-2 != -2; r1 nests a redundant equal-valued second entry guard (guard2 = count - 1; if (guard2 != -1)); r2 nests a redundant shifted-but-equivalent guard (guard2 = count - 2; if (guard2 != -2)). Results in s13/runall.txt, s13/rung.txt, s13/runr.txt; bodies banked at rejected/s13-inline-guard-both-sites-still-one-phantom.c, s13-loop-carried-const-guard-allocated-ninth-reg.c, s13-shifted-const-guard-no-phantom.c, s13-redundant-equal-guard-deleted-whole.c, s13-redundant-shifted-guard-costs-insns.c.
- result: KILLED for multiplicity. i5 = vars 8 / regs 8 / unalloc 1 / 73 insns - a small improvement in that it reaches the 8-byte slot WITHOUT the ninth callee-saved register s5's two-branch guard needed, but still one phantom. g2 = vars 0 / regs 9 / unalloc 0: the backedge constant is allocated and the entry phantom disappears, the direct measurement of the loop-carried-constant mechanism. g3 = vars 0 / regs 8 / unalloc 0 / 71 insns. r1 = vars 8 / unalloc 1 / 73 insns - cse2/jump deletes the redundant compare and its constant together. r2 = vars 8 / unalloc 1 / 76 insns - the shifted duplicate costs three extra emitted instructions for the same single phantom. Class B remains at 8 of the 32 bytes the target reserves.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 3c8d48d6, bodies at tmp/grind/func_800480C0/s13/bodies/{i5,g2,g3,r1,r2}*.c installed one at a time over the INCLUDE_ASM line; the annotated FAKE `arg0 = 0;` present in each; RTL attribution from s13/dumps/i5.{flow,lreg}

## s14 - synthesis (2026-09-05, chassis HEAD 298b7f40)

### Merged attack after re-reading the whole ledger

Thirteen sessions framed the 32-byte `vars` residual as a PHANTOM-COUNT problem: four
unallocated pseudos are needed, this body reaches at most one, and the tree-wide mult-free
ceiling is three (SetDrawEnv). s14 accepts that framing as correct for phantoms and adds the
finding that phantoms are not the only producer of `vars`. Reading the compiler instead of the
tree turned up a third producer that the ledger had never named, and it reaches the target frame
exactly on the first try.

The gating fact is `tools/gcc-2.7.2/reload1.c:2382-2385`: `alter_reg` pays for a pseudo only when
`reg_renumber < 0 && reg_n_refs > 0 && reg_equiv_constant == 0 && reg_equiv_memory_loc == 0`, and
pays `assign_stack_local (mode, total_size, -1)` - alignment -1 = BIGGEST_ALIGNMENT, which is why
one phantom is 8 bytes. But `get_frame_size()` is fed by every `assign_stack_local` /
`assign_stack_temp` call in the function, and `tools/gcc-2.7.2/integrate.c:2085-2092` makes one
per inline expansion: `assign_stack_temp (BLKmode, DECL_FRAME_SIZE (map->fndecl), 1)`, a verbatim
copy of the inline callee's own frame, `keep=1`. `DECL_FRAME_SIZE` is snapshotted pre-optimisation
at `integrate.c:345`. So a `static __inline__` helper with a 32-byte local donates 32 bytes of
`vars` to func_800480C0 whether or not anything survives to touch them - measured
`.frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24`, and at 72 insns (the candidate's own count)
the full scorer prints score 0.

Three of the four donation spellings measured are frame-pad respellings (unreferenced array, dead
load, dead-conditional-store) and are banked in rejected/, not proposed - the standing Judge
constraint on this function bans an unwritten frame pad in any spelling, and relocating it into an
inline helper is a spelling, not a family. The one spelling with a live use (i8) emits the one
`sw` that GCC 2.7.2 cannot remove (`flow.c:1740-1742` deletes a memory store only when a later
store to the identical address follows). That is the whole shape of what is left.

### Frontier reset

1. **A shared inline helper whose 32-byte local is genuinely live at a DIFFERENT call site, and
   dead only on this one, is the first donation spelling that is not a relocated pad.** The
   object would be load-bearing in the program; at func_800480C0's call site ordinary constant
   propagation removes its traffic while `integrate.c:2085-2092` still charges the frame.
   *Next probe:* census text1b.c for a COMPLETED-C sibling that already owns a ~32-byte local
   aggregate (`grep -n "\[8\]\|\[16\]\|\[32\]" src/text1b.c` around the pure-C bodies), and test
   whether factoring it into a `static __inline__` helper leaves that sibling byte-identical while
   donating 32 untouched bytes here. If a candidate exists, the construct is a genuine
   ruling-request (shared utility with a call-site-constant flag), NOT a self-approvable form.

2. **The array is not the only route to `DECL_FRAME_SIZE == 32`, and the alternatives have
   different traffic profiles.** `function.c:3605` / `function.c:3888` give an inline callee a
   frame-resident stack home for a parameter, and `function.c:1347` (`put_var_into_stack`) for an
   addressable scalar; both feed `DECL_FRAME_SIZE` the same way an array does.
   *Next probe:* a `static __inline__` helper taking a 32-byte struct BY VALUE (or with several
   addressable scalars) called from the loop, measuring `# vars=` and insns with
   `tmp/grind/func_800480C0/s14/probe.sh`. A struct-by-value parameter that the inliner maps
   straight into pseudos may donate 32 bytes with no store at all, which an array cannot.

3. **Calibration that bounds how much freedom the donation gives.** Unknown: whether GCC 2.7.2 at
   -O2 auto-inlines a `static` helper with no `__inline__` keyword (which would make the helper
   look like ordinary factoring rather than a codegen device), and whether the donated size is
   exactly `sizeof` or rounded.
   *Next probe:* two probe.sh runs - the i9 helper with `__inline__` removed, and the same helper
   with `u32 t[7]` (28 bytes) - reading the `# vars=` term. Cheap, and it decides the shape of
   every item-1 and item-2 form.

### Do NOT re-spell

Class A (combine orphan USE), class B (entry-block compare constant), the DImode route, the
inline-helper-with-empty-frame route, the structural axes, split shifts as ONE EXPRESSION (s6) or
ACROSS STATEMENTS (s14 t1-t4), and cross-block named step constants (s14 n1) are all measured
dead: s4 (~50k permuter iterations), s6-s9, s10 (55 forms), s12 (14), s13 (7), s14 (5).

## [s14] A static __inline__ helper carrying a 32-byte local donates 32 bytes of vars to func_800480C0 via integrate.c:2085-2092 (assign_stack_temp of DECL_FRAME_SIZE, keep=1), producing the target's exact frame decomposition .frame $sp,88 with vars=32, regs=8/0, args=24.
- mechanism: copy_rtx_and_substitute's VIRTUAL_STACK_VARS_REGNUM case allocates a verbatim copy of the inline callee's frame in the caller for every expansion; DECL_FRAME_SIZE is snapshotted pre-optimisation at integrate.c:345, so the caller pays for inlinee frame objects even when every reference to them is later deleted.
- probe: tmp/grind/func_800480C0/s14/probe.sh on bodies i8/i9/i10/i11 (helper pack_off with a u32 t[8] local, called from the loop in place of the inline mask expression), plus the full scorer on i9 and i11.
- result: All four print .frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24. i8 (real use of t[0]) emits one surviving sw $4,24($sp) at 73 insns; i9 (unreferenced array), i10 (dead load) and i11 (store inside a never-taken if) emit 72 insns, the banked candidate's own count. Full scorer: i9 prints score 20 because the sandbox cheat-stripper removes the unreferenced array; i11 prints score 0 with target_insns 74, build_insns 74, rules_dropped 0. i11 is a banned construct (dead-conditional-store plus a parameter that is literal 0 at its only call site) and is banked in rejected/, not proposed.
- verdict: CONFIRMED

## [s14] An inline-callee frame donation whose donated object is live on the inlined path emits at least one store, so the i8 spelling lands at 73 insns instead of the target's 74-insn stream.
- mechanism: GCC 2.7.2's only dead-store rule for memory is flow.c:1740-1742, which deletes a store only when a later store to the identical address follows with no intervening memory reference; the last store to an address always survives, and cse can fold the loads but not the store.
- probe: bodies/i8_inline_localarray_frame.c through tmp/grind/func_800480C0/s14/probe.sh, with the emitted stream read out of tmp/grind/func_800480C0/s14/last_i8_inline_localarray_frame.s.
- result: vars=32 as required but insns=73 with sw $4,24($sp) inside the loop, and a small scheduling shift around it. The target stream contains no store in the 0x18-0x37 window.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 298b7f40, body installed over the INCLUDE_ASM line; the base candidate's single FAKE (arg0 = 0;) present, no other FAKE construct.

## [s14] Splitting a shift into two shifts across separate statements through a named variable leaves a middle insn for combine to merge and orphan, at the loop mask, the four narrow sign-extends, and the entry arg1 shift.
- mechanism: s6 killed the one-expression spelling because the tree folder collapses constant-shift-of-constant-shift before RTL; separate statements through a named variable are not foldable at tree level, so two RTL shift insns exist and a 3-insns-in-2-insns-out combine could strand the intermediate's REG_DEAD note at the block head (combine.c:10820-10841).
- probe: Four bodies t1 (loop >>2 split), t2 (all four sign-extends split as e = (s32)raw << 8; e = e << 8; e = e >> 16), t3 (entry arg1 << 16 >> 14 split), t4 (all of them), each through tmp/grind/func_800480C0/s14/probe.sh.
- result: All four print .frame $sp,56,$31 # vars= 0, regs= 8/0, args= 24 at 72 insns - byte-identical to the banked candidate. cse/combine merge the split pair with no orphan (use) planted, so no phantom appears at any of the six split sites.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 298b7f40, bodies at tmp/grind/func_800480C0/s14/bodies/t{1,2,3,4}*.c installed one at a time over the INCLUDE_ASM line; base candidate's single FAKE (arg0 = 0;) present in each.

## [s14] Giving the loop's five pointer advances named constant variables initialised in the entry block leaves each constant pseudo with a single use that combine absorbs into an immediate field, stranding its REG_DEAD note and producing phantoms.
- mechanism: The class-B phantom s13 named is an entry-block compare constant absorbed at its single use; five singly-used advance constants would repeat that shape at five sites.
- probe: bodies/n1_five_step_consts.c (adv4=4 and four adv2*=2, declared and initialised before the entry test, used once each in the loop) through tmp/grind/func_800480C0/s14/probe.sh.
- result: .frame $sp,64 # vars= 0, regs= 10/0, insns=78, unalloc=3. The constants are materialised into live callee-saved registers and the pointer advances become addu instead of addiu, costing six extra instructions. Combine builds LOG_LINKs within a basic block only, so a constant defined in the entry block is never absorbed at a use inside the loop. The unalloc=3 with vars=0 also corrects the ledger's instrument: reg_equiv_constant pseudos are unallocated but pay zero frame bytes (reload1.c:2382-2385).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 298b7f40, bodies/n1_five_step_consts.c installed over the INCLUDE_ASM line; base candidate's single FAKE (arg0 = 0;) present.

## [s14] An inline callee with 6 or 8 parameters forces some arguments through memory and donates an incoming-args block to the caller's frame via integrate.c:2118-2131.
- mechanism: copy_rtx_and_substitute's VIRTUAL_INCOMING_ARGS_REGNUM case allocates assign_stack_temp (BLKmode, FUNCTION_ARGS_SIZE (inlinee), 1) whenever the inlinee's saved RTL references the incoming-args pointer.
- probe: bodies/i6_inline6_argblock.c (6-parameter emit_rec helper) and bodies/i7_inline8_argblock.c (8-parameter emit_rec8), each through tmp/grind/func_800480C0/s14/probe.sh.
- result: Both print .frame $sp,56 # vars= 0, regs= 8/0, args= 24 at 71 insns. The inliner maps every parameter straight to a pseudo, so the inlinee's RTL never references virtual_incoming_args and the block is never allocated. Parameter count alone does not trigger the args-block donation.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 298b7f40, bodies at tmp/grind/func_800480C0/s14/bodies/i6_inline6_argblock.c and i7_inline8_argblock.c installed one at a time over the INCLUDE_ASM line; base candidate's single FAKE (arg0 = 0;) present.

## [s14] The banked candidate still measures a floor of 20 on the current chassis and its single FAKE unit (arg0 = 0;) is load-bearing rather than masking a lever.
- mechanism: Mandated kill re-audit: a lever measured while a FAKE carrier occupies its target pseudo is not a kill, so the closest banked form is re-measured with the FAKE ablated before anything new is proposed.
- probe: tmp/grind/func_800480C0/s14/probe.sh on candidate.c, the full scorer with candidate.c installed, and tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c.
- result: probe: .frame $sp,56 # vars= 0, regs= 8/0, args= 24, insns=72, unalloc=0. Scorer: score 20, target_insns 74, build_insns 74, rules_dropped 0. Ablation: one FAKE unit, keep-all 20 at 74 build insns, drop-1 32 at 73 - the FAKE is load-bearing and masks no lever, identical to the s12 and s13 audits. The s13 comment-terminator repair holds and the dispatch chassis check's measurement-unavailable reading is resolved.
- verdict: CONFIRMED

## s15 - synthesis (2026-09-05, chassis HEAD e18d7715, candidate.c + its one FAKE `arg0 = 0;`)

Modality: synthesis. The whole ledger was re-read; the mandated kill re-audit was run on the
candidate and on the two closest-to-target instance kills (s14 i8 and i9) before any new probe.
Frontier items 2 and 3 from s14 were both executed to a measurement; frontier item 1 is retired
by argument plus a measured control. The session's structural contribution is a COMPLETE
source-level census of the producers of frame `vars` in this compiler, which converts fifteen
sessions of "producer classes we have thought of" into a closed list of eight sites.

### KILL RE-AUDIT (mandated; run first)
- **RE-MEASURED, VERDICT UNCHANGED.** candidate: sandbox 20 (74/74, rules_dropped 0), probe
  vars=0 / 72 insns. s14 i9 (donation with an unreferenced 32-byte local): vars=32 / 72 insns,
  sandbox 20 (the array is stripped). s14 i8 (donation with a written 32-byte local):
  vars=32 / 73 probe insns, sandbox **13** / 75 build insns. All three carry exactly one FAKE
  unit and `tools/fake_ablate.py` reports it load-bearing in each (20/32, 20/32, 13/26), so no
  kill in this ledger was measured with a FAKE carrier occupying a lever's target pseudo.
  Artifacts `tmp/grind/func_800480C0/s15/{floor.txt,fake_ablate_i8_i9.txt,run_a.txt}`.
- NEW FACT FROM THE RE-AUDIT: i8's 13 is the lowest score ever measured on this function and it
  shows the exact price of the residual - a correct frame costs 20 points, and a single
  surviving `sw` inside the untouched window costs 13. It is not proposed and not a floor:
  an eight-word array holding one scalar intermediate fails T1/T2 and is the
  dead-vars-local-array family.

### H-s15-1 (KILLED, instance) - the addressable-parameter donation (s14 frontier 2)
STATEMENT: an inline callee whose frame comes from an address-taken parameter or from
address-taken scalars, rather than from a declared array, donates its frame to this caller
without emitting a caller-side instruction.
MECHANISM: `function.c:1347` (put_var_into_stack) and `function.c:3605`/`3888` (assign_parms)
both feed `DECL_FRAME_SIZE` exactly as `expand_decl` does, and the inliner maps actual
arguments into pseudos, so the stack home might be paid for and never referenced.
PROBE: `bodies/a3_addr_of_param.c` (one address-taken parameter) and
`bodies/a4_eight_addressable_scalars.c` (eight address-taken scalars), measured with
`tmp/grind/func_800480C0/s15/probe.sh`.
RESULT: a3 reads `.frame $sp,72 # vars= 16, regs= 8/0, args= 24` at 73 insns; the diff against
the candidate is exactly one added instruction, `sw $4,24($sp)` - a store into the first word
of the very window the target leaves untouched - plus a scheduler reshuffle of the sign-extend
block. a4 reaches the target decomposition `vars= 32, regs= 8/0, args= 24` but at 80 insns,
eight more than the candidate. Taking a parameter's address makes the parameter live in memory,
and the store that establishes its home is emitted in the CALLER after inlining.
VERDICT: KILLED. kill_scope: instance. measured_on: HEAD e18d7715, bodies at
tmp/grind/func_800480C0/s15/bodies/a3,a4 installed one at a time over the INCLUDE_ASM line,
base candidate's single FAKE (`arg0 = 0;`) present.

### H-s15-2 (KILLED, instance) - the two donation calibrations (s14 frontier 3)
STATEMENT: GCC 2.7.2 at -O2 auto-inlines a `static` helper with no `__inline__` keyword, and the
donated frame size equals `sizeof` of the callee's object exactly.
MECHANISM: if auto-inlining fires, the helper reads as ordinary factoring rather than as a
codegen device; if the size is exact, a donated object must be shaped to exactly 32 bytes.
PROBE: `bodies/a1_autoinline_no_keyword.c` (keyword removed), `bodies/a2_size28.c`
(`u32 t[7]`, 28 bytes), `bodies/a6_size16.c` (`u32 t[4]`), `bodies/a7_two_expansions_16.c`.
RESULT: BOTH HALVES FALSE. a1 emits a real call - `.frame $sp,64 # vars= 0, regs= 10/0`,
75 insns - so this compiler does not auto-inline without `-finline-functions` and the
`__inline__` keyword is load-bearing for the whole donation route. a2 (28 bytes) reads
`vars= 32`: the donation is ROUNDED UP to the 8-byte frame boundary, so a donated object of
25..32 bytes all give 32. a6 (16 bytes) reads `vars= 16` at 72 insns, so the donation is
proportional and byte-neutral at any size; a7's second expansion had a constant argument and
folded away, leaving `vars= 16`, so per-expansion stacking remains unconfirmed.
VERDICT: KILLED. kill_scope: instance. measured_on: HEAD e18d7715, bodies at
tmp/grind/func_800480C0/s15/bodies/a1,a2,a6,a7, base candidate FAKE present.

### H-s15-3 (KILLED, class) - the unevaluated BLKmode conditional as a zero-RTL frame charge
STATEMENT: a BLKmode conditional expression (`cond ? structA : structB`) charges the frame in
the C front end at parse time, so writing one in an unevaluated context reserves frame bytes
with no RTL emitted at all.
MECHANISM: `c-typeck.c:3536` builds a `tempvar` for a BLKmode `COND_EXPR` and calls
`assign_stack_local` on it unconditionally, before the fold and before any expansion - the only
site in the entire compiler that could charge the frame for an expression that is never
evaluated, and therefore the only theoretical escape from the flow.c store-survival rule.
PROBE: `bodies2/c1_sizeof_blk_cond.c` (a `sizeof` of a 32-byte struct conditional),
`c2_sizeof_blk_cond_used.c` (the same conditional feeding a real use),
`c4_sizeof_blk_cond_16.c` (16-byte variant), `c3_sizeof_plain_struct.c` (control).
RESULT: all four read `.frame $sp,56 # vars= 0, regs= 8/0, args= 24` at 71-72 insns - no charge
of any kind. Reading the source explains it: the entire block from `c-typeck.c:3513` is inside
`#if 0`, so the BLKmode-conditional tempvar path does not exist in GCC 2.7.2. No spelling of a
struct conditional can charge this function's frame.
VERDICT: KILLED. kill_scope: class. predicate_cite: `tools/gcc-2.7.2/c-typeck.c:3513`.
measured_on: HEAD e18d7715, bodies2/c1..c4 installed one at a time, base candidate FAKE present.

### H-s15-4 (CONFIRMED, and it corrects the ledger) - four alter_reg slots ARE reachable here
STATEMENT: the ledger's "phantom ceiling of 1 on this body" is a ceiling of the zero-traffic
regime only; under register pressure this body reaches four unallocated pseudos and exactly
`vars= 32`.
MECHANISM: `reload1.c:2382-2385` gives any pseudo with `reg_renumber < 0 && reg_n_refs > 0` an
`assign_stack_local` slot aligned -1, i.e. 8 bytes each, whether the pseudo is a zero-traffic
combine orphan or an ordinary spill.
PROBE: `bodies3/d1_twelve_call_crossing.c` - twelve extra values live across the in-loop call to
func_800482C8.
RESULT: `.frame $sp,96 # vars= 32, regs= 10/0, args= 24`, unalloc=4, 108 insns. The sp-offset
histogram of the emitted body puts the four slots at 24/32/40/48 - exactly the target's
untouched window - each touched twice, one store and one load. So the multiplicity the target
needs is reachable, but only in the regime where every slot is a real spill: 36 extra
instructions, two extra callee-saved registers, and eight memory references inside bytes the
target never touches. Recorded as CONFIRMED because the mechanism fired; it is not a lever.

### H-s15-5 (KILLED, instance) - the shared-helper donation (s14 frontier 1)
STATEMENT: a `static __inline__` helper whose 32-byte local is genuinely live at another call
site, and dead only on func_800480C0's path, is a donation spelling that is not a relocated
frame pad.
MECHANISM: `DECL_FRAME_SIZE` is a pre-optimisation snapshot charged per expansion, so the
sibling keeps the object load-bearing while this function still gains 32 untouched bytes.
PROBE/RESULT: retired without spending a full sibling build, on two measured grounds.
(1) The donation is charged at EVERY expansion (`integrate.c:2092`, `keep=1`), so any second
caller inside this tree gains the same 32 bytes; the three siblings that would be the natural
second callers (func_80047EE8, func_80047FBC, func_800481E8) are COMPLETED-C at their current
frames and a donation would move them, so no live second caller exists that the construct does
not break. s9 already established the family has exactly four members and no fifth caller.
(2) Even granting a second caller, byte-neutrality HERE still requires the object to be
unreferenced on THIS path: a5 (`bodies/a5_struct_byval_param.c`, a helper declared but never
called) measures completely inert, a3/a4/i8 measure that any reference costs an insn inside the
untouched window, and `flow.c:1740-1742` is the reason the last store to an address always
survives. A "live elsewhere, dead here" object is the dead-vars-local-array construct with a
second caller bolted on, not a different family.
VERDICT: KILLED. kill_scope: instance. measured_on: HEAD e18d7715, a5 control measured with the
base candidate FAKE present; the sibling-breakage half is an argument from integrate.c:2092 and
the s9 census, not a fresh build.

### THE MERGED ATTACK, as this session leaves it
The residual is one quantity, `get_frame_size() == 32`, proven end-to-end by s14's i11 at
score 0. Frame `vars` in GCC 2.7.2 has exactly eight producer sites (evidence.md s15 census),
and for this body they now partition cleanly:
  - ZERO-TRAFFIC, ANY MULTIPLICITY: `stmt.c:3412` expand_decl of an unreferenced local
    aggregate. This is the frame-pad family. Banned for this function (Judge 2026-09-02);
    granted by owner ruling to three siblings; bytes already proven (s1 build.log SHA1 match).
  - ZERO-TRAFFIC, MULTIPLICITY CAPPED: the combine-orphan phantom (`reload1.c:2404` reached via
    `combine.c:10832`). ~75 spellings across s2-s15 cap it at ONE on this body; the tree-wide
    maximum on a mult-free body is three (SetDrawEnv/SetDrawEnv2/func_80041AC8). Four has no
    precedent in this tree.
  - TRAFFIC-BEARING: everything else - reload spills (d1, four slots but eight memory refs),
    parameter homes and address-taken scalars (a3/a4), referenced donated objects (i8), BLKmode
    expand temps (need a call or a copy).
  - NOT A PRODUCER AT ALL: the BLKmode-conditional front-end path (`#if 0`).
The only remaining way this function matches in ordinary C is a zero-traffic producer with
multiplicity four, and the census says the only such producer is a declared unreferenced
aggregate. That is the shape three siblings ship under an owner grant.

### FRONTIER FOR s16 (reset to three)
1. The combine-orphan phantom is the ONLY zero-traffic producer whose multiplicity is not
   fixed by a declaration, and the tree maximum is three. Nobody has yet asked WHAT makes
   SetDrawEnv carry three where this body carries one. Next probe: dump SetDrawEnv's .combine
   and .lreg (`pwsh tools/grinder/dump.ps1` on src/display.c) and run s6's count_uses.py to
   extract the three orphans' defining RTL shapes, then test whether any of the three shapes
   can be spelled onto this body's four narrow-field sites at zero byte cost. If the answer is
   that all three come from the same `(ashift (subreg (reg:HI)))` producer s9 already closed
   here, the zero-traffic phantom route is class-dead and the ledger can say so with a cite.
2. The i8 measurement (score 13, frame exactly right, one surviving `sw $4,24($sp)`) is the
   closest any legal-shaped body has come. Next probe: ask whether the surviving store can be
   made to coincide with a store the target ALREADY ships - the target's only sp-relative store
   outside the register saves is `sw $v0,0x10($sp)`, the fifth outgoing argument at offset 16.
   A donated object placed so that its single store lands at 16 rather than 24 would be
   byte-neutral; frame layout order (args below vars) says it cannot, but the layout is
   `function.c` code that has never been read for this question, and the answer is one grep.
3. The producer census has one entry nobody has measured on this body: `caller-save.c:315`.
   Its slots are allocated per call-clobbered hard register that is the home of a call-crossing
   pseudo, sized by `regno_save_mode` (4 bytes each, not 8), and `setup_save_areas` runs inside
   reload's iteration - so a slot allocated in one iteration can survive an allocation change
   that removes the need for the save. Next probe: a body with exactly ten to twelve
   call-crossing values (d1 used twelve and produced spills instead), reading `vars` and the
   sp-offset histogram to see whether any 4-byte caller-save slot is ever left untouched.

## [s15] An inline callee whose frame comes from an address-taken parameter or from address-taken scalars, rather than from a declared array, donates its frame to func_800480C0 without emitting a caller-side instruction.
- mechanism: function.c:1347 (put_var_into_stack) and function.c:3605/3888 (assign_parms stack home) feed DECL_FRAME_SIZE exactly as expand_decl does, and integrate.c maps actual arguments into pseudos, so the donated home might be charged and never referenced.
- probe: tmp/grind/func_800480C0/s15/bodies/a3_addr_of_param.c (one address-taken parameter) and a4_eight_addressable_scalars.c (eight address-taken scalars), each installed over the INCLUDE_ASM line and measured with tmp/grind/func_800480C0/s15/probe.sh.
- result: a3 reads .frame $sp,72 # vars= 16, regs= 8/0, args= 24 at 73 insns, and the diff against the candidate is exactly one added instruction, sw $4,24($sp) - a store into the first word of the 0x18-0x37 window the target leaves untouched - plus a scheduler reshuffle of the sign-extend block. a4 reaches the target decomposition vars= 32, regs= 8/0, args= 24 but at 80 insns, eight more than the candidate. Taking a parameter's address makes it live in memory and the store establishing its home is emitted in the caller after inlining. Closes s14 frontier item 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD e18d7715, bodies a3/a4 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present

## [s15] GCC 2.7.2 at -O2 auto-inlines a static helper with no __inline__ keyword, and the donated frame size equals sizeof of the callee's object exactly.
- mechanism: If auto-inlining fires the helper reads as ordinary factoring rather than a codegen device; if the donated size is exact the donated object must be shaped to exactly 32 bytes rather than merely at least 32.
- probe: tmp/grind/func_800480C0/s15/bodies/{a1_autoinline_no_keyword.c,a2_size28.c,a6_size16.c,a7_two_expansions_16.c} measured with probe.sh.
- result: Both halves false. a1 emits a real call (.frame $sp,64 # vars= 0, regs= 10/0, 75 insns) - no auto-inlining without -finline-functions, so the __inline__ keyword is load-bearing for the whole donation route. a2 (28-byte local) reads vars= 32: the donation is rounded up to the 8-byte frame boundary. a6 (16-byte local) reads vars= 16 at 72 insns, so the donation is proportional and byte-neutral at any size but always with a dead object; a7's second expansion folded away (constant argument), leaving vars= 16, so per-expansion stacking is unconfirmed. Closes s14 frontier item 3.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD e18d7715, bodies a1/a2/a6/a7 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE present

## [s15] A BLKmode conditional expression (cond ? structA : structB) charges the frame in the C front end at parse time, so writing one in an unevaluated context reserves frame bytes with no RTL emitted at all.
- mechanism: c-typeck.c:3536 builds a tempvar for a BLKmode COND_EXPR and calls assign_stack_local on it unconditionally, before the fold and before any expansion - the only site in the compiler that could charge the frame for an expression that is never evaluated, and therefore the only theoretical escape from flow.c's store-survival rule.
- probe: tmp/grind/func_800480C0/s15/bodies2/{c1_sizeof_blk_cond.c,c2_sizeof_blk_cond_used.c,c3_sizeof_plain_struct.c,c4_sizeof_blk_cond_16.c} measured with probe.sh, plus a read of the enclosing source.
- result: All four bodies read .frame $sp,56 # vars= 0, regs= 8/0, args= 24 at 71-72 insns - no frame charge of any kind. The source explains it: the entire block is inside an #if 0 that opens at tools/gcc-2.7.2/c-typeck.c:3513, so the BLKmode-conditional tempvar path does not exist in this compiler and no spelling of a struct conditional can charge a frame.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD e18d7715, bodies2/c1..c4 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE present
- predicate_cite: tools/gcc-2.7.2/c-typeck.c:3513

## [s15] The ledger's phantom ceiling of one on this body bounds the zero-traffic regime only; under register pressure the body reaches four unallocated pseudos and exactly vars= 32.
- mechanism: reload1.c:2382-2385 gives every pseudo with reg_renumber < 0 and reg_n_refs > 0 an assign_stack_local slot aligned -1 (8 bytes), whether that pseudo is a zero-traffic combine orphan or an ordinary reload spill.
- probe: tmp/grind/func_800480C0/s15/bodies3/d1_twelve_call_crossing.c - twelve extra values live across the in-loop call to func_800482C8 - measured with probe.sh, then the sp-offset histogram of tmp/grind/func_800480C0/s15/last_d1_twelve_call_crossing.s.
- result: .frame $sp,96 # vars= 32, regs= 10/0, args= 24 with unalloc=4 at 108 insns. The histogram puts the four slots at offsets 24/32/40/48 - exactly the target's untouched window - each touched twice, one store and one load. The multiplicity the target needs is therefore reachable on this body, but only in the regime where every slot is a real spill: 36 extra instructions, two extra callee-saved registers and eight memory references inside bytes the target never touches.
- verdict: CONFIRMED

## [s15] A shared static __inline__ helper whose 32-byte local is genuinely live at another call site, and dead only on func_800480C0's path, is a donation spelling that is not a relocated frame pad.
- mechanism: DECL_FRAME_SIZE is a pre-optimisation snapshot charged per expansion (integrate.c:2092, keep=1), so a sibling keeps the object load-bearing while this function still gains 32 untouched bytes.
- probe: tmp/grind/func_800480C0/s15/bodies/a5_struct_byval_param.c as the declaration-only control, plus the integrate.c:2092 per-expansion charge and the s9 caller census.
- result: a5 (helper declared, never called) is completely inert - vars= 0, 72 insns - so only an expansion donates. Since every expansion is charged, any second caller inside this tree gains the same 32 bytes, and the only candidate second callers are the three COMPLETED-C siblings (func_80047EE8, func_80047FBC, func_800481E8) whose frames the donation would move; s9 established the family has exactly four members and no fifth caller. And even granting a second caller, byte-neutrality here still requires the object to be unreferenced on this path (a3/a4/i8 each cost an insn inside the untouched window; flow.c:1740-1742 keeps the last store to an address alive), so the construct is the dead local aggregate with a second caller bolted on. Closes s14 frontier item 1.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD e18d7715, a5 control installed over the INCLUDE_ASM line with the base candidate's single FAKE present; the sibling-breakage half is an argument from integrate.c:2092 plus the s9 census, not a fresh sibling build

## s16 - synthesis (2026-09-05, chassis HEAD a5ebaa6f, candidate.c + its one FAKE `arg0 = 0;`)

Modality: synthesis. The whole ledger was re-read before any probe; the mandated kill
re-audit was re-run on the current chassis on the candidate and on the two
closest-to-target instance kills (s14 i8 at 13 and i9 at 20) and all three reproduce s15
exactly. The session's structural contribution is that the residual was MIS-STATED for
fifteen sessions: it is not `get_frame_size() == 32` but `var_size + args_size == 56`, and
`current_function_outgoing_args_size` is a ninth producer that the s15 census - which
grepped only for `assign_stack_local` / `assign_stack_temp` - could not see. The split is
then resolved back to (24, 32) by measurement rather than by assumption, and the last two
unmeasured census entries are closed.

### KILL RE-AUDIT (mandated; run first)
- **RE-MEASURED ON HEAD a5ebaa6f, VERDICT UNCHANGED.** candidate keep-all 20 (74/74,
  rules_dropped 0) / drop-1 32; s14 i9 20 / 32; s14 i8 13 / 26. One FAKE unit each, load
  bearing in each, occupying no lever's target pseudo. Artifacts
  `tmp/grind/func_800480C0/s16/fake_ablate_reaudit.txt`.

### H-s16-1 (CONFIRMED, and it corrects fifteen sessions) - the frame decomposition was under-determined
STATEMENT: the shipped `.frame $sp,88` with saves at 0x38-0x54 does not imply
`get_frame_size() == 32`; MIPS lays the callee-saved block at
`args_size + extra_size + var_size + gp_reg_size - 4`, so only the SUM is observable.
MECHANISM: `tools/gcc-2.7.2/config/mips/mips.c:4557` (gp_sp_offset) and mips.c:4475
(total_size) both take `var_size` and `args_size` additively; `MIPS_STACK_ALIGN` rounds each
to 8. Five pairs sum to 56: (24,32), (32,24), (40,16), (48,8), (56,0).
PROBE: `bodies/e4_widen_existing_call14.c` - the body's ONE call widened to fourteen
arguments, no second call site - measured with `tmp/grind/func_800480C0/s16/probe.sh`, plus
`bodies/e2_call7.c`, `e3_call10.c`, `e1_call14.c` as a scaling ladder.
RESULT: e4 reads `.frame $sp,88,$31 # vars= 0, regs= 8/0, args= 56, extra= 0` at 81 insns,
with the eight register saves at 56/60/64/68/72/76/80/84 - the target's exact save offsets -
and ZERO frame vars. The scaling ladder confirms the rounding: 7 args -> `args= 32`,
10 -> 40, 14 -> 56. The target's frame total, register count and save offsets are all
reachable with `get_frame_size() == 0`.

### H-s16-2 (KILLED, class) - the outgoing-args area as a zero-traffic substitute for frame vars
STATEMENT: `current_function_outgoing_args_size` can be raised above 24 without emitting an
instruction, so the 56 bytes can be bought from the args side instead of the vars side.
MECHANISM: `current_function_outgoing_args_size` is raised at exactly three sites, all
inside `expand_call` / `emit_library_call` (`tools/gcc-2.7.2/calls.c:1400`, `calls.c:2400`,
`calls.c:2750`); each is reached only by expanding a real call, and every argument word past
the fourth is written into the block by `store_one_arg` before the call.
PROBE: the e1/e2/e3/e4 ladder above, with the sp-offset histogram of each emitted body, and
a sandbox score on the frame-exact e4.
RESULT: the stores are contiguous and unconditional - e2 `args= 32` stores at 16/20/24, e3
`args= 40` at 16..36, e4 `args= 56` at 16..52, one `sw` per word with no gaps. Any
`args_size` above 24 therefore places 2/4/6/8 stores inside the target's untouched
0x18-0x37 window, and `asm/funcs/func_800480C0.s` carries exactly one sp-relative store
outside its register saves. e4 scores **22** (83 build insns vs 74) despite a byte-exact
frame - worse than the candidate's 20 - a marginal cost of one instruction per four bytes
against the pad family's zero. A separate call site is worse (e1 90 insns, plus a `jal` the
target does not have). Independently, a fabricated argument list on a callee whose real
signature has five parameters fails cheat-checklist T1 and T2.
COROLLARY, and the reason this hypothesis was worth spending: because every raise costs a
visible store, the target's `args_size` is pinned to 24 and its `var_size` to 32. The
decomposition the ledger has assumed since s2 is now measured.
VERDICT: KILLED. kill_scope: class. predicate_cite: `tools/gcc-2.7.2/calls.c:2400`.
measured_on: HEAD a5ebaa6f, bodies e1/e2/e3/e4 installed one at a time over the INCLUDE_ASM
line, base candidate's single FAKE (`arg0 = 0;`) present.

### H-s16-3 (KILLED, class) - a frame object placed below the outgoing-args block
STATEMENT (s15 frontier 2): the single store that a referenced donated object emits could be
made to land at offset 16 and coincide with the target's own `sw $v0,0x10($sp)`, making the
donation byte-neutral.
MECHANISM: `assign_stack_local` starts allocating at `STARTING_FRAME_OFFSET`, which
`tools/gcc-2.7.2/config/mips/mips.h:1651` defines as
`current_function_outgoing_args_size` (plus a word only under `TARGET_ABICALLS`, which is
off here - the probes read `extra= 0`).
PROBE: read of mips.h:1645-1655 and mips.c:4444-4560, cross-checked against the measured
histograms: every var slot in every body measured this session and last sits at an offset
>= args_size (i8 at 24, a3 at 24, f7's five slots at 24/32/40/48/56).
RESULT: frame objects are allocated strictly ABOVE the outgoing-args block, so no declared,
donated or spilled object can be placed at offset 16 at any size. The i8 body's surviving
`sw $4,24($sp)` could never have been moved to coincide with the target's store.
VERDICT: KILLED. kill_scope: class. predicate_cite:
`tools/gcc-2.7.2/config/mips/mips.h:1651`. measured_on: HEAD a5ebaa6f, histograms of the
s15 i8/a3 and s16 f7 emitted bodies with the base candidate FAKE present.

### H-s16-4 (KILLED, instance) - the caller-save area as a zero-traffic producer (s15 frontier 3)
STATEMENT: `setup_save_areas` allocates a 4-byte slot per call-clobbered hard register that
homes a call-crossing pseudo, and a slot allocated in one reload iteration can survive an
allocation change that removes the need for the save, leaving untouched 4-byte frame bytes.
MECHANISM: `tools/gcc-2.7.2/caller-save.c:315` sizes its slots by `regno_save_mode`
(SImode, 4 bytes) rather than by `alter_reg`'s -1 alignment, and runs before the final
reload decisions.
PROBE: `bodies/f7_cc7.c` .. `bodies/f11_cc11.c` - 7, 8, 9, 10 and 11 extra values live
across the in-loop call to func_800482C8, filling the gap below s15's d1 at twelve - each
measured with probe.sh, plus the sp-offset histogram of f7.
RESULT: `vars= 40/48/56/64/72` with `unalloc= 5/6/7/8/9` and `regs= 10/0` at every level.
Vars tracks the unallocated-pseudo count one for one at exactly 8 bytes each, so every byte
comes from `alter_reg` (`reload1.c:2382-2385`), never from a 4-byte `regno_save_mode` slot;
no 4-byte-granular offset appears at any level, and f7's five slots at 24/32/40/48/56 are
each touched exactly twice. Once the callee-saved file is exhausted this compiler's global
allocator spills rather than caller-saves, so the caller-save area is 0 bytes at every
pressure level reachable on this body.
VERDICT: KILLED. kill_scope: instance. measured_on: HEAD a5ebaa6f, bodies f7..f11 installed
one at a time over the INCLUDE_ASM line, base candidate's single FAKE present.

### H-s16-5 (KILLED, instance) - assign_parms as a producer of untouched frame bytes
STATEMENT: a parameter of this function can be given a stack home that GCC charges to this
frame and that no caller-side or callee-side instruction ever writes.
MECHANISM: `function.c:3605` / `function.c:3888` call `assign_stack_local` for a parameter
whose incoming location is unsuitable, feeding `DECL_FRAME_SIZE` exactly as `expand_decl`
does - the last entry of the s15 census never measured on this body.
PROBE: `bodies/g1_param_struct32_unused.c` (an unused 32-byte by-value struct as a 7th
parameter), `g2_param_s32_unused.c` (an unused s32 7th parameter),
`g3_param_struct32_first_unused.c` (the same struct in FIRST position, so its
register-passed words need a home) and `g4_param_struct32_read_once.c`.
RESULT: all four read `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`. g3 costs 7 extra
instructions storing `$a0-$a3`, but charges them to the CALLER's frame: under
`REG_PARM_STACK_SPACE` a MIPS o32 parameter home lives above this function's frame, never
inside it. With this measurement the s15 eight-site census has no unmeasured entry left.
VERDICT: KILLED. kill_scope: instance. measured_on: HEAD a5ebaa6f, bodies g1..g4 installed
one at a time over the INCLUDE_ASM line, base candidate's single FAKE present.

### s15 FRONTIER ITEM 1 - NOT RE-RUN, BECAUSE s9 ALREADY ANSWERED IT
s15 proposed dumping SetDrawEnv to learn what makes it carry three combine orphans where
this body carries one. The s9 forensics table already answers it: SetDrawEnv's three
(pseudos 140/137/128) and func_80041AC8's three (115/105/85) are byte-for-byte the same
class-A producer - combine deleting the `ashift` half of a shift-pair sign extension of an
HImode pseudo after substituting its MEMORY equivalent, which is why both bodies ship an
`lh`. Multiplicity is just the count of qualifying sites; this body has none, because its
four halfword values come from `lhu` through a base register advanced by `addiu` between
each load and its `sll/sra` pair, and its four s16 parameters have no memory home. Spending
a dump here would have re-derived s9 rather than adding a measurement.

### THE MERGED ATTACK, as this session leaves it
`var_size + args_size == 56` (mips.c:4557). `args_size` is pinned to 24 because every
argument word past the fourth is stored (H-s16-2), so `var_size == 32` - measured, no
longer assumed. `alter_reg` slots are 8-byte aligned (reload1.c:2382-2385), so an allocation
route needs FOUR zero-traffic slots; this body caps at one across roughly 85 measured
spellings, the mult-free tree maximum over 1096 functions is three (s8), and four is
attested nowhere without a mult in the emitted stream. Nine producer sites are now
enumerated and every one is measured on this body; exactly one reaches 32 bytes in a single
step at zero emitted instructions - `expand_decl` of an unreferenced local aggregate, the
pad family, banned here by the 2026-09-02 Judge ruling and granted by owner ruling to the
three siblings that carry the identical untouched window.

### FRONTIER FOR s17 (reset to three)
1. The four-zero-traffic-slot requirement is now the whole question, and it has never been
   attacked from the ALIGNMENT side. `alter_reg` (`reload1.c:2382-2385`) asks
   `assign_stack_local` for the pseudo's mode size with alignment -1, which is why each
   orphan costs 8 bytes rather than 4; if a DImode or larger mode orphan exists the same
   single orphan would be worth 8 or 16. Next probe: look for a zero-traffic orphan in a
   mode wider than SImode - the two 6-phantom mult bodies func_80042874 / func_80042A88
   (src/text1a_c.c, vars=48) are class-B DImode HILO and are the tree's only instance;
   dump one with the instrumented cc1 and read whether its orphan slots are 8 or 16 bytes
   each and whether the producer needs a `mult` insn in the emitted stream or only a DImode
   pseudo that combine leaves unset.
2. Nothing in the ledger has ever measured what a SECOND zero-traffic orphan would cost on
   THIS body if the first is present - every spelling reported orphan counts of 0 or 1 and
   was scored as a whole. Next probe: take the one spelling that does produce an orphan
   (s2's folded loop-guard compare, `guard = count - 1; if (guard != -1)`, vars= 8) and
   stack a second independent instance of the SAME construct on a different value, reading
   the orphan-USE count from a `-da` .combine with s6's count_uses.py rather than the frame
   alone; the ledger's cap of one may be a cap on the SPELLING rather than on the body.
3. The producer census is closed for `get_frame_size` and for
   `current_function_outgoing_args_size`, but not for `current_function_pretend_args_size`,
   which `compute_frame_size` adds to the total only under `ABI_64BIT` (mips.c:4530) and
   which is therefore inert here - and not for the possibility that the shipped `.frame`
   directive itself is not what the assembler used. Next probe: confirm from
   `asm/funcs/func_800480C0.s` and the maspsx stage whether the 0x58 in the shipped
   prologue is reproduced from the `.frame` pseudo-op or from the literal
   `addiu $sp,$sp,-0x58`, i.e. whether any part of the 56 bytes could come from a source
   other than compute_frame_size.

## [s16] The shipped .frame $sp,88 with saves at 0x38-0x54 does not imply get_frame_size() == 32: MIPS lays the callee-saved block at args_size + extra_size + var_size + gp_reg_size - 4, so only the SUM var_size + args_size is observable and five pairs sum to 56.
- mechanism: tools/gcc-2.7.2/config/mips/mips.c:4557 (gp_sp_offset) and mips.c:4475 (total_size) both take var_size and args_size additively, each rounded by MIPS_STACK_ALIGN to 8 (mips.h:2063).
- probe: bodies/e4_widen_existing_call14.c - the body's ONE call widened to fourteen arguments, no second call site - measured with tmp/grind/func_800480C0/s16/probe.sh, plus the scaling ladder e2_call7 / e3_call10 / e1_call14.
- result: e4 reads .frame $sp,88,$31 # vars= 0, regs= 8/0, args= 56, extra= 0 at 81 cc1 insns, with the eight register saves at 56/60/64/68/72/76/80/84 - i.e. 0x38..0x54, the target's exact save offsets - and ZERO frame vars. The ladder confirms the rounding: 7 args -> args= 32, 10 -> 40, 14 -> 56. The target's frame total, register count and save offsets are all reachable with get_frame_size() == 0, so the ledger's fifteen-session framing of the residual as a 32-byte vars gap was one decomposition out of five.
- verdict: CONFIRMED

## [s16] current_function_outgoing_args_size can be raised above 24 without emitting an instruction, so the 56 bytes can be bought from the outgoing-args side instead of the frame-vars side.
- mechanism: current_function_outgoing_args_size is raised at exactly three sites, all inside expand_call / emit_library_call (calls.c:1400, calls.c:2400, calls.c:2750); each is reached only by expanding a real call, and store_one_arg writes every argument word past the fourth into the block before the call.
- probe: The e1/e2/e3/e4 ladder above with the sp-offset histogram of each emitted body (tmp/grind/func_800480C0/s16/last_e*.s), plus a sandbox score on the frame-exact e4.
- result: The stores are contiguous and unconditional: e2 args= 32 stores at 16/20/24, e3 args= 40 at 16..36, e4 args= 56 at 16..52 - one sw per word, no gaps. Any args_size above 24 therefore places 2/4/6/8 stores inside the target's untouched 0x18-0x37 window, and asm/funcs/func_800480C0.s carries exactly one sp-relative store outside its register saves. e4 scores 22 (83 build insns vs the target's 74) despite a byte-exact frame - worse than the candidate's 20 - a marginal cost of one instruction per four bytes of frame, against the pad family's zero; a separate call site is worse still (e1 at 90 insns plus a jal the target does not have). Independently, a fabricated argument list on a callee whose real signature has five parameters fails cheat-checklist T1 and T2. The corollary is the session's most useful product: because every raise costs a visible store, the target's args_size is pinned to 24 and its var_size to 32, so the decomposition the ledger has assumed since s2 is now measured rather than assumed.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a5ebaa6f, bodies e1/e2/e3/e4 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present
- predicate_cite: tools/gcc-2.7.2/calls.c:2400

## [s16] The single store that a referenced donated object emits can be made to land at offset 16 and coincide with the target's own sw $v0,0x10($sp), making the donation byte-neutral (s15 frontier item 2).
- mechanism: assign_stack_local begins allocating at STARTING_FRAME_OFFSET, which mips.h:1651 defines as current_function_outgoing_args_size (plus a word only under TARGET_ABICALLS, which is off here - every probe reads extra= 0), so frame objects sit strictly above the outgoing-args block.
- probe: Read of mips.h:1645-1655 and mips.c:4444-4560, cross-checked against the measured sp-offset histograms of every var-carrying body in the ledger: s15 i8 at 24, s15 a3 at 24, s16 f7's five slots at 24/32/40/48/56.
- result: No declared, donated or spilled frame object can be placed below args_size at any size, so the i8 body's surviving sw $4,24($sp) - the closest any legal-shaped body has come, at score 13 - could never have been moved to offset 16. s15 frontier item 2 is closed with a source cite rather than by enumeration.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a5ebaa6f, histograms of the s15 i8/a3 and s16 f7 emitted bodies with the base candidate's single FAKE present
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:1651

## [s16] setup_save_areas allocates 4-byte caller-save slots on this body that survive an allocation change and go untouched, giving zero-traffic frame bytes at 4-byte granularity (s15 frontier item 3).
- mechanism: caller-save.c:315 sizes its slots by regno_save_mode (SImode, 4 bytes) rather than by alter_reg's -1 alignment, and setup_save_areas runs before the final reload decisions, so a slot allocated for a call-crossing pseudo can outlive the need for the save.
- probe: bodies/f7_cc7.c .. f11_cc11.c - 7, 8, 9, 10 and 11 extra values live across the in-loop call to func_800482C8, filling the gap below s15's d1 at twelve - each measured with probe.sh, plus the sp-offset histogram of f7.
- result: vars= 40/48/56/64/72 with unalloc= 5/6/7/8/9 and regs= 10/0 at every level: vars tracks the unallocated-pseudo count one for one at exactly 8 bytes each, so on these five bodies every frame byte came from alter_reg (reload1.c:2382-2385) and no 4-byte-granular offset appeared at any level. f7's five slots at 24/32/40/48/56 are each touched exactly twice, one store and one load. At every pressure level reachable on this body the global allocator spilled rather than caller-saved once the callee-saved file was exhausted, and the caller-save area measured 0 bytes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a5ebaa6f, bodies f7..f11 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present

## [s16] A parameter of this function can be given a stack home that GCC charges to this frame and that no instruction writes, making assign_parms a zero-traffic producer of the 32 bytes.
- mechanism: function.c:3605 and function.c:3888 call assign_stack_local for a parameter whose incoming location is unsuitable, feeding DECL_FRAME_SIZE exactly as expand_decl does - the last entry of the s15 eight-site census never measured on this body.
- probe: bodies/g1_param_struct32_unused.c (an unused 32-byte by-value struct as a 7th parameter), g2_param_s32_unused.c (an unused s32 7th parameter), g3_param_struct32_first_unused.c (the same struct in FIRST position so its register-passed words need a home) and g4_param_struct32_read_once.c, each measured with probe.sh.
- result: All four read .frame $sp,56 # vars= 0, regs= 8/0, args= 24. g3 costs 7 extra instructions storing $a0-$a3 but charges them to the CALLER's frame: under REG_PARM_STACK_SPACE a MIPS o32 parameter home lives above this function's frame, not inside it. On these four bodies assign_parms contributed 0 frame bytes, and with this measurement the s15 producer census has no unmeasured entry left on this body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a5ebaa6f, bodies g1..g4 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present


## s17 (solver, 2026-09-05, chassis HEAD eab57aaf) - THE RESIDUAL HAS A SECOND DECOMPOSITION, AND IT IS FREE

Mandated solver step first. `inverse_compose.py classify text1b func_800480C0` REFUSES to
run on this function (zero-rule since rules-to-zero 2026-08-25: with no regfix/asmfix rules
the src-derived tgt.s cannot carry the target stream, so the text classifier would emit a
fictitious PRE-RA verdict). The object-level route it names,
`tools/ra_solver/goal_from_tgt.py classify text1b func_800480C0`, returns:

    FIRST DIVERGENCE: PRE-RA
      next tool: none - the residual is upstream of every model
      ours only : addiu #,#,-56 ; sw #,24..52(#) ; lw #,44..52(#)
      target only: addiu #,#,-88 ; sw #,56..84(#) ; lw #,84(#),104(#),108(#)

So the RA and scheduler layers are FORECLOSED as the site of this residual: the two streams
differ in nothing but the frame constant and every $sp displacement derived from it. That is
the solver modality's typed verdict for this function and it should not be re-run.

### THE SESSION'S RESULT: args_size, not var_size

Sixteen sessions have hunted 32 bytes of zero-traffic frame VARS. s16 pinned args_size to 24
by arguing that any larger outgoing-args block would put argument stores inside the target's
untouched window. THAT INFERENCE IS WRONG, and this session measured why.

current_function_outgoing_args_size is set by expand_call at RTL-generation time and is
never revised. If the call insn is afterwards deleted - by jump.c as unreachable - the
outgoing-args block it sized REMAINS in compute_frame_size (mips.c:4467,
args_size = MIPS_STACK_ALIGN (current_function_outgoing_args_size)), while its argument
stores are gone with the block that held them. An outgoing-args block can therefore be
allocated with no stores in it at all.

Measured end to end (bodies in tmp/grind/func_800480C0/s17/bodies, table in
tmp/grind/func_800480C0/s17/measurements.txt):

  - q1 (a 14-argument extern called after `for (;;) {}`): .frame $sp,88 - vars= 0, regs= 8/0,
    args= 56, at 73 insns.
  - r1 (the same call placed after the function's `return;`): .frame $sp,88 - vars= 0,
    regs= 8/0, args= 56, at 72 insns - the candidate's own instruction count.
  - r2 (the same call jumped over by a forward `goto`): identical, 72 insns.
  - q2 (the control: the same call left REACHABLE): args= 56 as well, but 97 insns - the ten
    argument stores cost 25 insns, which is the effect s16 correctly predicted for a live call.

`sandbox func_800480C0 --disable all` with r1 installed over the INCLUDE_ASM line prints
**"score": 0**, target_insns 74, build_insns 74, rules_dropped 0, cheat_asm_stripped 162.

The two decompositions are byte-indistinguishable and both are now measured, not inferred:
compute_frame_size (mips.c:4475, 4547) places the callee-saved block at
args_size + extra_size + var_size + gp_reg_size - 4, which depends only on the SUM, and the
shipped body (asm/funcs/func_800480C0.s) has exactly one non-save $sp reference,
`sw $v0,0x10($sp)`, which lands inside the first 24 bytes either way. args=24/var=32 and
args=56/var=0 produce the identical 74 instructions.

WHAT CHANGES: the two routes have completely different PRICES. Every var_size=32 producer
measured in sixteen sessions costs instructions unless it is the banned frame pad. The
args_size=56 route costs ZERO instructions. The honest-producer hunt should move to
outgoing_args_size.

WHAT DOES NOT CHANGE: r1/r2 are CHEATS and are not proposed. A call to a 14-parameter extern
that does not exist in the game, placed in code that can never execute, fails cheat-checklist
T1 (no observable effect), T2 (no programmer writes it) and T5 (no sanctioned family covers
dead-code-to-inflate-outgoing-args; first reach of the family). Both are banked in
memory/grind/func_800480C0/rejected/s17-BYTES-0-BUT-CHEAT-*.c. The open question they leave is
sharp and narrow: **is there an ordinary-C construct that raises
current_function_outgoing_args_size to 56 without leaving a live call?** Every spelling found
this session needs dead code; a next session should look for one that does not (a real
many-parameter BB2 callee in a genuinely-guarded path; a libcall or block-move that sizes the
block; an inline expansion whose internal call constant-folds away).

### FRONTIER ITEM 2 (stack multiple combine orphans) - KILLED ON THIS BODY

The orphan-USE producer is combine.c:10835-10841: a REG_DEAD note that distribute_notes
cannot rehome, when the forward scan reaches a CODE_LABEL, becomes (use reg) after the
label; the pseudo then has reg_n_refs > 0, no allocation, and alter_reg (reload1.c:2404) buys
it a stack slot. Frontier item 2 asked whether multiplicity is a property of the SPELLING
rather than the body. Measured, three independent ways, all negative:

  - m1 (the s2 folded entry guard, `g0 = count - 1; if (g0 != -1)`) reproduces vars= 8, one
    orphan, 73 insns.
  - m2 adds a second folded guard on the loop-exit test: vars STAYS 8 (one orphan), regs rises
    to 9, insns to 76. m3 (the exit fold alone) reads vars= 0 - the loop-bottom compare is not
    an orphan site at all.
  - n1..n4, copy chains of depth 1, 2, 3 and 4 feeding the entry compare, ALL read vars= 8 at
    73 insns. cse/copy-prop collapses the chain before combine sees it, so chain depth cannot
    multiply orphans.
  - o2/o3/o4, two/three/four NESTED folded guards, all read vars= 0 at 76/78/80 insns. A
    second branch does not double the orphan, it DESTROYS the one - the extra branch gives the
    forward scan a reference before it reaches the label.

So on this body the orphan count is capped at one across every multiplicity spelling tried,
and four 8-byte slots are not reachable that way.

### FRONTIER ITEM 1 (the ALIGNMENT side of alter_reg) - KILLED BY SOURCE

alter_reg calls assign_stack_local (GET_MODE (regno_reg_rtx[i]), total_size, -1)
(reload1.c:2404) with total_size = MAX (inherent_size, reg_max_ref_width[i])
(reload1.c:2389). The align == -1 branch of assign_stack_local sets
alignment = BIGGEST_ALIGNMENT / BITS_PER_UNIT and rounds the size UP to it
(function.c:687). BIGGEST_ALIGNMENT is 64 on this target (mips.h:1082), so a slot is 8 bytes
for SImode AND for DImode alike: a wider-mode orphan buys nothing. The ledger's own s8 census
corroborates it arithmetically - func_80042874 carries six phantoms in vars= 48, i.e. 8 bytes
each, no 16-byte slot anywhere. Frontier item 1's premise ("a wider-mode orphan would be worth
8 or 16 on its own") is false; a dump of func_80042874 is not needed and should not be spent.

### FRONTIER ITEM 3 (the 0x58 from a post-cc1 stage) - CLOSED NEGATIVE

The Makefile pipeline is cpp | cc1 | prologue_fix | maspsx | multu_pad | as (Makefile:149).
tools/prologue_fix.py reads three config files and rewrites a function's prologue only when
that function is NAMED in one of them: tools/prologue_config.json is {}, and
tools/delay_slot_ra_funcs.txt and tools/frame_fix_funcs.txt contain only comment lines. Its
frame-size rewriter apply_frame_fix fires only under `if func_name in frame_fix_funcs`. No
post-cc1 stage can adjust this function's frame constant; the 0x58 comes from
compute_frame_size and nowhere else. (It is also a tools/ surface an agent may not edit, so
even a non-empty list would be an integration matter, not a lever.)

### MANDATED KILL RE-AUDIT (chassis eab57aaf)

`tools/fake_ablate.py --func func_800480C0 --file text1b --candidate candidate.c`: one FAKE
unit (arg0 = 0;), keep-all 20 / drop-1 32. Load-bearing, masking no lever - unchanged from
s14/s15/s16 on three earlier chassis. Candidate floor re-measured 20 this session; HEAD
(INCLUDE_ASM) measures 74.

## [s17] The func_800480C0 residual is a register-allocation or scheduling divergence that the ra_solver / sched_solver models can attack.
- mechanism: tools/ra_solver/inverse_compose.py classify triages a residual PRE-RA / RA / SCHED / IDENTICAL from the two instruction streams; a PRE-RA verdict means no allocation or ordering model applies.
- probe: inverse_compose.py classify text1b func_800480C0 refused (zero-rule guard: with regfix/asmfix empty the src-derived tgt.s cannot carry the target stream, so it would print a fictitious PRE-RA verdict) and named the object-level route. tools/ra_solver/goal_from_tgt.py classify text1b func_800480C0 returned FIRST DIVERGENCE: PRE-RA, 'next tool: none - the residual is upstream of every model', with the one-stream-only shapes being exactly addiu -56 vs -88 and every sp displacement derived from that constant.
- result: KILLED. The solver suite has nothing to search here: ours and target agree on 74 instructions, on every opcode and on every register, and disagree only on the frame constant and the sp offsets it induces. This is a class result for the solver modality on this function - the residual is produced before register allocation runs, by compute_frame_size, so neither the global/local/reload models nor either scheduler pass can move it. Do not spend another solver session on func_800480C0.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD eab57aaf, candidate.c installed over the INCLUDE_ASM line, its single FAKE unit (arg0 = 0;) present; sandbox --disable all = 20, 74/74 insns, rules_dropped 0
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4475

## [s17] The target's untouched sp+0x14..0x37 window requires 32 bytes of frame VARS, so args_size is pinned to 24 by the shipped body's single non-save sp store.
- mechanism: s16 argued that any outgoing-args block larger than 24 bytes would place argument stores at offsets 16..args_size-1, i.e. inside the target's untouched window, because e2/e3/e4 measured every argument word past the fourth stored contiguously into that range.
- probe: Built a 14-argument extern call and placed it three ways where jump.c deletes it after expand_call has already sized the block: after the function's return (r1), jumped over by a forward goto (r2), and after a for (;;) {} (q1). All three read .frame $sp,88 - vars= 0, regs= 8/0, args= 56; r1 and r2 at 72 cc1 insns, the candidate's own count, q1 at 73. Control q2 with the same call left reachable read args= 56 at 97 insns (the ten argument stores cost 25 insns). r1 installed over the INCLUDE_ASM line made engine sandbox func_800480C0 --disable all print score 0, target_insns 74, build_insns 74, rules_dropped 0, cheat_asm_stripped 162.
- result: KILLED as stated. current_function_outgoing_args_size is set by expand_call during RTL generation and never revised, so a block whose call is later deleted as unreachable survives into compute_frame_size (mips.c:4467) with no stores inside it. The single store therefore pins live argument traffic, not the block's size, and args=24/var=32 and args=56/var=0 are byte-indistinguishable: mips.c:4547 places the saved-register block at args_size + extra_size + var_size + gp_reg_size - 4, a function of the SUM only, and the shipped body's one non-save sp reference (sw $v0,0x10($sp), asm/funcs/func_800480C0.s:62) lies in the first 24 bytes under either reading. The spellings that measure 0 are CHEATS and are not proposed - a call to a 14-parameter extern that does not exist in the game, in code that can never execute, fails cheat-checklist T1 (no observable effect), T2 (no programmer writes it) and T5 (first reach of a dead-code-to-inflate-outgoing-args family); both are banked in rejected/s17-BYTES-0-BUT-CHEAT-*.c. What they establish is a price: every var_size=32 producer measured in sixteen sessions costs instructions unless it is the banned frame pad, whereas reaching the target frame through args_size costs zero.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD eab57aaf, bodies q1/q2/r1/r2 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present in each; src/text1b.c restored from HEAD after every install

## [s17] The ledger's cap of one combine orphan on this body is a cap on the SPELLING, so stacking two, three or four independent instances of the folded-compare construct would plant two, three or four orphan slots.
- mechanism: The orphan is planted per REG_DEAD note that distribute_notes cannot rehome when its forward scan reaches a CODE_LABEL (combine.c:10835-10841); multiplicity should be the count of qualifying sites, and four 8-byte alter_reg slots would be exactly the 32 zero-traffic bytes wanted.
- probe: Four spelling families measured with the instrumented cc1: m1 (the s2 folded entry guard) reproduced vars= 8 / one orphan / 73 insns; m2 added a second folded guard on the loop-exit test and m3 used the exit fold alone; n1..n4 fed the entry compare through copy chains of depth 1, 2, 3 and 4; o2/o3/o4 nested two, three and four folded guards.
- result: KILLED on this body. m2 still reads vars= 8 (one orphan) at regs= 9 and 76 insns, and m3 reads vars= 0 - the loop-bottom compare is not an orphan site at all, because the forward scan finds a reference before any label. n1 through n4 all read vars= 8 at 73 insns, identical at every depth: cse and copy propagation collapse the chain before combine sees it, so chain depth cannot multiply REG_DEAD notes. o2/o3/o4 read vars= 0 at 76/78/80 insns - a second branch does not double the orphan, it DESTROYS the one, for the same reason m3 has none. Orphan count on this body is capped at one across every multiplicity spelling tried, so four 8-byte slots are not reachable this way.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD eab57aaf, bodies m1/m2/m3, n1..n4, o2/o3/o4 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present in each

## [s17] An alter_reg phantom slot for a pseudo of a mode wider than SImode is worth 16 bytes rather than 8, so one or two wide orphans could reach 32 zero-traffic frame bytes.
- mechanism: Frontier item 1 held that alter_reg asks assign_stack_local for the pseudo's mode size, so the slot size follows the mode.
- probe: Read the source path end to end: reload1.c:2389 computes total_size = MAX (inherent_size, reg_max_ref_width[i]) and reload1.c:2404 calls assign_stack_local (GET_MODE (regno_reg_rtx[i]), total_size, -1); the align == -1 branch of assign_stack_local sets alignment = BIGGEST_ALIGNMENT / BITS_PER_UNIT and rounds the size UP to it (function.c:687); mips.h:1082 defines BIGGEST_ALIGNMENT 64.
- result: KILLED. Every alter_reg slot is rounded to 8 bytes on this target, so a DImode orphan is worth exactly what an SImode orphan is worth and no wider integer mode exists in this configuration. The ledger's own s8 tree census corroborates it arithmetically: func_80042874 carries six phantoms in vars= 48, i.e. 8 bytes each, with no 16-byte slot anywhere. The 32-byte requirement therefore needs FOUR orphans no matter how the pseudos are typed, and the instrumented-cc1 dump of func_80042874 that frontier item 1 asked for need not be spent.
- verdict: KILLED
- kill_scope: class
- measured_on: source-level, tools/gcc-2.7.2 as pinned at HEAD eab57aaf; corroborated by the s8 census figure vars= 48 over six phantoms in func_80042874
- predicate_cite: tools/gcc-2.7.2/function.c:687

## [s17] Some part of the shipped 0x58 prologue constant may come from a post-cc1 stage rather than from compute_frame_size, in which case the 56-byte var+args sum is the wrong quantity to attack.
- mechanism: The build pipeline runs cpp | cc1 | prologue_fix | maspsx | multu_pad | as (Makefile:149), and tools/prologue_fix.py carries an apply_frame_fix rewriter that adjusts a function's frame size.
- probe: Read prologue_fix.py's main() and inspected its three config files. Every rewriter is gated on the function being named: reorder_prologue under `if func_name in config`, apply_delay_slot_ra under `if func_name in delay_slot_ra_funcs`, apply_frame_fix under `if func_name in frame_fix_funcs`. tools/prologue_config.json contains {}, tools/delay_slot_ra_funcs.txt and tools/frame_fix_funcs.txt contain only comment lines, and regfix.txt / asmfix.txt do not exist (rules-to-zero, 2026-08-25).
- result: KILLED. No post-cc1 stage touches this function's prologue; the 0x58 is written by compute_frame_size and nothing downstream can adjust it. The var_size + args_size == 56 constraint is the right quantity. (tools/ is also a surface an agent may not edit, so a non-empty list would be an integration matter rather than a lever.)
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD eab57aaf, tools/prologue_config.json = {} and both funcs lists comment-only
- predicate_cite: tools/prologue_fix.py:14

## [s18] Mandated kill re-audit: the candidate's single FAKE is still load-bearing and still masks no lever on the current chassis.
- mechanism: tools/fake_ablate.py installs the candidate over the INCLUDE_ASM line, then re-measures with each /* FAKE */ unit removed; a lever measured inert while a FAKE occupies its target pseudo is not a kill (func_8002EA24 s8).
- probe: `python3 tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c` on chassis e25a492f.
- result: one FAKE unit (`arg0 = 0;`), keep-all 20 at 74 build insns, drop-1 32 at 73 build insns. Identical to the s14/s15/s16/s17 audits on four earlier chassis. CONFIRMED (the FAKE is load-bearing); no banked instance kill is voided by it.
- verdict: CONFIRMED

## [s18] A LIVE call can raise current_function_outgoing_args_size above 24 on this body without placing an argument store at sp+0x18 or above.
- mechanism: mips.h:1651 makes STARTING_FRAME_OFFSET == current_function_outgoing_args_size and mips.c:4475 totals the frame as var_size + args_size + extra_size + gp_reg_rounded, so args_size is byte-interchangeable with var_size; s17 reopened this axis by showing an args block survives deletion of its call. The open question was whether a REACHABLE call could size the block without filling it, since the target's 0x18..0x37 window is store-free.
- probe: the body's own in-loop call widened to 6, 7, 8 and 10 live arguments (tmp/grind/func_800480C0/s18/bodies/b{6,7,8,10}_call*args.c) measured with s18/probe.sh, and the sp store/load histogram read off each emitted listing (s18/last_b*.s).
- result: KILLED. args = 24/32/32/40 with outgoing-arg stores at {16,20} / {16,20,24} / {16,20,24,28} / {16,20,24,28,32,36} - one store per argument word past the fourth, contiguous, no gaps, emitted by store_one_arg's emit_push_insn. The SIXTH word is free of the window (its store lands at 0x14) but does not raise args_size at all (still 24, frame grows only by the extra saved register); the SEVENTH word is the first to raise args_size, and its store lands at 0x18, inside the window. Since the target has exactly one non-save sp reference (sw $v0,0x10($sp)), every reachable-call body with args_size > 24 diverges. args_size is pinned at 24 and var_size == 32 for every body without dead code.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/calls.c:3135
- measured_on: HEAD e25a492f, bodies b6/b7/b8/b10 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present in each; src/text1b.c restored from HEAD around every measurement.

## [s18] An 8-byte-aligned argument leaves an alignment HOLE in the outgoing-args block, so a live call could size the block to 56 with the target's 0x18..0x37 window left store-free.
- mechanism: on MIPS o32 a long long / double argument is aligned to an even word slot, so a misaligned one skips a word; the skipped word is never stored. This is the only mechanism in expand_call that can size outgoing-args bytes without emitting a store into them, and no prior session considered it.
- probe: c1_ll6th.c (the five existing arguments plus a 6th long long) and c2_ll6th7th.c (plus a 7th) measured with s18/probe.sh; sp histograms from s18/last_c*.s.
- result: KILLED. The hole is REAL - c1 reads .frame $sp,72 # vars= 0, regs= 10/0, args= 32 with stores at 16, 24, 28 and nothing at 20 - but it falls at 0x14, below the window, while the long long's own two words land at 0x18/0x1C inside it. c2 reads args= 40 with stores at 16,24,28,32,36 and NO second hole, because the second long long is already aligned. A hole costs at most one word per misaligned 8-byte argument, so the eight words needed to raise args_size from 24 to 56 carry at least four stores inside 0x18..0x37 under any argument-type mixture.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD e25a492f, bodies c1_ll6th and c2_ll6th7th installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present in each.

## [s18] compute_frame_size has a third term, extra_size, that some C-level construct could raise to reach the 32-byte residual.
- mechanism: mips.c:4475 totals the frame as var_size + args_size + extra_size + gp_reg_rounded, and fifteen sessions treated extra_size as zero without reading its definition.
- probe: read tools/gcc-2.7.2/config/mips/mips.c:4462 and its neighbourhood, and check the build's compiler flags.
- result: KILLED. extra_size = MIPS_STACK_ALIGN (((TARGET_ABICALLS) ? UNITS_PER_WORD : 0)) - it is a function of the -mabicalls target flag alone and the build compiles -mno-abicalls, so extra_size is 0 for every C body compiled by this toolchain and even under -mabicalls it would be 8, not 32. The adjacent bump at mips.c:4471 (args_size = 4*UNITS_PER_WORD when args_size == 0 && current_function_calls_alloca) cannot fire on a body whose args_size is 24. Every .frame emitted by every s18 probe reads `extra= 0`.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4462

## [s18] A frame producer whose object is REFERENCED could still charge this frame, because references can hide behind a base register copied from $sp rather than $sp itself.
- mechanism: [[base-register-store-invisible-to-symbol-grep]] - a store made through a base register into a frame object is invisible to a `$sp` grep, so s1's "ZERO sw/lw in 0x18..0x37" would not have excluded a referenced object. If such an object existed, the whole class of referenced producers (put_var_into_stack address-taken locals, struct-return temps at calls.c:698, block-move temps in expr.c) would be back in play.
- probe: enumerate EVERY mention of $sp in asm/funcs/func_800480C0.s, not just sw/lw, and classify each.
- result: KILLED as a possibility - the grep returns exactly 25 lines and every one is accounted for: two addiu $sp frame adjustments, 8 register saves at 0x38..0x54, 8 restores, two incoming-parameter loads at 0x68/0x6C (above this frame), and the single sw $v0,0x10($sp). There is no addiu $r,$sp,N, no move $r,$sp and no addu involving $sp, so no sp-derived base register exists in the target at all and nothing can be hidden behind one. The 32 bytes are charged for an object that is unreferenced in the emitted RTL, which leaves exactly the four zero-instruction producers the ledger has already enumerated (unreferenced aggregate / alter_reg phantom / inline donation / dead-call args block).
- verdict: KILLED
- kill_scope: instance
- measured_on: target listing asm/funcs/func_800480C0.s at HEAD e25a492f (a property of the shipped bytes, independent of any installed body or FAKE state).

## [s18] The candidate's single FAKE unit (arg0 = 0;) is still load-bearing on the current chassis and still masks no lever's target pseudo.
- mechanism: tools/fake_ablate.py installs the candidate over the INCLUDE_ASM line and re-measures with each /* FAKE */ unit removed; a lever measured inert while a FAKE occupies its target pseudo is not a kill (func_8002EA24 s8), which is why the audit is mandated when the floor is flat.
- probe: python3 tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c on chassis e25a492f.
- result: One FAKE unit; keep-all 20 at 74 build insns, drop-1 32 at 73 build insns - identical to the s14/s15/s16/s17 audits on four earlier chassis. No banked instance kill is voided; the ledger floor of 20 is re-confirmed on this chassis.
- verdict: CONFIRMED

## [s18] A LIVE call on this body can raise current_function_outgoing_args_size above 24 without placing an outgoing-argument store at sp+0x18 or above.
- mechanism: mips.h:1651 makes STARTING_FRAME_OFFSET == current_function_outgoing_args_size and mips.c:4475 totals the frame as var_size + args_size + extra_size + gp_reg_rounded, so args_size is byte-interchangeable with var_size; s17 reopened this axis by showing that an args block survives deletion of its call. The open question was whether a REACHABLE call could size the block without filling it, since the target's 0x18..0x37 window is store-free.
- probe: The body's own in-loop call widened to 6, 7, 8 and 10 live arguments (s18/bodies/b{6,7,8,10}_call*args.c), measured with s18/probe.sh, with the sp store/load histogram read off each emitted listing (s18/last_b*.s).
- result: args = 24/32/32/40 with outgoing-arg stores at {16,20} / {16,20,24} / {16,20,24,28} / {16,20,24,28,32,36} - one store per argument word past the fourth, contiguous, no gaps, emitted by store_one_arg's emit_push_insn. The SIXTH word is free of the window (its store lands at 0x14) but raises args_size by nothing (still 24); the SEVENTH word is the first that raises args_size, and its store lands at 0x18, inside the window. The target has exactly one non-save sp reference, sw $v0,0x10($sp). args_size is therefore pinned at 24 and var_size == 32 for every body whose call is reachable, restoring s16's decomposition and bounding s17's reopening to the dead-code spellings s17 itself banked as cheats.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD e25a492f, bodies b6/b7/b8/b10 installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present in each; src/text1b.c restored from HEAD around every measurement.
- predicate_cite: tools/gcc-2.7.2/calls.c:3135

## [s18] An 8-byte-aligned argument leaves an alignment hole in the outgoing-args block, so a live call could size that block to 56 bytes on this body while leaving the 0x18..0x37 window store-free.
- mechanism: On MIPS o32 a long long / double argument is aligned to an even word slot, so a misaligned one skips a word that is never stored. This is the only mechanism inside expand_call that can size outgoing-args bytes without emitting a store into them, and no prior session considered it.
- probe: c1_ll6th.c (the five existing arguments plus a 6th long long) and c2_ll6th7th.c (plus a 7th), measured with s18/probe.sh; sp histograms from s18/last_c*.s.
- result: The hole is real - c1 reads .frame $sp,72 # vars= 0, regs= 10/0, args= 32 with stores at 16, 24, 28 and nothing at 20 - but it falls at 0x14, below the target's untouched window, while the long long's own two words land at 0x18/0x1C inside it. c2 reads args= 40 with stores at 16,24,28,32,36 and no second hole, because the second long long is already aligned. A hole costs at most one word per misaligned 8-byte argument, so the eight words needed to lift args_size from 24 to 56 carry at least four stores inside 0x18..0x37 on the argument-type mixtures measured here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD e25a492f, bodies c1_ll6th and c2_ll6th7th installed one at a time over the INCLUDE_ASM line, base candidate's single FAKE (arg0 = 0;) present in each.

## [s18] compute_frame_size's third term extra_size can be raised from C to supply the 32-byte residual.
- mechanism: mips.c:4475 totals the frame as var_size + args_size + extra_size + gp_reg_rounded, and fifteen sessions treated extra_size as zero without reading its definition - if any C-level construct could set it, the residual would have a producer nobody had enumerated.
- probe: Read tools/gcc-2.7.2/config/mips/mips.c:4462 and its neighbourhood; cross-check the build's compiler flags and the extra= field of every .frame emitted by this session's probes.
- result: extra_size = MIPS_STACK_ALIGN (((TARGET_ABICALLS) ? UNITS_PER_WORD : 0)) - a function of the -mabicalls target flag alone, and the build compiles -mno-abicalls, so extra_size is 0 for every C body this toolchain produces; even under -mabicalls it would be 8, not 32. The adjacent bump at mips.c:4471 (args_size = 4*UNITS_PER_WORD when args_size == 0 && current_function_calls_alloca) cannot fire on a body whose args_size is 24. Every .frame emitted by every s18 probe reads extra= 0. The frame identity is exactly var_size + args_size + gp_reg_rounded.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD e25a492f; read from compiler source plus the extra= field of the candidate, b6/b7/b8/b10 and c1/c2 probe listings, base candidate's single FAKE present.
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4462

## [s18] A referenced frame object could still be charged to this frame with its traffic hidden behind a base register copied from $sp, which s1's sw/lw-only grep would not have seen.
- mechanism: A store made through a base register into a frame object is invisible to a $sp grep ([[base-register-store-invisible-to-symbol-grep]]). If such a reference existed in the target, the whole class of REFERENCED producers - put_var_into_stack address-taken locals (function.c:1347), struct-return temps (calls.c:698), block-move temps (expr.c) - would be back in play as honest producers of the 32 bytes.
- probe: Enumerate every mention of $sp in asm/funcs/func_800480C0.s, not just sw/lw, and classify each of the 25 hits.
- result: Every hit is accounted for: two addiu $sp frame adjustments, 8 register saves at 0x38..0x54, 8 restores, two INCOMING-parameter loads at 0x68/0x6C (above this frame), and the single sw $v0,0x10($sp). There is no addiu $r,$sp,N, no move $r,$sp and no addu involving $sp, so no sp-derived base register exists in the target and nothing can hide behind one. The 32 bytes are charged for an object that is unreferenced in the emitted RTL, which leaves exactly the four zero-instruction producers already enumerated: expand_decl of an unreferenced aggregate (the banned pad), alter_reg phantom slots (8 bytes each, capped at one on this body), inline-callee frame donation (killed s14/s15), and a dead-call outgoing-args block (a cheat, s17).
- verdict: KILLED
- kill_scope: instance
- measured_on: target listing asm/funcs/func_800480C0.s at HEAD e25a492f - a property of the shipped bytes, independent of any installed body or FAKE state.

## s19 - forensics (2026-09-05, chassis HEAD e0174a57, candidate.c + its one FAKE `arg0 = 0;`)

Modality: forensics. The mandated kill re-audit was run first; then the ledger's two live
frontier items were both executed to a measurement. Frontier item 1 (the three-way 8+24 split)
is CONFIRMED to reach the target frame and KILLED on the instruction stream; frontier item 2
(a mask register whose save is deleted) is KILLED class from the compiler source with an
empirical cross-check.

### KILL RE-AUDIT (mandated; run first)
- **RE-MEASURED, VERDICT UNCHANGED.** candidate.c on chassis e0174a57: sandbox
  `"score": 20, target_insns 74, build_insns 74, rules_dropped 0`; probe
  `.frame $sp,56,$31 # vars= 0, regs= 8/0, args= 24, extra= 0`, insns=72, unalloc=0.
  `tools/fake_ablate.py` (`tmp/grind/func_800480C0/s19/fake_ablate.txt`): one FAKE unit
  (`arg0 = 0;`), keep-all 20 / drop-1 32. Sixth consecutive chassis with this reading, so no
  kill in this ledger was taken with a FAKE carrier sitting on a lever's target pseudo.

### H-s19-1 (CONFIRMED) - frame charges from different producers are additive
STATEMENT: an alter_reg phantom slot and an integrate.c inline-callee frame donation are
independent calls into assign_stack_local, so their frame charges add, and 8 + 24 reaches the
target's exact frame decomposition.
MECHANISM: `reload1.c:2382-2385` gives an unallocated pseudo an `assign_stack_local` slot
rounded to 8 bytes (`function.c:687`, BIGGEST_ALIGNMENT 64 at `mips.h:1082`), and
`integrate.c:2085-2092` allocates `assign_stack_temp (BLKmode, DECL_FRAME_SIZE (map->fndecl), 1)`
per inline expansion; `compute_frame_size` reads only the total `get_frame_size()`, so nothing
distinguishes one 32-byte object from 8 + 24.
PROBE: five bodies through `tmp/grind/func_800480C0/s19/probe.sh` - p0 (the s17 m1 folded entry
guard alone), p2 (m1 + a `static __inline__` helper with `u32 t[4]`), p1 (m1 + `u32 t[6]`),
p4 (m1 + `u32 t[8]`), plus the p3 control (candidate + `u32 t[6]`, no guard).
RESULT: p0 vars=8/73 insns; p3 vars=24/72; p2 vars=24/73; **p1 vars=32, `.frame $sp,88,$31 #
vars= 32, regs= 8/0, args= 24, extra= 0`, 73 insns, unalloc=1**; p4 vars=40/73. The sum is
exact at every size. p1 is the first body in nineteen sessions to reach the target's frame from
two producers rather than one.
VERDICT: CONFIRMED.

### H-s19-2 (KILLED, instance) - the three-way split loses on the stream, not the frame
STATEMENT: the s17 m1 folded-entry-guard body, which supplies the 8-byte half of the split,
emits the guard value into a second pseudo and moves it back at the loop head, so this body's
stream is one instruction longer than the target's even when the frame is exactly right.
MECHANISM: the phantom is a combine orphan planted by `distribute_notes`
(`combine.c:10832-10841`) on an intermediate the guard creates; that intermediate is a real
live value in the C, so regalloc gives it its own hard register and a copy is needed to merge
it with the loop counter at the loop head.
PROBE: mnemonic-level diff of `tmp/grind/func_800480C0/s19/last_p1_m1_donate24.s` against
`asm/funcs/func_800480C0.s` (alias table subu/addiu, move/addu, li/addiu, b/beq; nops ignored),
plus the full scorer on p1 with the body installed over the INCLUDE_ASM line.
RESULT: 73 emitted instructions against the target's 72 non-nop instructions. The target's
entry sequence is `beqz $s1,.L800481BC` / `addiu $s0,$s0,0x4` / `addiu $s1,$s1,-0x1`; p1 emits
`addu $16,$16,4` / `beq $17,$0,.L16` / `addu $4,$17,-1` and then `addu $17,$4` at the loop head.
Full scorer: `"score": 25, target_insns 74, build_insns 74, rules_dropped 0,
cheat_asm_stripped 163` - WORSE than the candidate's 20, because the sandbox strips the
unreferenced donation array (leaving the frame wrong) while the guard's surplus instruction
stays. The candidate's own stream already matches the target's length exactly, so on this body
a producer that costs an instruction is disqualified regardless of what it does to the frame.
VERDICT: KILLED. kill_scope: instance. measured_on: HEAD e0174a57, bodies
tmp/grind/func_800480C0/s19/bodies/p{0,1,2,3,4}.c installed one at a time over the INCLUDE_ASM
line; base candidate's single FAKE (`arg0 = 0;`) present in each, no other FAKE construct.
Banked at memory/grind/func_800480C0/rejected/s19-phantom-plus-24byte-donation-target-frame-wrong-stream.c
and s19-donation24-alone-vars24-short-of-32.c.

### H-s19-3 (KILLED, class) - no register can sit in the frame mask without an emitted restore
STATEMENT: every register counted in gp_reg_size emits a restore instruction that no
optimisation pass can remove, so the target's eight restores pin gp_reg_size at 32 and
var_size + args_size at 56 for every C body this compiler can produce.
MECHANISM: `compute_frame_size` builds `gp_reg_size` and `mask` in one loop over
`MUST_SAVE_REGISTER` (`mips.c:4479-4486`), so `gp_reg_size == 4 * popcount(mask)` identically;
`save_restore_insns` emits exactly one memory reference per set bit (`mips.c:4680`). The
prologue half IS RTL - `mips.md:6029` expands `prologue` via `mips_expand_prologue`, generated
in `thread_prologue_and_epilogue_insns` after `reload_completed = 1` (`toplev.c:3096-3103`) -
which is what made the deletion loophole formally open. The epilogue half is NOT: the
`epilogue` expander is commented out (`mips.md:6056`), `FUNCTION_EPILOGUE` (`mips.h:2036`)
calls `function_epilogue`, and `mips.c:5174` calls `save_restore_insns (FALSE, tmp_rtx, tsize,
file)` with a live FILE*, printing the restores as text at final directly from
`current_frame_info.mask`. Text emitted at final is not visible to any pass.
PROBE: source reading of the four sites above, plus an empirical cross-check over all twelve
banked probe listings (s18 b6/b7/b8/b10/c1/c2 and s19 candidate/p0..p4) comparing the printed
`regs= N` field against the emitted restore count, and a re-read of the target's own restore
block (`asm/funcs/func_800480C0.s:67-74`).
RESULT: `regs= N` equals the restore count in every listing (candidate regs=8 with ten
`lw ...($sp)` = 8 restores plus the two incoming-parameter loads at 0x68/0x6C; b6 regs=9 with
eleven). The target restores exactly eight registers, so popcount(mask) = 8, gp_reg_size = 32,
and the arithmetic closes: a 9-register mask would need var+args = 52, which is not a multiple
of 8 after `MIPS_STACK_ALIGN`, and a 10-register mask reaching the same 0x54 top-of-block would
have to print ten restores where the target prints eight.
VERDICT: KILLED. kill_scope: class. predicate_cite: `tools/gcc-2.7.2/config/mips/mips.c:4680`.
measured_on: HEAD e0174a57, compiler source in tools/gcc-2.7.2 plus the twelve banked listings;
base candidate's single FAKE (`arg0 = 0;`) present in the measured bodies.

## [s19] An alter_reg phantom slot and an integrate.c inline-callee frame donation are independent calls into assign_stack_local, so their frame charges add and 8 + 24 reaches the target's exact frame decomposition.
- mechanism: reload1.c:2382-2385 gives an unallocated pseudo an assign_stack_local slot rounded to 8 bytes (function.c:687, BIGGEST_ALIGNMENT 64 at mips.h:1082); integrate.c:2085-2092 allocates assign_stack_temp (BLKmode, DECL_FRAME_SIZE (map->fndecl), 1) per inline expansion; compute_frame_size (mips.c:4463) reads only the total get_frame_size(), so nothing distinguishes one 32-byte object from 8 + 24.
- probe: Five bodies through tmp/grind/func_800480C0/s19/probe.sh (instrumented cc1, BB2_ALLOC_DEBUG), each installed over the INCLUDE_ASM line and reverted after: p0 = the s17 m1 folded entry guard alone; p2 = m1 + a static __inline__ helper with u32 t[4]; p1 = m1 + u32 t[6]; p4 = m1 + u32 t[8]; p3 control = candidate + u32 t[6] with no guard.
- result: p0 vars=8 at 73 insns (unalloc=1); p3 vars=24 at 72 (unalloc=0); p2 vars=24 at 73; p1 vars=32 printing .frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24, extra= 0 at 73 insns (unalloc=1) - the target's exact decomposition and its exact 0x58; p4 vars=40 at 73. The sum is exact at every donated size. First body in nineteen sessions to reach the target frame from two producers rather than one.
- verdict: CONFIRMED

## [s19] The s17 m1 folded-entry-guard body, which supplies the 8-byte half of the split, emits the guard value into a second pseudo and moves it back at the loop head, so with a 24-byte donation added its stream is one instruction longer than the target's while its frame is exactly right.
- mechanism: The phantom is a combine orphan planted by distribute_notes (combine.c:10832-10841) on the intermediate the folded guard creates; that intermediate is a real live value in the C, so regalloc seats it in its own hard register and a copy is required to merge it with the loop counter at the loop head.
- probe: Mnemonic-level diff of tmp/grind/func_800480C0/s19/last_p1_m1_donate24.s against asm/funcs/func_800480C0.s (alias table subu/addiu, move/addu, li/addiu, b/beq; nops ignored), plus the full scorer 'sandbox func_800480C0 --disable all' with p1 installed over the INCLUDE_ASM line.
- result: 73 emitted instructions against the target's 72 non-nop instructions. The target's entry sequence is beqz $s1,.L800481BC / addiu $s0,$s0,0x4 / addiu $s1,$s1,-0x1; p1 emits addu $16,$16,4 / beq $17,$0,.L16 / addu $4,$17,-1 and then addu $17,$4 at the loop head. Full scorer: score 25, target_insns 74, build_insns 74, rules_dropped 0, cheat_asm_stripped 163 - worse than the candidate's 20, because the sandbox strips the unreferenced donation array (leaving the frame wrong) while the guard's surplus instruction stays. The candidate's own stream already matches the target's length exactly (72 cc1 insns / 74 build insns, score 20 = the 20 sp-relative operands alone), so on this body a producer that costs an instruction is disqualified whatever it does to the frame. Bodies banked at memory/grind/func_800480C0/rejected/s19-phantom-plus-24byte-donation-target-frame-wrong-stream.c and s19-donation24-alone-vars24-short-of-32.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD e0174a57, bodies tmp/grind/func_800480C0/s19/bodies/p{0,1,2,3,4}.c installed one at a time over the INCLUDE_ASM line in src/text1b.c and reverted after each; base candidate's single FAKE (arg0 = 0;) present in every body, no other FAKE construct.

## [s19] Every register counted in gp_reg_size emits a restore instruction that no optimisation pass can remove, so the target's eight restores pin gp_reg_size at 32 and var_size + args_size at 56 for every C body this compiler can produce.
- mechanism: compute_frame_size builds gp_reg_size and mask in one loop over MUST_SAVE_REGISTER (mips.c:4479-4486), so gp_reg_size == 4 * popcount(mask) identically; save_restore_insns emits exactly one memory reference per set bit (mips.c:4680). The prologue half IS RTL - mips.md:6029 expands 'prologue' via mips_expand_prologue, generated in thread_prologue_and_epilogue_insns after reload_completed = 1 (toplev.c:3096-3103) - which is what left the deletion loophole formally open. The epilogue half is not: the 'epilogue' expander is commented out (mips.md:6056), FUNCTION_EPILOGUE (mips.h:2036) calls function_epilogue, and mips.c:5174 calls save_restore_insns (FALSE, tmp_rtx, tsize, file) with a live FILE*, printing the restores as text at final directly from current_frame_info.mask. Text emitted at final is invisible to every pass.
- probe: Source reading of the four sites above in tools/gcc-2.7.2, an empirical cross-check over all twelve banked probe listings (s18 b6/b7/b8/b10/c1/c2 and s19 candidate/p0..p4) comparing the printed 'regs= N' field against the emitted restore count, and a re-read of the target's own restore block (asm/funcs/func_800480C0.s:67-74).
- result: regs= N equals the restore count in every listing (candidate regs=8 with ten lw ...($sp) = 8 restores plus the two incoming-parameter loads at 0x68/0x6C; b6 regs=9 with eleven). The target restores exactly eight registers ($ra,$s6,$s5,$s4,$s3,$s2,$s1,$s0 at 0x54..0x38), so popcount(mask) = 8 and gp_reg_size = 32. The arithmetic closes the neighbours too: a 9-register mask would need var+args = 52, not a multiple of 8 after MIPS_STACK_ALIGN, and a 10-register mask reaching the same 0x54 top-of-block would have to print ten restores where the target prints eight. The ledger's frontier item 2 is retired.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD e0174a57, compiler source in tools/gcc-2.7.2 plus the twelve banked probe listings under tmp/grind/func_800480C0/s18 and s19; base candidate's single FAKE (arg0 = 0;) present in the measured bodies.
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4680

## [s19] The banked candidate still measures a floor of 20 on the current chassis and its single FAKE unit (arg0 = 0;) is load-bearing rather than masking a lever.
- mechanism: Mandated kill re-audit: a lever measured while a FAKE carrier occupies its target pseudo is not a kill, so the closest banked form is re-measured on the current chassis with the FAKE ablated before anything new is proposed.
- probe: sandbox func_800480C0 --disable all with candidate.c installed over the INCLUDE_ASM line; tools/fake_ablate.py --func func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c; tmp/grind/func_800480C0/s19/probe.sh on candidate.c.
- result: Sandbox: score 20, target_insns 74, build_insns 74, rules_dropped 0, cheat_asm_stripped 162. Probe: .frame $sp,56,$31 # vars= 0, regs= 8/0, args= 24, extra= 0, insns=72, unalloc=0. Ablation (tmp/grind/func_800480C0/s19/fake_ablate.txt): one FAKE unit, keep-all 20 at 74 build insns, drop-1 32 at 73. Sixth consecutive chassis with the identical reading; the dispatch chassis check's 'measurement unavailable' resolves to 20.
- verdict: CONFIRMED

## s20 (rederive) hypotheses

- **H-s20-1 CONFIRMED (instance -> the function's whole residual).** *The residual of
  func_800480C0 is exactly a 32-byte `get_frame_size()` charge and contains no second
  defect.* Probe: install `s20/bodies/h1_stmt_tail.c` (candidate body + a 32-byte inline
  donation at a value-free site) over the `INCLUDE_ASM` line and run
  `sandbox func_800480C0 --disable all`. Result: `score 0`, 74/74, rules_dropped 0; raw cc1
  listing differs from the candidate's by the single `.frame` line. Measured on HEAD
  a4c735da with the candidate's single FAKE (`arg0 = 0;`) present.
- **H-s20-2 KILLED (instance).** *A value-flowing call site for the inline-callee donation
  keeps this function's stream unperturbed.* Ten placements measured; every site whose value
  reaches the program moved instructions (entry sites hoist the two incoming-parameter loads
  above the register saves and add a load-delay nop, sandbox 6; loop sites hoist the
  `new_var` srl/sll/addu, sandbox 16; register-param sites cost one insn). Only the two
  discarded-statement sites are byte-neutral. Measured on HEAD a4c735da, bodies
  `s20/bodies/{d2,d3,e1,e5,e6,f1,f3,g1,g3,g5,g6,g7,g8,h1,h2}.c`, candidate FAKE present.
- **H-s20-3 KILLED (instance).** *The inline-callee donation family contains an honest
  spelling for this body.* Every byte-neutral spelling is `pack_off(arg1, 0);` - a discarded
  call to a side-effect-free helper whose 32-byte local nothing on this path needs, i.e. the
  banned unwritten pad relocated into a callee. It fails cheat-checklist T1 and T2 on its own
  terms. Banked in `rejected/s20-inline-donation-dead-tail-call-bytes0-but-relocated-pad.c`.
  Measured on HEAD a4c735da with the candidate FAKE present. (Instance, not class: the kill
  is of the spellings reachable on THIS body - a helper with a genuine 25..32-byte local and
  a genuine reason to be called from here would re-open it, which is why frontier item 1 is
  restated below in its only surviving form.)
- **H-s20-4 CONFIRMED (instance).** *m2c's `s32 arg4/arg5` + `(s16)`-cast shape is
  byte-neutral on this body.* k1 listing is line-for-line identical to the candidate's; under
  a donation it is required (s16 params make cc1 emit `lhu` where the target emits `lw`).

## [s20] The residual of func_800480C0 is exactly a 32-byte get_frame_size() charge and contains no second defect.
- mechanism: integrate.c:2085-2092 charges assign_stack_temp (BLKmode, DECL_FRAME_SIZE (map->fndecl), 1) at every inline expansion and DECL_FRAME_SIZE is snapshotted pre-optimisation at integrate.c:345, so a static __inline__ helper carrying a 32-byte local donates the full block to this frame while constant propagation deletes all of its traffic on this call path; mips.c:4475 then totals the frame as var_size + args_size + gp_reg_rounded = 32 + 24 + 32 = 88.
- probe: Installed tmp/grind/func_800480C0/s20/bodies/h1_stmt_tail.c (the banked candidate body verbatim, with s32 arg4/arg5 + (s16) casts, plus static __inline__ s32 pack_off(s32 v, s32 n) whose u32 t[8] is written only under if (n != 0), invoked as the discarded statement pack_off(arg1, 0); after the loop) over the INCLUDE_ASM line and ran sandbox func_800480C0 --disable all.
- result: score 0, target_insns 74, build_insns 74, scorable true, rules_dropped 0. The raw cc1 listing s20/last_h1_stmt_tail.s differs from the candidate's s20/last_candidate.s in exactly one line: .frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24 versus .frame $sp,56,$31 # vars= 0. Donation ladder on this body: 16 bytes -> vars 16, 24 -> vars 24, 32 -> vars 32. Fifteen sessions of frame-decomposition inference are now a measurement.
- verdict: CONFIRMED

## [s20] A value-flowing call site for the inline-callee donation keeps this function's stream unperturbed.
- mechanism: The inlined expansion introduces a pseudo on the dataflow path at its call site, which changes sched.c's list-scheduling decisions for the surrounding block: at entry sites the two incoming-parameter loads lw $3,104($sp) / lw $4,108($sp) hoist above the eight callee-saved stores and a load-delay nop appears after lw $2,0($16); at loop sites the srl/sll/addu that forms new_var hoists to the loop head.
- probe: Measured ten placements of the same 32-byte helper (all reaching .frame $sp,88 # vars= 32) with tmp/grind/func_800480C0/s20/probe.sh plus the full scorer: entry shift (d3, f1), stack params sx_arg4/sx_arg5 (g8, g3), loop new_var (e6, d2), discarded call at top (e1), register params and loop values (f3, g1, g5, g6, g7), and the two discarded-statement sites (h1, h2).
- result: d3/f1/g8 sandbox 6 with build_insns 75; e6/d2 sandbox 16; e1 clusters all eight register saves; f3/g1/g6/g7 cost one cc1 insn (73 vs 72). Only h1 and h2 - the discarded call after the loop and at the end of the if-block - leave the listing bit-identical to the candidate apart from the .frame line.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a4c735da, bodies tmp/grind/func_800480C0/s20/bodies/*.c installed one at a time over the INCLUDE_ASM line, the base candidate's single FAKE (arg0 = 0;) present in every body

## [s20] The inline-callee donation family contains an honest spelling for this body.
- mechanism: Cheat-checklist T1/T2 applied to the only byte-neutral spellings: pack_off(arg1, 0); is a discarded call to a side-effect-free helper, so it has no observable effect on the function's output and no human programmer writes it; the helper's u32 t[8] is an object nothing on this path needs, i.e. the unwritten leading pad relocated into a callee, which is the construct the standing Judge constraint (docs/grind/decisions.md:20349) bars for this function in any spelling.
- probe: Enumerated the byte-neutral placements from the previous hypothesis and applied the six-test checklist to each; banked the score-0 body in memory/grind/func_800480C0/rejected/s20-inline-donation-dead-tail-call-bytes0-but-relocated-pad.c rather than proposing it.
- result: Both byte-neutral placements are discarded statements and fail T1 and T2 outright. Not proposed, not submitted. Recorded detector gap: engine/volatile_cheats.py::find_unused_local_arrays strips a fixed-size local array only when it is unreferenced inside its own function body, so t[8] referenced in the helper's dead if (n != 0) arm survives stripping and the cheat-invisible scorer printed 0 for a relocated pad; the control d1_cand_donate32.c with an UNREFERENCED t[8] is stripped and scores 20. The honest floor stays 20 and this session claims no improvement.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a4c735da, bodies h1_stmt_tail.c / h2_stmt_ifend.c / d1_cand_donate32.c installed one at a time over the INCLUDE_ASM line, candidate FAKE present

## [s20] m2c's rederive shape - s32 arg4/arg5 with (s16) casts at the two sign-extend sites - is byte-neutral on the banked candidate body.
- mechanism: The parameters are consumed only through a full-word sll/sra 16 pair, so widening the declared type and casting at the use site produces the same RTL; under a donation it is load-bearing because s16 stack parameters make cc1 select lhu $3,104($sp) / lhu $4,108($sp) where the target emits lw (this GCC fork's narrow-stack-parameter behaviour, .claude/rules/narrow-stack-param-subword-offset.md).
- probe: Built k1_cand_s32args45.c (candidate body, s32 arg4/arg5, sx_arg4 = (s16)arg4; sx_arg5 = (s16)arg5;) and diffed its cc1 listing against last_candidate.s; separately compared d3 (s16 params) with f1 (s32 params) under the donation.
- result: k1 is line-for-line identical to the candidate (zero diff, .frame $sp,56 # vars= 0, 72 insns). Under the donation, s16 params emit lhu and s32 params emit lw, worth two diffs. Kept out of candidate.c only because review verdicts are body-keyed and the banked body should stay unchanged.
- verdict: CONFIRMED


## [s21] A fresh m2c decompile, run without reference to twenty sessions of ledger, recovers a structurally different C shape for func_800480C0 than the one candidate.c spells
- mechanism: rederive modality leg 1. m2c reconstructs source shape from the shipped instruction stream alone, so if the candidate's shape were an artifact of accumulated ledger bias rather than of the bytes, an independent decompile would diverge - and a divergent shape could carry a frame object the candidate's shape has no room for.
- probe: `python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c -f func_800480C0 asm/funcs/func_800480C0.s` under WSL, output banked at `tmp/grind/func_800480C0/s21/m2c_fresh.txt`, compared statement by statement against `memory/grind/func_800480C0/candidate.c`.
- result: m2c returns the candidate's shape line for line - the same locate-prologue `arg0 + ((*(arg0 + ((arg1 << 0x10) >> 0xE)) >> 2) * 4)`, the same `temp_s1 = *temp_s0` count guard with the pre-decrement, the same four `M2C_FIELD(..., u16 *, ...)` record reads with the pointer advanced 4/2/2/2, and the same five-argument call to func_800482C8 with the four sign-extended addends. Its only spelling differences from candidate.c are cosmetic (it types the two 5th/6th stack parameters as s32 rather than s16, and it emits no `arg0 = 0` FAKE because it has no cse2 to defeat). It declares NO aggregate and no address-taken object. The recovered source has no representation of the 32-byte `vars` charge at all.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, target listing asm/funcs/func_800480C0.s, m2c run with no ledger input; candidate body installed over the INCLUDE_ASM line for the comparison, FAKE `arg0 = 0;` present in the candidate half of the comparison only.

## [s21] The inline-callee frame donation (integrate.c:2085-2092) has an honest carrier in src/text1b.c - a static or __inline__ helper this function has a real reason to call whose 25..32-byte local is genuinely consumed by a different caller in the same TU
- mechanism: s20's frontier item 1. integrate.c:2085-2092 charges `assign_stack_temp (BLKmode, DECL_FRAME_SIZE (map->fndecl), 1)` into the caller's frame for every expansion, snapshotted pre-optimisation at integrate.c:345, so a helper with a 32-byte local donates 32 bytes of `vars` to func_800480C0 even when the helper's own traffic optimises away here. For the donation to be honest rather than a relocated pad, the helper's local must be genuinely needed by at least one OTHER caller in the TU.
- probe: full census of src/text1b.c - `grep -nE "^\s*static\s+(__inline__\s+|inline\s+)?..."` for helper definitions across all 6,700+ lines and 583 function definitions, plus an aggregate census for any local of 25..32 bytes shared between func_800480C0 and another body in the file.
- result: src/text1b.c defines ZERO static functions and ZERO inline functions. Every `static` token in the file is either the struct field name `p_static` (lines 2840, 2888, 2988-3021, 6159) or prose inside a comment (line 323). The only 32-byte locals in the TU are `u8 sp30[32]` at lines 3881 and 3938, both private to unrelated bodies and neither shared with func_800480C0. The construct exists elsewhere in the project (`src/main.c:1069` `static inline void vmSetStartAddr`, `src/main.c:2187` `static inline void _memcpy`), so it is not foreign to the codebase - it is foreign to this translation unit. Any donation carrier for func_800480C0 would have to be a helper invented for this one function, with no second caller to make its 32-byte local genuine.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, src/text1b.c at HEAD (func_800480C0 represented as INCLUDE_ASM during the census), no FAKE constructs involved - this is a census of the source file, not a build.

## [s21] s20's kill of value-flowing donation call sites is a kill of the whole helper axis, so no inline helper can be added to this body without perturbing its 74-instruction stream
- mechanism: s20 measured eight value-flowing placements of a donation helper and found every one perturbed the stream (sandbox 6 at entry sites via hoisted incoming-parameter loads plus a load-delay nop, 16 at loop sites via a hoisted new_var srl/sll/addu, +1 cc1 insn at register-parameter sites), and recorded the conclusion that only a discarded-statement call site is byte-neutral. Every one of those helpers was an ADDITION to the body; none carried computation the body already performed.
- probe: factor computation the body ALREADY performs into `static __inline__` helpers and re-measure. r3 = `static __inline__ s32 rec_base(s32 base, u32 word) { return base + (((u32)word >> 2) << 2); }` substituted at both of its sites (the locate-prologue and the loop's `new_var`). r4 = `static __inline__ s32 sxadd(s16 v, s32 b) { return (s32)v + b; }` substituted at all four of the loop's sign-extend-and-add sites. Each installed over the INCLUDE_ASM line and scored with `sandbox func_800480C0 --disable all`; raw `.frame` read with `tmp/grind/func_800480C0/s21/frame.sh`.
- result: KILLED, and the correction is load-bearing for the frontier. r3 scores 20 at build_insns 74, rules_dropped 0. r4 scores 20 at build_insns 74, rules_dropped 0. r4's raw cc1 `.frame` line is `$sp,56,$31 # vars= 0, regs= 8/0, args= 24, extra= 0` - term for term identical to the candidate's. Honest, value-flowing, four-times-per-iteration inline helpers are FREE on this body. s20's kill bounds ADDED helpers only; it does not bound helpers that carry existing computation, and this function has at least two such fragments. The boundary is that the fragment must be a leaf expression: r1/r2 (the shared locate-prologue factored as `find_list(base, idx)`, with and without the candidate's `arg0 = 0;` FAKE) both build 73 instructions - one FEWER than the target - and score 25, because inlining the prologue lets GCC fold the `move $18,$16` base copy the target keeps.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, bodies tmp/grind/func_800480C0/s21/bodies/{r1_helper_prologue,r2_helper_prologue_nofake,r3_helper_recbase,r4_helper_sxadd}.c installed one at a time over the INCLUDE_ASM line; base candidate FAKE `arg0 = 0;` present in r1/r3/r4 and ablated in r2.

## [s21] Because a byte-neutral honest helper exists on this body, the integrate.c donation can buy the target's 32-byte vars charge through it without the pad
- mechanism: with r3/r4 established as byte-neutral carriers, integrate.c:2085-2092 should charge DECL_FRAME_SIZE of the helper into func_800480C0's frame at every expansion. If the helper's local can be made both genuinely used and traffic-free at this call site, the target's `.frame $sp,88 # vars= 32` is reachable with an honest construct and the Judge-banned pad is bypassed.
- probe: r5 = r4's `sxadd` plus `u32 t[8];`, r6 = r3's `rec_base` plus `u32 t[8];` (unreferenced carriers, to establish that the donation fires at all); then r7 = `sxadd` with `u32 t[8]; t[0] = (u32)(s32)v; return (s32)t[0] + b;` and r8 the same with `t[7]` (referenced carriers, to establish the traffic price); then r9/r10/r11 = the referenced carrier restricted to a single call site (a1v, a2v, a3v respectively). Each scored with `sandbox --disable all` and its raw `.frame` read with `tmp/grind/func_800480C0/s21/frame.sh`.
- result: KILLED on the mechanism, and the two halves of the disjunction are now measured rather than argued. (a) THE DONATION FIRES EXACTLY: r5 and r6 both make raw cc1 print `.frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24, extra= 0` for func_800480C0 - the target's exact frame line - with the 74-instruction stream untouched. (b) BUT AN UNREFERENCED CARRIER IS THE PAD AT ONE REMOVE: both score 20, not 0, because `engine/volatile_cheats.py::_LOCAL_ARRAY_DECL_RE` (2 <= N <= 64, name unreferenced in the body) recognises `u32 t[8]` as the frame-pad family and strips it before scoring. Relocating the pad into a helper does not make it honest and does not move the honest floor. (c) AND A REFERENCED CARRIER COSTS EXACTLY ONE INSTRUCTION THAT NO FOLD REMOVES: r7 and r8 both build 75 instructions and score 1, and the entire delta against the target is a single `sw $2,24($sp)` into sp+0x18, the first word of the target's untouched window. Three of the four inline sites fold their store/load pair away completely; the fourth cannot, because all four sites write the same frame address and each write kills the previous one, leaving the LAST write with nothing to kill it. r9/r10/r11 (single site) are worse still at 75 insns and score 5. THE SCORE OF 1 IS A DETECTOR GAP, NOT A FLOOR, AND IS NOT PROPOSED: `u32 t[8]` in which only one element is ever written or read is an eight-word aggregate holding one scalar - it fails cheat-checklist T1 (seven words with no semantic purpose), T2 (no programmer writes it) and T5 (`dead-vars-local-array`, whose 2026-07-01 carve-out requires the dead stores to appear in the TARGET bytes, and this store is precisely an instruction the target does not have), and it materialises a byte the target does not contain so it cannot match in any case. The reportable finding is that the sandbox's stripper keys on the array name being unreferenced, so one live element defeats it; a future session that lands on "score 1 with an array inside a helper" is looking at this gap and must not bank it as progress.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, bodies tmp/grind/func_800480C0/s21/bodies/{r5_sxadd_donate32,r6_recbase_donate32,r7_sxadd_t1write,r8_sxadd_t_last,r9_sxaddt_+,r10_sxaddt_+,r11_sxaddt_+}.c installed one at a time over the INCLUDE_ASM line; base candidate FAKE `arg0 = 0;` present in all seven.

## [s21] A fresh m2c decompile, run without reference to twenty sessions of ledger, recovers a structurally different C shape for func_800480C0 than the one candidate.c spells.
- mechanism: Rederive modality leg 1. m2c reconstructs source shape from the shipped instruction stream alone, so if the candidate's shape were an artifact of accumulated ledger bias rather than of the bytes, an independent decompile would diverge - and a divergent shape could carry a frame object the candidate's shape has no room for.
- probe: python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c -f func_800480C0 asm/funcs/func_800480C0.s under WSL, banked at tmp/grind/func_800480C0/s21/m2c_fresh.txt, compared statement by statement against memory/grind/func_800480C0/candidate.c.
- result: m2c returns the candidate's shape line for line: the same locate-prologue arg0 + ((*(arg0 + ((arg1 << 0x10) >> 0xE)) >> 2) * 4), the same count guard with the pre-decrement, the same four u16 record reads with the pointer advanced 4/2/2/2, the same five-argument call with four sign-extended addends. Its only differences are cosmetic (s32 rather than s16 for the two stack parameters; no arg0 = 0 FAKE, since m2c has no cse2 to defeat). It declares no aggregate and no address-taken object at all. The recovered source has no representation of the 32-byte vars charge.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, target listing asm/funcs/func_800480C0.s, m2c run with no ledger input; candidate body installed over the INCLUDE_ASM line for the comparison half, FAKE arg0 = 0; present there only.

## [s21] The inline-callee frame donation (integrate.c:2085-2092) has an honest carrier in src/text1b.c - a static or inline helper this function has a real reason to call, whose 25..32-byte local is genuinely consumed by a different caller in the same TU.
- mechanism: s20's frontier item 1. integrate.c:2085-2092 charges assign_stack_temp (BLKmode, DECL_FRAME_SIZE (map->fndecl), 1) into the caller's frame per expansion, snapshotted pre-optimisation at integrate.c:345, so a helper with a 32-byte local donates 32 bytes of vars even when its own traffic optimises away here. For the donation to be honest rather than a relocated pad, the local must be genuinely needed by at least one OTHER caller in the TU.
- probe: Full census of src/text1b.c: grep for every static / __inline__ / inline function definition across all 6,700+ lines and 583 function definitions, plus an aggregate census for any local of 25..32 bytes shared between func_800480C0 and another body in the file.
- result: src/text1b.c defines ZERO static functions and ZERO inline functions. Every 'static' token in the file is either the struct field name p_static (lines 2840, 2888, 2988-3021, 6159) or prose in a comment (line 323). The only 32-byte locals are u8 sp30[32] at lines 3881 and 3938, both private to unrelated bodies. The construct is not foreign to the project (src/main.c:1069 static inline void vmSetStartAddr, src/main.c:2187 static inline void _memcpy) but it is foreign to this translation unit, so any donation carrier here would be a helper invented for this one function with no second caller to make its 32-byte local genuine.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, src/text1b.c at HEAD with func_800480C0 represented as INCLUDE_ASM; a source census, no build and no FAKE constructs involved.

## [s21] s20's kill of value-flowing donation call sites is a kill of the whole helper axis, so no inline helper can be added to this body without perturbing its 74-instruction stream.
- mechanism: s20 measured eight value-flowing placements and found every one perturbed the stream (sandbox 6 at entry sites via hoisted parameter loads plus a load-delay nop, 16 at loop sites via a hoisted new_var srl/sll/addu, +1 cc1 insn at register-parameter sites), concluding only a discarded-statement site is byte-neutral. Every one of those helpers was an ADDITION to the body; none carried computation the body already performed.
- probe: Factor computation the body ALREADY performs into static __inline__ helpers and re-measure. r3 = rec_base(base, word) returning base + (((u32)word>>2)<<2), substituted at both of its sites (locate-prologue and the loop's new_var). r4 = sxadd(v, b) returning (s32)v + b, substituted at all four sign-extend-and-add sites. r1/r2 = the shared locate-prologue factored as find_list(base, idx), with and without the candidate's FAKE. Each installed over the INCLUDE_ASM line, scored with sandbox --disable all, raw .frame read with tmp/grind/func_800480C0/s21/frame.sh.
- result: KILLED, and the correction is load-bearing. r3 scores 20 at build_insns 74, rules_dropped 0. r4 scores 20 at build_insns 74, rules_dropped 0, and its raw cc1 line is .frame $sp,56,$31 # vars= 0, regs= 8/0, args= 24, extra= 0 - term for term identical to the candidate's. Honest, value-flowing, four-times-per-iteration inline helpers are free on this body; s20's kill bounds ADDED helpers only. The boundary is that the factored fragment must be a leaf expression: r1 and r2 both build 73 instructions (one FEWER than the target) and score 25, because inlining the prologue lets GCC fold the move $18,$16 base copy the target keeps.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, bodies tmp/grind/func_800480C0/s21/bodies/{r1_helper_prologue,r2_helper_prologue_nofake,r3_helper_recbase,r4_helper_sxadd}.c installed one at a time over the INCLUDE_ASM line; candidate FAKE arg0 = 0; present in r1/r3/r4 and ablated in r2.

## [s21] Because a byte-neutral honest helper exists on this body, the integrate.c donation can buy the target's 32-byte vars charge through it without the banned pad.
- mechanism: With r3/r4 established as byte-neutral carriers, integrate.c:2085-2092 charges DECL_FRAME_SIZE of the helper into func_800480C0's frame at every expansion. If the helper's local can be made both genuinely used and traffic-free at this call site, .frame $sp,88 # vars= 32 is reachable with an honest construct.
- probe: r5 = r4's sxadd plus an unreferenced u32 t[8]; r6 = r3's rec_base plus the same, to establish that the donation fires at all. Then r7 = sxadd with u32 t[8]; t[0] = (u32)(s32)v; return (s32)t[0] + b; and r8 the same on t[7], to price a referenced carrier. Then r9/r10/r11 = the referenced carrier restricted to a single call site (a1v, a2v, a3v). Each scored with sandbox --disable all and its raw .frame read with tmp/grind/func_800480C0/s21/frame.sh; r7's full listing read at tmp/grind/func_800480C0/s21/r7_sxadd_t1write.s.
- result: KILLED on the mechanism, with both halves of the disjunction measured. (a) The donation fires exactly: r5 and r6 both make raw cc1 print .frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24, extra= 0 for func_800480C0 - the target's exact frame line - with the 74-instruction stream untouched. (b) An unreferenced carrier is the pad at one remove: both score 20, not 0, because engine/volatile_cheats.py _LOCAL_ARRAY_DECL_RE (2 <= N <= 64, name unreferenced in the body) recognises u32 t[8] as the frame-pad family and strips it before scoring. (c) A referenced carrier costs exactly one instruction no fold removes: r7 and r8 both build 75 instructions and score 1, the entire delta being a single sw $2,24($sp) into sp+0x18 - the first word of the target's untouched window - because all four inline sites write the same frame address, each write kills the previous one, and the LAST write has nothing to kill it; three of the four store/load pairs fold away completely. r9/r10/r11 are worse at 75 insns and score 5. THE SCORE OF 1 IS A DETECTOR GAP, NOT A FLOOR, AND IS NOT PROPOSED: u32 t[8] with only one element ever written or read is an eight-word aggregate holding one scalar, failing cheat-checklist T1, T2 and T5 (dead-vars-local-array, whose carve-out requires the dead stores to appear in the TARGET bytes, and this store is precisely an instruction the target does not have), and it materialises a byte the target does not contain so it cannot match in any case. The reportable finding is that the stripper keys on the array name being unreferenced, so one live element defeats it.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 16834fcf, bodies tmp/grind/func_800480C0/s21/bodies/{r5_sxadd_donate32,r6_recbase_donate32,r7_sxadd_t1write,r8_sxadd_t_last,r9_sxaddt_+,r10_sxaddt_+,r11_sxaddt_+}.c installed one at a time over the INCLUDE_ASM line; candidate FAKE arg0 = 0; present in all seven.

## [s21] The candidate's single FAKE (arg0 = 0;) is load-bearing on the current chassis and masks no lever underneath it.
- mechanism: Mandated kill re-audit. A lever measured inert while a FAKE carrier occupies its target pseudo is not a kill, so the FAKE must be ablated on the current chassis before any new probe is trusted.
- probe: tools/fake_ablate.py returned ERR / bi None for BOTH variants this session, and did so on the s20 candidate too (git show HEAD:memory/grind/func_800480C0/candidate.c), so the failure is tool-side; the ablation was performed directly instead. tmp/grind/func_800480C0/s21/bodies/base_nofake.c is the candidate body with the single arg0 = 0; line deleted, installed over the INCLUDE_ASM line and scored with sandbox --disable all.
- result: CONFIRMED. keep-all scores 20 at build_insns 74; drop-1 scores 32 at build_insns 73. Identical to the s19 and s20 numbers - the eighth chassis on which this ablation has reproduced. The FAKE is load-bearing and masks no lever. Tool-health note banked for the next session: fake_ablate.py needs looking at before it is trusted again on this function.
- verdict: CONFIRMED
