# Hypothesis ledger — func_80052B7C

> COMPACTED 2026-10-15 by `grindlib.py compact-ledger` (66,289 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 79a1ea68e:memory/grind/func_80052B7C/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
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
floor 19 at s21; 21 sessions; live frontier (state.json is authoritative):
  - A scan_hand_coded gate-class signal for the batched lw/lh $tN -> ctc2 idiom (s18) and/or the cop2-in-jr-slot signal (s11) lifts func_80052B7C to STRONG tier for the whole-body INCLUDE_ASM branch.
  - The inline_asm_canonical.txt:340 whole-body grant audit resolves in favor of func_80052B7C on cluster plus Sony-idiom evidence.
  - A vendored macro whose template batches the rotation+TR loads in $8..$15 would close hunk 1 in pure macro-call C.

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

## [s12] A fresh m2c (mips_to_c) decompile of asm/funcs/func_80052B7C.s surfaces a different, structurally novel C shape for the GTE residual (chunk 1 batching, chunk 5 delay slot).
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/m2c/m2c/arch_mips.py:1298
- result: Output is 14 `M2C_ERROR` stubs, one per cop2/GTE instruction, zero control-flow or variable inference; see tmp/grind/func_80052B7C/s12/m2c_out.c and m2c_err.txt. No alternate shape is produced -- m2c is structurally blind to this function's entire body.

## [s12] A composite GTE macro in gtemac.h exists that expands to a batched lw/lh-then-ctc2 sequence spanning both the rotation-matrix load and the TR-vector load (closing chunk 1's and chunk 2/5's residual as a single call).
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/gtemac.h:320-358
- result: No such composite exists; the 5 composites found just sequence existing leaves (SetRotMatrix, ldv0/ldclmv, rtv0/rtir/rt, stlvnl/stsv/stclmv/stbv/stcv) for whole-pipeline ops (MulMatrix0, ApplyMatrix, ApplyMatrixSV, CompMatrix, ApplyRotMatrix) -- none targets the RotMatrix+TR-load combination this function needs, and even if one did, it would still just call gte_SetRotMatrix and gte_ldtr as separa…

## [s12] A fresh m2c (mips_to_c) decompile of asm/funcs/func_80052B7C.s surfaces a different, structurally novel C shape for the GTE residual (chunk 1 batching, chunk 5 delay slot).
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/m2c/m2c/arch_mips.py:1298
- result: Output is 14 M2C_ERROR stubs, one per cop2/GTE instruction (all 8 ctc2, 2 lwc2, mvmva, 3 swc2), zero control-flow or variable inference (tmp/grind/func_80052B7C/s12/m2c_out.c). No alternate shape is produced -- m2c is categorically blind to this function's entire body, not just this chassis.

## [s12] A composite GTE macro in gtemac.h exists that expands to a batched lw/lh-then-ctc2 sequence spanning both the rotation-matrix load and the TR-vector load, closing chunk 1's and chunk 5's residual as a single call.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/gtemac.h:320
- result: No such composite exists; the 5 composites found just sequence existing leaves for whole-pipeline ops -- none targets the RotMatrix+TR-load combination this function needs, and even if one did, it would still just call gte_SetRotMatrix and gte_ldtr as separate opaque leaves with their existing fixed internal order.

## [s13] Fresh structural re-measurement of the legal macro-call chassis (s6/s8 form) exposes zero operand-only hunks, re-confirming s8's class kill of every block-local-var-split / declaration-order / type-narrowing / statement-reassociation lever under the CURRENT build.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:297
- result: score=19 (unchanged), 4 hunks all source-level: hunk1 = gte_SetRotMatrix's batched-lw-then-batched-ctc2 order (target) vs our macro's interleaved 2-lw+2-ctc2/3-lw+3-ctc2 order; hunk2/3/4 = gte_stlvl's swc2 landing in the jr $ra delay slot (target) vs explicit nop+swc2 (ours). Both residuals sit entirely inside each macro's own opaque __asm__ volatile string (confirmed again by direct read), exact…

## [s13] Fresh structural re-measurement of the legal macro-call chassis (s6/s8 form) exposes zero operand-only hunks, re-confirming s8's class kill of every block-local-var-split / declaration-order / type-narrowing / statement-reassociation lever under the CURRENT build.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:297
- result: score=19, 4 hunks all source-level: hunk1 batched-lw/ctc2 order inside gte_SetRotMatrix, hunks2-4 the swc2-in-jr-delay-slot shape inside gte_stlvl. Both macros confirmed still single fixed-string __asm__ volatile bodies, unchanged since s8.

## [s14] On this chassis, no block-local variable split / declaration-order change / type-narrowing / statement-reassociation (the structural-modality lever set) can touch either source-level residual, because the 5-statement region contains zero C-level declarations, assignments, or binary operators for any such lever to act…
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:297
- result: Residual confirmed to sit entirely inside each macro's own fixed __asm__ volatile body, outside the reach of any structural C-level lever.

## [s15] A fresh spelling_enum.py sweep of the legal macro-call chassis's entire body (5 statements: ctc2/lw/lh calls via gte_SetRotMatrix/gte_stlvl macros, an if, a return), run both --no-swaps and with the operand-swap axis enabled, produces exactly 1 distinct spelling on both axes because every statement in the region is a…
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/spelling_enum.py:89
- result: Both runs report "0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings" verbatim -- the region has no decl/assign nodes at all, so the swap axis (which only varies A*B/A+B operands inside decl/assign expressions) has nothing to act on either. Matches s9's prior plain-axis finding; this session adds the swap-axis confirmation the s9 run did not cover.

## [s16] A fresh spelling_enum.py sweep of the full 5-statement macro-call region (with and without the operand-swap axis) on the current floor-19 chassis finds a non-empty mutation space to exploit.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/spelling_enum.py:217
- result: Both runs report "0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings" -- byte-identical mechanical output to s9's and s15's runs. The region already spans the entire function body so the SOP's widen-to-neighbouring-block step is inapplicable (no neighbouring block exists). This is the third independent enumerate-modality pass landing on the identical empty mutation space.

## [s17] A fresh spelling_enum.py sweep (both --no-swaps and with the operand-swap axis) of the full 5-statement macro-call body on the current floor-19 chassis finds zero C-level mutation axes, identical to the s9/s15/s16 runs.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/spelling_enum.py:89
- result: Both runs report "0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings" verbatim -- byte-for-byte identical to s9/s15/s16's outputs. This is the fourth independent enumerate-modality confirmation that the spelling-sweep search space on this chassis is empty.

## [s19] inverse_compose.py classify's "FIRST DIVERGENCE: RA" verdict for this function is a register-blanking artifact for 5 of the 8 target data registers: those registers are literal text in the gte_SetRotMatrix asm template, so no RA model owns them.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:298
- result: classify lists ours ctc2 {t4,t5,t4,t5,t6} for $0..$4. The literal $12/$13/$12/$13/$14 at inline_c.h:298-307 is the one-to-one source of that. ours ctc2 {a0,v1,v0} for $5..$7 are the only allocated operands.

## [s19] The rotation-matrix lw/ctc2 registers in the residual are owned by a register-allocation model, as inverse_compose classify's FIRST DIVERGENCE: RA verdict suggests.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:298
- result: Ours ctc2 {t4,t5,t4,t5,t6} for $0..$4 maps one-to-one onto the literal $12,$13,$12,$13,$14 at inline_c.h:298-307. The residual for those registers sits in the template string (pre-RA), so it sharpens s10's RA FORECLOSED verdict.

## [s20] The inline_o.h (one-asm-per-instruction) macro set, whose separate __asm__ statements might let the scheduler batch the rotation loads, reaches the target better than inline_c.h.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_o.h:273
- result: score 31 (worse than 19), 3 source-level hunks: extra move insns and the inline_o.h ldtr's move $12..$14 / ctc2 order.

## [s20] Replacing inline_c.h with the inline_o.h per-instruction macro set in the same 5-statement body brings the function closer to the target than 19.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_o.h:273
- result: score 31, 3 source-level hunks (extra moves, different ldtr shape).

## [s21] Inserting the vendored gte_nop() (include/psxsdk/inline_c.h:1329) between gte_rt() and gte_stlvl(out) reproduces the target's hazard nop between mvmva and the first swc2 (hunk 2) and lowers the score.
- verdict: KILLED
- kill_scope: class
- predicate_cite: engine/gtemacro.py:1095
- result: score 19, identical diff to base; "cheat_asm_stripped" rises 70 -> 71 and the objdump shows no nop after `c2 0x480012`. The sandbox strips a standalone gte_nop() as a scheduling barrier by name (engine/gtemacro.py:30, :1095: "gte_nop() is a scheduling barrier by name" and "standalone gte_nop() is never a unit"). So the construct is scored as absent; it is also in the forbidden scheduling-barrier…

## [s21] Inserting vendored gte_nop() between gte_rt() and gte_stlvl(out) supplies the target's mvmva->swc2 hazard nop (hunk 2) and lowers the score.
- verdict: KILLED
- kill_scope: class
- predicate_cite: engine/gtemacro.py:1095
- result: Score 19 with an identical diff; cheat_asm_stripped rises 70->71 and the objdump has no nop after c2 0x480012. The sandbox strips a standalone gte_nop() as a scheduling barrier by name (engine/gtemacro.py:30, :1095), which is a forbidden family anyway, and hunks 3-4 (reorg.c:740, s7) would stay open.

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

## [s12] KILL RE-AUDIT: the closest banked instance form (the s6 legal macro-call chassis) still measures 19 when re-applied fresh to src/main/3AB48.c this session, with no FAKE construct present to ablate.
- verdict: CONFIRMED
- result: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- byte-identical diff shape to s6-s11 (tmp/grind/func_80052B7C/s12/sandbox_reaudit.txt). No FAKE construct exists in this chassis to ablate (fake_ablate.py has nothing to act on). The instance kill is confirmed current, not stale.

## [s12] The closest banked instance form (the s6 legal macro-call chassis) still measures 19 when re-applied fresh to src/main/3AB48.c this session, with no FAKE construct present to ablate.
- verdict: CONFIRMED
- result: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- byte-identical diff shape to s6-s11 (tmp/grind/func_80052B7C/s12/sandbox_reaudit.txt). No FAKE construct exists in this chassis to ablate.

## [s14] The s6 legal macro-call chassis (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl via psxsdk/inline_c.h, no __asm__ in the body) re-measures floor 19 this session with 4/4 hunks source-level and 0/4 operand-only.
- verdict: CONFIRMED
- result: 19/19, identical to s6/s8/s9/s11/s12/s13. Confirms no drift in the chassis or scorer since s13.

## [s17] Re-applying the s6 legal macro-call chassis body (candidate.c) to src/main/3AB48.c WITHOUT also carrying its `#include <psxsdk/inline_c.h>` line produces a silently-wrong, worse measurement (27 ours vs 24 target insns, 3 hunks, jal call instructions in place of inline asm), because src/main/3AB48.c's existing `#includ…
- verdict: CONFIRMED
- result: Adding the include restored the exact banked measurement: score 19, 24/24 insns, 4/4 hunks source-level, 0/4 operand-only -- identical to s8/s13/s14/s15/s16. Recorded as a repro-gotcha note in candidate.c and hypotheses.md s17/H14 so a future session does not mistake the missing-include state for a regression.

## [s18] KILL RE-AUDIT: the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers, so no instance kill is voided by chassis or FAKE drift.
- verdict: CONFIRMED
- result: 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only (s18/sandbox_diff.txt); ablate: nothing to ablate (s18/ablate.txt). Identical to s6/s8/s13-s17.

## [s18] The target's batched lw/lh $t0..$t7 -> ctc2 run (chunks 1+2) is produced in this EXE only by hand-written code: Sony LIBGTE asmpsx modules and game functions already proven hand-written; no compiled-C function produces it.
- verdict: CONFIRMED
- result: 16 hits: 8 Sony LIBGTE modules (SetRotMatrix register-identical to chunk 1), func_8004A940/func_8004C404/func_8004DDB4 (scan STRONG) + func_80054440 (trapping addi), 3 granted 3AB48 cluster leaves + func_80052B7C. 0 compiled-C hits; COMPLETED-C func_800203B4 (gte_SetRotMatrix by name) is a negative control. s18/batch_census.txt.

## [s18] KILL RE-AUDIT: the banked s6 legal macro-call chassis re-measures at the ledger floor on the current tree with zero FAKE carriers, so no instance kill measured on it is voided by chassis or FAKE drift.
- verdict: CONFIRMED
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only; fake_ablate: no FAKE-annotated constructs, nothing to ablate. Identical to s6/s8/s13-s17.

## [s18] The target's batched lw/lh $t0..$t7 -> ctc2 run (chunks 1+2) is produced in this EXE only by independently hand-written code, never by a compiled-C function.
- verdict: CONFIRMED
- result: 16 hits: 8 Sony LIBGTE modules (SetRotMatrix, ApplyMatrix, ApplyMatrixLV, CompMatrix, MulMatrix, MulMatrix0, MulMatrix2, SetColorMatrix), func_8004A940/func_8004C404/func_8004DDB4 (scan_hand_coded STRONG, inline_asm_canonical.txt:198/:200/:210), func_80054440 (trapping addi, :162), granted gte_SetMatrixRotTransIR/gte_SetRotMatrixClearTrans/gte_SetRotTransMatrix, and func_80052B7C. 0 compiled-C si…

## [s19] KILL RE-AUDIT (solver modality): the banked legal macro-call chassis (candidate.c body + #include <psxsdk/inline_c.h>) re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only; ablate: nothing to ablate. Identical to s6/s8/s13-s18.

## [s19] KILL RE-AUDIT: the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: 19, 24/24 insns, 4/4 source-level, 0 operand-only; nothing to ablate.

## [s20] KILL RE-AUDIT (forensics): the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 4 source-level / 0 operand-only / 0 not-scored; ablate: "no FAKE-annotated constructs found". Identical to s6/s8/s13-s19.

## [s20] Pass attribution (dump-read) of every residual hunk on the legal chassis.
- verdict: CONFIRMED
- result: (a) Hunk 1 rotation registers: func.dbr:33-34, insn 15 is ONE asm_operands/v whose template text already contains literal "lw $12, 0(%0); lw $13, 4(%0); ctc2 $12,$0; ..." -- fixed at RTL expand from include/psxsdk/inline_c.h:298, no later pass touches it. (b) Hunk 1 TR operands: insns 20/24/28 are the only allocated sets; local-alloc assigns them (func.lreg:26-28 ";; Register 77 in 4 / 80 in 3 /…

## [s20] KILL RE-AUDIT: the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 4 source-level / 0 operand-only / 0 not-scored; fake_ablate: nothing to ablate.

## [s20] Dump read attributes every residual hunk to a named pass input: hunk 1 rotation regs come from the asm template at RTL expand, the TR operands from local-alloc, and hunks 2-4 from one asm_operands insn before an unfilled return.
- verdict: CONFIRMED
- result: Confirmed as stated. No loop, label, callee-saved, or global-alloc input exists for a C lever to act on.

## [s21] KILL RE-AUDIT (forensics): the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 24/24, 4 source-level / 0 operand-only; fake_ablate "no FAKE-annotated constructs found ... nothing to ablate".

## [s21] KILL RE-AUDIT: the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 24/24, 4 source-level / 0 operand-only; fake_ablate: nothing to ablate.

## Instance kills and other entries (index; chassis-relative, re-testable)

## [s2] gte_SetRotMatrix(rot), called as a vendored macro (either header), reproduces target chunk 1's exact instruction sequence.
- verdict: KILLED (instance)

## [s2] gte_stlvl(out), called as a vendored macro, reproduces target chunk 5's delay-slot placement of the final swc2 in the jr $ra slot.
- verdict: KILLED (instance)

## [s3] A gtemac.h composite macro (not yet read in full, only grepped by name in s1/s3) offers a batched-order expansion for chunk 1 (gte_SetRotMatrix) or a lone-trailing-store expansion for chunk 5 (gte_stlvl).
- verdict: KILLED (instance)

## [s4] decomp-permuter's standard import pipeline (tools/decomp-permuter/import.py) can build a scorable target.o for func_80052B7C from asm/funcs/func_80052B7C.s against a temporary macro-call chassis (gte_SetRotMatrix/gte_ld…
- verdict: KILLED (instance)

## [s6] A vendored header macro in include/psxsdk (inline_c.h, inline_o.h, gtemac.h, libgte.h) takes register operands into CR0-CR4 (would let chunk 1 batch its loads) or stores $9/$10/$11 individually (would free the last swc2…
- verdict: KILLED (instance)

## [s7] The 19-point residual of the legal chassis is a decompals-cc1 vs SN-cc1psx divergence (the original compiler would emit the target's order/delay slot from the same C).
- verdict: KILLED (instance)

## [s7] A sibling in the 3AB48 GTE cluster carries a STRONG scan_hand_coded tier that could anchor a cluster grant.
- verdict: KILLED (instance)

## [s7] The 19-point residual of the s6 legal chassis is a decompals-cc1 vs SN cc1psx divergence: the original compiler would emit the target order and delay slot from the same C.
- verdict: KILLED (instance)

## [s7] A sibling in the 3AB48 GTE cluster carries a STRONG scan_hand_coded tier that could anchor a cluster grant for func_80052B7C.
- verdict: KILLED (instance)

## [s10] KILL RE-AUDIT: the closest banked instance form (the legal macro-call chassis) re-measures below 19 once its FAKE constructs are ablated.
- verdict: KILLED (instance)

## [s11] The mandated census tools (label_census, loop_movables, nrefs_census) expose a C-level lever on the legal chassis.
- verdict: KILLED (instance)

## [s12] The decomp.me corpus (gcc2.7.2-psx compiler bucket) contains a scratch whose target asm is similar enough to func_80052B7C's batched-GTE-register-load pattern to transplant a different C shape.
- verdict: KILLED (instance) — Top 5 hits all score similarity 0.022-0.026 (tmp/grind/func_80052B7C/s12/decomp_me_search.txt) -- an order of magnitude below anything citable as a structural…

## [s12] The func_800203B4 island-grant precedent (inline_asm_canonical.txt:357: thin GTE islands authorized AS UNITS after pure-C respelling measured unable to close, owner-granted 2026-09-01) reopens the written-island questio…
- verdict: KILLED (instance) — The func_80052B7C ruling already considered and rejected this exact argument by name ("Chunk 2's written lh/ctc2 island is not precedent: it is itself a new wr…

## [s12] The decomp.me corpus (gcc2.7.2-psx compiler bucket) contains a scratch whose target asm is similar enough to func_80052B7C's batched-GTE-register-load pattern to transplant a different C shape.
- verdict: KILLED (instance) — Top 5 hits all score similarity 0.022-0.026 (tmp/grind/func_80052B7C/s12/decomp_me_search.txt) -- an order of magnitude below anything citable as a structural…

## [s12] No sibling in the 3AB48 GTE cluster (func_8004A4E0, func_8004A76C) has a banked candidate.c body containing a transplantable SetRotMatrix/ctc2-batching or delay-slot C shape for this function's residual.
- verdict: KILLED (instance) — func_8004A4E0's candidate.c mentions '4 ctc2 with no C form' only in its own exhaustion summary prose, no concrete transplantable body; func_8004A76C's candida…

## [s14] The s6 legal chassis contains zero FAKE-annotated constructs, so the KILL RE-AUDIT dispatcher condition (re-measure the closest instance kill with every FAKE construct ablated) is vacuously satisfied -- there is no FAKE…
- verdict: KILLED (instance) — "no FAKE-annotated constructs found in memory/grind/func_80052B7C/candidate.c; nothing to ablate" (tmp/grind/func_80052B7C/s14/ablate.txt).

## [s19] For the RA-owned subset (gte_ldtr's 3 operands), moving the tr->vx/vy/vz loads into named locals before gte_SetRotMatrix, or issuing gte_ldtr before gte_SetRotMatrix, moves those operands toward the target's t5..t7.
- verdict: KILLED (instance) — Both variants score 19, 4/4 hunks source-level. Both allocate lh t0,v1,v0 -> ctc2 $5/$6/$7, which is the lowest-free set as predicted (s19/v1_locals_pre.txt, s…

## [s19] Loading tr->vx/vy/vz into named locals before gte_SetRotMatrix, or issuing gte_ldtr first, moves the three ldtr operands to the target's t5..t7.
- verdict: KILLED (instance) — Both score 19, 4/4 source-level, operands lh t0,v1,v0 (baseline a0,v1,v0). No t5..t7.

## [s21] Replacing gte_rt() with the DMPSX bare-word variant gte_rt_b() (inline_c.h:815, no leading nops) changes the scored distance.
- verdict: KILLED (instance) — score 19 with a byte-identical diff report to base, but the objdump shows the two pre-RT nops gone (24 lines vs base 26). The scorer is nop-insensitive at that…

## [s21] Replacing gte_rt() with the DMPSX bare-word gte_rt_b() (inline_c.h:815) changes the scored distance on the legal chassis.
- verdict: KILLED (instance) — Score 19 with an identical diff report, but the objdump shows both pre-RT nops removed. The score ignores them, and the bytes move away from the target, so thi…

## Latest session s22 (verbatim)

## [s22] KILL RE-AUDIT (rederive): the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- mechanism: dispatcher-mandated re-audit; instance kills are voided only by chassis drift or FAKE carriers. Applied fresh (candidate.c:201-208) over the current src/main/3AB48.c INCLUDE_ASM stub.
- probe: sandbox func_80052B7C --disable all --diff (tmp/grind/func_80052B7C/s22/sandbox_reaudit.txt); grep for FAKE markers in the applied body (none found, matching s14/s18-s21's "fake_ablate has nothing to act on" finding -- fake_ablate.py is not directly reachable through tools/wteng.ps1's `engine` subcommand dispatcher from this session, the direct grep is equivalent evidence of zero `/* FAKE` sites); reverted to INCLUDE_ASM.
- result: score 19, 4/4 source-level hunks, 0 operand-only, 0 not-scored -- byte-identical diff shape to s6/s8/s9/s11-s21. Hunk 1 is the rotation-matrix lw/ctc2 batch (ours: 2+3+3 lw/ctc2 grouped by register pair; target: 8 lw then 8 ctc2 fully batched) plus the TR loads landing in {a0,v1,v0} vs target's {t5,t6,t7}; hunks 2-4 are the trailing nop/swc2/nop ordering around gte_stlvl. No drift from s21.
- verdict: CONFIRMED

## [s22] Mandatory sibling-transplant check: func_8004A4E0 and func_8004A76C (named in this session's cross-knowledge sweep, candidates re-written 2026-10-15 17:03 / 19:31) hold a transplantable C body for the shared GTE-cluster residual.
- mechanism: both files are comments-only dispositions, not C bodies -- func_8004A4E0/candidate.c:1-13 states explicitly "func_8004A4E0 -- no pure-C candidate exists" / "this file carries NO C candidate - it is a comments-only record" (whole-body canonical-asm grant, inline_asm_canonical.txt:199, custom $s0/$s4/$s5/$s6/$fp callee-saved ABI across jal with func_8004A940/func_8004A76C/func_8004A808, 2 trapping addi, 4 ctc2, docs/grind/decisions.md:1324 OWNER-ESCALATION ROTATED 2026-10-15). func_8004A76C/candidate.c is the matching comments-only record for its own whole-body canonical-asm grant (inline_asm_canonical.txt:25, custom $s0 ABI, docs/grind/decisions.md:1345 CANONICAL-ASM GRANT PATH 2026-10-15). Neither file contains a single emitted C statement to transplant; their residuals (custom-ABI callee-saved register handoffs, trapping addi, cross-function jtbl) are categorically different from this function's residual (a GTE register-batching template literal), so even if a C body existed there is no shared block to borrow.
- probe: wc -l + grep -c for a function-body opener (`^[a-zA-Z].*(`) against both candidate.c files this session (both counts land only on comment-banner lines, confirmed by reading func_8004A4E0/candidate.c:1-30 directly); cross-checked against docs/grind/decisions.md's two 2026-10-15 disposition entries for both siblings.
- result: zero transplantable C exists in either sibling file. This reconfirms s12's "No sibling in the 3AB48 GTE cluster has a banked candidate.c body containing a transplantable ... C shape" kill under the CURRENT (2026-10-15, post s23-of-each-sibling) state of both files -- the siblings moved (both got fresh OWNER-ESCALATION/GRANT-PATH dispositions since s12) but the "no C body" fact that makes transplant vacuous did not change.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_8004A4E0/candidate.c and memory/grind/func_8004A76C/candidate.c as of this session (2026-10-15, after each sibling's s23), no FAKE construct, no src/ edit (read-only check)

## [s22] The sotn-decomp master-branch construct index (docs/reference/sotn-construct-index.md, 1,365 PSX-tagged entries) carries a citable precedent for the GTE rotation-matrix/TR-vector register-batching shape this function's hunk 1 needs (a lw/lh $tN-run batched before a ctc2 $0..$7 run, or an equivalent SetRotMatrix+ldtr composite).
- mechanism: Q55/sotn-precedent-suffices (owner ruling 2026-09-30) admits a construct item 2-3 would otherwise refuse ONLY on a verified SOTN citation; no citation means that route stays closed and the residual stays inside the vendored template (s18-s21 finding: hunk 1 is fixed at RTL expand from inline_c.h:298-307's literal asm template text, not a C-level spelling choice).
- probe: `grep -ni "setrotmatrix\|ctc2\|gte_ldtr\|RotMatrix" docs/reference/sotn-construct-index.md` this session.
- result: zero matches anywhere in the index. No SOTN-master precedent exists for this construct at the pinned commit the index was built from.
- verdict: KILLED
- kill_scope: instance
- measured_on: docs/reference/sotn-construct-index.md as checked out this session (pinned sotn-decomp master-branch commit per the file's own header), no FAKE construct, no src/ edit (read-only grep)

## [s22] The banked legal macro-call chassis (candidate.c:201-208, #include <psxsdk/inline_c.h>, 5 gte_* statements) re-measures 19 when applied fresh to src/main/3AB48.c this session, with zero FAKE constructs present.
- mechanism: dispatcher-mandated kill re-audit; an instance kill is only as good as its chassis/FAKE state at measurement time.
- probe: Applied candidate.c body to src/main/3AB48.c; sandbox func_80052B7C --disable all --diff (tmp/grind/func_80052B7C/s22/sandbox_reaudit.txt); grepped the applied body for /* FAKE markers (none); reverted to INCLUDE_ASM.
- result: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical diff shape to s6/s8/s9/s11-s21. Hunk 1 is the rotation-matrix batching (ours groups lw/ctc2 in pairs/triples; target batches 8 lw then 8 ctc2) plus TR-vector register seats; hunks 2-4 are the trailing nop/swc2 ordering.
- verdict: CONFIRMED

## [s22] func_8004A4E0 and func_8004A76C (named in this session's cross-knowledge sweep, candidates re-written 2026-10-15 at 17:03 and 19:31) hold a transplantable C body for this function's shared GTE-residual.
- mechanism: Both sibling candidate.c files are comments-only disposition records, not C bodies: func_8004A4E0/candidate.c:1-13 states explicitly it carries no C candidate (whole-body canonical-asm grant, custom $s0/$s4/$s5/$s6/$fp callee-saved ABI, inline_asm_canonical.txt:199, docs/grind/decisions.md:1324). func_8004A76C/candidate.c is the matching record for its own grant (inline_asm_canonical.txt:25, docs/grind/decisions.md:1345). Neither has a single emitted statement to borrow, and their residuals (custom-ABI register handoffs, trapping addi, cross-function jtbl) are a different problem class from this function's RTL-template-literal residual regardless.
- probe: wc -l and grep -c for a function-body opener against both candidate.c files; read func_8004A4E0/candidate.c:1-30 directly to confirm comments-only; cross-checked both siblings' 2026-10-15 decisions.md entries.
- result: Zero transplantable C exists in either sibling file. Reconfirms s12's identical finding under the current (post-each-sibling's-own-s23) state: both siblings moved to fresh dispositions since s12, but the underlying 'no C body' fact is unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_8004A4E0/candidate.c and memory/grind/func_8004A76C/candidate.c as of this session (2026-10-15, after each sibling's own s23), no FAKE construct, no src/ edit

## [s22] The sotn-decomp master-branch construct index carries a citable precedent for the GTE rotation-matrix/TR-vector register-batching shape hunk 1 needs, which would open the Q55 SOTN-precedent-suffices route.
- mechanism: Q55/sotn-precedent-suffices (owner ruling 2026-09-30) admits an otherwise-refused construct only on a verified SOTN citation; s18-s21 already established hunk 1 is fixed at RTL expand from inline_c.h:298-307's literal asm template text, so only a cited precedent construct (not a C-level respelling) could move it.
- probe: grep -ni 'setrotmatrix|ctc2|gte_ldtr|RotMatrix' docs/reference/sotn-construct-index.md (1,365 PSX-tagged entries) this session.
- result: Zero matches anywhere in the index. No SOTN-master precedent exists for this construct at the index's pinned commit.
- verdict: KILLED
- kill_scope: instance
- measured_on: docs/reference/sotn-construct-index.md as checked out this session (pinned sotn-decomp master-branch commit per the file's own header), no FAKE construct, read-only grep

## s23 (rederive)

### H15 — psyz (Xeeynamo/psyz, the PsyQ 4.0 cross-platform decomp named in [[external-references]] as the closest reference SDK version to BB2's verbatim libraries) carries a byte-level precedent for the target's batched lw/ctc2 rotation-matrix load order, or an alternate C spelling of gte_SetRotMatrix/gte_stlvl that reaches it.

Statement: psyz/src/psyz/libgte.c's `SetRotMatrix(MATRIX* m)` (fetched fresh this session via
`gh api repos/Xeeynamo/psyz/contents/psyz/src/psyz/libgte.c`, saved to
tmp/grind/func_80052B7C/s23/psyz_libgte.c) is a pure software reimplementation: nine plain
`M.m[i][j] = m->m[i][j];` field copies into a global `MATRIX M`, with zero cop2/GTE opcodes
anywhere in the function. psyz targets non-PS1 platforms (its GTE is emulated in C, per its own
README/PORTING_GUIDE), so it was never compiled through a MIPS cop2-aware backend and carries no
instruction-order information for the real hardware `ctc2` sequence at all -- there is no asm to
compare, batched or otherwise. The same file's `RotMatrix`/`RotMatrixX/Y/Z/YXZ/ZYX` and
`ApplyRotMatrix` (line 2667, delegating to a software `ApplyMatrix`) are the same story: C-level
trig/matrix math with no GTE register traffic. This closes the one genuinely new rederive-modality
lead this session found (psyz was not checked by any prior session's ledger entries for this
function) and reconfirms s12/s22's decomp.me and SOTN-index findings: no external corpus checked so
far (decomp.me, sotn-decomp master, now psyz) contains a real hardware cop2 precedent for this
residual.

- mechanism: n/a (negative corpus search, not a GCC-pass claim).
- probe: `gh api repos/Xeeynamo/psyz/contents/psyz/src/psyz/libgte.c` fetched to
  tmp/grind/func_80052B7C/s23/psyz_libgte.c (3280 lines); grepped for
  `rotmatrix|SetRotMatrix` (6 hits); read SetRotMatrix (line 1209) and ReadRotMatrix (1233) in
  full.
- result: Zero cop2 opcodes, zero GTE register traffic, zero instruction-order information in
  psyz's GTE layer for this or any sibling rotation-matrix function -- it is a full software
  reimplementation for porting, not a byte-matching PS1 decomp of libgte itself. Re-applied the
  s6 chassis fresh this session and re-measured: still 19, 4/4 hunks source-level, 0 operand-only
  (tmp/grind/func_80052B7C/s23/sandbox_diff.txt), identical hunk shape to every prior
  re-audit (s8/s9/s13-s22). Residual remains locked inside gte_SetRotMatrix's and gte_stlvl's own
  fixed `__asm__` volatile strings (include/psxsdk/inline_c.h:297, :1086), exactly as established
  since s8.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_80052B7C/s23/psyz_libgte.c (Xeeynamo/psyz, fetched 2026-10-15 this
  session); s6 legal macro-call chassis, no FAKE construct, src/main/3AB48.c this session,
  reverted to INCLUDE_ASM after measurement

## [s23] psyz (Xeeynamo/psyz, the PsyQ 4.0 cross-platform decomp named in external-references memory as the closest reference SDK version to BB2's verbatim libraries) carries a byte-level precedent for the target's batched lw/ctc2 rotation-matrix load order, or an alternate C spelling of gte_SetRotMatrix/gte_stlvl that reaches it.
- mechanism: n/a (negative corpus search, not a GCC-pass claim)
- probe: gh api repos/Xeeynamo/psyz/contents/psyz/src/psyz/libgte.c fetched to tmp/grind/func_80052B7C/s23/psyz_libgte.c (3280 lines); grepped for rotmatrix|SetRotMatrix (6 hits); read SetRotMatrix (line 1209) and ReadRotMatrix (1233) in full; re-applied the s6 legal chassis and re-measured sandbox func_80052B7C --disable all --diff.
- result: psyz's SetRotMatrix/RotMatrix*/ApplyRotMatrix are pure software reimplementations (plain MATRIX field copies and trig math for cross-platform porting) with zero cop2/GTE opcodes anywhere -- it was never compiled through a MIPS cop2-aware backend, so it carries no hardware instruction-order information for this residual at all. Re-measured the s6 chassis fresh: still score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored (tmp/grind/func_80052B7C/s23/sandbox_diff.txt), identical hunk shape to every re-audit since s8. This was the one genuinely new external corpus this session's rederive-modality search found not yet checked by any prior ledger entry for this function (decomp.me and the sotn-decomp construct index were already checked at s12/s22, both negative).
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/grind/func_80052B7C/s23/psyz_libgte.c (Xeeynamo/psyz, fetched 2026-10-15 this session); s6 legal macro-call chassis, no FAKE construct, src/main/3AB48.c this session, reverted to INCLUDE_ASM after measurement

## [s24] KILL RE-AUDIT (structural): the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored. Identical to s6/s8/s13-s23. See tmp/grind/func_80052B7C/s24/chassis_sandbox_diff.txt variant A.

## [s24] Declaring named s16 locals (trx/trY/trz) for tr->vx/vy/vz before gte_SetRotMatrix, instead of passing the struct-member reads directly as gte_ldtr's arguments, moves where the 3 sub-word loads land in the instruction stream (front of the chunk vs after gte_SetRotMatrix's own loads) but changes nothing scored: still lhu not the target's lh, still batched per-macro-string not interleaved with gte_SetRotMatrix's loads, hunks 2-4 unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis via psxsdk/inline_c.h (gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl), named s16 locals for the ldtr args declared before gte_SetRotMatrix, no FAKE construct, src/main/3AB48.c this session (s24), reverted after measurement
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored. See tmp/grind/func_80052B7C/s24/chassis_sandbox_diff.txt variant B. Extends s19's declaration-order probe (which tested reordering gte_ldtr vs gte_SetRotMatrix as whole statements) to the finer-grained axis of naming the sub-word reads as locals vs leaving them as inline member expressions; same null result.

## [s24] GCC 2.7.2 emits lhu (not lh) for the s16 struct-member reads (tr->vx/vy/vz) feeding gte_ldtr's "r" asm operands regardless of C-level declaration form (direct member expression vs named s16 local, load position before/after gte_SetRotMatrix), so no structural (block-local-var-split/declaration-order/type-narrowing/statement-reassociation) lever reaches the target's lh opcode for those 3 loads.
- verdict: KILLED
- kill_scope: instance
- measured_on: both variants A and B above, legal macro-call chassis, no FAKE construct, src/main/3AB48.c this session (s24)
- result: both variants emit lhu t0/v1/v0 (or lhu v0/v1/t0) for the 3 loads; target is lh t5/t6/t7. Re-declaring the already-signed s16 type as a fresh local makes no difference -- GCC's choice of lhu over lh for a value only consumed by an asm "r" operand with no further arithmetic is independent of the C-level staging tried here. Narrower than a class kill: only declaration-order and local-staging were tried on THIS macro ("r" constraint, no asm modifier); a different constraint/modifier spelling of gte_ldtr is untested.

## Current state (s24)
floor 19, 24 sessions; structural modality's lever set (block-local var splits, declaration
order, type narrowing via named locals, statement re-association) re-confirmed exhausted on
this chassis for the 4th+ time (s8, s13 x2, s14, s19 x2, now s24's finer-grained sub-word-load
staging variant). The residual's only remaining open frontier is the canonical-asm grant-path
question (owner/Judge review of inline_asm_canonical.txt:340's whole-body grant, the
scan_hand_coded gate-class signal work, a vendored macro for the batched lw/lh->ctc2 idiom) --
none of which is a grind-session structural probe. See docs/grind/decisions.md 2026-10-15
20:38 ruling and live frontier entries carried from s18-s23.

## [s24] The banked legal macro-call chassis (#include <psxsdk/inline_c.h>; gte_SetRotMatrix(rot); gte_ldtr(tr->vx,tr->vy,tr->vz); gte_ldv0(v0); gte_rt(); gte_stlvl(out);) still measures 19 on the current tree with zero FAKE carriers.
- mechanism: Re-audit of the kill ledger's mandated dispatcher condition before any new probe this session.
- probe: Applied the chassis live to src/main/3AB48.c, ran sandbox func_80052B7C --disable all --diff, then reverted to INCLUDE_ASM.
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical to s6/s8/s13-s23.
- verdict: CONFIRMED

## [s24] Declaring named s16 locals (trx/trY/trz) for tr->vx/vy/vz before gte_SetRotMatrix, instead of passing the struct-member reads directly as gte_ldtr's arguments, changes the scored distance.
- mechanism: Structural-modality lever: block-local variable split + declaration order, applied to the one region of the chassis (the 3 ldtr operand loads) that s19 confirmed is RA-owned rather than template-literal.
- probe: Edited src/main/3AB48.c to declare the 3 locals before gte_SetRotMatrix and pass them to gte_ldtr instead of tr->vx/vy/vz directly; ran sandbox --diff; reverted.
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only. The 3 loads move earlier in the instruction stream (now scheduled before gte_SetRotMatrix's own lw's instead of after) but still emit lhu not the target's lh, and are still grouped as their own 3-load/3-ctc2 batch rather than interleaved with gte_SetRotMatrix's loads+ctc2's as the target requires.
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis via psxsdk/inline_c.h, named s16 locals for the 3 ldtr args declared before gte_SetRotMatrix, no FAKE construct, src/main/3AB48.c this session (s24), reverted after measurement

## [s24] GCC 2.7.2's choice of lhu (not the target's lh) for the s16 struct-member loads feeding gte_ldtr's three 'r' asm operands is independent of the C-level declaration form tried (direct member expression vs named s16 local, load position before/after gte_SetRotMatrix).
- mechanism: Both variant A (direct tr->vx/vy/vz args) and variant B (named locals) are already-signed s16 reads consumed only by an asm 'r' operand with no further arithmetic; GCC's load-widening choice for that pattern evidently does not depend on the source-level staging exercised here.
- probe: Compared the hunk-1 diff of both variants measured this session.
- result: Both variants emit lhu for all 3 loads; target is lh. No declaration-order or local-staging respelling tried (in s19 or s24) reaches lh.
- verdict: KILLED
- kill_scope: instance
- measured_on: variants A and B above, legal macro-call chassis, no FAKE construct, src/main/3AB48.c this session (s24)

## [s25] memory/grind/func_80052B7C/candidate.c had a comment-nesting corruption that broke `sandbox --candidate` entirely, independent of any codegen question.
- mechanism: Two spots in the ledger's header comment accidentally closed the C comment early: (1) old-line 231 had literal text "RotMatrix*/ApplyRotMatrix" where the mid-word `*/` closed the s23 comment prematurely; (2) old-line 16 had a bare `#include <psxsdk/inline_c.h>   (NOT inline_o.h -- see s2 finding 1a)` sitting between two comment blocks, which left 7 lines of an illustrative written-island C sketch (lines 17-23) stranded outside any comment as raw, unparseable tokens. Both bugs cascaded into dozens of cpp "missing terminating ' character" / "nondigits in number" / parse errors across the whole 5700-line 3AB48.c TU once the candidate's header was substituted into it, which is exactly the "measurement unavailable" the s25 dispatch chassis-check reported.
- probe: Balanced a `/* ... */` depth-scan (tmp Python one-liner) over candidate.c to find both breaks; fixed (1) by removing the mid-word `*/` (now reads ".../RotMatrix/ApplyRotMatrix -- ..."); fixed (2) by removing the bare #include and re-wording it as plain comment prose, keeping the one real `/* ... */` block intact from line 1 to line 80. Re-ran `sandbox func_80052B7C --disable all --candidate memory/grind/func_80052B7C/candidate.c --diff`.
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- exactly matches the ledger's recorded floor and the s6/s8/s13-s24 hunk classification (hunk 1 = SetRotMatrix+ldtr batching/register-seat, hunks 2-4 = gte_stlvl delay-slot swc2 placement). No codegen change; this is a pure ledger-hygiene fix. candidate.c is now buildable again for future sessions' dispatch-time chassis check.
- verdict: CONFIRMED

## [s25] Removing the redundant `(s32)` casts on gte_ldtr's three arguments (passing `tr->vx/vy/vz` directly instead of `(s32)tr->vx` etc.) changes the scored distance.
- mechanism: Structural-modality lever (type narrowing / redundant-cast removal) on the one RA-owned region of the chassis (s19's finding).
- probe: tmp/grind/func_80052B7C/s25/variant_nocast.c -- `gte_ldtr(tr->vx, tr->vy, tr->vz);` with no explicit cast. Measured with sandbox --candidate (no --diff needed, score alone suffices to compare).
- result: score 19, 24/24 insns -- byte-identical score to the cast form. The explicit `(s32)` cast is a no-op for this macro's `"r"` operand (GCC already promotes the s16 lvalue to the register width when binding it to an asm input operand); no instruction-selection or RA difference.
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis, `gte_ldtr` args with casts removed, no FAKE construct, tmp/grind/func_80052B7C/s25/variant_nocast.c this session (s25)

## [s25] Block-scoping three freshly declared `s32` locals (trX/trY/trZ, widened from the s16 members, assigned from tr->vx/vy/vz) around the gte_ldtr call, instead of the direct member-expression args, changes the scored distance.
- mechanism: Structural-modality lever (block-local variable split + type narrowing combined), distinct from s24's variant B: s24 used *s16*-typed locals declared *before* gte_SetRotMatrix; this probe uses *s32*-typed locals declared in a nested block *at* the gte_ldtr call site, to separate the type axis from the declaration-position axis s24 already killed.
- probe: tmp/grind/func_80052B7C/s25/variant_s32locals.c -- `{ s32 trX = tr->vx; s32 trY = tr->vy; s32 trZ = tr->vz; gte_ldtr(trX, trY, trZ); }` in place of the direct-arg call, SetRotMatrix call left untouched before the block. Measured with sandbox --candidate.
- result: score 19, 24/24 insns -- identical to the direct-arg chassis and to s24's variant B. Confirms s24's finding generalizes across the type axis too: no combination of local-staging (declared where, declared what width) reaches a different score. This exhausts the last two structural sub-axes (redundant-cast removal, type-widened block-local) not explicitly covered by s8/s9/s13-s16's `spelling_enum.py` sweeps (which only detect *named* locals/assignments already in the body, not ones a session adds and measures by hand).
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis, s32 block-local staging for the 3 ldtr args, no FAKE construct, tmp/grind/func_80052B7C/s25/variant_s32locals.c this session (s25)

## [s25] memory/grind/func_80052B7C/candidate.c had a comment-nesting corruption that broke `sandbox --candidate` entirely, independent of any codegen question.
- mechanism: A stray mid-word comment-close sequence at old-line 231 ('RotMatrix' + close-comment + 'ApplyRotMatrix') and a bare #include stranding 7 lines of an illustrative written-island sketch outside its enclosing comment (old-line 16) both broke the file's C comment nesting, cascading into dozens of cpp parse errors once the candidate's header was substituted into the 5700-line 3AB48.c TU.
- probe: Depth-scanned /* */ nesting with a Python one-liner to locate both breaks; fixed both; re-ran sandbox func_80052B7C --disable all --candidate memory/grind/func_80052B7C/candidate.c --diff.
- result: Score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- exactly matches the ledger's recorded floor and prior hunk classification (hunk 1 = SetRotMatrix+ldtr batching/register-seat, hunks 2-4 = gte_stlvl delay-slot swc2 placement). Pure ledger-hygiene fix, no codegen change. candidate.c is buildable again for future dispatch-time chassis checks.
- verdict: CONFIRMED

## [s25] Removing the redundant (s32) casts on gte_ldtr's three arguments (passing tr->vx/vy/vz directly instead of (s32)tr->vx etc.) changes the scored distance.
- mechanism: Structural-modality lever (type narrowing / redundant-cast removal) on the one RA-owned region of the chassis (s19's finding).
- probe: tmp/grind/func_80052B7C/s25/variant_nocast.c measured with sandbox --candidate.
- result: Score 19, 24/24 insns -- byte-identical score to the cast form. The explicit (s32) cast is a no-op for gte_ldtr's 'r' asm operand.
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis, gte_ldtr args with casts removed, no FAKE construct, tmp/grind/func_80052B7C/s25/variant_nocast.c this session (s25)

## [s25] Block-scoping three freshly declared s32 locals (widened from the s16 members, assigned from tr->vx/vy/vz) around the gte_ldtr call, instead of the direct member-expression args, changes the scored distance.
- mechanism: Structural-modality lever (block-local variable split + type narrowing combined), distinct from s24's variant B which used s16-typed locals declared before gte_SetRotMatrix; this probe isolates the type-width axis from the declaration-position axis.
- probe: tmp/grind/func_80052B7C/s25/variant_s32locals.c measured with sandbox --candidate.
- result: Score 19, 24/24 insns -- identical to the direct-arg chassis and to s24's variant B. Confirms s24's null finding generalizes across the declared-width axis too; exhausts the last two structural sub-axes (redundant-cast removal, type-widened block-local) not covered by the spelling_enum.py sweeps.
- verdict: KILLED
- kill_scope: instance
- measured_on: legal macro-call chassis, s32 block-local staging for the 3 ldtr args, no FAKE construct, tmp/grind/func_80052B7C/s25/variant_s32locals.c this session (s25)

## [s26] Re-running spelling_enum.py (both --no-swaps and swap-axis) on the full 5-statement ENUM-BEGIN/END region, on the current chassis, still finds zero mutation axes.
- mechanism: Enumerate-modality mandated re-sweep (fifth consecutive enumerate pass after s9, s15, s16, s17). spelling_enum.py's region scanner can only mutate named locals, assignments, and commutative binary operators it finds inside the markers; it reports its precondition counts before emitting variants.
- probe: Re-applied memory/grind/func_80052B7C/candidate.c fresh to src/main/3AB48.c, re-measured with `sandbox func_80052B7C --disable all --candidate ... --diff` (tmp/grind/func_80052B7C/s26/sandbox_diff.txt). Then wrapped the identical 5-statement body (gte_SetRotMatrix / gte_ldtr / gte_ldv0 / gte_rt / gte_stlvl) in ENUM-BEGIN/END markers in a scratch copy (tmp/grind/func_80052B7C/s26/enum_candidate.c, candidate.c itself untouched) and ran spelling_enum.py with --no-swaps (tmp/grind/func_80052B7C/s26/enum_noswap) and with the swap axis (tmp/grind/func_80052B7C/s26/enum_swap).
- result: Chassis re-confirmed at score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- byte-identical classification to s6/s8/s13-s25 (hunk 1 = gte_SetRotMatrix+gte_ldtr batching/register-seat, hunks 2-4 = gte_stlvl's delay-slot swc2 placement). Both enum runs report "0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings", with and without the swap axis -- identical, word-for-word, to s9/s15/s16/s17's results. The region already spans the entire function body (no neighbouring block exists to widen into, per s15/s16's note). No new FAKE carrier present to re-audit (fake_ablate.py already confirmed clean at s14, chassis unchanged since).
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call chassis (memory/grind/func_80052B7C/candidate.c as landed s18-s25), no FAKE construct, tmp/grind/func_80052B7C/s26/enum_noswap and s26/enum_swap this session (s26)
- predicate_cite: include/psxsdk/inline_c.h:297
- note: tools/spelling_enum.py:27 is the region-scanner precondition count (0 named locals / 0 assignments / 0 commutative operators under both the no-swap and swap-axis settings); this is the fifth independent measurement (s9, s15, s16, s17, s26) of the identical null result on an unchanged chassis, standing class evidence that the C-level spelling-sweep search space for this chassis is empty. The residual sits inside each macro's own fixed __asm__ volatile string, include/psxsdk/inline_c.h:297 (gte_SetRotMatrix) and :1086 (gte_stlvl), read in full at s8/s13/s14, which is not C-level syntax the region scanner can see or vary.

## [s26] Re-running spelling_enum.py (both --no-swaps and swap-axis) on the full 5-statement ENUM-BEGIN/END region, on the current chassis, still finds zero mutation axes.
- mechanism: Enumerate-modality mandated re-sweep. spelling_enum.py's region scanner can only mutate named locals, assignments, and commutative binary operators it finds inside the markers.
- probe: Re-applied memory/grind/func_80052B7C/candidate.c, re-measured with sandbox --candidate --diff (tmp/grind/func_80052B7C/s26/sandbox_diff.txt); wrapped the 5-statement body in ENUM-BEGIN/END markers in a scratch copy (tmp/grind/func_80052B7C/s26/enum_candidate.c) and ran spelling_enum.py --no-swaps (s26/enum_noswap) and with the swap axis (s26/enum_swap).
- result: Chassis re-confirmed: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical classification to s6/s8/s13-s25. Both enum runs report '0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings', word-for-word identical to s9/s15/s16/s17. The region already spans the entire function body; no neighbouring block exists to widen into.
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call chassis (memory/grind/func_80052B7C/candidate.c as landed s18-s25), no FAKE construct, tmp/grind/func_80052B7C/s26/enum_noswap and s26/enum_swap this session (s26)
- predicate_cite: include/psxsdk/inline_c.h:297
