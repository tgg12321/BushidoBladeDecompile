# Hypothesis ledger — func_8004A4E0

> COMPACTED 2026-10-15 by `grindlib.py compact-ledger` (66,541 bytes, cap 64K). Older entries are one-line index items;
> full text: `git show 7bdc5a300:memory/grind/func_8004A4E0/hypotheses.md` (history before 2026-10-01: tag pre-slim-2026-10-01).
> Look one up only when its index line bears on your probe. Append below as usual.

## H1 — a pure-C (or partial C-island) form can replace the whole-body asm
**Mechanism:** none proposed this session — this is the audit's own conclusion, inherited,
not re-derived. `docs/audits/func_8004A4E0-canonical-2026-09-29.md` (owner ruling Q40,
803d0fea1) shows 5 cross-call register handoffs ($s4/$s5/$s6/$fp/$s0) that only make sense
under a custom callee-saved-register ABI shared with `func_8004A940`/`func_8004A76C`/
`func_8004A808`: dead-looking pre-call stores the callee actually reads, a `3-$s5` the callee
itself writes (so GCC would fold the subtraction to a constant, which the asm doesn't do), a
ghost `$s3` save/restore the callee clobbers unsaved, and a callee-advanced `$s0` read back
after return (GCC assumes a callee-save is unchanged across any call it didn't write to
itself). Covering each handoff with C islands would require forbidden GPR-steering asm at
every site, which collapses the "island" boundary to the full body. Plus 2 non-overflow-safe
trapping `addi` and 4 `ctc2` with no C form.
**Probe:** `canonical func_8004A4E0` (ASM-WHOLE, this session) + re-read of the audit; no new
measurement attempted — `sandbox --disable all` confirms `no_c_body: true` at 163, i.e. there
is currently no C to even partially diff.
**Result:** KILLED.
**kill_scope:** class.
**measured_on:** whole-body INCLUDE_ASM chassis, zero FAKE constructs (none exist — no C body
at all); inherited from the 2026-09-29 audit, re-confirmed by this session's `canonical` +
`sandbox` runs.
… (preamble truncated — full text in git, above)

## Current state (at compaction)
floor 163 at s17; 17 sessions; live frontier (state.json is authoritative):
  - No live pure-C or rederive-axis hypothesis remains for func_8004A4E0; the cross-function jtbl dispatch + custom callee-saved ABI class kill (s8/s10/s12/s13) is independently re-confirmed by every modality tried across 1…

## Class-level kills (binding until the cited predicate changes)

## [s1] A pure-C (or partial C-island) form can replace the whole-body asm for func_8004A4E0.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: No C body exists; the audit's cross-call ABI argument accounts for every operand this session re-confirmed in asm/funcs/func_8004A4E0.s (D_80095328, D_80095488, D_80095508, D_800FF610, g_draw_queue, D_80103608, jtbl_8001541C, jtbl_8001545C all present as real operands, none reachable from any C statement since the body is whole-body INCLUDE_ASM).

## [s2] structural-modality dispatch — re-confirm, do not re-derive, the standing class kill.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: standing H1 class kill holds unchanged; structural modality has no applicable

## [s2] A pure-C (or partial C-island) form can replace the whole-body asm for func_8004A4E0, re-tested under structural modality.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: Re-confirmed the standing s1 class kill; no new ground covered, no regression.

## [s4] The enumerate modality's systematic spelling sweep (owner ruling 2026-09-08) has an applicable region to sweep for func_8004A4E0.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: Re-confirmed the standing s1 class kill for a fourth consecutive modality (recon, structural, permuter, enumerate all land on the same no-C-body fact). No regression, no new ground.

## [s4] The enumerate modality's systematic spelling sweep (owner ruling 2026-09-08) has an applicable region to sweep for func_8004A4E0.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: Re-confirmed the standing s1 class kill for a fourth consecutive modality (recon, structural, permuter, enumerate all land on the same no-C-body fact). No regression, no new ground covered.

## [s8] REDERIVE — NEW, DECISIVE: func_8004A4E0's own switch body computed-jumps into the MIDDLE of a different function (func_8004A940), past that function's prologue, sharing live GPR state. No C switch/goto/function-pointer construct can express this under GCC 2.7.2's per-function compilation model.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/toplev.c:2647 (rest_of_compilation compiles one function's DECL to completion before the next); tools/gcc-2.7.2/stmt.c:4704 (expand_end_case's…
- result: 8 of 16 jump-table entries resolve to labels that exist only inside a different function's own body, with no shared/export marking; GCC's per-DECL compilation model and per-function label scoping mean no switch/goto lowering can reproduce this from any C source.

## [s8] func_8004A4E0's own switch-style computed jump (via jtbl_8001541C) transfers control directly into the middle of a DIFFERENT function's (func_8004A940) compiled body, bypassing func_8004A940's own prologue and sharing func_8004A4E0's live register state with it — a shape no switch/goto/function-pointer lowering under…
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/toplev.c:2647
- result: confirmed by direct grep: those 8 labels are private to func_8004A940's own body. func_8004A4E0's dispatch therefore free-falls into func_8004A940 mid-function, past its prologue, with func_8004A4E0's $s0-$s7/$fp still live from its own prologue (asm/funcs/func_8004A4E0.s:2-12). This is architecturally impossible to emit from C under this compiler for ANY source shape — stronger than the prior st…

## [s9] The jtbl/jal-reached helper func_8004A940 saves AND restores $ra through the scratchpad ($s1 = 0x1F800000: `sw $ra,4($s1)` … `lw $ra,4($s1); jr $ra`), a $ra spill base no GCC 2.7.2 MIPS lowering emits
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4632
- result: compiled C 0/1164 store $ra off-$sp; ASM 12/273 do: InitGeom, _ExitCard, __SN_ENTRY_POINT, _patch_gte, _patch_pad, _remove_ChgclrPAD, _send_pad, setjmp (all known Sony/SN hand-asm) + func_8004A940, func_8004BB68, func_8004BCC0, func_8004C1F4. func_8004A4E0's dispatch table and 5 jal sites (asm/funcs/func_8004A4E0.s:93,114,138,155,171) enter this helper; together they form a hand-written bytecode…

## [s9] func_8004A940 (reached from func_8004A4E0's jump table and 5 jal sites) stores $ra to 4($s1) and reloads it from there before jr $ra; GCC 2.7.2 MIPS addresses the $ra save slot only off $sp (or a temp derived from $sp), and has no RETURN_ADDR_RTX to name $ra from C
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4632
- result: compiled C 0/1164 store $ra off-$sp; asm 12/273 do (setjmp, _patch_gte, __SN_ENTRY_POINT, InitGeom, pad/card stubs, func_8004A940, func_8004BB68, func_8004BCC0, func_8004C1F4) - only hand-written code

## [s13] structural-modality levers (block-local var splits, declaration order, type narrowing, statement re-association) can make progress on func_8004A4E0 despite the s8 class kill.
- verdict: KILLED
- kill_scope: class
- predicate_cite: `docs/audits/func_8004A4E0-canonical-2026-09-29.md:31` (same predicate as H1 -- cross-function jtbl/ABI handoffs have no C form), applied here specifically to…
- result: `no_c_body: true` (this session's sandbox run above) means there is no C source text for a structural lever to act on in the first place -- the gating failure is upstream of codegen entirely (s8: 8/16 of `jtbl_8001541C`'s entries are jlabels inside `func_8004A940`'s own body, a different function). Structural levers only ever reshape how an EXISTING single-function C body lowers; they cannot be t…

## [s13] Structural-modality levers (block-local var splits, declaration order, type narrowing, statement re-association) can make progress on func_8004A4E0 despite the standing s8 class kill.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: No C source text exists for a structural lever to act on (no_c_body:true) — the gate is upstream of codegen entirely. s8/s12 independently established 8/16 of jtbl_8001541C's entries are jlabels living inside func_8004A940's own body (a different function); no structural rewrite of a hypothetical C switch can place a case arm inside another function's prologue-protected interior. Structural modal…

## [s14] Structural-modality levers (block-local var splits, declaration order, type narrowing, statement re-association) have an applicable surface on func_8004A4E0 this session.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: sandbox returned {"score":163,"no_c_body":true,"build_insns":0} and printed "no diff: function is still whole-body INCLUDE_ASM — there is no C to compare"; canonical re-confirmed verdict ASM-WHOLE (asm_insns=7, total=163, reason: whole-function canonical-asm grant in inline_asm_canonical.txt). Identical score fields to s2 and s13.

## [s15] The SYSTEMATIC SPELLING SWEEP procedure (mark a differing block ENUM-BEGIN/END in a candidate C file, run tools/spelling_enum.py + tools/sweep_variants.py over it) has an applicable surface on func_8004A4E0 this session.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: sandbox returned {"score":163,"no_c_body":true,"build_insns":0} and printed 'no diff: function is still whole-body INCLUDE_ASM — there is no C to compare'; canonical re-confirmed verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt'. Identical to s2/s13/s14's measured output char-for-char. There is no block of differing instructions to mark ENUM-BEGIN/END arou…

## [s16] The SYSTEMATIC SPELLING SWEEP procedure has an applicable surface on func_8004A4E0 this session (enumerate modality, second re-run).
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: sandbox returned {"score":163,"no_c_body":true,"build_insns":0} and printed 'no diff: function is still whole-body INCLUDE_ASM — there is no C to compare'. Identical to s2/s13/s14/s15's measured output char-for-char. Zero C statements exist in src/main/3AB48.c to mark ENUM-BEGIN/END around; sweep precondition unmet for the third consecutive enumerate-axis measurement.

## [s16] The SYSTEMATIC SPELLING SWEEP procedure (mark a differing block ENUM-BEGIN/END in a candidate C file, run tools/spelling_enum.py + tools/sweep_variants.py over it) has an applicable surface on func_8004A4E0 this session.
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: sandbox returned {"score":163,"no_c_body":true,"build_insns":0} and printed 'no diff: function is still whole-body INCLUDE_ASM — there is no C to compare'. Byte-for-byte identical to s2/s13/s14/s15's measured output. Zero C statements exist in src/main/3AB48.c to mark ENUM-BEGIN/END around; sweep precondition remains unmet for the third consecutive enumerate-axis measurement.

## [s17] The SYSTEMATIC SPELLING SWEEP procedure has an applicable surface on func_8004A4E0 this session (enumerate modality, third consecutive re-run).
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: canonical returned verdict ASM-WHOLE, reason "whole-function canonical-asm grant in inline_asm_canonical.txt", identical to s1-s16. sandbox returned {"score":163,"scorable":true,"target_insns":163,"build_insns":0,"no_c_body":true}, byte-for-byte identical fields to s2/s13/s14/s15/s16. src/main/3AB48.c is still `INCLUDE_ASM("asm/funcs", func_8004A4E0);` with zero C statements; no region exists to…

## [s17] The SYSTEMATIC SPELLING SWEEP procedure has an applicable surface on func_8004A4E0 this session (enumerate modality, third consecutive re-run).
- verdict: KILLED
- kill_scope: class
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31
- result: canonical returned verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt', identical to s1-s16. sandbox returned {"score":163,"scorable":true,"target_insns":163,"build_insns":0,"no_c_body":true}, byte-for-byte identical fields to s2/s13/s14/s15/s16. src/main/3AB48.c is still INCLUDE_ASM with zero C statements; no region exists to mark ENUM-BEGIN/END around. Che…

## Confirmed levers

## [s5] KILL RE-AUDIT: the s3 instance kill (permuter has no chassis) still holds on the current chassis with every FAKE construct ablated.
- verdict: CONFIRMED
- result: sandbox 163 / no_c_body true / build_insns 0 (s5/sandbox.txt); fake_ablate "No FAKE-annotated constructs found ... nothing to ablate" (s5/fake_ablate.txt). Chassis unchanged, no carrier — kill conditions intact.

## [s5] SYNTHESIS: the function's layout carries a GCC-2.7.2-absent signal (code after the single epilogue) that the scanner does not test, so the 2026-10-08 re-audit gate (scan_hand_coded STRONG + Judge) is blind on this function for the same reason S7 is (caller-side, not callee-side, evidence).
- verdict: CONFIRMED
- result: compiled-C side 0 / 1225 functions have code after a single `jr $ra`; asm side 2 / 160 — func_8004A4E0 (69 insns: all five case bodies) and _patch_gte (PsyQ hand-written). Plus re-verified $s4 (callee-input only, never read locally) and $s5 (`3-$s5`, written once to 0) in the asm itself. Combined with the 2026-09-29 audit's cross-call ABI table, the hand-written verdict is supported by a second,…

## [s5] KILL RE-AUDIT: the s3 instance kill (permuter has no chassis to mutate) still holds on the current chassis with every FAKE construct ablated.
- verdict: CONFIRMED
- result: sandbox score 163, no_c_body true, build_insns 0; fake_ablate: no FAKE-annotated constructs, nothing to ablate. Chassis unchanged, kill conditions intact.

## [s5] The function's layout carries a signal absent from GCC 2.7.2 output (case-block code after the single epilogue) that scan_hand_coded.py does not test, so the owner 2026-10-08 re-audit gate (STRONG tier + Judge) is blind here for the same caller-side reason S7 is.
- verdict: CONFIRMED
- result: Compiled-C side: 0/1225 functions have code after a single jr $ra. Asm side: 2/160 — func_8004A4E0 (69 insns) and _patch_gte (PsyQ hand-written, 9). $s4 is never read locally except its own addi (pure callee input before each jal func_8004A940); $s5 written once to 0 then read 5x as 3-$s5. A bare hazard-free nop sits at 0x8004A718. Independent corroboration of the 2026-09-29 audit's hand-written…

## [s6] DUMP-LEVEL PREDICATE for the s5 layout signal: GCC 2.7.2 (our cc1, project CC_FLAGS) emits the epilogue exactly once, as the last code of the function, after every switch case block — so case bodies after `jr $ra` are not compiler layout.
- verdict: CONFIRMED
- result: Both functions: jump-table dispatch (`j $2`), all case bodies jump back (`j .L14`/`j .L16`), and the single `j $31` is the final instruction before `.end` (sketch.s lines 153/251), restores immediately preceding it. Even the `default: return` arm in variant 2 is routed to the trailing epilogue rather than an in-body return. Compiler layout matches the 1225/1225 compiled-C corpus finding of s5 and…

## [s6] Our GCC 2.7.2 cc1 at project CC_FLAGS emits the epilogue once as the final code of a switch-in-loop function, after every case block, so func_8004A4E0's 69 case-body insns after jr $ra are not compiler layout
- verdict: CONFIRMED
- result: Both: j $reg dispatch, every case jumps back to the loop head, single j $31 is the last insn before .end (sketch.s:153, :251); the default: return arm is routed to the trailing epilogue. Matches s5's 0/1225 corpus count; contradicts the target layout.

## [s7] FORENSICS: re-audit of the standing instance kills on the current chassis.
- verdict: CONFIRMED
- result: chassis unchanged — score 163, no_c_body true, ASM-WHOLE grant, zero FAKE constructs, dump.ps1 has no C to dump (s7/sandbox_diff.txt, canonical.txt, fake_ablate.txt, dump_ps1.txt). Kills stand as recorded.

## [s7] FORENSICS: $s6 is a callee-saved register written but never read locally — a shape absent from compiled C.
- verdict: CONFIRMED
- result: compiled-C 0/506; asm 1/91 (func_8004A4E0 $s6 only). Third independent corroboration (after the 2026-09-29 custom-ABI audit and the s5/s6 code-after-epilogue signal) that the body is hand-written.

## [s7] Re-audit: the s3/s6 instance kills (no C body on the INCLUDE_ASM chassis) still hold on the current chassis with FAKE constructs ablated
- verdict: CONFIRMED
- result: score 163, no_c_body true; canonical ASM-WHOLE (whole-function grant in inline_asm_canonical.txt); fake_ablate finds no FAKE constructs; dump.ps1 has no C to dump. Kills stand.

## [s7] func_8004A4E0 writes callee-saved $s6 (addu $s6,$fp,$zero at 0x8004A538) and never reads it locally, a shape the compiled-C corpus never shows
- verdict: CONFIRMED
- result: compiled-C 0/506; asm side 1/91, func_8004A4E0 $s6 only. Third independent signal for the hand-written verdict, after the 2026-09-29 custom-ABI audit and the s5/s6 code-after-epilogue signal

## [s8] REDERIVE: m2c fresh decompile attempt on asm/funcs/func_8004A4E0.s
- verdict: CONFIRMED
- result: m2c refuses outright — "Found jr instruction at func_8004A4E0.s line 33, but the corresponding jump table is not provided" (the jtbl_8001541C/jtbl_8001545C tables live in src/main/d_5BF0.c as C arrays, not as .s data m2c can ingest standalone). Investigating WHY the table can't simply be appended led directly to the s8 finding below — m2c's refusal was itself the correct signal, not a tooling gap…

## [s9] KILL RE-AUDIT: the INCLUDE_ASM chassis all prior instance kills (s3/s6/s7/s8) were measured on is unchanged and carries zero FAKE constructs
- verdict: CONFIRMED
- result: canonical = ASM-WHOLE ("whole-function canonical-asm grant in inline_asm_canonical.txt"); sandbox scores the INCLUDE_ASM stub, "no diff: function is still whole-body INCLUDE_ASM". No chassis drift, no FAKE carrier occupying any pseudo → every prior instance kill stands as measured.

## [s9] Callee-saved registers live-in at entry (read before any write) in func_8004A4E0's helpers is a hand-written-ABI signal
- verdict: CONFIRMED
- result: compiled C 0/1164; ASM 6/273: setjmp, func_8004BCC0 (all 9 callee-saved), func_800790A4 ($s5), and the three func_8004A4E0 helpers func_8004A76C ($s0), func_8004A808 ($s0), func_8004A940 ($s0,$s1) — i.e. the custom callee-saved register ABI cited in the 2026-09-29 canonical audit is independently visible in a mechanical census.

## [s9] KILL RE-AUDIT: the INCLUDE_ASM chassis the s3/s6/s7/s8 instance kills were measured on is unchanged and carries zero FAKE constructs
- verdict: CONFIRMED
- result: canonical ASM-WHOLE (whole-function grant in inline_asm_canonical.txt); sandbox scores the INCLUDE_ASM stub, no diff; no chassis drift, no FAKE carrier, so prior instance kills stand

## [s9] Callee-saved registers read before any write at entry (live-in) in func_8004A4E0's helpers mark the custom register ABI cited by the canonical audit
- verdict: CONFIRMED
- result: compiled C 0/1164; asm 6/273: setjmp, func_8004BCC0, func_800790A4 and all three func_8004A4E0 helpers func_8004A76C ($s0), func_8004A808 ($s0), func_8004A940 ($s0,$s1)

## [s10] KILL RE-AUDIT: INCLUDE_ASM chassis unchanged, zero FAKE constructs
- verdict: CONFIRMED
- result: ASM-WHOLE grant; sandbox no diff on stub; s3/s6/s7/s8 instance kills stand as measured

## [s10] KILL RE-AUDIT: the INCLUDE_ASM chassis the s3/s6/s7/s8 instance kills were measured on is unchanged and carries zero FAKE constructs
- verdict: CONFIRMED
- result: canonical ASM-WHOLE (whole-function grant inline_asm_canonical.txt:199); sandbox scores the INCLUDE_ASM stub with no diff; prior instance kills stand

## [s11] KILL RE-AUDIT: chassis + canonical verdict unchanged since s10
- verdict: CONFIRMED
- result: canonical ASM-WHOLE (inline_asm_canonical.txt whole-function grant); sandbox score 163, no_c_body true, build_insns 0 — identical to s10. Prior instance/class kills stand.

## [s11] The ASM-WHOLE canonical-grant verdict and INCLUDE_ASM chassis for func_8004A4E0 are unchanged since s10, with zero FAKE constructs present.
- verdict: CONFIRMED
- result: canonical verdict ASM-WHOLE (inline_asm_canonical.txt whole-function grant), sandbox score 163, no_c_body true, build_insns 0 -- identical to s10. All prior instance and class kills (s3, s6, s7, s8, s9, s10) stand unweakened.

## Instance kills and other entries (index; chassis-relative, re-testable)

## [s3] The permuter can mutate a pure-C chassis for func_8004A4E0 on this body (comments/whitespace aside) to search for a lower-distance form.
- verdict: KILLED (instance) — canonical() returned verdict ASM-WHOLE again (reason: whole-function canonical-asm grant in inline_asm_canonical.txt), identical to s1/s2; sandbox --disable al…

## [s6] SOLVER: the ra_solver/sched_solver triage has a residual to classify for func_8004A4E0.
- verdict: KILLED (instance) — No solver axis exists on this chassis. Same no-C-body fact as s1-s5 (sandbox 163, no_c_body true).

## [s6] inverse_compose.py classify can type a PRE-RA/RA/SCHED residual for func_8004A4E0 on the current INCLUDE_ASM chassis
- verdict: KILLED (instance) — classify refused with its INCLUDE_ASM guard (s6/classify.txt); goal_from_tgt fallback failed to spawn the toolchain from Windows Python (s6/goal_classify.txt)…

## [s7] FORENSICS: func_8004A4E0's saved-but-unreferenced $s3 is a hand-written discriminator.
- verdict: KILLED (instance) — compiled-C 5/506 do save a never-referenced callee-saved reg (func_8002E6B0, func_8005763C, math_RotMatrixXYZ/YXZ/ZXY) — regs_ever_live can be set by allocatio…

## [s7] The saved-but-never-referenced $s3 in func_8004A4E0 discriminates hand-written from compiled code in this corpus
- verdict: KILLED (instance) — compiled-C 5/506 also save a never-referenced callee-saved reg (func_8002E6B0 $s4, func_8005763C $s0, math_RotMatrixXYZ $s1, math_RotMatrixYXZ $s2, math_RotMat…

## [s8] m2c can produce a fresh pure-C decompile of func_8004A4E0's asm that the prior 7 sessions' manual analysis missed
- verdict: KILLED (instance) — m2c refuses outright: 'Found jr instruction at func_8004A4E0.s line 33, but the corresponding jump table is not provided' (jtbl_8001541C/jtbl_8001545C are C ar…

## [s10] scan_hand_coded.py's own tier for func_8004A4E0 is STRONG (so the grant has direct scanner backing)
- verdict: KILLED (instance) — func_8004A4E0 tier=LOW 0/8; helpers A940/A76C/A808 STRONG via S7. The scanner has no signal for code-after-epilogue, written-never-read $sN, trapping add/addi,…

## [s10] Jump-table targets outside the referencing function occur in compiled C in this corpus
- verdict: KILLED (instance) — compiled C 0/58 pairs cross a function boundary; asm 1/8 — only func_8004A4E0/jtbl_8001541C (8/16 targets in func_8004A940)

## [s10] scan_hand_coded.py's own tier for func_8004A4E0 is STRONG, giving the ASM-WHOLE grant direct scanner backing
- verdict: KILLED (instance) — func_8004A4E0 tier=LOW 0/8. Helpers func_8004A940 (S3+S4+S7, $s0..$s7 unsaved), func_8004A76C (S7 $s0), func_8004A808 (S3+S4+S7 $s0) are STRONG. The scanner ha…

## [s10] Jump-table entries targeting code outside the referencing function occur among this corpus's compiled-C switch tables
- verdict: KILLED (instance) — compiled C 0/58 pairs cross a function boundary; asm 1/8 crosses, only func_8004A4E0/jtbl_8001541C (8/16 targets in func_8004A940). This is the statistical com…

## [s11] REDERIVE: sibling-ledger transplant check (math_RotMatrixXYZ/YXZ/ZXY, puts, InitGeom, _patch_gte, _remove_ChgclrPAD, func_800790A4)
- verdict: KILLED (instance) — all 7 siblings are closed/COMPLETED (0 grind sessions each), and the only overlap with func_8004A4E0 is shared GLOBAL DATA (g_draw_queue, D_80103608), not shar…

## [s11] REDERIVE: decomp.me corpus mining (tools/decomp_me_scrape.py) as a new rederive source
- verdict: KILLED (instance) — ModuleNotFoundError: curl_cffi not installed in .venv; no cached corpus exists locally. Tool is infeasible in this sandbox session without a package install. S…

## [s11] A sibling ledger (math_RotMatrixXYZ/YXZ/ZXY, puts, InitGeom, _patch_gte, _remove_ChgclrPAD, func_800790A4) carries a transplantable C form or block for func_8004A4E0's chassis.
- verdict: KILLED (instance) — All 7 siblings are unrelated completed utility/library functions; the only overlap with func_8004A4E0 is shared GLOBAL DATA references, not shared code shape.…

## [s11] tools/decomp_me_scrape.py can mine the decomp.me GCC-2.7.2/PsyQ corpus this session for a rederive candidate for func_8004A4E0.
- verdict: KILLED (instance) — curl_cffi dependency is not installed in .venv and no local corpus cache exists under tmp/decomp_me_corpus/; the probe is infeasible in this sandbox session. S…

## [s12] decomp.me corpus search (gcc2.7.2-psx) finds a transplantable close-match body for func_8004A4E0's whole-body asm, now that curl_cffi is installed.
- verdict: KILLED (instance) — best similarity 0.057 (func_80044240, unrelated function, score 0/8900 on ITS OWN target — not a func_8004A4E0 candidate at all), rest of top-5 at 0.044-0.051.…

## [s12] A decomp.me corpus search (gcc2.7.2-psx compiler tag) finds a transplantable close-match body for func_8004A4E0's whole-body asm, now that curl_cffi is installed in .venv.
- verdict: KILLED (instance) — Best similarity 0.057, against an unrelated function (func_80044240, not a func_8004A4E0 candidate); rest of top-5 at 0.044-0.051. No transplant-viable match a…

## [s13] Mandated kill re-audit (floor flat since s5): re-measure the closest banked instance kill on the current chassis with every FAKE construct ablated.
- verdict: KILLED (instance) — tool reports "no FAKE-annotated constructs found in memory/grind/func_8004A4E0/candidate.c; nothing to ablate" -- confirms there is no FAKE carrier anywhere in…

## [s13] The mandated kill re-audit (floor flat since s5) finds a FAKE construct propping up a prior instance kill, invalidating it.
- verdict: KILLED (instance) — Tool reports zero FAKE-annotated constructs in candidate.c (it is comments-only over the unmodified INCLUDE_ASM stub) — there was never a FAKE carrier to begin…

## Latest session s18 (verbatim)

## [s18] Kill re-audit: some instance kill in state.json was measured on a different chassis or under a FAKE construct no longer present, so re-measuring it could move the floor.
- mechanism: n/a — re-audit of measurement conditions.
- probe: compared measured_on of all 10 instance kills against the current chassis; fresh sandbox --disable all --diff, canonical, and tools/fake_ablate.py on candidate.c at HEAD 7bdc5a300 (tmp/grind/func_8004A4E0/s18/{sandbox,canonical,fake_ablate}.txt).
- result: every instance kill was measured on the whole-body INCLUDE_ASM stub with zero FAKE constructs, the same as now; sandbox 163 no_c_body:true, canonical ASM-WHOLE, fake_ablate "nothing to ablate". No kill is voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD 7bdc5a300, zero FAKE constructs

## [s18] scan_hand_coded.py backs the canonical grant at STRONG tier, which would satisfy the owner's 2026-10-08 reopen criterion mechanically.
- mechanism: n/a — scanner tier check.
- probe: python3 tools/scan_hand_coded.py --single func_8004A4E0 (tmp/grind/func_8004A4E0/s18/scan.txt).
- result: tier LOW, score 0/8, all of S1-S8 unchecked; matches s1/s10. The decisive signals (unbounded jtbl index, cross-function jtbl targets, code after jr $ra, $ra saved off-$sp in the helper, trapping addi, filler nop) are not in the scanner's catalog.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_8004A4E0.s with tools/scan_hand_coded.py at HEAD 7bdc5a300; INCLUDE_ASM stub chassis, zero FAKE constructs

## [s18] FRONTIER RESET (synthesis)
1. Judge ruling: does the non-scanner evidence chain (evidence.md s18 items a-i, each with a corpus census) satisfy the 2026-10-08 reopen's "hand-written" bar, given scanner tier LOW? (filed as the s18 ruling-request)
2. If the ruling says no: escalation-modality proof-of-foreclosure record in docs/grind/decisions.md (floor 163 > 5 → LADDER EXHAUSTED title), rotation.
3. Only if a ruling rejects the hand-written verdict: start a pure-C chassis attempt starting from the jtbl_8001545C arm bodies (5 short call arms) — none exists today.

## [s18b] A C body compiled by the build cc1 can emit func_8004A4E0's two trapping `addi` instructions (0x8004A5A0, 0x8004A67C).
- mechanism: GCC 2.7.2 MIPS add emission — addsi3_internal template (mips.md:381) is `addu`; the backend's only `add` templates (mips.c:1140-1145) are HALF_PIC-gated (mips.c:1111).
- probe: read mips.md/mips.c templates; compiled 4 add spellings with tools/gcc-2.7.2/build/cc1 + Makefile CC_FLAGS (s18b/addi_probe.*); corpus census of trapping addi across 1300 non-INCLUDE_ASM vs 137 INCLUDE_ASM functions (s18b/addi_census.*).
- result: every probe emits `addu`; compiled C 0/1299 (only hit _patch_gte, inline-asm-canonical); func_8004A4E0 has 2. Any pure-C chassis for this function leaves at least these 2 words different.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.md:381
- measured_on: whole-body INCLUDE_ASM chassis at HEAD 9d185896b, zero FAKE constructs; probe file tmp/grind/func_8004A4E0/s18b/addi_probe.c

## [s18b] Kill re-audit: an instance kill was measured on a different chassis or under a FAKE carrier no longer present.
- probe: fresh sandbox --disable all --diff + fake_ablate at HEAD 9d185896b (s18b/sandbox.txt, s18b/fake_ablate.txt).
- result: 163 / no_c_body; nothing to ablate. Same chassis and zero-FAKE state as every banked instance kill; none voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD 9d185896b, zero FAKE constructs

## [s18b] A trapping-add signal absent from scan_hand_coded's S1-S8 catalog is corpus-clean and would rate func_8004A4E0 STRONG (Judge ruling door 1).
- probe: scanner header S1-S8 review; addi census (s18b/addi_census.txt).
- result: no S-signal covers trapping add; compiled C 0/1299; func_8004A4E0 2 hits, helper func_8004A940 28.
- verdict: CONFIRMED

## [s18b] FRONTIER RESET (synthesis)
1. Operator/engine lever (outside grind surface): add a trapping-add (maspsx "handwritten instruction") signal to tools/scan_hand_coded.py — predicate mips.md:381, census s18b/addi_census.txt — then re-run the 2026-10-08 hand-written re-audit; func_8004A4E0 would score STRONG.
2. Absent that: escalation-modality LADDER EXHAUSTED record (floor 163 > 5) and rotation, per the Judge constraint.
3. A pure-C chassis is foreclosed by the s18b addi class kill plus s8 (cross-function jtbl, toplev.c:2647) and s9 ($ra via scratchpad, mips.c:4632); do not spend a session on one.

## [s18] A C body compiled by the build cc1 can emit func_8004A4E0's two trapping addi instructions (0x8004A5A0, 0x8004A67C).
- mechanism: GCC 2.7.2 MIPS add emission: addsi3_internal template is addu (mips.md:381); the backend's only add templates (mips.c:1140-1145) are HALF_PIC-gated (mips.c:1111), never enabled under -mno-abicalls -G0.
- probe: Read mips.md/mips.c templates; compiled p+1, a+0xB0, a+b and a char*-cast global bump with tools/gcc-2.7.2/build/cc1 and Makefile CC_FLAGS; corpus census of trapping addi over 1300 non-INCLUDE_ASM vs 137 INCLUDE_ASM functions.
- result: Every probe emits addu. Compiled C carries trapping addi 0/1299 (only hit _patch_gte, which is inline-asm-canonical); func_8004A4E0 has 2, helper func_8004A940 28. Any pure-C chassis leaves these words different.
- verdict: KILLED
- kill_scope: class
- measured_on: whole-body INCLUDE_ASM chassis at HEAD 9d185896b, zero FAKE constructs; probe tmp/grind/func_8004A4E0/s18b/addi_probe.c
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.md:381

## [s18] Kill re-audit: an instance kill in state.json was measured on a different chassis or under a FAKE carrier that is no longer present.
- mechanism: n/a - re-audit of measurement conditions
- probe: Fresh sandbox --disable all --diff and tools/fake_ablate.py on candidate.c at HEAD 9d185896b.
- result: score 163, no_c_body true, no diff (whole-body INCLUDE_ASM); fake_ablate: nothing to ablate. Every instance kill shares this chassis and zero-FAKE state; none voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD 9d185896b, zero FAKE constructs

## [s18] A trapping-add signal absent from scan_hand_coded's S1-S8 catalog is corpus-clean in compiled C and would rate func_8004A4E0 STRONG, matching door 1 of the 2026-10-15 16:50 Judge ruling.
- mechanism: maspsx tags trapping add/addi as 'handwritten instruction'; GCC never emits them (mips.md:381).
- probe: Reviewed scan_hand_coded.py header S1-S8 (lines 7-53); census tmp/grind/func_8004A4E0/s18b/addi_census.txt.
- result: No S-signal covers trapping add. Compiled C 0/1299, INCLUDE_ASM 58/137, func_8004A4E0 2 hits.
- verdict: CONFIRMED

## [s19] (solver) inverse_compose.py classify can triage a PRE-RA / RA / SCHED residual for func_8004A4E0 on the current chassis.
- mechanism: the solver models compare a compiled C body's RTL/asm against the target; they need a function symbol in the sandbox object.
- probe: text classify `inverse_compose.py classify main/3AB48 func_8004A4E0` (guard refuses: INCLUDE_ASM target, would report a fictitious PRE-RA verdict); object classify with `--target-object build/src/main/3AB48.o --ours-object tmp/sandbox/func_8004A4E0/main/3AB48.o` -> "func_8004A4E0 not found in tmp/sandbox/.../3AB48.o". Artifacts tmp/grind/func_8004A4E0/s19/{classify.txt,classify_obj.txt}.
- result: no residual to classify. The sandbox --disable all object has no func_8004A4E0 symbol (score 163, build_insns 0, no_c_body true; tmp/grind/func_8004A4E0/s19/sandbox.txt). This is the same as s6. The RA and scheduler solvers have nothing to model until a C chassis exists, and s8 (toplev.c:2647), s9 and s18b (mips.md:381) class kills foreclose building one.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c:29 unchanged) at HEAD d0cc26461, zero FAKE constructs

## [s19] Kill re-audit (mandated, floor flat): an instance kill was measured on a different chassis or under a FAKE carrier no longer present.
- probe: fresh sandbox --disable all --diff + tools/fake_ablate.py on candidate.c at HEAD d0cc26461 (s19/sandbox.txt, s19/fake_ablate.txt).
- result: 163 / no_c_body / "no diff: whole-body INCLUDE_ASM"; fake_ablate "no FAKE-annotated constructs ... nothing to ablate". Every banked instance kill shares this chassis and zero-FAKE state; none voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis at HEAD d0cc26461, zero FAKE constructs

## [s19] FRONTIER (unchanged from s18b)
1. Operator/engine: trapping-add signal in tools/scan_hand_coded.py (mips.md:381, s18b/addi_census.txt), then the hand-written re-audit.
2. Otherwise: escalation-modality LADDER EXHAUSTED (floor 163) record + rotation, per the Judge constraint.

## [s19] inverse_compose.py classify can triage a PRE-RA/RA/SCHED residual for func_8004A4E0 on the current whole-body INCLUDE_ASM chassis
- mechanism: the solver models compare a compiled C body against the target and need a function symbol in the sandbox object
- probe: text classify (guard refuses on INCLUDE_ASM target); object classify --target-object build/src/main/3AB48.o --ours-object tmp/sandbox/func_8004A4E0/main/3AB48.o
- result: 'func_8004A4E0 not found' in the sandbox object; sandbox score 163, build_insns 0, no_c_body true. Nothing to model, same as s6. Building a chassis is foreclosed by the s8/s9/s18b class kills.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c:29 unchanged) at HEAD d0cc26461, zero FAKE constructs

## [s19] Kill re-audit: a banked instance kill was measured on a different chassis or under a FAKE carrier no longer present
- mechanism: n/a - re-audit of measurement conditions
- probe: fresh sandbox --disable all --diff and tools/fake_ablate.py on candidate.c
- result: 163 / no_c_body / no diff; fake_ablate found nothing to ablate. None of the banked kills is voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis at HEAD d0cc26461, zero FAKE constructs

## [s20] (forensics) Kill re-audit (mandated, floor flat): a banked instance kill was measured on a different chassis or under a FAKE carrier no longer present.
- probe: fresh `sandbox --disable all --diff` + tools/fake_ablate.py on candidate.c at HEAD 7aaefe365 (tmp/grind/func_8004A4E0/s20/sandbox.txt, s20/fake_ablate.txt).
- result: score 163, no_c_body true, "no diff: whole-body INCLUDE_ASM"; fake_ablate: no FAKE constructs, nothing to ablate. No banked kill is voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD 7aaefe365, zero FAKE constructs

## [s20] (forensics) An unbounded jtbl dispatch (lbu index -> sll -> lw table -> jr, no sltu bound check) is evidence that the code is not compiled C. This is the s18 audit signal (a), "58/58 compiled-C jtbls carry a range check".
- mechanism: switch lowering always emits the bound check. stmt.c:4991 expand_end_case calls do_tablejump, and expr.c:9963 always emits `emit_cmp_insn(index, range, GTU)` plus bgtu to default. The open question was whether any source shape avoids it.
- probe: instrumented cc1 (-da) on tmp/grind/func_8004A4E0/s20/probe.c (run.sh), five input shapes: A switch(u8) 16 cases; B switch(u8 & 15) 16 cases; C switch(u8) 256 cases, covering the whole u8 range; D switch(u32 >> 16) 16 cases; E GNU computed goto `static void *const tbl[16] = {&&l0..}; goto *tbl[*p];`.
- result: A-D all keep the bound check through final (`sltu $2,$3,16|256; beq $2,$0,default`). Even C keeps it: the sltu_si `(ltu:SI (reg 74) (const_int 256))` on a zero_extend:QI index survives .combine, .jump2 and .sched2 unfolded (probe.i.combine/jump2/sched2), so nonzero_bits does not elide it in 2.7.2. E (computed goto) emits exactly the target's dispatch shape, `lbu; sll $2,$2,2; lw $2,tbl($2); j $2`, with no bound check, and the table goes in .rodata (probe.s:648). So signal (a) is reachable from GNU C. It shows only that the original was not a `switch`, NOT that it was hand-written. Downgrade (a) from a not-compiled-C signal to a not-a-switch signal. The other signals are unaffected, in particular s8: computed-goto labels are function-local (expand_computed_goto, stmt.c:593, operates per function), so they cannot address func_8004A940's body, and the s18b trapping-addi kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone probe TU s20/probe.c compiled by tools/gcc-2.7.2/build/cc1 with the project CC flags; function chassis unchanged (whole-body INCLUDE_ASM, zero FAKE constructs)

## [s20] (forensics) The switch-lowered bound check is unconditional for every `switch` statement under GCC 2.7.2 tablejump lowering (no casesi on MIPS)
- mechanism: expr.c:9963 do_tablejump emits the GTU compare unconditionally; measured across 4 switch shapes (A-D) including full-u8-range coverage, none folded by cse/combine/jump.
- probe: s20/probe.c A-D, dumps probe.i.{rtl,cse,combine,jump2,sched2}
- result: the bound check is present in all 4 switch shapes at final. Any C reconstruction must dispatch via computed goto (or a fnptr table, which emits jalr, not jr), not via switch.
- verdict: KILLED
- kill_scope: class
- measured_on: s20/probe.c switch shapes A-D, project CC flags, instrumented cc1; no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/expr.c:9963

## [s20] FRONTIER (updated)
1. Operator/engine: trapping-add signal in scan_hand_coded (mips.md:381) and/or the s10 cross-function-jtbl signal. Do NOT propose "unbounded jtbl index" as a scanner signal: s20 shows computed goto produces it.
2. Otherwise: escalation-modality LADDER EXHAUSTED (floor 163) record + rotation, per the Judge constraint.

## [s20] Kill re-audit (mandated): a banked instance kill was measured on a different chassis or under a FAKE carrier no longer present
- mechanism: n/a - re-audit of measurement conditions
- probe: sandbox --disable all --diff + tools/fake_ablate.py on candidate.c at HEAD 7aaefe365
- result: score 163, no_c_body true, no diff; fake_ablate found nothing to ablate; no kill voided
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD 7aaefe365, zero FAKE constructs

## [s20] The unbounded jtbl dispatch (lbu index, sll, lw table, jr with no sltu bound check) in func_8004A4E0 is evidence that the code is not compiled C (s18 audit signal a)
- mechanism: stmt.c:4991 expand_end_case -> expr.c:9963 do_tablejump emits the GTU bound check for switch; question was whether another source shape avoids it
- probe: instrumented cc1 -da on s20/probe.c with 5 shapes: switch u8/16 cases, u8&15, u8/256 cases, u32>>16, GNU computed goto
- result: All 4 switch shapes keep sltu+beq default through .combine/.jump2/.sched2 (even the full-u8 256-case form; nonzero_bits does not fold it). Computed goto emits exactly lbu; sll; lw tbl($r); j $r with the table in .rodata (probe.s:648). So signal (a) shows only that the original was not a switch. Computed-goto labels are function-local (stmt.c:593), so the s8 cross-function-target kill and the s18b trapping-addi kill stand.
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone probe TU tmp/grind/func_8004A4E0/s20/probe.c under tools/gcc-2.7.2/build/cc1 with project CC flags; function chassis whole-body INCLUDE_ASM, zero FAKE constructs

## [s20] The bound check from GCC 2.7.2 switch lowering through do_tablejump is unconditional for every switch statement, and cse/combine/jump do not fold it
- mechanism: expr.c:9963 emit_cmp_insn(index, range, GTU) + bgtu default, unconditional; MIPS has no casesi
- probe: s20/probe.c switch shapes A-D, dumps probe.i.rtl/cse/combine/jump2/sched2
- result: sltu_si (ltu reg const) survives to final in all 4 shapes, including a zero_extend:QI index against 256. A C reconstruction would need computed goto for the dispatch, not switch.
- verdict: KILLED
- kill_scope: class
- measured_on: s20/probe.c switch shapes A-D, project CC flags, instrumented cc1, no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/expr.c:9963

## [s21] (forensics) Kill re-audit (mandated, floor flat): a banked instance kill was measured on a different chassis or under a FAKE carrier no longer present.
- mechanism: n/a - re-audit of measurement conditions
- probe: `sandbox func_8004A4E0 --disable all --diff` + tools/fake_ablate.py on candidate.c at HEAD baf1aac95 (tmp/grind/func_8004A4E0/s21/sandbox.txt, s21/fake_ablate.txt)
- result: score 163, no_c_body true, "no diff: function is still whole-body INCLUDE_ASM"; fake_ablate found no FAKE construct. No banked kill is voided.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD baf1aac95, zero FAKE constructs

## [s21] (forensics) The explicit nop at 0x8004A718 (`lui $s4; addiu $s4,%lo(D_80095328); nop; jal func_8004A940` in the .L8004A6F0 case) is producible by the project pipeline (cc1 | maspsx | as) from a C body
- mechanism: GCC 2.7.2 MIPS emits a nop only (1) in a branch/jump delay slot via the `%#` operand code (tools/gcc-2.7.2/config/mips/mips.c:3618) or (2) as the epilogue load-delay nop after `lw $ra` (mips.c:5200); maspsx adds nops only for load-delay hazards, HI/LO hazards before mult/div (tools/maspsx/maspsx/__init__.py:1070-1083) and inside div expansion. None of them puts a nop after an ALU `addiu` that is followed by `jal`.
- probe: (a) corpus census tmp/grind/func_8004A4E0/s21/nop_census.py: in every asm/funcs function, flag a `nop` whose predecessor is not a branch/jump, a load (lw/lh/lhu/lb/lbu/lwl/lwr/lwc2/mfc0/mfc2/cfc2), mfhi/mflo (within 2 insns), a cop0/cop2/mult/div op, or another nop, and is not the last instruction (alignment pad). (b) build-pipeline probe tmp/grind/func_8004A4E0/s21/probe.c (run.sh): the case block written as C three ways (pointer argument, global store, inside a loop with a second reload-and-call), through tools/gcc-2.7.2/build/cc1 (project flags) | maspsx --aspsx-version=2.34 | mipsel-linux-gnu-as, then objdump.
- result: (a) of 1300 functions that have a C representation, 3 hits, and all 3 are inline asm: DelDrv and _96_remove are BIOS trampolines with trailing pad nops (inline_asm_canonical.txt:129-130), and func_80067D14's double nop is the PsyQ gte_rtpt inline_c.h macro text (inline_asm_canonical.txt:403). GCC-emitted code: 0/1297. Of 137 INCLUDE_ASM functions, 39 hit, among them the known hand-written libgte bodies (RotMatrix/X/Y/Z/ZYX, ScaleMatrix(L), SquareRoot0/12, ApplyRotMatrixLV), func_8004A4E0 (1, at 0x8004A718), func_8004A940 (7), and 22 of the 0x8004C994-0x80051D08 polygon cluster. (b) every nop in the probe object is a jal or jr delay slot (s21/probe.objdump.txt, s21/probe_nops.txt). The address materialization is `lui a0; addiu a0,a0,%lo(D_80095328); jal` with no nop between them.
- verdict: KILLED
- kill_scope: class
- measured_on: whole corpus asm/funcs (1437 functions) + standalone probe s21/probe.c through the build cc1/maspsx/as with project flags; function chassis whole-body INCLUDE_ASM, zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:3618

## [s21] FRONTIER (updated)
1. Operator/engine (Judge door 1, docs/grind/decisions.md:1320): add one or more mechanized not-C signals to tools/scan_hand_coded.py. Corpus-clean candidates are the trapping add (s18b, 0/1299), the s10 cross-function jtbl target (0/58), and now the hazard-free nop (s21, 0/1297 GCC-emitted; it also fires on the known hand-written libgte bodies, so it discriminates in the right direction). Do NOT use "unbounded jtbl index" (s20: computed goto produces it).
2. Otherwise: escalation-modality LADDER EXHAUSTED (floor 163) record + rotation, per the Judge constraint.

## [s21] Kill re-audit (mandated, floor flat): a banked instance kill was measured on a different chassis or under a FAKE carrier no longer present
- mechanism: n/a - re-audit of measurement conditions
- probe: sandbox func_8004A4E0 --disable all --diff + tools/fake_ablate.py on candidate.c at HEAD baf1aac95
- result: score 163, no_c_body true, no diff (whole-body INCLUDE_ASM); fake_ablate found no FAKE construct; no kill voided
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD baf1aac95, zero FAKE constructs

## [s21] The explicit nop at 0x8004A718 (lui/addiu $s4 = &D_80095328; nop; jal func_8004A940) is producible by the project pipeline cc1 | maspsx | as from a C body
- mechanism: GCC 2.7.2 MIPS emits nop only via the %# delay-slot operand code (mips.c:3618) and the epilogue load nop (mips.c:5200); maspsx adds nops only for load and HI/LO hazards and in div expansion (maspsx/__init__.py:1070-1083). None of these follows an ALU addiu that precedes a jal.
- probe: corpus census s21/nop_census.py (a nop whose predecessor is not a branch, load, mfhi/mflo, cop or mult/div op and that is not end padding) over all 1437 asm/funcs; plus s21/probe.c, the case block as C in 3 variants, through the build cc1, maspsx and as, then objdump
- result: C-representation functions: 3 hits, all inline asm (DelDrv and _96_remove BIOS pad nops, the gte_rtpt macro in func_80067D14), so GCC-emitted code has 0/1297. INCLUDE_ASM: 39/137 hit, including func_8004A4E0 (1), func_8004A940 (7), the known hand-written libgte bodies and 22 polygon-cluster functions. In the probe object every nop is a jal/jr delay slot; the address materialization goes straight lui; addiu; jal.
- verdict: KILLED
- kill_scope: class
- measured_on: whole asm/funcs corpus + standalone probe tmp/grind/func_8004A4E0/s21/probe.c via build cc1/maspsx/as with project flags; function chassis whole-body INCLUDE_ASM, zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:3618

## [s22] (rederive) m2c cannot decompile func_8004A4E0 even with jtbl_8001541C supplied, because 8 of its 16 non-default entries resolve to labels interior to func_8004A940's body — a single C function can never emit a jump table whose targets are labels scoped to a different function
- mechanism: C label identifiers are scoped per-function in GCC 2.7.2's front end: push_label_level()/pop_label_level() (tools/gcc-2.7.2/c-decl.c:1182,1210) bracket each function's label namespace, so no `goto`/switch-lowered computed-jump inside one function body can ever resolve to a label belonging to a sibling function — the C language has no cross-function label reference. A jump table whose entries are such labels therefore cannot be the output of compiling any single C function under this (or any standard C) front end.
- probe: built tmp/grind/func_8004A4E0/s22/jtbl_data.s (a synthetic `.rodata` block spelling out jtbl_8001541C's 16 words as label references, using the actual values from src/main/d_5BF0.c: 0x8004A990, 0x8004ABD8, 0x8004A5D0, 0x8004AE0C x5, 0x8004AF84, 0x8004A5D0, 0x8004B0C8, 0x8004B0E4, 0x8004B4D8, 0x8004B27C), then ran `python3 tools/m2c/m2c.py asm/funcs/func_8004A4E0.s tmp/grind/func_8004A4E0/s22/jtbl_data.s -f func_8004A4E0` (prior s8/s12 runs never supplied the jump table at all, so m2c refused earlier at the `jr` instruction itself; this session supplies the real rodata to see what m2c does once that blocker is lifted).
- result: m2c now gets past the missing-jtbl refusal and fails on a different, more specific error: `Cannot find jtbl target .L8004A990` (tmp/grind/func_8004A4E0/s22/m2c_err2.txt). Grepping asm/funcs/*.s confirms `.L8004A990` and 7 of the other 8 non-default jtbl_8001541C targets (.L8004ABD8, .L8004AE0C, .L8004AF84, .L8004B0C8, .L8004B0E4, .L8004B27C, .L8004B4D8) are `jlabel` lines inside asm/funcs/func_8004A940.s (lines 24,171,313,408,490,498,601,753), not inside func_8004A4E0.s at all — confirming s10's cross-function-jtbl finding independently via m2c's own symbol resolver, and pinning down that it is specifically func_8004A4E0's own primary per-record-type dispatch table (indexed at .L8004A534 by `lbu $t0,0($s2); sll $t0,$t0,2`) that reaches into func_8004A940, not a side table.
- verdict: KILLED
- kill_scope: class
- measured_on: m2c (tools/m2c/m2c.py) run against asm/funcs/func_8004A4E0.s + a synthetic rodata file holding jtbl_8001541C's real values; cross-checked against asm/funcs/func_8004A940.s; whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged), zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/c-decl.c:1182

## [s22] FRONTIER (updated)
1. Operator/engine (Judge door 1, docs/grind/decisions.md:1320): teach tools/scan_hand_coded.py one of the three corpus-clean not-C signals already banked (trapping add s18b 0/1299; cross-function jtbl target s10/s22 0/58; hazard-free nop s21 0/1297) so func_8004A4E0 scores STRONG and the canonical-asm grant door opens.
2. Otherwise: the modality ladder is exhausted at floor 163 (structural/enumerate/synthesis/solver/forensics/rederive all re-confirm no_c_body / class kills; no pure-C axis remains). Escalation modality should file LADDER EXHAUSTED and rotate.

## [s22] m2c cannot decompile func_8004A4E0 even with jtbl_8001541C's real values supplied, because 8 of its 16 non-default entries resolve to labels interior to func_8004A940's body — a single C function can never emit a jump table whose targets are labels scoped to a different function.
- mechanism: C label identifiers are scoped per-function in GCC 2.7.2's front end: push_label_level()/pop_label_level() (tools/gcc-2.7.2/c-decl.c:1182,1210) bracket each function's label namespace, so no goto/switch-lowered computed-jump inside one function body can ever resolve to a label belonging to a sibling function.
- probe: Built tmp/grind/func_8004A4E0/s22/jtbl_data.s, a synthetic .rodata block spelling jtbl_8001541C's 16 words (from src/main/d_5BF0.c) as label references, then ran python3 tools/m2c/m2c.py asm/funcs/func_8004A4E0.s tmp/grind/func_8004A4E0/s22/jtbl_data.s -f func_8004A4E0 (prior s8/s12 m2c runs never supplied the jump table at all and refused earlier, at the jr instruction itself).
- result: m2c gets past the missing-jtbl refusal and fails with a more specific error: 'Cannot find jtbl target .L8004A990'. Grepping asm/funcs/*.s confirms .L8004A990 and 7 of the other 8 non-default jtbl_8001541C targets are jlabel lines inside asm/funcs/func_8004A940.s (lines 24,171,313,408,490,498,601,753), not func_8004A4E0.s — independently confirming s10's cross-function-jtbl finding via m2c's own symbol resolver, and pinning it to func_8004A4E0's own primary per-record-type dispatch table.
- verdict: KILLED
- kill_scope: class
- measured_on: m2c (tools/m2c/m2c.py) against asm/funcs/func_8004A4E0.s + synthetic rodata holding jtbl_8001541C's real values, cross-checked against asm/funcs/func_8004A940.s; whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged), zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/c-decl.c:1182

## [s22] Kill re-audit (mandated by floor-flat rule): re-measured this session's sandbox floor and no_c_body state on the current chassis before any new probe.
- mechanism: n/a - re-audit of measurement conditions
- probe: sandbox func_8004A4E0 --disable all at current HEAD
- result: score 163, no_c_body true, cheat_asm_stripped 71, no diff available (whole-body INCLUDE_ASM); no prior kill voided by this re-measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at current HEAD, zero FAKE constructs

## s23 [escalation] Kill re-audit + disposition
- statement: Re-measuring the banked candidate on the current chassis changes the floor.
- probe: sandbox --disable all (stub) and --candidate memory/grind/func_8004A4E0/candidate.c at HEAD c05458c8c.
- result: stub 163; candidate errors (func_8004A4E0 not in object — comments-only candidate). No FAKE constructs exist to ablate. Floor unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD c05458c8c, zero FAKE constructs
- disposition: both endgame gates FAIL (scan LOW 0/8; SOTN census 0) -> LADDER EXHAUSTED rotation entry in docs/grind/decisions.md.

## [s23] Re-measuring the banked candidate and the stub on the current chassis at HEAD c05458c8c lowers the floor below 163.
- mechanism: Mandated kill re-audit: instance kills are chassis-relative; a chassis or FAKE-state change could move the floor.
- probe: sandbox func_8004A4E0 --disable all (stub) and --candidate memory/grind/func_8004A4E0/candidate.c
- result: Stub scores 163; candidate measurement errors because func_8004A4E0 is absent from the object (candidate.c is comments-only). No FAKE constructs exist to ablate. Floor unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: whole-body INCLUDE_ASM chassis (src/main/3AB48.c unchanged) at HEAD c05458c8c, zero FAKE constructs

## [s23] scan_hand_coded reports func_8004A4E0 at STRONG tier, opening the canonical-asm grant path.
- mechanism: judge-sole-gate endgame AND-gate (a): canonical-asm needs STRONG scanner signals.
- probe: python3 tools/scan_hand_coded.py --single func_8004A4E0
- result: tier=LOW score 0/8; S1-S8 all unset.
- verdict: KILLED
- kill_scope: instance
- measured_on: current scanner at HEAD c05458c8c, whole-body INCLUDE_ASM chassis, zero FAKE constructs

## [s23] An in-hand SOTN-master precedent exists for the closing constructs (ctc2, trapping add, callee-saved-register handoff, computed goto).
- mechanism: judge-sole-gate endgame AND-gate (b): a family needs a citable SOTN precedent.
- probe: grep of docs/reference/sotn-construct-index.md for ctc2|trapping|callee-saved|computed goto|&&label
- result: 0 matches.
- verdict: KILLED
- kill_scope: instance
- measured_on: sotn-construct-index.md at HEAD c05458c8c; no C body, zero FAKE constructs
