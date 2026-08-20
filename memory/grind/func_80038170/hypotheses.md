# Hypothesis ledger — func_80038170

## s1 (2026-07-28, recon)
- H1 CONFIRMED: phantom +8 frame closable in pure C via same-base pair inside the conditional (phantom-frame-slots-gcc272). Probe: cc1 .frame bisect matrix. Result: vars 8->16 == target; floor 14->5; reviewer PASS.
- H1a KILLED: named u16 (HImode) temp in halfword copy loop triggers the slot — vars stays 8.
- H1b KILLED: named-pointer staging of the pair (u8 *t = &D_8008F19C + s3*2) — kills the temp, vars 8.
- H2 CONFIRMED-then-review-FAILED: chained `s1=s2=s3=0;` flips init emission to s3,s2,s1 -> floor 1 (text-artifact-only). Cheat-reviewer FAIL (order-flip family). Banked rejected/chained-zeroing-order.c. -> ruling-request filed s1.
- H3 CONFIRMED: floor-1 residual is symbol-vs-addend text artifact (D_8008F19C+1 vs D_8008F19D); linked bytes identical (word-diff proof). Sandbox 0 unreachable pre-integration by construction.

## s2 (2026-07-28, recon — post-Judge-ruling)
- H4 KILLED: Judge-sanctioned separate-statement spelling alone reaches byte parity. Probe: raw .o word diff vs build/ reference (oracle bytes). Result: save order correct, but i-init `move a3,zero` scheduled late + `sw ra` displaced — 5 words off target; engine masked score (1) conceals it.
- H5 CONFIRMED: standalone `i = 0;` before the mask load (empty for-init) restores the target schedule — sched1 emission order tracks source statement order for the independent init cluster. Probe: raw word diff. Result: 141/141 words match; only the linker-identical reloc spelling remains. Layer-1 reviewer PASS.
- H6 CONFIRMED: remaining carriers (regfix.txt:1250 + prologue_config entry) now MANGLE the correct output — rules-applied sandbox 4 vs clean 1. Must be retired at integration before any rebuild.

## [s1] The Judge-sanctioned separate-statement spelling (s3=0; s2=0; s1=0; decl order unchanged) alone reaches byte parity
- mechanism: prologue init/save pair emission order + sched1 scheduling of the pre-loop init cluster
- probe: sandbox --disable all + raw .o word diff vs build/ reference (oracle bytes), tmp/grind/func_80038170/s1/judge_form_diffcheck.sh
- result: Save/init pair order correct (s0,s3,s2,s1,ra), BUT i-init `move a3,zero` schedules after the li/lui/lw mask cluster and sw ra displaces to slot 12 — 5 words off target; the engine masked metric conceals this (still prints 1)
- verdict: KILLED

## [s1] Standalone `i = 0;` before `mask = D_80106A50;` (empty for-init) restores the target schedule
- mechanism: GCC 2.7.2 sched1 emission order tracks source statement order for the independent pre-loop init cluster; for-init clause places the i-init RTL after the mask load, standalone statement places it before
- probe: edit src, sandbox --disable all, raw word diff vs build/ reference
- result: 141/141 insn words match the oracle stream; sole remaining .o-text diff is the linker-identical reloc spelling D_8008F19C+1 vs D_8008F19D (hi 0x8009 / lo 0xF19D both ways, re-proven vs asm/funcs words); layer-1 cheat-reviewer PASS (ordinary live-statement order, store-before-jal family)
- verdict: CONFIRMED

## [s1] The two remaining cheat carriers now actively mangle the correct natural output
- mechanism: regfix.txt:1250 reorder @9-13 permutes the already-correct natural prologue window
- probe: sandbox func_80038170 with rules applied (no --disable)
- result: score 4 with rules applied vs 1 clean — carriers are harmful; a full build with them active would break the oracle, so build/ regeneration (and hence sandbox 0) is impossible until they are retired on the driver surface
- verdict: CONFIRMED

