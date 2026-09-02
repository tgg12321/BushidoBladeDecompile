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
