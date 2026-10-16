# Hypothesis ledger — func_80052B7C

> COMPACTED 2026-10-15 by `grindlib.py compact-ledger` (70,608 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 5f997f74d:memory/grind/func_80052B7C/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
> Look one up only when its index line bears on your probe. Append below as usual.

## s1 (recon)

### H1 — Four of five chunks land as direct PsyQ macro calls; chunk 2 is the open question

Statement: `func_80052B7C(MATRIX *rot, SVECTOR *bias, VECTOR *v0, VECTOR *out)` { `#include
<psxsdk/inline_o.h>` in this TU; `gte_SetRotMatrix(rot); <chunk-2 for bias>; gte_ldv0(v0);
gte_rt(); gte_stlvl(out); }` reproduces the 24-insn target, where chunk 2 is either (a) a
small written-out `__asm__` island (3x `lh` + 3x `ctc2` into CR5-7) citing the
`inline_asm_canonical.txt:340` grant directly (no macro exists to call), or (b) resolved by
an owner `ruling-request` on whether "no vendored macro exists for this op" is itself grounds
for the written-out island exception that item 3 of the 2026-10-15 GTE-macro-calls section
otherwise forbids.
Mechanism: not a codegen/GCC-internals question at all — this is a byte-for-byte,
instruction-for-instruction reproduction of an already-authorized whole-function asm leaf;
the only live question is which LANDING FORM (call vs. written island) the current policy
text permits for the one macro-less chunk.
Probe (for the next, construct-modality session): write the draft above into
src/main/3AB48.c (removing the `INCLUDE_ASM` line), add `#include <psxsdk/inline_o.h>`,
run `sandbox func_80052B7C --disable all` and `--diff`. If distance is not 0, the risk is
INSTRUCTION ORDER: GCC may not schedule the macro calls (each is its own set of `volatile`
asm statements) in the exact original sequence (loads-then-ctc2-batched vs. the macros'
… (preamble truncated — full text in git, above)

## Current state (at compaction)
floor 19 at s11; 11 sessions; live frontier (state.json is authoritative):
  - A scan_hand_coded gate-class signal for 'cop2-class insn in the jr $ra delay slot' lifts func_80052B7C to STRONG. The s11 census shows 5/5 hits are the GTE cluster, 4 of them already granted, with 0 compiled-C false pos…

## Class-level kills (binding until the cited predicate changes)

## [s3] A written __asm__ island reproducing or splitting an existing (or nonexistent) vendored GTE macro's body is admissible under the inline_asm_canonical.txt:340 grant when the macro itself cannot reach the target bytes (chunks 1 and 5) or no macro exists at all (chunk 2).
- verdict: KILLED
- kill_scope: class
- predicate_cite: .claude/rules/inline-asm-policy.md:55
- result: FAIL: 'the macro can't reach the bytes' is not an exception anywhere in the rule; the written chunk-2 island is itself a new written island and not precedent; the grant is not a licence for islands inside a C body.

## [s5] func_80052B7C's legal C chassis contains a region of ordinary C (named locals, assignments, or commutative sub-expressions) that tools/spelling_enum.py/sweep_variants.py can exhaustively sweep to close the residual.
- verdict: KILLED
- kill_scope: class
- predicate_cite: .claude/rules/inline-asm-policy.md:55
- result: False. candidate.c is a pure 72-line comment block with no function definition (sandbox --candidate errors 'func_80052B7C not found in ...3AB48.o'), by design -- its own header states no src/ edit is banked because chunks 1, 2 and 5 have zero legal C landing form under the current ruling (s3/H4 class kill on written islands) and chunks 3/4's legal form (gte_ldv0(v0); gte_rt(); via inline_c.h) is…

## [s7] Some C body compiled by GCC 2.7.2 places the final swc2 $11 (GTE store) in the jr $ra delay slot.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/reorg.c:740
- result: no C body can fill the slot with the GTE store; chunk 5's 2 hunks are closed for any C body. Only the hand-written (whole-body asm grant) branch reaches it.

## [s7] A C body compiled by GCC 2.7.2 can place the final GTE store swc2 $11 in the jr $ra delay slot.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/reorg.c:740
- result: The delay-slot GTE store is never produced by GCC 2.7.2 from C; chunk 5 (diff hunks 3-4) is only reachable by the whole-body hand-written branch.

## [s8] No block-local var split / declaration-order / type-narrowing / statement-reassociation lever on the legal macro-call chassis can touch chunk 1 or chunk 5's residual, because both chunks' divergent bytes are emitted entirely from literal, hardcoded __asm__ volatile template strings baked into the vendored macro defini…
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:297
- result: score 19 (target/build both 24 insns), 4/4 hunks source-level, 0 operand-only, 0 not-scored — identical classification to s6/s7. Hunk 1: target batches 5 lw then 8 ctc2 across the whole chunk 1+2 span; ours interleaves per the fixed macro bodies. Hunks 2-4: target's final swc2 sits in the jr delay slot; ours emits an explicit nop after all 3 stores.

## [s9] spelling_enum.py's region scan finds zero mutation axes (named locals, assignments, commutative swaps) on the s6-s8 legal macro-call chassis, re-confirming s5's finding at the current floor-19 body.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/spelling_enum.py:27
- result: Both runs: 'region: 0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings' (identity output only). No sweep was run against the sandbox score since a single-variant output has no alternatives.

## [s10] The residual's first divergence is RA, and GCC 2.7.2's allocator does not assign target's register disposition (8 data GPRs in $8-$15 with v0/v1 unused) to allocated pseudos in this function; those registers come only from a fixed asm string, and no vendored macro names lw $8-$11.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2249
- result: classify: FIRST DIVERGENCE RA (blanked multisets equal). Probe: lh $8,$3,$2 + lw $9..$13 (v0,v1,t0..t5, lh-first) vs target lw t0..t4 then lh t5..t7. Census 0 hits. Legal chassis re-measured 19, 4/4 hunks source-level.

## [s11] Chunk 5's divergence (target swc2 $11 in the jr $ra slot, ours jr+nop) is produced by maspsx, which appends a nop to every unfilled reorder-mode jump and has no path that swaps a preceding instruction into the slot; cc1 only leaves the j $31 unfilled after the asm block.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/maspsx/maspsx/__init__.py:1484
- result: maspsx_out.s shows `j $31` followed by `nop # DEBUG: branch/jump`, and objdump of the result reproduces our sandbox body word for word. mvmva is a `.word 0x4A480012`, so maspsx adds no hazard nop after it, which accounts for the second hunk. Sandbox still measures 19 with 4/4 hunks source-level.

## Confirmed levers

## [s2] func_80052B7C(MATRIX *rot, SVECTOR *tr, VECTOR *v0, VECTOR *out) using gte_SetRotMatrix/gte_ldv0/gte_rt/gte_stlvl from psxsdk/inline_c.h plus a written __asm__ island for the macro-less TR-halfword chunk reaches sandbox distance 0.
- verdict: CONFIRMED
- result: Score dropped from floor 24 to 16; build_insns now equals target_insns (24). Confirmed for chunks 3 (gte_ldv0) and 4 (gte_rt), which match target byte-for-byte with inline_c.h. Did NOT reach distance 0: chunks 1 and 5 still diverge in instruction order / delay-slot placement. Reverted src to INCLUDE_ASM per asm-until-matched rule 1; full form banked in candidate.c.

## [s6] gte_ldtr (inline_c.h:371), a register-operand macro, lands chunk 2 (lh x3 + ctc2 $5-$7) with the halfword loads as ordinary C, giving a pure vendored-macro body with no __asm__.
- verdict: CONFIRMED
- result: score 19 (24/24 insns); chunk 2 emits lh+ctc2 $5-7 (regs a0/v1/v0 inside hunk 1); without casts lhu, also 19. Legal floor 19 vs stub 24.

## [s6] Kill re-audit: the s2 instance kills (gte_SetRotMatrix interleaved order; gte_stlvl final swc2 not in jr delay slot) were measured with no FAKE carrier and still hold on the s6 chassis.
- verdict: CONFIRMED
- result: No FAKE constructs to ablate; hunk 1 (lw/ctc2 interleave) and hunk 4 (nop in jr slot) persist.

## [s11] The cop2-class-instruction-in-jr-$ra-delay-slot signature occurs in the oracle EXE only in the contiguous 3AB48 GTE cluster (5 functions including func_80052B7C), never in compiled C, and the other 4 cluster members already hold canonical-asm grants citing it.
- verdict: CONFIRMED
- result: There are exactly 5 slot sites: 0x80052A80, 0x80052AF8, 0x80052B3C, 0x80052B74 and 0x80052BDC (func_80052B7C). The 21 cop2/swc2-then-nop sites are Sony libgte plus func_80052C4C, and 1134 other jr sites have a nop slot. The siblings are granted at inline_asm_canonical.txt:335/:338/:339/:341.

## Instance kills and other entries (index; chassis-relative, re-testable)

## [s2] gte_SetRotMatrix(rot), called as a vendored macro (either header), reproduces target chunk 1's exact instruction sequence.
- verdict: KILLED (instance) — False with both headers: both macros interleave 2-lw+2-ctc2 then 3-lw+3-ctc2; target does all 5 lw first, then all 5 ctc2. No finer-grained vendored macro (che…

## [s2] gte_stlvl(out), called as a vendored macro, reproduces target chunk 5's delay-slot placement of the final swc2 in the jr $ra slot.
- verdict: KILLED (instance) — False: our build emits swc2 $11,8(a3) before jr $ra with an explicit nop filling the delay slot, instead of target's swc2-in-delay-slot placement. No vendored…

## [s3] A gtemac.h composite macro (not yet read in full, only grepped by name in s1/s3) offers a batched-order expansion for chunk 1 (gte_SetRotMatrix) or a lone-trailing-store expansion for chunk 5 (gte_stlvl).
- verdict: KILLED (instance) — No composite names or expands to gte_SetRotMatrix/gte_SetRotMatrixClearTrans or gte_stlvl; the closest shape (gte_RotTrans, line 23) uses gte_ldv0/gte_rt/gte_s…

## [s4] decomp-permuter's standard import pipeline (tools/decomp-permuter/import.py) can build a scorable target.o for func_80052B7C from asm/funcs/func_80052B7C.s against a temporary macro-call chassis (gte_SetRotMatrix/gte_ld…
- verdict: KILLED (instance) — FAIL: exit 1, no target.o produced. An earlier identical attempt (same chassis, output not saved) showed the proximate cause: target.s:105: Error: unrecognized…

## [s6] A vendored header macro in include/psxsdk (inline_c.h, inline_o.h, gtemac.h, libgte.h) takes register operands into CR0-CR4 (would let chunk 1 batch its loads) or stores $9/$10/$11 individually (would free the last swc2…
- verdict: KILLED (instance) — None found: only gte_stlvl's 3-store body (inline_c.h:1086-1089); no register-operand ctc2 into CR0-4.

## [s7] The 19-point residual of the legal chassis is a decompals-cc1 vs SN-cc1psx divergence (the original compiler would emit the target's order/delay slot from the same C).
- verdict: KILLED (instance) — byte-identical function bodies (cc1_vs_cc1psx.txt). Not a toolchain divergence.

## [s7] A sibling in the 3AB48 GTE cluster carries a STRONG scan_hand_coded tier that could anchor a cluster grant.
- verdict: KILLED (instance) — all LOW 0/12 (cluster_scan.txt); the scanner has no cop2-delay-slot signal.

## [s7] The 19-point residual of the s6 legal chassis is a decompals-cc1 vs SN cc1psx divergence: the original compiler would emit the target order and delay slot from the same C.
- verdict: KILLED (instance) — Function bodies byte-identical (interleaved SetRotMatrix lw/ctc2, lh x3, swc2 x3 then bare j $31). Residual is not a compiler-port divergence.

## [s7] A sibling in the 3AB48 GTE cluster carries a STRONG scan_hand_coded tier that could anchor a cluster grant for func_80052B7C.
- verdict: KILLED (instance) — All five LOW 0/12, no gate-class signal; the signal set has no cop2-in-delay-slot signal.

## [s10] KILL RE-AUDIT: the closest banked instance form (the legal macro-call chassis) re-measures below 19 once its FAKE constructs are ablated.
- verdict: KILLED (instance) — fake_ablate: no FAKE-annotated constructs, nothing to ablate. Sandbox: 19, unchanged.

## [s11] The mandated census tools (label_census, loop_movables, nrefs_census) expose a C-level lever on the legal chassis.
- verdict: KILLED (instance) — The target has 26 insns, 0 labels and 0 callee-saved refs. There are no loops and no ALLOCDBG rows; the only 3 allocated pseudos are the TR halfwords, already…

## Latest session s12 (verbatim)

## [s12] KILL RE-AUDIT: the closest banked instance form (the s6 legal macro-call chassis) still measures 19 when re-applied fresh to src/main/3AB48.c this session, with no FAKE construct present to ablate.
- mechanism: n/a (re-measurement, not a new lever).
- probe: Applied candidate.c's body verbatim (gte_SetRotMatrix/gte_ldtr((s32)..)/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h) to src/main/3AB48.c, ran `sandbox func_80052B7C --disable all --diff`, then reverted to INCLUDE_ASM.
- result: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- byte-identical diff shape to s6-s11 (tmp/grind/func_80052B7C/s12/sandbox_reaudit.txt). No FAKE construct exists in this chassis to ablate (fake_ablate.py has nothing to act on). The instance kill is confirmed current, not stale.
- verdict: CONFIRMED

## [s12] A fresh m2c (mips_to_c) decompile of asm/funcs/func_80052B7C.s surfaces a different, structurally novel C shape for the GTE residual (chunk 1 batching, chunk 5 delay slot).
- mechanism: m2c's instruction-model table has no entries for ctc2/lwc2/swc2/mvmva (cop2 GTE opcodes); unmodeled opcodes become `M2C_ERROR(/* unknown instruction: ... */);` placeholder statements with no inferred dataflow, so it cannot propose any structure, batching, or variable arrangement for GTE code.
- probe: `pip install git+https://github.com/matt-kempster/mips_to_c.git` (m2c 0.1.0, no prior install in this repo's venv) inside WSL; `m2c asm/funcs/func_80052B7C.s` with no context file (no C.func for this leaf exists to feed `--context`).
- result: Output is 14 `M2C_ERROR` stubs, one per cop2/GTE instruction, zero control-flow or variable inference; see tmp/grind/func_80052B7C/s12/m2c_out.c and m2c_err.txt. No alternate shape is produced -- m2c is structurally blind to this function's entire body.
- verdict: KILLED
- kill_scope: class
- measured_on: m2c 0.1.0 (matt-kempster/mips_to_c @ pip HEAD this session) against the oracle asm/funcs/func_80052B7C.s, no C context, no FAKE construct (tool run, no src/ edit)
- predicate_cite: tools/m2c/m2c/arch_mips.py:1298

## [s12] The decomp.me corpus (gcc2.7.2-psx compiler bucket) contains a scratch whose target asm is similar enough to func_80052B7C's batched-GTE-register-load pattern to transplant a different C shape.
- mechanism: `tools/decomp_me_scrape.py search` ranks every scratch under the matching compiler by assembly-level similarity to the target; a close hit would be citable corpus evidence for a batching variant none of gtemac.h's composites express.
- probe: `python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_80052B7C.s` (required installing curl_cffi, not previously in the venv).
- result: Top 5 hits all score similarity 0.022-0.026 (tmp/grind/func_80052B7C/s12/decomp_me_search.txt) -- an order of magnitude below anything citable as a structural match; none of the 5 (func_8004DA6C, func_8004ECF4, func_8004D984, func_80020344, func_8004D9EC) touch cop2/GTE code. No transplant candidate exists in the corpus for this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: decomp.me corpus as scraped this session (gcc2.7.2-psx compiler bucket, live site state 2026-10-15), no FAKE construct, no src/ edit

## [s12] A composite GTE macro in gtemac.h exists that expands to a batched lw/lh-then-ctc2 sequence spanning both the rotation-matrix load and the TR-vector load (closing chunk 1's and chunk 2/5's residual as a single call).
- mechanism: gtemac.h composites (gte_MulMatrix0, gte_ApplyMatrix, gte_ApplyMatrixSV, gte_CompMatrix, gte_ApplyRotMatrix, read in full this session lines 300-358) are sequences of CALLS to the same leaf macros (gte_SetRotMatrix, gte_ldv0, gte_rtv0/gte_rtir, gte_stclmv/gte_stsv/gte_stlvnl) -- none re-expresses gte_SetRotMatrix's or gte_ldtr's own fixed asm-string body with different internal instruction order, and none combines SetRotMatrix+ldtr into one call at all. Composing CALLS to opaque leaves cannot change what each leaf's own `__asm__ volatile` string contains (re-confirms s8's inline_c.h:297/:1086 finding from a different angle: the composite layer is calls-to-calls, not a different leaf).
- probe: Read include/psxsdk/gtemac.h:260-358 in full (every composite macro definition) looking for one that (a) includes a translation-vector load and (b) batches all lw/lh before all ctc2 across the rotation+translation boundary.
- result: No such composite exists; the 5 composites found just sequence existing leaves (SetRotMatrix, ldv0/ldclmv, rtv0/rtir/rt, stlvnl/stsv/stclmv/stbv/stcv) for whole-pipeline ops (MulMatrix0, ApplyMatrix, ApplyMatrixSV, CompMatrix, ApplyRotMatrix) -- none targets the RotMatrix+TR-load combination this function needs, and even if one did, it would still just call gte_SetRotMatrix and gte_ldtr as separate opaque leaves with their existing internal order.
- verdict: KILLED
- kill_scope: class
- measured_on: include/psxsdk/gtemac.h as vendored at this commit, visual read, no src/ edit, no FAKE construct
- predicate_cite: include/psxsdk/gtemac.h:320-358

## [s12] The func_800203B4 island-grant precedent (inline_asm_canonical.txt:357: thin GTE islands authorized AS UNITS after pure-C respelling measured unable to close, owner-granted 2026-09-01) reopens the written-island question for func_80052B7C.
- mechanism: Both functions share the shape "pure-C bound measured, residual sits in GCC's zero-cop2-mnemonic backend" -- the precedent's own text cites exactly that compiler-source proof as its justification.
- probe: Read inline_asm_canonical.txt:357 in full and cross-checked it against the 2026-10-15 20:38 func_80052B7C ruling in docs/grind/decisions.md already in this ledger's digest.
- result: The func_80052B7C ruling already considered and rejected this exact argument by name ("Chunk 2's written lh/ctc2 island is not precedent: it is itself a new written island, and the class grant requires every GTE operation to be a call by name, so a mixed body never qualifies"). func_800203B4's grant was an owner-direction widened-anchor exception (2026-09-01), not a standing class rule this function can invoke by analogy; re-raising it resubmits a question the Judge-equivalent ruling already answered.
- verdict: KILLED
- kill_scope: instance
- measured_on: inline_asm_canonical.txt:357 and docs/grind/decisions.md 2026-10-15 20:38 ruling, both read fresh this session, no src/ edit, no FAKE construct

## [s12] The closest banked instance form (the s6 legal macro-call chassis) still measures 19 when re-applied fresh to src/main/3AB48.c this session, with no FAKE construct present to ablate.
- mechanism: n/a (re-measurement, not a new lever).
- probe: Applied candidate.c's body verbatim (gte_SetRotMatrix/gte_ldtr((s32)..)/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h) to src/main/3AB48.c, ran sandbox func_80052B7C --disable all --diff, then reverted to INCLUDE_ASM.
- result: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- byte-identical diff shape to s6-s11 (tmp/grind/func_80052B7C/s12/sandbox_reaudit.txt). No FAKE construct exists in this chassis to ablate.
- verdict: CONFIRMED

## [s12] A fresh m2c (mips_to_c) decompile of asm/funcs/func_80052B7C.s surfaces a different, structurally novel C shape for the GTE residual (chunk 1 batching, chunk 5 delay slot).
- mechanism: m2c's MIPS instruction table (tools/m2c/m2c/arch_mips.py) has zero entries for ctc2/lwc2/swc2/mvmva (confirmed by grep: no matches for any of those mnemonics in the file); any opcode absent from that table always falls through to the generic unknown-instruction handler at arch_mips.py:1298, which emits an M2C_ERROR placeholder with no inferred dataflow.
- probe: python3 tools/m2c/m2c.py asm/funcs/func_80052B7C.s (m2c already vendored at tools/m2c/, mips-ido-c target, no --context file).
- result: Output is 14 M2C_ERROR stubs, one per cop2/GTE instruction (all 8 ctc2, 2 lwc2, mvmva, 3 swc2), zero control-flow or variable inference (tmp/grind/func_80052B7C/s12/m2c_out.c). No alternate shape is produced -- m2c is categorically blind to this function's entire body, not just this chassis.
- verdict: KILLED
- kill_scope: class
- measured_on: m2c as vendored at tools/m2c/ this commit, against the oracle asm/funcs/func_80052B7C.s, no C context, no FAKE construct (tool run, no src/ edit)
- predicate_cite: tools/m2c/m2c/arch_mips.py:1298

## [s12] The decomp.me corpus (gcc2.7.2-psx compiler bucket) contains a scratch whose target asm is similar enough to func_80052B7C's batched-GTE-register-load pattern to transplant a different C shape.
- mechanism: tools/decomp_me_scrape.py search ranks every scratch under the matching compiler by assembly-level shingle similarity to the target; a close hit would be citable corpus evidence for a batching variant none of gtemac.h's composites express.
- probe: python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_80052B7C.s --corpus tmp/decomp_me_corpus --top 5
- result: Top 5 hits all score similarity 0.022-0.026 (tmp/grind/func_80052B7C/s12/decomp_me_search.txt) -- an order of magnitude below anything citable as a structural match; none of the 5 (func_8004DA6C, func_8004ECF4, func_8004D984, func_80020344, func_8004D9EC) touch cop2/GTE code. No transplant candidate exists in the corpus for this function right now.
- verdict: KILLED
- kill_scope: instance
- measured_on: decomp.me corpus as scraped this session (gcc2.7.2-psx compiler bucket, live site state 2026-10-15), no FAKE construct, no src/ edit

## [s12] A composite GTE macro in gtemac.h exists that expands to a batched lw/lh-then-ctc2 sequence spanning both the rotation-matrix load and the TR-vector load, closing chunk 1's and chunk 5's residual as a single call.
- mechanism: gtemac.h composites (gte_MulMatrix0, gte_ApplyMatrix, gte_ApplyMatrixSV, gte_CompMatrix, gte_ApplyRotMatrix) are sequences of CALLS to the same leaf macros (gte_SetRotMatrix, gte_ldv0, gte_rtv0/gte_rtir, gte_stclmv/gte_stsv/gte_stlvnl); composing calls to opaque leaves cannot change what each leaf's own fixed __asm__ volatile string contains.
- probe: Read include/psxsdk/gtemac.h:260-358 in full (every composite macro definition) looking for one that batches the rotation-matrix and translation-vector loads across the lw/ctc2 boundary.
- result: No such composite exists; the 5 composites found just sequence existing leaves for whole-pipeline ops -- none targets the RotMatrix+TR-load combination this function needs, and even if one did, it would still just call gte_SetRotMatrix and gte_ldtr as separate opaque leaves with their existing fixed internal order.
- verdict: KILLED
- kill_scope: class
- measured_on: include/psxsdk/gtemac.h as vendored at this commit, visual read, no src/ edit, no FAKE construct
- predicate_cite: include/psxsdk/gtemac.h:320

## [s12] No sibling in the 3AB48 GTE cluster (func_8004A4E0, func_8004A76C) has a banked candidate.c body containing a transplantable SetRotMatrix/ctc2-batching or delay-slot C shape for this function's residual.
- mechanism: Sibling transplant check mandated by this session's brief: grep each sibling's candidate.c for SetRotMatrix/ctc2/batch/interleave spellings before any new probe.
- probe: grep -n 'SetRotMatrix|ctc2|batch|interleave' memory/grind/func_8004A4E0/candidate.c memory/grind/func_8004A76C/candidate.c
- result: func_8004A4E0's candidate.c mentions '4 ctc2 with no C form' only in its own exhaustion summary prose, no concrete transplantable body; func_8004A76C's candidate.c has zero hits. Both siblings are themselves INCLUDE_ASM stubs with no landed C to borrow from.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_8004A4E0/candidate.c and memory/grind/func_8004A76C/candidate.c as of this session, no FAKE construct, no src/ edit

## [s13] Fresh structural re-measurement of the legal macro-call chassis (s6/s8 form) exposes zero operand-only hunks, re-confirming s8's class kill of every block-local-var-split / declaration-order / type-narrowing / statement-reassociation lever under the CURRENT build.
- mechanism: `sandbox func_80052B7C --disable all --diff` buckets every scored hunk source-level / operand-only / not-scored; a structural (reg-alloc/scheduling) lever can only move bytes inside an operand-only hunk. This session's fresh run shows 4/4 hunks source-level, 0/4 operand-only -- there is no bucket a structural lever could act on.
- probe: Applied candidate.c (s6/s8 form, unchanged) to src/main/3AB48.c, ran `& tools/wteng.ps1 main sandbox func_80052B7C --disable all --diff`, saved output to tmp/grind/func_80052B7C/s13/sandbox_diff.txt, reverted src/main/3AB48.c to INCLUDE_ASM. Also re-read include/psxsdk/inline_c.h:297-301 (gte_SetRotMatrix) and :1086-1091 (gte_stlvl) to re-confirm both are still single fixed-string __asm__ volatile macros (unchanged since s8).
- result: score=19 (unchanged), 4 hunks all source-level: hunk1 = gte_SetRotMatrix's batched-lw-then-batched-ctc2 order (target) vs our macro's interleaved 2-lw+2-ctc2/3-lw+3-ctc2 order; hunk2/3/4 = gte_stlvl's swc2 landing in the jr $ra delay slot (target) vs explicit nop+swc2 (ours). Both residuals sit entirely inside each macro's own opaque __asm__ volatile string (confirmed again by direct read), exactly as s8 found -- no new structural lever surfaced.
- verdict: KILLED
- kill_scope: class
- measured_on: s6/s8 legal macro-call chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via include/psxsdk/inline_c.h), no FAKE construct, src/main/3AB48.c this session, reverted to INCLUDE_ASM afterward
- predicate_cite: include/psxsdk/inline_c.h:297

## [s13] Fresh structural re-measurement of the legal macro-call chassis (s6/s8 form) exposes zero operand-only hunks, re-confirming s8's class kill of every block-local-var-split / declaration-order / type-narrowing / statement-reassociation lever under the CURRENT build.
- mechanism: sandbox --diff buckets every scored hunk source-level / operand-only / not-scored; a structural reg-alloc/scheduling lever can only act inside an operand-only hunk, and this run shows 4/4 hunks source-level, 0/4 operand-only.
- probe: Applied candidate.c (unchanged s6/s8 form) to src/main/3AB48.c, ran sandbox func_80052B7C --disable all --diff, saved tmp/grind/func_80052B7C/s13/sandbox_diff.txt, reverted src to INCLUDE_ASM. Re-read include/psxsdk/inline_c.h:297-301 and :1086-1091.
- result: score=19, 4 hunks all source-level: hunk1 batched-lw/ctc2 order inside gte_SetRotMatrix, hunks2-4 the swc2-in-jr-delay-slot shape inside gte_stlvl. Both macros confirmed still single fixed-string __asm__ volatile bodies, unchanged since s8.
- verdict: KILLED
- kill_scope: class
- measured_on: s6/s8 legal macro-call chassis via include/psxsdk/inline_c.h, no FAKE construct, src/main/3AB48.c this session, reverted to INCLUDE_ASM afterward
- predicate_cite: include/psxsdk/inline_c.h:297

## [s14] The s6 legal macro-call chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h, no __asm__ in the body) re-measures floor 19 this session with 4/4 hunks source-level and 0/4 operand-only.
- mechanism: sandbox func_80052B7C --disable all --diff scores insn-level hunks; a hunk is operand-only only when the target and our instruction sequence match modulo register/scheduling choice. Here target batches 5 lw then 5 ctc2 (chunk 1) and places the final swc2 in the jr delay slot (chunk 5) -- both are instruction-sequence differences the C never expresses, so every hunk classes source-level.
- probe: Applied candidate.c's s6 body to src/main/3AB48.c, ran `tools/wteng.ps1 main sandbox func_80052B7C --disable all --diff`, reverted to INCLUDE_ASM.
- result: 19/19, identical to s6/s8/s9/s11/s12/s13. Confirms no drift in the chassis or scorer since s13.
- verdict: CONFIRMED

## [s14] The s6 legal chassis contains zero FAKE-annotated constructs, so the KILL RE-AUDIT dispatcher condition (re-measure the closest instance kill with every FAKE construct ablated) is vacuously satisfied -- there is no FAKE carrier that could have been occupying the target pseudo and invalidating s8's/s9's/s13's structural class kills.
- mechanism: tools/fake_ablate.py mechanically scans a candidate.c body for /* FAKE: ... */ sites and strips each one, re-measuring; with none present it reports nothing to ablate rather than a score.
- probe: python3 tools/fake_ablate.py --func func_80052B7C --file main/3AB48 --candidate memory/grind/func_80052B7C/candidate.c
- result: "no FAKE-annotated constructs found in memory/grind/func_80052B7C/candidate.c; nothing to ablate" (tmp/grind/func_80052B7C/s14/ablate.txt).
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 legal macro-call chassis via psxsdk/inline_c.h, no FAKE construct present, src/main/3AB48.c this session, reverted to INCLUDE_ASM afterward

## [s14] On this chassis, no block-local variable split / declaration-order change / type-narrowing / statement-reassociation (the structural-modality lever set) can touch either source-level residual, because the 5-statement region contains zero C-level declarations, assignments, or binary operators for any such lever to act on -- re-confirms s8/s9/s13's class kill with a fresh measurement this session.
- mechanism: gte_SetRotMatrix and gte_stlvl (include/psxsdk/inline_c.h:297, :1086) are each a single __asm__ volatile statement with a literal fixed instruction string, not computed from any C-level expression; spelling_enum.py's region scanner (s9) independently found 0 named locals, 0 assignments, 5 anchors -> 1 distinct spelling in this exact region.
- probe: Re-read inline_c.h:297/:1086 this session (unchanged since s8); cross-checked against the fresh --diff output's hunk classification (0/4 operand-only).
- result: Residual confirmed to sit entirely inside each macro's own fixed __asm__ volatile body, outside the reach of any structural C-level lever.
- verdict: KILLED
- kill_scope: class
- measured_on: s6 legal macro-call chassis via psxsdk/inline_c.h, no FAKE construct, src/main/3AB48.c this session, reverted
- predicate_cite: include/psxsdk/inline_c.h:297