## [s2] The banked Judge-sanctioned candidate (separate s3=0;s2=0;s1=0; + standalone i=0 hoist + one-table (&D_8008F19C)[s3*2+0/1] pair), re-applied verbatim to src/code6cac_c_mid.c, reproduces the s1 measurements exactly
- mechanism: src at session start still carried the OLD cheat form (register pins a3/a1, dummy0/dummy1 __asm__ m-constraint frame coercion, two-symbol D_8008F19C/D_8008F19D spelling, for-init i=0) — the prior session's src edits did not persist to HEAD; the ledger's integration-readiness claim was unverifiable against the tree until re-applied
- probe: Edit src to the banked candidate body; sandbox func_80038170 --disable all; word-level diff vs asm/funcs oracle words (tmp/grind/func_80038170/s1/word_diff.py); address-normalized 141-insn diff vs build/src reference .o (tmp/grind/func_80038170/s2/norm_diff.py)
- result: sandbox score 1 (141/141 insns, rules_dropped 1, cheat_asm_stripped 29 file-wide); word diff vs oracle stream: ONLY the pre-link jal func_80079194 reloc placeholder (0C000000 vs resolved 0C01E465); normalized diff vs stale build/ reference: 6 insns = 2 reloc-spelling words (lui/lbu D_8008F19C+1 vs D_8008F19D, proven linker-identical, both resolve to 3C018009/9022F19D) + 4 R_MIPS_26 j-words that differ only because the sandbox .o strips 29 cheat-asm instances file-wide shifting every function's section offset by 4 bytes (stale-reference artifact, vanishes on rebuild)
- verdict: CONFIRMED

## [s3] src at HEAD reverted to the old cheat form again; re-applying the Judge-sanctioned candidate verbatim reproduces the s2 floor exactly
- mechanism: grind-session src edits are reverted by the driver between sessions; the banked candidate.c is the only durable carrier of the form
- probe: Edit src/code6cac_c_mid.c to the banked candidate body; sandbox func_80038170 --disable all
- result: score 1, target_insns 141, build_insns 141, rules_dropped 1, cheat_asm_stripped 29 — byte-identical to the s2 measurement (artifact s3/sandbox_judge_form.txt, re-confirmed after probe revert in s3/sandbox_judge_form_final.txt)
- verdict: CONFIRMED

## [s3] Rebasing the one-symbol pair on D_8008F19D ((&D_8008F19D)[s3*2-1] / [s3*2]) — the only untested one-symbol spelling class — can zero the pre-link reloc diff and reach sandbox 0 pre-integration
- mechanism: the phantom-frame temp needs a shared-index pair off ONE symbol (s1 trigger matrix); the only two one-symbol bases covering bytes 0x8008F19C..D are D_8008F19C (+0/+1 addends) and D_8008F19D (-1/+0 addends); if the -1 addend happened to be masked differently the diff could vanish
- probe: temporary src edit to the D_8008F19D-rebased pair; sandbox func_80038170 --disable all
- result: score 1 at 141/141 — identical floor; the nonzero-addend artifact just moves from out[0x43] (+1) to out[0x42] (-1). Any one-symbol pair carries a nonzero addend on one access while the stale build/ reference has addend 0 on both; two-symbol spellings kill the frame temp (s1). Therefore NO source spelling reaches sandbox 0 before build/ regenerates. Banked rejected/d8008f19d-rebased-pair.c
- verdict: KILLED

## s4 (2026-08-19, permuter modality — pre-empted by chassis re-measure)

