# Hypothesis ledger — func_80052B7C

> COMPACTED 2026-10-15 by `grindlib.py compact-ledger` (65,749 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 15fbdbf1c:memory/grind/func_80052B7C/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
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
floor 19 at s26; 26 sessions; live frontier (state.json is authoritative):
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

## [s26] Re-running spelling_enum.py (both --no-swaps and swap-axis) on the full 5-statement ENUM-BEGIN/END region, on the current chassis, still finds zero mutation axes.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:297
- result: Chassis re-confirmed at score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- byte-identical classification to s6/s8/s13-s25 (hunk 1 = gte_SetRotMatrix+gte_ldtr batching/register-seat, hunks 2-4 = gte_stlvl's delay-slot swc2 placement). Both enum runs report "0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings", with and without the swap axis -- identical,…

## [s26] Re-running spelling_enum.py (both --no-swaps and swap-axis) on the full 5-statement ENUM-BEGIN/END region, on the current chassis, still finds zero mutation axes.
- verdict: KILLED
- kill_scope: class
- predicate_cite: include/psxsdk/inline_c.h:297
- result: Chassis re-confirmed: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical classification to s6/s8/s13-s25. Both enum runs report '0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings', word-for-word identical to s9/s15/s16/s17. The region already spans the entire function body; no neighbouring block exists to widen into.

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

## [s22] KILL RE-AUDIT (rederive): the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 4/4 source-level hunks, 0 operand-only, 0 not-scored -- byte-identical diff shape to s6/s8/s9/s11-s21. Hunk 1 is the rotation-matrix lw/ctc2 batch (ours: 2+3+3 lw/ctc2 grouped by register pair; target: 8 lw then 8 ctc2 fully batched) plus the TR loads landing in {a0,v1,v0} vs target's {t5,t6,t7}; hunks 2-4 are the trailing nop/swc2/nop ordering around gte_stlvl. No drift from s21.

## [s22] The banked legal macro-call chassis (candidate.c:201-208, #include <psxsdk/inline_c.h>, 5 gte_* statements) re-measures 19 when applied fresh to src/main/3AB48.c this session, with zero FAKE constructs present.
- verdict: CONFIRMED
- result: score 19, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical diff shape to s6/s8/s9/s11-s21. Hunk 1 is the rotation-matrix batching (ours groups lw/ctc2 in pairs/triples; target batches 8 lw then 8 ctc2) plus TR-vector register seats; hunks 2-4 are the trailing nop/swc2 ordering.

## [s24] KILL RE-AUDIT (structural): the banked legal macro-call chassis re-measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored. Identical to s6/s8/s13-s23. See tmp/grind/func_80052B7C/s24/chassis_sandbox_diff.txt variant A.

## [s24] The banked legal macro-call chassis (#include <psxsdk/inline_c.h>; gte_SetRotMatrix(rot); gte_ldtr(tr->vx,tr->vy,tr->vz); gte_ldv0(v0); gte_rt(); gte_stlvl(out);) still measures 19 on the current tree with zero FAKE carriers.
- verdict: CONFIRMED
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- identical to s6/s8/s13-s23.

## [s25] memory/grind/func_80052B7C/candidate.c had a comment-nesting corruption that broke `sandbox --candidate` entirely, independent of any codegen question.
- verdict: CONFIRMED
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- exactly matches the ledger's recorded floor and the s6/s8/s13-s24 hunk classification (hunk 1 = SetRotMatrix+ldtr batching/register-seat, hunks 2-4 = gte_stlvl delay-slot swc2 placement). No codegen change; this is a pure ledger-hygiene fix. candidate.c is now buildable again for future sessions' dispatch-time chassis…

## [s25] memory/grind/func_80052B7C/candidate.c had a comment-nesting corruption that broke `sandbox --candidate` entirely, independent of any codegen question.
- verdict: CONFIRMED
- result: Score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored -- exactly matches the ledger's recorded floor and prior hunk classification (hunk 1 = SetRotMatrix+ldtr batching/register-seat, hunks 2-4 = gte_stlvl delay-slot swc2 placement). Pure ledger-hygiene fix, no codegen change. candidate.c is buildable again for future dispatch-time chassis checks.

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
- verdict: KILLED (instance)

## [s12] The func_800203B4 island-grant precedent (inline_asm_canonical.txt:357: thin GTE islands authorized AS UNITS after pure-C respelling measured unable to close, owner-granted 2026-09-01) reopens the written-island questio…
- verdict: KILLED (instance)

## [s12] The decomp.me corpus (gcc2.7.2-psx compiler bucket) contains a scratch whose target asm is similar enough to func_80052B7C's batched-GTE-register-load pattern to transplant a different C shape.
- verdict: KILLED (instance)

## [s12] No sibling in the 3AB48 GTE cluster (func_8004A4E0, func_8004A76C) has a banked candidate.c body containing a transplantable SetRotMatrix/ctc2-batching or delay-slot C shape for this function's residual.
- verdict: KILLED (instance)

## [s14] The s6 legal chassis contains zero FAKE-annotated constructs, so the KILL RE-AUDIT dispatcher condition (re-measure the closest instance kill with every FAKE construct ablated) is vacuously satisfied -- there is no FAKE…
- verdict: KILLED (instance)

## [s19] For the RA-owned subset (gte_ldtr's 3 operands), moving the tr->vx/vy/vz loads into named locals before gte_SetRotMatrix, or issuing gte_ldtr before gte_SetRotMatrix, moves those operands toward the target's t5..t7.
- verdict: KILLED (instance)

## [s19] Loading tr->vx/vy/vz into named locals before gte_SetRotMatrix, or issuing gte_ldtr first, moves the three ldtr operands to the target's t5..t7.
- verdict: KILLED (instance)

## [s21] Replacing gte_rt() with the DMPSX bare-word variant gte_rt_b() (inline_c.h:815, no leading nops) changes the scored distance.
- verdict: KILLED (instance)

## [s21] Replacing gte_rt() with the DMPSX bare-word gte_rt_b() (inline_c.h:815) changes the scored distance on the legal chassis.
- verdict: KILLED (instance)

## [s22] Mandatory sibling-transplant check: func_8004A4E0 and func_8004A76C (named in this session's cross-knowledge sweep, candidates re-written 2026-10-15 17:03 / 19:31) hold a transplantable C body for the shared GTE-cluster…
- verdict: KILLED (instance) — zero transplantable C exists in either sibling file. This reconfirms s12's "No sibling in the 3AB48 GTE cluster has a banked candidate.c body containing a tran…

## [s22] The sotn-decomp master-branch construct index (docs/reference/sotn-construct-index.md, 1,365 PSX-tagged entries) carries a citable precedent for the GTE rotation-matrix/TR-vector register-batching shape this function's…
- verdict: KILLED (instance) — zero matches anywhere in the index. No SOTN-master precedent exists for this construct at the pinned commit the index was built from.

## [s22] func_8004A4E0 and func_8004A76C (named in this session's cross-knowledge sweep, candidates re-written 2026-10-15 at 17:03 and 19:31) hold a transplantable C body for this function's shared GTE-residual.
- verdict: KILLED (instance) — Zero transplantable C exists in either sibling file. Reconfirms s12's identical finding under the current (post-each-sibling's-own-s23) state: both siblings mo…

## [s22] The sotn-decomp master-branch construct index carries a citable precedent for the GTE rotation-matrix/TR-vector register-batching shape hunk 1 needs, which would open the Q55 SOTN-precedent-suffices route.
- verdict: KILLED (instance) — Zero matches anywhere in the index. No SOTN-master precedent exists for this construct at the index's pinned commit.

## [s23] psyz (Xeeynamo/psyz, the PsyQ 4.0 cross-platform decomp named in external-references memory as the closest reference SDK version to BB2's verbatim libraries) carries a byte-level precedent for the target's batched lw/ct…
- verdict: KILLED (instance) — psyz's SetRotMatrix/RotMatrix*/ApplyRotMatrix are pure software reimplementations (plain MATRIX field copies and trig math for cross-platform porting) with zer…

## [s24] Declaring named s16 locals (trx/trY/trz) for tr->vx/vy/vz before gte_SetRotMatrix, instead of passing the struct-member reads directly as gte_ldtr's arguments, moves where the 3 sub-word loads land in the instruction st…
- verdict: KILLED (instance) — score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored. See tmp/grind/func_80052B7C/s24/chassis_sandbox_diff.txt variant B. Extends s19's…

## [s24] GCC 2.7.2 emits lhu (not lh) for the s16 struct-member reads (tr->vx/vy/vz) feeding gte_ldtr's "r" asm operands regardless of C-level declaration form (direct member expression vs named s16 local, load position before/a…
- verdict: KILLED (instance) — both variants emit lhu t0/v1/v0 (or lhu v0/v1/t0) for the 3 loads; target is lh t5/t6/t7. Re-declaring the already-signed s16 type as a fresh local makes no di…

## [s24] Declaring named s16 locals (trx/trY/trz) for tr->vx/vy/vz before gte_SetRotMatrix, instead of passing the struct-member reads directly as gte_ldtr's arguments, changes the scored distance.
- verdict: KILLED (instance) — score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only. The 3 loads move earlier in the instruction stream (now scheduled before gte_SetRotMatrix's own…

## [s24] GCC 2.7.2's choice of lhu (not the target's lh) for the s16 struct-member loads feeding gte_ldtr's three 'r' asm operands is independent of the C-level declaration form tried (direct member expression vs named s16 local…
- verdict: KILLED (instance) — Both variants emit lhu for all 3 loads; target is lh. No declaration-order or local-staging respelling tried (in s19 or s24) reaches lh.

## [s25] Removing the redundant `(s32)` casts on gte_ldtr's three arguments (passing `tr->vx/vy/vz` directly instead of `(s32)tr->vx` etc.) changes the scored distance.
- verdict: KILLED (instance) — score 19, 24/24 insns -- byte-identical score to the cast form. The explicit `(s32)` cast is a no-op for this macro's `"r"` operand (GCC already promotes the s…

## [s25] Block-scoping three freshly declared `s32` locals (trX/trY/trZ, widened from the s16 members, assigned from tr->vx/vy/vz) around the gte_ldtr call, instead of the direct member-expression args, changes the scored distan…
- verdict: KILLED (instance) — score 19, 24/24 insns -- identical to the direct-arg chassis and to s24's variant B. Confirms s24's finding generalizes across the type axis too: no combinatio…

## [s25] Removing the redundant (s32) casts on gte_ldtr's three arguments (passing tr->vx/vy/vz directly instead of (s32)tr->vx etc.) changes the scored distance.
- verdict: KILLED (instance) — Score 19, 24/24 insns -- byte-identical score to the cast form. The explicit (s32) cast is a no-op for gte_ldtr's 'r' asm operand.

## [s25] Block-scoping three freshly declared s32 locals (widened from the s16 members, assigned from tr->vx/vy/vz) around the gte_ldtr call, instead of the direct member-expression args, changes the scored distance.
- verdict: KILLED (instance) — Score 19, 24/24 insns -- identical to the direct-arg chassis and to s24's variant B. Confirms s24's null finding generalizes across the declared-width axis too…

## Latest session s27 (verbatim)

## [s27] KILL RE-AUDIT: the banked legal macro-call chassis (candidate.c: #include <psxsdk/inline_c.h>; gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl) re-measures 19 on the current tree with zero FAKE carriers.
- mechanism: dispatcher-mandated re-audit (floor flat 3+ sessions, instance kills exist); an instance kill is only as good as its chassis/FAKE state at measurement time.
- probe: sandbox func_80052B7C --disable all --diff (tmp/grind/func_80052B7C/s27/sandbox_diff.txt); python3 tools/fake_ablate.py --func func_80052B7C --file main/3AB48 --candidate memory/grind/func_80052B7C/candidate.c
- result: score 19, 24/24 insns, 4/4 hunks source-level, 0 operand-only, 0 not-scored, byte-identical diff shape to s6-s26; fake_ablate reports no FAKE-annotated constructs, nothing to ablate (identical to s14/s18-s26). No instance kill voided by chassis or FAKE drift.
- verdict: CONFIRMED

## [s27] A fresh spelling_enum.py sweep of the exact 5-statement macro-call region (gte_SetRotMatrix through gte_stlvl, no decl/signature lines, both --no-swaps and the operand-swap axis) on the current floor-19 chassis finds a non-empty C-level mutation space.
- mechanism: n/a (mechanical sweep, not a GCC-pass claim).
- probe: python3 tools/spelling_enum.py --candidate tmp/grind/func_80052B7C/s27/enum_candidate2.c --out tmp/grind/func_80052B7C/s27/enum_noswap2 --no-swaps; same without --no-swaps -> enum_swap2.
- result: Both axes report '0 named locals (), 0 assignments, 5 anchors -> 1 distinct spellings' verbatim, byte-identical to s9/s15/s16/s17/s26. Sixth independent enumerate-modality confirmation the spelling-sweep search space on this exact chassis is empty. A separate wider-region attempt this session (also wrapping the #include and function-signature lines) produced a spurious '16 distinct spellings' result caused by spelling_enum.py's commutative-swap pass misreading a pointer declarator 'T * name' as a binary multiply and swapping it into invalid C ('out * VECTOR') -- none of those 16 variants is syntactically valid, so this is a tool artifact from mis-scoping the region, not a real finding; documented in candidate.c and hypotheses.md so no future session re-proposes it.
- verdict: KILLED
- kill_scope: class
- measured_on: legal macro-call chassis (candidate.c body, #include <psxsdk/inline_c.h>), no FAKE construct, exact 5-statement region scoping identical to s9/s15/s16/s17/s26, this session
- predicate_cite: tools/spelling_enum.py:89

## [s28] KILL RE-AUDIT + disposition: banked legal macro-call chassis re-measures 19 on current tree, no FAKE; both endgame gates fail.
- probe: sandbox --disable all --diff --candidate candidate.c (tmp/grind/func_80052B7C/s28/sandbox_diff.txt); fake_ablate.py (s28/fake_ablate.txt); scan_hand_coded.py --single (s28/scan.txt).
- result: score 19, 4 source-level / 0 operand-only / 0 not-scored hunks, diff shape identical to s6-s27; nothing to ablate; scan LOW 0/12 no gate-class signal; cc1psx_check ours 19 = psx 19. Rotation record filed: docs/grind/decisions.md "2026-10-15 — func_80052B7C — OWNER-ESCALATION — LADDER EXHAUSTED (non-endgame residual, floor 19): ROTATED".
- verdict: CONFIRMED

## [s28] KILL RE-AUDIT: the banked legal macro-call chassis (inline_c.h gte_SetRotMatrix/gte_ldtr/gte_ldv0/gte_rt/gte_stlvl) re-measures at floor 19 on the current tree with zero FAKE carriers, so no instance kill is voided by chassis or FAKE drift.
- mechanism: Dispatcher-mandated re-audit; an instance kill is only valid under its chassis/FAKE state.
- probe: sandbox func_80052B7C --disable all --diff --candidate memory/grind/func_80052B7C/candidate.c; tools/fake_ablate.py on the same candidate.
- result: score 19, 24/24 insns, 4 source-level / 0 operand-only / 0 not-scored hunks, identical shape to s6-s27; fake_ablate: no FAKE constructs, nothing to ablate.
- verdict: CONFIRMED

## [s28] The scan_hand_coded gate (a) reaches STRONG tier for func_80052B7C on the current scanner signal set.
- mechanism: Endgame-lock AND-gate (a): canonical-asm grant path requires a gate-class signal S1/S2/S6-S11.
- probe: python3 tools/scan_hand_coded.py --single func_80052B7C > tmp/grind/func_80052B7C/s28/scan.txt
- result: tier LOW, score 0/12, gate-class signals none (26 insns; S3/S4 inapplicable as too short).
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_80052B7C.s target, scanner as of this session, no C chassis involved