## [s4] The s3 "sandbox 0 unreachable by construction" deadlock is chassis-relative and has been dissolved by the asm-until-matched migration
- mechanism: the deadlock had two legs — (i) the build/ reference .o was the OLD cheat-form object, so our correct source differed from it by the linker-identical D_8008F19C+1 vs D_8008F19D reloc spelling, and (ii) regfix.txt:1250's `reorder @9-13` would MANGLE the now-correct natural prologue on any real build, so build/ could never be regenerated from the correct source without first retiring a rule no grind session may touch. Commit 4faaa384 retired that rule as part of the asm-until-matched migration and moved the function to INCLUDE_ASM, removing leg (ii) entirely; splicing the banked body over the INCLUDE_ASM line then makes leg (i) self-clearing on the next build.
- probe: splice the banked candidate body over `INCLUDE_ASM("asm/funcs", func_80038170);` in src/code6cac_c_mid.c; `sandbox func_80038170 --disable all`; `engine build`; re-run the sandbox
- result: pre-build sandbox 1 (the reloc-spelling artifact only, rules_dropped 0 — confirming the regfix carrier is gone); `engine build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH; post-build sandbox 0 at 141/141
- verdict: CONFIRMED

## [s4] The banked s3 candidate compiles and links as-is on the current chassis
- mechanism: n/a — assumed inheritance
- probe: `engine build` with the banked body verbatim
- result: KILLED — link failure `undefined reference to func_80079194`. The symbol does not exist; 0x80079194 is `strcpy` (asm/funcs/strcpy.s). Corrected to `strcpy(out + 4, D_8008F1C0)` using the prototype already at src/code6cac_c_mid.c:279. Also corrected two `&`-of-array index spellings to plain array indexing (`D_8008F204[i]`, `D_8008F1A8[...]`) to match their `extern u8 X[];` declarations in include/code6cac.h. All three edits are codegen-neutral; the oracle match is unaffected.
- verdict: KILLED

## [s4] The surviving tools/prologue_config.json func_80038170 entry is load-bearing for the match
- mechanism: prologue_fix replaces the emitted prologue window with a hardcoded instruction list; if the natural cc1 prologue differed, the entry would be silently carrying the match
- probe: `sandbox func_80038170 --disable all` (which STRIPS prologue_fix) against a build/ reference produced WITH prologue_fix active
- result: KILLED — score 0 with prologue_fix stripped. The natural prologue is textually identical to the hardcoded list, so the entry is a measured NO-OP and pure dead weight for `retire func_80038170` to delete.
- verdict: KILLED

## s4b (2026-08-19, synthesis — post layer-1 FAIL)

## [s4b] The layer-1-banned statement-order lever (standalone `i = 0;` + empty for-init) is REQUIRED for byte parity, as s2 concluded
- mechanism: s2's claim was that GCC 2.7.2 sched1 emits the i-init `move a3,zero` after the li/lui/lw mask cluster when `i = 0` sits in the for-init clause, displacing `sw ra` by two slots — 5 words off target
- probe: restore the ordinary `for (i = 0; i < 0x1B; i++)`, delete the standalone `i = 0;`, change nothing else; `engine build`; `sandbox func_80038170 --disable all`
- result: KILLED — build/bb2.exe sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH, sandbox score 0 at 141/141, rules_dropped 0. The s2 residual was a stale-reference artifact (build/ still held the cheat-form object), never a property of the compiled source. No scheduling lever of any kind is needed for this function.
- verdict: KILLED

## [s4b] The `(&D_8008F19C)[s3*2+n]` &-of-scalar spelling is load-bearing for the target's -0x38 frame
- mechanism: s1-s3 ledger asserted that the scalar declaration plus &-indexing was what kept the two table reads on ONE base and thereby allocated the phantom 8-byte compiler temp (vars 8 -> 16)
- probe: correct include/code6cac.h:80 to `extern u8 D_8008F19C[];` (symbol referenced only by this function) and write plain `D_8008F19C[s3*2+n]`; `engine build`; sandbox; canonical
- result: KILLED — sha1 == oracle MATCH, sandbox 0 at 141/141, canonical verdict C / asm_insns 0 / distance 0. Byte-identical codegen. The declared TYPE of the base is irrelevant; only SHARING ONE BASE across the two reads inside the `if (s3 > 0)` arm matters for the temp. The header correction is the honest declaration for the use sites and removes the out-of-bounds-index smell.
- verdict: KILLED

## [s4b] The Judge-bound form alone, with no added construct anywhere, reaches oracle MATCH and honest sandbox 0
- mechanism: `s32 s1, s2, s3;` declaration order unchanged + separate `s3 = 0; s2 = 0; s1 = 0;` produces the target save/init pair order s0,s3,s2,s1,ra naturally; the one-base conditional table pair produces the -0x38 frame naturally; everything else is ordinary live C
- probe: full `engine build` + `sandbox --disable all` + `canonical`, with the body in place in src/code6cac_c_mid.c
- result: CONFIRMED — sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH; sandbox score 0, 141/141, rules_dropped 0, scorable true; canonical verdict C, asm_insns 0, distance 0. Diff carries no dead local, no volatile, no asm, no pin, no rule, claims no sanctioned family, owes no annotation (self_vet.md)
- verdict: CONFIRMED

## [s4c] The s4b solved form reproduces from candidate.c on the current chassis, so the s4b discard was a paperwork failure and not a code failure
- mechanism: the driver reverts grind-session src edits between sessions, so candidate.c is the only durable carrier of the form; the discard came from the self-vet ban tripwire (2 content-word hits from `state.json judge_constraints[0] binding spelling`), which reads only the CONSTRUCTS: block and never looks at the C at all
- probe: splice candidate.c body over the INCLUDE_ASM line + apply the include/code6cac.h:80 array-declaration fix; then sandbox --disable all, engine build, canonical; then rerun grindlib.check_banned_constructs against the rewritten self_vet.md
- result: sandbox score 0 at 141/141 with rules_dropped 0; full build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH; canonical verdict C / asm_insns 0 / distance 0; check_banned_constructs -> (True, '')
- verdict: CONFIRMED

## [s4c] FRONTIER RESET (synthesis): no search axis remains open on this function
- mechanism: every hypothesis on the s1-s3 frontier has been resolved by measurement — frame gap (one-base indexing allocates the 8-byte compiler temp), prologue pair order (natural under the declared order plus three separate zeroing statements), the claimed 5-word scheduling residual (KILLED as a stale-reference artifact), the reloc-addend deadlock (chassis-relative; dissolved by the asm-until-matched migration), and the func_80079194 link failure (it is strcpy)
- probe: full re-read of evidence.md + hypotheses.md + rejected/ against a fresh measurement of the banked form
- result: floor 0 with zero rules and zero cheat-asm; the function is byte-matched by natural compilation and the remaining work is acceptance review, not search
- verdict: CONFIRMED

## s4d (2026-08-19, synthesis — re-dispatch)

## [s4d] The banked solved form still reaches oracle MATCH + honest sandbox 0 on the chassis as it stands at this dispatch (i.e. the s4b/s4c results were not chassis-luck)
- mechanism: nothing in the tree that this function depends on has moved since s4c — no regfix/asmfix rule exists for it (rules_dropped 0), the body is the natural-compilation form, and the only companion edit is a header declaration correction
- probe: re-splice candidate.c over the INCLUDE_ASM line + re-apply the include/code6cac.h:80 array declaration; `sandbox func_80038170 --disable all`; `engine build`; `canonical func_80038170`
- result: CONFIRMED — sandbox score 0, 141/141, rules_dropped 0, scorable true; build/bb2.exe sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH; canonical verdict C / asm_insns 0 / distance 0
- verdict: CONFIRMED

## [s4d] FRONTIER (reset, synthesis): the only remaining work on func_80038170 is acceptance, not search
- mechanism: all three codegen mechanisms are settled and measured (one-base conditional table pair -> the 8-byte compiler temp / -0x38 frame; declared order plus three separate zeroing statements -> the natural save/init pair order; no scheduling construct needed at all), and the honest floor is 0 with zero rules and zero cheat-asm
- probe: layer-1 cheat-reviewer on the diff, then the Judge; on PASS the operator runs `retire func_80038170` (deletes the no-op tools/prologue_config.json entry) and `queue done func_80038170`
- result: pending review — no measurement left to take
- verdict: CONFIRMED (as a frontier statement: search space on this function is empty)

## s5 (2026-08-19, synthesis — post out-of-scope ruling)

## [s5] The companion include/code6cac.h retype is required to reach the solved bytes, so the driver's out-of-scope ruling blocks the solution
- mechanism: s1-s3 asserted the shared-base spelling was load-bearing for the -0x38 frame, and s4b-s4d expressed that base as an array via a one-line header declaration correction; if the frame really depended on the declaration, no src-only form could exist
- probe: revert include/code6cac.h to the unmodified `extern u8 D_8008F19C;` and write the two conditional table reads as `(&D_8008F19C)[s3 * 2 + 0]` / `[s3 * 2 + 1]`, changing nothing else in the s4d body; then `sandbox func_80038170 --disable all`, `engine build`, `canonical func_80038170`
- result: KILLED — sandbox score 0 at 141/141 with rules_dropped 0; build/bb2.exe sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle MATCH; canonical verdict C / asm_insns 0 / distance 0. The header edit was never needed for the bytes; it was a readability change that happened to be out of scope. Only src/code6cac_c_mid.c is touched by the candidate. This is consistent with (and is the other direction of) the s4b kill that the declared type of the shared base is irrelevant — what allocates the compiler temp is sharing ONE base across the two reads inside the conditional arm.
- verdict: KILLED

## [s5] FRONTIER (synthesis reset): search space on func_80038170 is empty; the only remaining work is acceptance
- mechanism: all three codegen mechanisms are settled and measured (one-base conditional table pair -> the 8-byte compiler temp and the -0x38 frame; declaration order plus three separate zeroing statements -> the natural save/init pair order; no scheduling construct at all), the honest floor is 0, there are zero regfix/asmfix rules and zero cheat-asm, no sanctioned family is claimed and no annotation is owed, and the candidate is confined to the one in-scope file
- probe: layer-1 cheat-reviewer on the src-only diff, then the Judge; on PASS the operator runs `retire func_80038170` (deletes the measured-no-op tools/prologue_config.json entry) and `queue done func_80038170`
- result: pending review — no measurement left to take on this function
- verdict: CONFIRMED (as a frontier statement)

## [s4 — forensics, 2026-08-19]

- **H-s4a — CONFIRMED.** The target's `vars= 16` (frame 56) is produced by a reload
  spill slot allocated for a pseudo left SET-less by combine's address fold, and
  that pseudo exists ONLY when the second table read carries a source-level `+1`
  addend on the same symbol.
  - mechanism: expand rejects `reg + CONST(PLUS(sym,1))` as a MIPS address and
    forces it into pseudo 120; combine folds it back into the mem and leaves
    `(use (reg:SI 120))`; global.c forms no allocno (no SET); reload1.c:2403
    `alter_reg` calls `assign_stack_local(..., total_size=8, -1)`; frame_offset
    8 -> 16. No spill insn is ever emitted.
  - probe: instrumented cc1 `BB2_FRAME_DEBUG=1` frame census + `-da` dumps for both
    spellings; asm diff of the two variants.
  - result: one-base `ctx=spill_new_p120 ... frame_offset=16`, `.frame $sp,56 vars=16`,
    sandbox 0; two-base has no p120 slot, `.frame $sp,48 vars=8`, sandbox 13. The two
    bodies differ in exactly one instruction operand.

- **H-s4b — KILLED.** "The two-symbol (D_8008F19C / D_8008F19D) program model can
  still reach the target." It cannot: it provably cannot produce `vars= 16`, so it
  cannot produce the target prologue. Banked as
  rejected/two-base-no-frame-temp.c.

- **H-s4c — KILLED.** "Floor 1 is a reloc-addend artifact that is unreachable-by-
  construction until the operator retires regfix.txt:1250 + the prologue_config.json
  entry" (the s1–s3 frontier). Chassis-relative and now false: after the
  asm-until-matched migration HEAD carries `INCLUDE_ASM` for this function, neither
  carrier exists, and the one-base form measures sandbox **0** directly. The whole
  "circular integration gate" framing is retired.

- **H-s4d — CONFIRMED.** The correct fix is a data-model correction, not a codegen
  construct: `include/code6cac.h:80-81` must declare `extern u8 D_8008F19C[];`
  (absorbing the dead per-byte `D_8008F19D` auto-name), after which the function
  body is plain C with no construct of any kind. Proven end to end: sandbox 0 AND
  full-build SHA1 == oracle in this session.

- **FRONTIER — the only thing left is scope.** The header is out of scope for this
  function's candidates (judge_constraints[2]) and the src-only pointer-pun spelling
  of the same reads is a banned construct (banned_constructs[2], layer-1 FAIL
  2026-08-19 21:23). There is no third in-scope spelling: with `D_8008F19C` declared
  as a scalar, every way to reach byte `+s3*2` is a pointer pun. Filed as an
  INTEGRATION HANDOFF / scope-widening request in docs/grind/decisions.md
  (2026-08-19) and returned `owner-gated`.

## [s4] The target's vars=16 (frame 0x38) comes from a reload spill slot allocated for a pseudo that combine left SET-less, and that pseudo exists ONLY when the second table read carries a source-level +1 addend on the same symbol.
- mechanism: expand rejects `reg + CONST(PLUS(symbol_ref,1))` as a legal MIPS address, so memory_address() forces the sum into pseudo 120 (.rtl insn 238); combine's try_combine folds the address back into the mem operand and leaves a bare `(insn 439 (use (reg:SI 120)))`; global.c forms no allocno for a SET-less pseudo so reg_renumber[120] stays < 0; reload1.c:2403 alter_reg's from_reg == -1 arm calls assign_stack_local(mode, total_size=8, -1), moving frame_offset 8 -> 16. No spill store or load is ever emitted — the slot is pure reservation.
- probe: instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_FRAME_DEBUG=1 frame-slot census for both spellings, plus canonical -da pass dumps (.rtl/.cse/.flow/.combine/.greg) and a full asm text diff of the two variants
- result: one-base: FRAMEDBG ctx=spill_new_p98 frame_offset=8 THEN ctx=spill_new_p120 size=8 frame_offset=16; `.frame $sp,56 # vars= 16, regs= 5/0, args= 16` = target; sandbox 0. two-base: only ctx=spill_new_p98, `.frame $sp,48 # vars= 8`; sandbox 13. The two emitted bodies differ in exactly one instruction operand (lbu $2,D_8008F19C+1($3) vs lbu $2,D_8008F19D($3)) plus the frame size and the five save/restore offsets it shifts.
- verdict: CONFIRMED

## [s4] The two-symbol program model (D_8008F19C and D_8008F19D as separate bases, the shape the scalar header invites and the pre-migration cheat form used) can still reach the target.
- mechanism: a bare SYMBOL_REF is already a legal `lbu sym($r)` address operand, so no address pseudo is created, no combine residue exists, and reload never reserves the second 8-byte slot
- probe: spliced the two-base form into src/code6cac_c_mid.c, read cc1's .frame comment and ran sandbox func_80038170 --disable all
- result: .frame $sp,48 # vars= 8 (target needs 16); sandbox 13. It provably cannot produce the target prologue. Banked at memory/grind/func_80038170/rejected/two-base-no-frame-temp.c.
- verdict: KILLED

## [s4] The floor-1 residual is a reloc-addend artifact that is unreachable-by-construction until the operator retires regfix.txt:1250 and the tools/prologue_config.json entry (the entire s1-s3 frontier and the 2026-07-28 OWNER-ESCALATION premise).
- mechanism: s1-s3 believed the sandbox reference object was the stale two-symbol cheat-form .o whose per-symbol addend is 0, so %lo(D_8008F19C+1) could never compare equal
- probe: re-measured on the current chassis after the 2026-08-19 asm-until-matched migration, with the one-base form spliced into src
- result: sandbox --disable all = 0, 141/141. HEAD now carries INCLUDE_ASM for this function; neither named carrier exists any more. The premise is chassis-relative and false; the circular-integration-gate framing is retired.
- verdict: KILLED

## [s4] The correct fix is a data-model (header) correction, after which the function body is plain C claiming no sanctioned family and owing no annotation.
- mechanism: include/code6cac.h:80-81 `extern u8 D_8008F19C; extern u8 D_8008F19D;` -> `extern u8 D_8008F19C[];`, letting the body read D_8008F19C[s3*2+0] / [s3*2+1] with no pointer pun
- probe: applied the header one-liner plus the plain-C body, then sandbox func_80038170 --disable all and a full build
- result: sandbox 0 (141/141) and full build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE, MATCH. Both files reverted to HEAD before session end; tree clean.
- verdict: CONFIRMED

## [s5] With include/code6cac.h granted in scope, the banked s4 form lands as an ordinary candidate-ready with no construct of any kind and no residual floor.
- mechanism: apply candidate.c's body over src/code6cac_c_mid.c:281 plus the single header declaration correction; both paths are scope-checked AND staged by the driver per tools/grinder/scope_allow.txt:26, so the committed tree is the byte-verified tree
- probe: applied both edits to the working tree, ran `build` (SHA1 vs oracle) and `sandbox func_80038170 --disable all`, re-ran the -da dumps to confirm the s4 frame mechanism still holds for the submitted form, and validated self_vet.md against grindlib's own validate_self_vet / _ban_trips
- result: full build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE; sandbox 0 (141/141, rules_dropped 0); .frame $sp,56 vars=16 as required; vet validates clean and trips no ban. Edits left in place in the tree.
- verdict: CONFIRMED

## [s5] The s1-s3 "floor 1 is a reloc-spelling artifact we cannot dissolve" residual is a stale-build/-reference measurement artifact, dissolved by ordering, not by any source property.
- mechanism: engine sandbox scores the fresh .o against reference objects in build/; when the candidate changes WHICH relocation a load uses, a build/ predating the edit makes exactly that one word compare unequal
- probe: sandbox immediately after applying the edits (no intervening build), then `build`, then the identical sandbox command again, with zero source changes between the two sandbox runs
- result: 1 -> (build, SHA1 == oracle) -> 0. Same source, same command, different reference freshness. Three sessions' worth of "unreachable by construction" framing traces to this.
- verdict: CONFIRMED


## [s5] rederive (2026-08-20)

- **H-s5-1: a structurally different declaration shape (`extern u8 D_8008F19C[][2];`
  + `D_8008F19C[s3][0]` / `[s3][1]`) reproduces the target while stating the
  Shift-JIS 2-byte-character data model explicitly.**
  - mechanism probed: whether GCC 2.7.2 still produces the `symbol_ref + 1`
    address-fold residue (and therefore the 8-byte phantom frame slot) when the
    two bytes are addressed as row/column of a 2-D array instead of as
    `base[i*2]` / `base[i*2+1]`.
  - probe: applied both edits, full `build`, byte-diffed the linked image against
    `disc/SLUS_006.63`, then re-ran `sandbox --disable all` against a clean
    `build/`.
  - result: SHA1 `e3ae7aae...` != oracle; 12 differing bytes, all frame words
    (`-0x30` vs `-0x38`, all save offsets 8 lower); clean-reference sandbox 12.
    GCC forms the row address in a pseudo and uses displacements 0/1, so no const
    addend, no combine fold residue, no alter_reg slot, vars= 8.
  - verdict: **KILLED**. Banked as
    `memory/grind/func_80038170/rejected/2d-array-index-no-addend.c`.

- **H-s5-2: the `extern u8 D_8008F19C[];` header correction has evidence
  independent of codegen, contrary to the 2026-08-20 layer-1 FAIL.**
  - mechanism: the claim is a data-model claim, so it must be settled from the
    shipped DATA bytes and from sibling declarations, not from frame sizes.
  - probe: dumped 0x8008F190-0x8008F1CF out of `disc/SLUS_006.63`; decoded as
    Shift-JIS; compared against the already-array-declared sibling D_8008F1A8.
  - result: 0x8008F19C holds `82 4F 82 50 82 51 82 52 82 53 00 00` = the
    full-width digits 0-4 + 2-byte terminator -- five 2-byte characters, stride
    2; D_8008F1A8 is the same construction with ten entries and is already
    `extern u8 D_8008F1A8[];`. splat's `D_8008F19D` names the LOW BYTE of the
    first character.
  - verdict: **CONFIRMED** (data-content proof, zero codegen reasoning).

- **H-s5-3: `sandbox --disable all` run AFTER a full `build` with the candidate
  applied is self-referential and returns a false 0.**
  - mechanism: the sandbox scores the fresh `.o` against the reference objects in
    `build/`; a full build overwrites those references with the candidate's own
    output.
  - probe: the same 2-D source measured both ways in the same session.
  - result: build-then-sandbox = 0 while the linked image was 12 bytes wrong;
    sandbox-against-clean-reference = 12.
  - verdict: **CONFIRMED**. The s4 candidate.c "apply -> build -> sandbox" recipe
    is the false-zero recipe and must not be followed; only the full-build SHA1
    is trustworthy after a candidate build.

## s6 (2026-08-20, rederive)

- **H-s6-1: the `scope_allow.txt` grant for `undefined_syms_auto.txt` lets a
  session make the prong-(c) deletion.**
  - mechanism: the 2026-08-19/2026-08-20 integration-handoffs wrote
    `func_80038170 include/code6cac.h undefined_syms_auto.txt` into
    `tools/grinder/scope_allow.txt:27`, which the driver documents as "BOTH
    allowed by the candidate scope check AND staged into the Match commit".
  - probe: read the driver. `Get-ExtraScope` (grind.ps1:544) is called only from
    `Invoke-CandidatePath`; the *session* scope check (grind.ps1:987) tests the
    whole dirty tree against the hard-coded literal `$AllowedDirtyPattern`
    (grind.ps1:880 — `memory/grind/|docs/grind/|tmp/|metrics/events.jsonl|src/|include/`)
    and never reads `scope_allow.txt`. Corroborated empirically: the previous
    session was discarded with "SCOPE VIOLATION … ( M undefined_syms_auto.txt)"
    WITH the grant already in place.
  - verdict: **KILLED**. No grind session can ever satisfy prong (c); a
    scope_allow grant for a path outside `src/` / `include/` is inert. Only an
    operator (or a driver change to grind.ps1:880) can land it.

- **H-s6-2: with a truly pristine `build/` reference, the banked candidate
  scores sandbox 0 (s4's claim).**
  - mechanism: s4 reported 0; s5 discovered the false-zero effect but measured it
    on the *rejected* 2-D form, so the correct form's honest score was never
    taken against a clean reference.
  - probe: full `build` at pristine HEAD (INCLUDE_ASM form → reference objects ==
    oracle objects), then apply the candidate, then `sandbox --disable all`.
  - result: **score 1** (141/141 insns, rules_dropped 0), then full `build` →
    SHA1 == ORACLE.
  - verdict: **KILLED**. The honest floor is 1 and structurally cannot be 0: the
    scored instruction is the `D_8008F19C+1` vs `D_8008F19D` R_MIPS_LO16 addend
    spelling, which `engine/score.py` does not mask
    ([[sandbox-lo16-text-addend-false-distance]]) and which vanishes at link
    time. The driver's candidate gate (`"score": 0` required) is therefore
    unpassable for this function by honest means — a second, independent
    mechanical deadlock on top of H-s6-1.

- **H-s6-3: deleting `undefined_syms_auto.txt:46` (prong (c)) perturbs the
  image.**
  - mechanism: `D_8008F19D = 0x8008F19D;` is a linker symbol assignment; if
    anything still referenced it, or if it contributed to layout, the SHA1 would
    move.
  - probe: candidate applied + line 46 deleted → full `build`.
  - result: SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == ORACLE, MATCH.
  - verdict: **KILLED** (the perturbation does not happen). The prong-(c) tree is
    the same image; the operator step is pre-verified and banked as
    `memory/grind/func_80038170/integration_patch.diff`.

## [s5] The tools/grinder/scope_allow.txt grant for undefined_syms_auto.txt lets a grind session make the prong-(c) deletion the 2026-08-20 Judge constraint demands.
- mechanism: The 2026-08-19/2026-08-20 integration-handoffs wrote `func_80038170 include/code6cac.h undefined_syms_auto.txt` into tools/grinder/scope_allow.txt:27, which that file's own header documents as 'BOTH allowed by the candidate scope check AND staged into the Match commit'.
- probe: Read the driver: Get-ExtraScope (tools/grinder/grind.ps1:544) is called only from Invoke-CandidatePath, whose file filter is src/|include/ only; the SESSION scope check (grind.ps1:987) runs first over the whole dirty tree against the hard-coded literal $AllowedDirtyPattern at grind.ps1:880 (memory/grind/|docs/grind/|tmp/|metrics/events.jsonl|src/|include/) and never reads scope_allow.txt. Corroborated empirically: the previous session was discarded with 'SCOPE VIOLATION ... ( M undefined_syms_auto.txt)' WITH the grant already in place.
- result: No grind session can leave that edit in its tree; a scope_allow.txt grant for any path outside src/ or include/ is inert. Prong (c) of the aggregate-merge family is mechanically unsatisfiable from inside the pipeline.
- verdict: KILLED

## [s5] With a truly pristine build/ reference, the banked candidate scores sandbox 0 (the s4 claim the ledger floor of 0 rests on).
- mechanism: s4 reported 0; s5 discovered the false-zero effect but measured it on the REJECTED 2-D form, so the correct form's honest score against a clean reference had never been taken.
- probe: full `build` at pristine HEAD (INCLUDE_ASM form, so build/'s reference objects are the oracle's own objects) -> apply the banked body + the include/code6cac.h array declaration -> `sandbox func_80038170 --disable all` -> then full `build`.
- result: sandbox score = 1 (target_insns 141, build_insns 141, rules_dropped 0); the subsequent full build gives SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE. The one scored instruction is our `lbu $2,D_8008F19C+1($3)` vs the target .o's `lbu $2,D_8008F19D($3)` - the same address, an R_MIPS_LO16 addend spelling that engine/score.py does not mask.
- verdict: KILLED

## [s5] Deleting undefined_syms_auto.txt:46 (`D_8008F19D = 0x8008F19D;`), the prong-(c) completeness step, perturbs the linked image.
- mechanism: It is an absolute linker-symbol assignment; a surviving reference or any layout contribution would move the SHA1.
- probe: candidate body + header correction + line 46 deleted -> full `build`.
- result: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE, MATCH. The deletion is byte-neutral; the operator step is pre-verified and the exact three-file tree is banked as memory/grind/func_80038170/integration_patch.diff.
- verdict: KILLED

## [s5] The rederive modality can find a structurally different C shape for the two odd bytes that avoids the header correction.
- mechanism: A different declaration/indexing shape (record table, row indexing, TU-local declaration, sibling-table rebasing) might reach the same bytes without touching include/code6cac.h.
- probe: Enumerated against the ledger's banked measurements: `[][2]` / struct-row spellings build a row address and never form the `symbol_ref + 1` const addend the target's `lbu D_8008F19C+1($3)` requires (rejected/2d-array-index-no-addend.c, SHA1 e3ae7aae..., frame 48 vs the target's 56); the TU-local `(&D_8008F19C)[...]` pointer pun is banned_constructs[2]; a TU-local array declaration is refused by prong (d) of the aggregate-merge entry.
- result: The flat `extern u8 D_8008F19C[];` + `[s3*2+0]`/`[s3*2+1]` spelling - identical to how this same function already reads its sibling table D_8008F1A8[], declared that way at include/code6cac.h:81 - is the unique surviving shape. The rederive axis is closed; the body is final.
- verdict: KILLED
