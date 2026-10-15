# Hypothesis ledger — func_8004A4E0

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
**predicate_cite:** `docs/audits/func_8004A4E0-canonical-2026-09-29.md:31` (the "Why islands
can't work" paragraph, lines 31-34) — states the general predicate (every cross-call
register-ABI handoff site would need forbidden GPR asm to express in C), not just one
instance form.

## Frontier (for the next session)
1. **No live pure-C axis.** H1's class kill leaves nothing to grind on the C side absent new
   evidence (e.g. a toolchain finding that makes the $s4/$s5/$s0 handoffs expressible without
   GPR asm — none known). Mechanism: n/a. Next probe: n/a unless a new `canonical`/scanner
   signal appears on a toolchain update.
2. **Confirm landing path, not grind.** This function is a canonical-asm grant already
   validated by an owner ruling (Q40) post-dating `judge-sole-gate`'s policy — the open
   question is operator-side (does `queue done`/the authorize-bucket path still need to run
   for this function), not a grind hypothesis. Mechanism: n/a (no codegen lever). Next probe:
   none from the grind lane; this is `CLAUDE.md`'s "authorize bucket ... Judge-gated
   canonical-grant path" territory.
3. **Re-check scanner tier only if policy changes.** `scan_hand_coded.py --single` reads LOW
   (0/8) because its S7 heuristic checks callee-side unsaved callee-saves and this function is
   the caller; that gap is already noted in the audit and is not actionable by a grind agent.
   Mechanism: scanner heuristic coverage gap, not a GCC pass. Next probe: none — informational
   only, in case a future scanner revision adds a caller-side ABI heuristic.

## [s1] A pure-C (or partial C-island) form can replace the whole-body asm for func_8004A4E0.
- mechanism: Custom callee-saved-register ABI across jal with func_8004A940/func_8004A76C/func_8004A808 ($s4/$s5/$s6/$fp/$s0 cross-call handoffs, a ghost $s3 save/restore, a callee-folded 3-$s5) plus 2 non-overflow-safe trapping addi and 4 ctc2 with no C form; covering each handoff with a C island requires forbidden GPR-steering asm at every call site, collapsing the island boundary to the whole body.
- probe: canonical func_8004A4E0 (verdict ASM-WHOLE this session) + sandbox func_8004A4E0 --disable all (score 163, no_c_body: true, no --diff possible) + re-read of docs/audits/func_8004A4E0-canonical-2026-09-29.md (owner ruling Q40, 803d0fea1).
- result: No C body exists; the audit's cross-call ABI argument accounts for every operand this session re-confirmed in asm/funcs/func_8004A4E0.s (D_80095328, D_80095488, D_80095508, D_800FF610, g_draw_queue, D_80103608, jtbl_8001541C, jtbl_8001545C all present as real operands, none reachable from any C statement since the body is whole-body INCLUDE_ASM).
- verdict: KILLED
- kill_scope: class
- measured_on: whole-body INCLUDE_ASM chassis, zero FAKE constructs (no C body exists at all); inherited from the 2026-09-29 owner-ruled audit, re-confirmed this session by canonical + sandbox runs.
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31

## [s2] structural-modality dispatch — re-confirm, do not re-derive, the standing class kill.
- mechanism: n/a — no new hypothesis; structural levers (block-local var splits, decl order,
  type narrowing, statement re-association) all require an existing C body to apply to, and
  none exists on this function (whole-body INCLUDE_ASM, ASM-WHOLE canonical verdict).
- probe: `canonical func_8004A4E0` re-run this session — verdict ASM-WHOLE, same reason string
  ("whole-function canonical-asm grant in inline_asm_canonical.txt"), same 7 asm_insns/163
  total/regions as prior sessions. No `sandbox --diff` possible (no_c_body).
- result: standing H1 class kill holds unchanged; structural modality has no applicable
  surface on this function this session. candidate.c, evidence.md unchanged. My mandated
  modality is `structural`, not `escalation`, so per the owner's standing auto-ruling (A) I do
  not file an owner-gated disposition this session — that determination belongs to the driver
  once it assigns escalation modality. Outcome: progress (re-confirmation only, no new ground
  covered, no regression).
- verdict: KILLED (re-confirmation of s1's class kill, not a new kill)
- kill_scope: class
- measured_on: whole-body INCLUDE_ASM chassis, zero FAKE constructs (no C body exists);
  identical to s1's measured_on, re-confirmed by this session's `canonical` run.
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31

## [s2] A pure-C (or partial C-island) form can replace the whole-body asm for func_8004A4E0, re-tested under structural modality.
- mechanism: n/a — structural levers (block-local var splits, decl order, type narrowing, statement re-association) require an existing C body to apply to; none exists on this function.
- probe: canonical func_8004A4E0 (verdict ASM-WHOLE, 7 asm_insns, 163 total, same regions and reason string as prior sessions).
- result: Re-confirmed the standing s1 class kill; no new ground covered, no regression.
- verdict: KILLED
- kill_scope: class
- measured_on: whole-body INCLUDE_ASM chassis, zero FAKE constructs (no C body exists at all); identical to s1's measured_on.
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31

## [s3] The permuter can mutate a pure-C chassis for func_8004A4E0 on this body (comments/whitespace aside) to search for a lower-distance form.
- mechanism: n/a — permuter requires an existing C candidate as its seed chassis; this function's src/main/3AB48.c body is INCLUDE_ASM("asm/funcs", func_8004A4E0) with no C form, per the s1 canonical gate ASM-WHOLE verdict (inline_asm_canonical.txt whole-function grant) re-confirmed this session.
- probe: Re-ran `& tools/wteng.ps1 main canonical func_8004A4E0` and `& tools/wteng.ps1 main sandbox func_8004A4E0 --disable all` this session; saved raw output to tmp/grind/func_8004A4E0/s3/canonical.txt and tmp/grind/func_8004A4E0/s3/sandbox.txt.
- result: canonical() returned verdict ASM-WHOLE again (reason: whole-function canonical-asm grant in inline_asm_canonical.txt), identical to s1/s2; sandbox --disable all scored the INCLUDE_ASM stub itself (no_c_body: true, score 163, build_insns 0) — there is no C body for the permuter to perturb.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c INCLUDE_ASM stub (no C chassis, no FAKE constructs present); canonical+sandbox outputs this session identical to s1/s2 banked verdict

## [s4] The enumerate modality's systematic spelling sweep (owner ruling 2026-09-08) has an applicable region to sweep for func_8004A4E0.
- mechanism: n/a — the enumerate sweep (tools/spelling_enum.py + tools/sweep_variants.py) requires a C chassis with an identifiable differing block to rewrite in fully-named form between ENUM-BEGIN/ENUM-END markers; src/main/3AB48.c has no C body at all (INCLUDE_ASM stub), so there is no block to enumerate.
- probe: Re-ran `canonical func_8004A4E0` (verdict ASM-WHOLE again, same reason string "whole-function canonical-asm grant in inline_asm_canonical.txt", same regions/asm_insns=7/total=163 as s1/s2/s3) and `sandbox func_8004A4E0 --disable all` (score 163, no_c_body: true, build_insns 0, identical to s1-s3). Saved to tmp/grind/func_8004A4E0/s4/sandbox.txt.
- result: Re-confirmed the standing s1 class kill for a fourth consecutive modality (recon, structural, permuter, enumerate all land on the same no-C-body fact). No regression, no new ground.
- verdict: KILLED
- kill_scope: class
- measured_on: whole-body INCLUDE_ASM chassis, zero FAKE constructs (no C body exists at all); identical to s1-s3's measured_on.
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31

## [s4] The enumerate modality's systematic spelling sweep (owner ruling 2026-09-08) has an applicable region to sweep for func_8004A4E0.
- mechanism: n/a — the enumerate sweep (tools/spelling_enum.py + tools/sweep_variants.py) requires a C chassis with an identifiable differing block to rewrite in fully-named form between ENUM-BEGIN/ENUM-END markers; src/main/3AB48.c has no C body at all (INCLUDE_ASM stub), so there is no block to enumerate.
- probe: Re-ran canonical func_8004A4E0 (verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt', asm_insns=7, total=163, same regions as s1-s3) and sandbox func_8004A4E0 --disable all (score 163, no_c_body: true, build_insns 0, identical to s1-s3). Output saved to tmp/grind/func_8004A4E0/s4/sandbox.txt.
- result: Re-confirmed the standing s1 class kill for a fourth consecutive modality (recon, structural, permuter, enumerate all land on the same no-C-body fact). No regression, no new ground covered.
- verdict: KILLED
- kill_scope: class
- measured_on: whole-body INCLUDE_ASM chassis, zero FAKE constructs (no C body exists at all); identical to s1-s3's measured_on
- predicate_cite: docs/audits/func_8004A4E0-canonical-2026-09-29.md:31

## [s5] KILL RE-AUDIT: the s3 instance kill (permuter has no chassis) still holds on the current chassis with every FAKE construct ablated.
- mechanism: an instance kill is void if measured on a drifted chassis or under a FAKE carrier; s3 was measured on the INCLUDE_ASM stub with zero FAKE constructs.
- probe: `sandbox func_8004A4E0 --disable all` + `python3 tools/fake_ablate.py --func func_8004A4E0 --file main/3AB48 --candidate memory/grind/func_8004A4E0/candidate.c` this session.
- result: sandbox 163 / no_c_body true / build_insns 0 (s5/sandbox.txt); fake_ablate "No FAKE-annotated constructs found ... nothing to ablate" (s5/fake_ablate.txt). Chassis unchanged, no carrier — kill conditions intact.
- verdict: CONFIRMED

## [s5] SYNTHESIS: the function's layout carries a GCC-2.7.2-absent signal (code after the single epilogue) that the scanner does not test, so the 2026-10-08 re-audit gate (scan_hand_coded STRONG + Judge) is blind on this function for the same reason S7 is (caller-side, not callee-side, evidence).
- mechanism: GCC 2.7.2 emits the function epilogue once, after the body's last insn; jump-table case blocks that loop back with `j` precede it. A body whose case blocks sit after `jr $ra` is not the shape the compiler lays out.
- probe: corpus scan tmp/grind/func_8004A4E0/s5/midret.py over asm/funcs/*.s, split by whether the function is pure C in src/ today.
- result: compiled-C side 0 / 1225 functions have code after a single `jr $ra`; asm side 2 / 160 — func_8004A4E0 (69 insns: all five case bodies) and _patch_gte (PsyQ hand-written). Plus re-verified $s4 (callee-input only, never read locally) and $s5 (`3-$s5`, written once to 0) in the asm itself. Combined with the 2026-09-29 audit's cross-call ABI table, the hand-written verdict is supported by a second, independent, corpus-measured signal.
- verdict: CONFIRMED

## [s5] KILL RE-AUDIT: the s3 instance kill (permuter has no chassis to mutate) still holds on the current chassis with every FAKE construct ablated.
- mechanism: An instance kill is void if measured on a drifted chassis or under a FAKE carrier; s3 was measured on the INCLUDE_ASM stub with zero FAKE constructs.
- probe: sandbox func_8004A4E0 --disable all; python3 tools/fake_ablate.py --func func_8004A4E0 --file main/3AB48 --candidate memory/grind/func_8004A4E0/candidate.c
- result: sandbox score 163, no_c_body true, build_insns 0; fake_ablate: no FAKE-annotated constructs, nothing to ablate. Chassis unchanged, kill conditions intact.
- verdict: CONFIRMED

## [s5] The function's layout carries a signal absent from GCC 2.7.2 output (case-block code after the single epilogue) that scan_hand_coded.py does not test, so the owner 2026-10-08 re-audit gate (STRONG tier + Judge) is blind here for the same caller-side reason S7 is.
- mechanism: GCC 2.7.2 emits the epilogue once after the body's last insn; switch case blocks that loop back via j precede it. func_8004A4E0's five jump-table case bodies (.L8004A614/644/694/6F0/730) sit after jr $ra at 0x8004A60C.
- probe: Corpus scan tmp/grind/func_8004A4E0/s5/midret.py over all asm/funcs/*.s, split by pure-C-in-src vs still-asm; plus direct re-read of $s4/$s5 operands in asm/funcs/func_8004A4E0.s.
- result: Compiled-C side: 0/1225 functions have code after a single jr $ra. Asm side: 2/160 — func_8004A4E0 (69 insns) and _patch_gte (PsyQ hand-written, 9). $s4 is never read locally except its own addi (pure callee input before each jal func_8004A940); $s5 written once to 0 then read 5x as 3-$s5. A bare hazard-free nop sits at 0x8004A718. Independent corroboration of the 2026-09-29 audit's hand-written verdict.
- verdict: CONFIRMED

## [s6] SOLVER: the ra_solver/sched_solver triage has a residual to classify for func_8004A4E0.
- mechanism: inverse_compose.py classify compares a target text/object stream against our compiled object; with the function committed as INCLUDE_ASM there is no compiled body on our side, so no PRE-RA/RA/SCHED residual exists to type.
- probe: `python3 tools/ra_solver/inverse_compose.py classify main/3AB48 func_8004A4E0` (tmp/grind/func_8004A4E0/s6/classify.txt) — refused with its INCLUDE_ASM guard ("would report a FICTITIOUS PRE-RA verdict"); the suggested `goal_from_tgt.py classify` fallback could not run from the Windows interpreter (s6/goal_classify.txt, FileNotFoundError spawning the toolchain) and in any case has no ours-side body to compare.
- result: No solver axis exists on this chassis. Same no-C-body fact as s1-s5 (sandbox 163, no_c_body true).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c INCLUDE_ASM stub, no C chassis, zero FAKE constructs.

## [s6] DUMP-LEVEL PREDICATE for the s5 layout signal: GCC 2.7.2 (our cc1, project CC_FLAGS) emits the epilogue exactly once, as the last code of the function, after every switch case block — so case bodies after `jr $ra` are not compiler layout.
- mechanism: final.c:1066 — final_end_function calls FUNCTION_EPILOGUE (config/mips/mips.c:5073 function_epilogue) after the whole insn stream is output; the epilogue never exists as RTL, so no pass (jump2/dbr/block reorder) can move a case block past it.
- probe: tmp/grind/func_8004A4E0/s6/sketch.c — two switch-in-loop sketches of func_8004A4E0's control shape (5-case jump table inside a loop; variant 1 break+loop-tail, variant 2 `continue` arms + `default: return`) — compiled with tools/gcc-2.7.2/build/cc1 at the Makefile CC_FLAGS plus -da (run.sh; dumps sketch.i.*; output sketch.s).
- result: Both functions: jump-table dispatch (`j $2`), all case bodies jump back (`j .L14`/`j .L16`), and the single `j $31` is the final instruction before `.end` (sketch.s lines 153/251), restores immediately preceding it. Even the `default: return` arm in variant 2 is routed to the trailing epilogue rather than an in-body return. Compiler layout matches the 1225/1225 compiled-C corpus finding of s5 and contradicts func_8004A4E0's 69 case-body insns after `jr $ra` at 0x8004A60C.
- verdict: CONFIRMED

## [s6] inverse_compose.py classify can type a PRE-RA/RA/SCHED residual for func_8004A4E0 on the current INCLUDE_ASM chassis
- mechanism: classify needs a compiled ours-side body; INCLUDE_ASM has none, and the tool's guard refuses rather than emit a fictitious PRE-RA verdict
- probe: python3 tools/ra_solver/inverse_compose.py classify main/3AB48 func_8004A4E0; goal_from_tgt.py classify fallback
- result: classify refused with its INCLUDE_ASM guard (s6/classify.txt); goal_from_tgt fallback failed to spawn the toolchain from Windows Python (s6/goal_classify.txt) and has no ours-side body regardless. sandbox 163, no_c_body true (s6/sandbox.txt).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c INCLUDE_ASM stub, no C chassis, zero FAKE constructs

## [s6] Our GCC 2.7.2 cc1 at project CC_FLAGS emits the epilogue once as the final code of a switch-in-loop function, after every case block, so func_8004A4E0's 69 case-body insns after jr $ra are not compiler layout
- mechanism: final.c:1066 final_end_function emits FUNCTION_EPILOGUE (config/mips/mips.c:5073) as text after the whole insn stream; the epilogue is never RTL, so no pass can place a case block after it
- probe: compiled tmp/grind/func_8004A4E0/s6/sketch.c (two 5-case jump-table switch-in-loop sketches: break+loop-tail, and continue arms with default: return) with tools/gcc-2.7.2/build/cc1 -O2 -G0 ... -da
- result: Both: j $reg dispatch, every case jumps back to the loop head, single j $31 is the last insn before .end (sketch.s:153, :251); the default: return arm is routed to the trailing epilogue. Matches s5's 0/1225 corpus count; contradicts the target layout.
- verdict: CONFIRMED

## [s7] FORENSICS: re-audit of the standing instance kills on the current chassis.
- mechanism: instance kills s3/s6 rest on "no C body on the INCLUDE_ASM chassis"; re-check chassis + FAKE state.
- probe: sandbox --disable all --diff; canonical; fake_ablate on candidate.c; dump.ps1.
- result: chassis unchanged — score 163, no_c_body true, ASM-WHOLE grant, zero FAKE constructs, dump.ps1 has no C to dump (s7/sandbox_diff.txt, canonical.txt, fake_ablate.txt, dump_ps1.txt). Kills stand as recorded.
- verdict: CONFIRMED

## [s7] FORENSICS: $s6 is a callee-saved register written but never read locally — a shape absent from compiled C.
- mechanism: flow.c:1719 insn_dead_p deletes sets of dead registers; flow.c:1758 exempts hard regs only when global_regs[] (register-asm global = forbidden pin) or the frame pointer. A C-visible value copied into $s6 and never read would be deleted before final.
- probe: tmp/grind/func_8004A4E0/s7/savescan4.py corpus census over asm/funcs/*.s, sw/lw pairing for save/restore, C split = C definition in src minus INCLUDE_ASM minus inline_asm_canonical.txt.
- result: compiled-C 0/506; asm 1/91 (func_8004A4E0 $s6 only). Third independent corroboration (after the 2026-09-29 custom-ABI audit and the s5/s6 code-after-epilogue signal) that the body is hand-written.
- verdict: CONFIRMED

## [s7] FORENSICS: func_8004A4E0's saved-but-unreferenced $s3 is a hand-written discriminator.
- mechanism: MUST_SAVE_REGISTER (config/mips/mips.h:2056) saves only regs_ever_live callee-saved regs; hypothesis was that compiled C never leaves one unreferenced.
- probe: same census, signal A.
- result: compiled-C 5/506 do save a never-referenced callee-saved reg (func_8002E6B0, func_8005763C, math_RotMatrixXYZ/YXZ/ZXY) — regs_ever_live can be set by allocation and the referencing insns later disappear. Not a discriminator.
- verdict: KILLED
- kill_scope: instance
- measured_on: corpus census s7/savescan4.txt against the INCLUDE_ASM chassis (no C body, zero FAKE constructs)

## [s7] Re-audit: the s3/s6 instance kills (no C body on the INCLUDE_ASM chassis) still hold on the current chassis with FAKE constructs ablated
- mechanism: instance kills are chassis-relative; KILL RE-AUDIT required by the brief
- probe: sandbox func_8004A4E0 --disable all --diff; canonical func_8004A4E0; tools/fake_ablate.py on candidate.c; tools/grinder/dump.ps1
- result: score 163, no_c_body true; canonical ASM-WHOLE (whole-function grant in inline_asm_canonical.txt); fake_ablate finds no FAKE constructs; dump.ps1 has no C to dump. Kills stand.
- verdict: CONFIRMED

## [s7] func_8004A4E0 writes callee-saved $s6 (addu $s6,$fp,$zero at 0x8004A538) and never reads it locally, a shape the compiled-C corpus never shows
- mechanism: flow.c:1719 insn_dead_p deletes sets of dead registers; flow.c:1758 exempts a hard reg only if global_regs[] (file-scope register-asm variable = forbidden pin) or the frame pointer
- probe: tmp/grind/func_8004A4E0/s7/savescan4.py census over asm/funcs/*.s; save/restore found by sw/lw R,off($sp) pairing; compiled-C = C definition in src/, not INCLUDE_ASM, not in inline_asm_canonical.txt
- result: compiled-C 0/506; asm side 1/91, func_8004A4E0 $s6 only. Third independent signal for the hand-written verdict, after the 2026-09-29 custom-ABI audit and the s5/s6 code-after-epilogue signal
- verdict: CONFIRMED

## [s7] The saved-but-never-referenced $s3 in func_8004A4E0 discriminates hand-written from compiled code in this corpus
- mechanism: MUST_SAVE_REGISTER (config/mips/mips.h:2056) saves only regs_ever_live callee-saved registers
- probe: same census (savescan4), signal A
- result: compiled-C 5/506 also save a never-referenced callee-saved reg (func_8002E6B0 $s4, func_8005763C $s0, math_RotMatrixXYZ $s1, math_RotMatrixYXZ $s2, math_RotMatrixZXY $s1), so it does not discriminate
- verdict: KILLED
- kill_scope: instance
- measured_on: corpus census tmp/grind/func_8004A4E0/s7/savescan4.txt; function chassis is the INCLUDE_ASM stub, no C body, zero FAKE constructs

## [s8] REDERIVE: m2c fresh decompile attempt on asm/funcs/func_8004A4E0.s
- mechanism: rederive modality mandates a fresh m2c decompile pass before any other probe, to check for a pure-C shape the prior 7 sessions' manual analysis missed.
- probe: `python3 tools/m2c/m2c.py -t mips-gcc asm/funcs/func_8004A4E0.s` (tmp/grind/func_8004A4E0/s8/m2c_out.c, m2c_err.txt).
- result: m2c refuses outright — "Found jr instruction at func_8004A4E0.s line 33, but the corresponding jump table is not provided" (the jtbl_8001541C/jtbl_8001545C tables live in src/main/d_5BF0.c as C arrays, not as .s data m2c can ingest standalone). Investigating WHY the table can't simply be appended led directly to the s8 finding below — m2c's refusal was itself the correct signal, not a tooling gap to route around.
- verdict: CONFIRMED (m2c cannot produce a candidate for this body; superseded by the s8 class-kill below, which explains why no C form can exist, not just why m2c choked)

## [s8] REDERIVE — NEW, DECISIVE: func_8004A4E0's own switch body computed-jumps into the MIDDLE of a different function (func_8004A940), past that function's prologue, sharing live GPR state. No C switch/goto/function-pointer construct can express this under GCC 2.7.2's per-function compilation model.
- mechanism: jtbl_8001541C (src/main/d_5BF0.c:21-25, 16 entries) is func_8004A4E0's own case-dispatch table (loaded at asm/funcs/func_8004A4E0.s:29-31, jr $t0 at line 33). 6/16 entries point at 0x8004A5D0 = .L8004A5D0, a jlabel INSIDE func_8004A4E0 itself (line 66) — ordinary. The other 8 distinct entries (0x8004A990, 0x8004ABD8, 0x8004AE0C, 0x8004AF84, 0x8004B0C8, 0x8004B0E4, 0x8004B27C, 0x8004B4D8) are jlabels that exist ONLY inside func_8004A940's own body (asm/funcs/func_8004A940.s lines 24/171/313/408/490/498/601/753, between its own glabel line 1 and endlabel line 1186) — a wholly separate function symbol with its own prologue. func_8004A4E0's dispatch therefore transfers control directly into func_8004A940's mid-body code, bypassing func_8004A940's own register-save prologue entirely, with func_8004A4E0's live registers ($s0-$s7,$fp from its own prologue at lines 2-12) still resident. GCC 2.7.2 compiles one FUNCTION_DECL at a time (toplev.c:2647 rest_of_compilation(decl) — each call finishes RTL generation, register allocation, and final assembly for exactly one function before the next DECL is processed) and a switch's jump-table labels are generated and bound entirely within that single compilation pass (stmt.c:4704 `table_label = gen_label_rtx()` inside expand_end_case, operating on the per-function `case_stack`/`struct nesting` torn down when that function's expansion ends). There is no C construct — not `switch`, not `goto`, not a function-pointer call, not computed `&&label` goto — whose lowering under this compiler can emit a jump-table entry that resolves to a label physically inside a DIFFERENT function's compiled body; every label `gen_label_rtx()` produces for one function's case statement is local to that function's insn chain and is never visible to, or interleaved with, another function's RTL. This is independent of and architecturally stronger than the s5 (code-after-epilogue) and s7 ($s6 written-never-read) signals: those show statistical absence from the 506-function compiled-C corpus, this shows the construct cannot be *expressed* by the compiler at all, for any C source.
- probe: grep for each jtbl_8001541C target label across asm/funcs/*.s; confirmed exclusive membership in func_8004A940.s (tmp/grind/func_8004A4E0/s8/jtbl_cross_function_evidence.txt); read toplev.c:2647 and stmt.c:4681-4740 for the per-function compilation/label-scope invariant.
- result: 8 of 16 jump-table entries resolve to labels that exist only inside a different function's own body, with no shared/export marking; GCC's per-DECL compilation model and per-function label scoping mean no switch/goto lowering can reproduce this from any C source.
- verdict: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A4E0.s + asm/funcs/func_8004A940.s + src/main/d_5BF0.c jump-table data (ground truth; no C chassis, zero FAKE constructs — this is a structural/architectural kill, not a chassis-relative one)
- predicate_cite: tools/gcc-2.7.2/toplev.c:2647 (rest_of_compilation compiles one function's DECL to completion before the next); tools/gcc-2.7.2/stmt.c:4704 (expand_end_case's table_label is generated and scoped within the current function's case_stack, never shared across functions)

## [s8] m2c can produce a fresh pure-C decompile of func_8004A4E0's asm that the prior 7 sessions' manual analysis missed
- mechanism: m2c.py -t mips-gcc parses the function's .s and jump tables to synthesize structured C
- probe: python3 tools/m2c/m2c.py -t mips-gcc asm/funcs/func_8004A4E0.s
- result: m2c refuses outright: 'Found jr instruction at func_8004A4E0.s line 33, but the corresponding jump table is not provided' (jtbl_8001541C/jtbl_8001545C are C arrays in src/main/d_5BF0.c, not inline .s data). Chasing why the table can't simply be supplied led to the s8 finding below.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_8004A4E0.s via tools/m2c/m2c.py, no C chassis, zero FAKE constructs (tmp/grind/func_8004A4E0/s8/m2c_out.c, m2c_err.txt)

## [s8] func_8004A4E0's own switch-style computed jump (via jtbl_8001541C) transfers control directly into the middle of a DIFFERENT function's (func_8004A940) compiled body, bypassing func_8004A940's own prologue and sharing func_8004A4E0's live register state with it — a shape no switch/goto/function-pointer lowering under GCC 2.7.2 can produce from any C source, because the compiler finishes one FUNCTION_DECL's RTL generation and final assembly before starting the next, and a switch's jump-table labels are generated and scoped entirely within that one function's compilation pass
- mechanism: toplev.c:2647 rest_of_compilation(decl) compiles exactly one function to completion before the next DECL is processed; stmt.c:4704 (inside expand_end_case) generates the jump table's labels via gen_label_rtx() bound to the current function's case_stack/struct nesting, which is torn down when that function's expansion ends — no mechanism exists for one function's jump table to reference a label live inside another function's own insn stream
- probe: grepped every one of jtbl_8001541C's 16 word entries (src/main/d_5BF0.c:21-25) against every jlabel in asm/funcs/*.s: 6/16 resolve to .L8004A5D0 inside func_8004A4E0 itself (ordinary); the other 8 distinct targets (0x8004A990, 0x8004ABD8, 0x8004AE0C, 0x8004AF84, 0x8004B0C8, 0x8004B0E4, 0x8004B27C, 0x8004B4D8) match jlabels that exist ONLY inside func_8004A940.s, between its own glabel (line 1) and endlabel (line 1186), with no export/shared marking
- result: confirmed by direct grep: those 8 labels are private to func_8004A940's own body. func_8004A4E0's dispatch therefore free-falls into func_8004A940 mid-function, past its prologue, with func_8004A4E0's $s0-$s7/$fp still live from its own prologue (asm/funcs/func_8004A4E0.s:2-12). This is architecturally impossible to emit from C under this compiler for ANY source shape — stronger than the prior statistical/corpus signals (s5 code-after-epilogue, s7 $s6 written-never-read), which showed absence from the 506-function compiled corpus; this shows the construct cannot be generated by the compiler at all.
- verdict: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A4E0.s + asm/funcs/func_8004A940.s + src/main/d_5BF0.c ground truth (no C chassis exists for func_8004A4E0; zero FAKE constructs; this kill does not rest on chassis or FAKE state)
- predicate_cite: tools/gcc-2.7.2/toplev.c:2647

## [s9] KILL RE-AUDIT: the INCLUDE_ASM chassis all prior instance kills (s3/s6/s7/s8) were measured on is unchanged and carries zero FAKE constructs
- mechanism: driver-mandated re-audit (floor flat 3+ sessions with instance kills)
- probe: `canonical func_8004A4E0` + `sandbox func_8004A4E0 --disable all --diff` re-run this session (tmp/grind/func_8004A4E0/s9/canonical.txt, sandbox_diff.txt). fake_ablate is not applicable: candidate.c is a comment-only record (no C body, no FAKE construct to ablate).
- result: canonical = ASM-WHOLE ("whole-function canonical-asm grant in inline_asm_canonical.txt"); sandbox scores the INCLUDE_ASM stub, "no diff: function is still whole-body INCLUDE_ASM". No chassis drift, no FAKE carrier occupying any pseudo → every prior instance kill stands as measured.
- verdict: CONFIRMED

## [s9] The jtbl/jal-reached helper func_8004A940 saves AND restores $ra through the scratchpad ($s1 = 0x1F800000: `sw $ra,4($s1)` … `lw $ra,4($s1); jr $ra`), a $ra spill base no GCC 2.7.2 MIPS lowering emits
- mechanism: mips.c save_restore_insns (config/mips/mips.c:4588, base chosen at :4632/:4640/:4659) only ever addresses the $ra slot off stack_pointer_rtx or MIPS_TEMP2 computed from $sp; MIPS 2.7.2 defines no RETURN_ADDR_RTX (expr.c:7196 is #ifdef'd), so C cannot even name $ra to store it elsewhere, let alone reload $ra from memory and return through it.
- probe: corpus census tmp/grind/func_8004A4E0/s9/rascan.py over all 1437 asm/funcs/*.s, split as in s7 (C = has a C definition, not INCLUDE_ASM, not canonical).
- result: compiled C 0/1164 store $ra off-$sp; ASM 12/273 do: InitGeom, _ExitCard, __SN_ENTRY_POINT, _patch_gte, _patch_pad, _remove_ChgclrPAD, _send_pad, setjmp (all known Sony/SN hand-asm) + func_8004A940, func_8004BB68, func_8004BCC0, func_8004C1F4. func_8004A4E0's dispatch table and 5 jal sites (asm/funcs/func_8004A4E0.s:93,114,138,155,171) enter this helper; together they form a hand-written bytecode interpreter ($s0 = instruction pointer over u16 opcodes, $s4 = handler table loaded from 0x1F80001C, $s1 = scratchpad context, jalr dispatch at func_8004A940.s:11-12).
- verdict: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A940.s:2,20 ground truth + 1164-function compiled-C corpus census; no C chassis exists for func_8004A4E0 (INCLUDE_ASM stub), zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4632

## [s9] Callee-saved registers live-in at entry (read before any write) in func_8004A4E0's helpers is a hand-written-ABI signal
- mechanism: compiled C reads an $s-reg only after RA assigns a pseudo to it; a pseudo live at function start (flow.c:1199-1203) arises only from an uninitialized local, which no matched function exhibits — so this is a statistical, not architectural, signal.
- probe: same rascan.py, signal L (linear scan to first branch).
- result: compiled C 0/1164; ASM 6/273: setjmp, func_8004BCC0 (all 9 callee-saved), func_800790A4 ($s5), and the three func_8004A4E0 helpers func_8004A76C ($s0), func_8004A808 ($s0), func_8004A940 ($s0,$s1) — i.e. the custom callee-saved register ABI cited in the 2026-09-29 canonical audit is independently visible in a mechanical census.
- verdict: CONFIRMED

## [s9] KILL RE-AUDIT: the INCLUDE_ASM chassis the s3/s6/s7/s8 instance kills were measured on is unchanged and carries zero FAKE constructs
- mechanism: driver-mandated re-audit after 3+ flat sessions with instance kills
- probe: canonical + sandbox --disable all --diff re-run (tmp/grind/func_8004A4E0/s9/canonical.txt, sandbox_diff.txt); fake_ablate n/a since candidate.c has no C body
- result: canonical ASM-WHOLE (whole-function grant in inline_asm_canonical.txt); sandbox scores the INCLUDE_ASM stub, no diff; no chassis drift, no FAKE carrier, so prior instance kills stand
- verdict: CONFIRMED

## [s9] func_8004A940 (reached from func_8004A4E0's jump table and 5 jal sites) stores $ra to 4($s1) and reloads it from there before jr $ra; GCC 2.7.2 MIPS addresses the $ra save slot only off $sp (or a temp derived from $sp), and has no RETURN_ADDR_RTX to name $ra from C
- mechanism: config/mips/mips.c save_restore_insns:4588, base_reg_rtx = stack_pointer_rtx (:4632) or MIPS_TEMP2 = offset+$sp (:4640/:4659); expr.c:7196 RETURN_ADDR_RTX undefined for MIPS
- probe: corpus census tmp/grind/func_8004A4E0/s9/rascan.py over 1437 asm/funcs/*.s split compiled-C vs asm as in s7
- result: compiled C 0/1164 store $ra off-$sp; asm 12/273 do (setjmp, _patch_gte, __SN_ENTRY_POINT, InitGeom, pad/card stubs, func_8004A940, func_8004BB68, func_8004BCC0, func_8004C1F4) - only hand-written code
- verdict: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A940.s:2,20 ground truth + 1164-function compiled-C census; func_8004A4E0 chassis is the INCLUDE_ASM stub, zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.c:4632

## [s9] Callee-saved registers read before any write at entry (live-in) in func_8004A4E0's helpers mark the custom register ABI cited by the canonical audit
- mechanism: compiled C only reads an $s-reg after RA assigns it a pseudo; entry-live pseudos (flow.c:1199-1203) arise only from uninitialized locals - statistical, not architectural
- probe: rascan.py signal L (linear scan to first branch)
- result: compiled C 0/1164; asm 6/273: setjmp, func_8004BCC0, func_800790A4 and all three func_8004A4E0 helpers func_8004A76C ($s0), func_8004A808 ($s0), func_8004A940 ($s0,$s1)
- verdict: CONFIRMED

## [s10] KILL RE-AUDIT: INCLUDE_ASM chassis unchanged, zero FAKE constructs
- mechanism: driver-mandated re-audit (flat floor, instance kills exist)
- probe: canonical + sandbox --disable all --diff (tmp/grind/func_8004A4E0/s10/canonical.txt, sandbox_diff.txt)
- result: ASM-WHOLE grant; sandbox no diff on stub; s3/s6/s7/s8 instance kills stand as measured
- verdict: CONFIRMED

## [s10] scan_hand_coded.py's own tier for func_8004A4E0 is STRONG (so the grant has direct scanner backing)
- mechanism: S1/S2/S6/S7/S8 GCC-impossible signals (tools/scan_hand_coded.py:163-182)
- probe: python3 tools/scan_hand_coded.py --single func_8004A4E0 (+ the 3 helpers)
- result: func_8004A4E0 tier=LOW 0/8; helpers A940/A76C/A808 STRONG via S7. The scanner has no signal for code-after-epilogue, written-never-read $sN, trapping add/addi, ctc2, or cross-function jtbl targets; backing is transitive + owner-ruled audit
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_8004A4E0.s with tools/scan_hand_coded.py at HEAD 45f59e10a; INCLUDE_ASM stub chassis, zero FAKE constructs (tmp/grind/func_8004A4E0/s10/scan_*.txt)

## [s10] Jump-table targets outside the referencing function occur in compiled C in this corpus
- mechanism: GCC 2.7.2 expand_end_case builds table labels from the current function's case_stack (stmt.c:4704); compiled C should show 0
- probe: tmp/grind/func_8004A4E0/s10/jtblscan.py over all asm/funcs jtbl references, table words from disc/SLUS_006.63
- result: compiled C 0/58 pairs cross a function boundary; asm 1/8 — only func_8004A4E0/jtbl_8001541C (8/16 targets in func_8004A940)
- verdict: KILLED
- kill_scope: instance
- measured_on: 58 compiled-C + 8 asm (referencer, jtbl) pairs at HEAD 45f59e10a; func_8004A4E0 chassis INCLUDE_ASM stub, zero FAKE constructs (tmp/grind/func_8004A4E0/s10/jtblscan.txt)

## [s10] KILL RE-AUDIT: the INCLUDE_ASM chassis the s3/s6/s7/s8 instance kills were measured on is unchanged and carries zero FAKE constructs
- mechanism: driver-mandated re-audit after a flat floor with instance kills
- probe: canonical func_8004A4E0 + sandbox func_8004A4E0 --disable all --diff (tmp/grind/func_8004A4E0/s10/canonical.txt, sandbox_diff.txt); fake_ablate n/a, candidate.c has no C body
- result: canonical ASM-WHOLE (whole-function grant inline_asm_canonical.txt:199); sandbox scores the INCLUDE_ASM stub with no diff; prior instance kills stand
- verdict: CONFIRMED

## [s10] scan_hand_coded.py's own tier for func_8004A4E0 is STRONG, giving the ASM-WHOLE grant direct scanner backing
- mechanism: GCC-impossible signals S1/S2/S6/S7/S8 in tools/scan_hand_coded.py:163-182
- probe: python3 tools/scan_hand_coded.py --single on func_8004A4E0, func_8004A940, func_8004A76C, func_8004A808
- result: func_8004A4E0 tier=LOW 0/8. Helpers func_8004A940 (S3+S4+S7, $s0..$s7 unsaved), func_8004A76C (S7 $s0), func_8004A808 (S3+S4+S7 $s0) are STRONG. The scanner has no signal for code after the epilogue jr $ra, written-never-read $sN, trapping add/addi, ctc2, or cross-function jtbl targets; the grant rests on the owner-ruled Q40 audit plus this ledger, and on the helpers' STRONG tier transitively.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/func_8004A4E0.s with tools/scan_hand_coded.py at HEAD 45f59e10a; INCLUDE_ASM stub chassis, zero FAKE constructs (tmp/grind/func_8004A4E0/s10/scan_*.txt)

## [s10] Jump-table entries targeting code outside the referencing function occur among this corpus's compiled-C switch tables
- mechanism: GCC 2.7.2 expand_end_case generates table labels from the current function's case_stack only (stmt.c:4704), so compiled C should show none
- probe: tmp/grind/func_8004A4E0/s10/jtblscan.py: every (referencing function, jtbl_ symbol) pair in asm/funcs, table words read from disc/SLUS_006.63 (bounded by declared C array size, else next jtbl symbol or first non-.text word), each target mapped to its containing function by glabel ranges
- result: compiled C 0/58 pairs cross a function boundary; asm 1/8 crosses, only func_8004A4E0/jtbl_8001541C (8/16 targets in func_8004A940). This is the statistical complement of the s8 class kill. Side note: the words after jtbl_8001545C[5] are function-entry addresses (func_8004BCC0, func_8004C994, ...), consistent with the interpreter's handler table.
- verdict: KILLED
- kill_scope: instance
- measured_on: 58 compiled-C + 8 asm (referencer, jtbl) pairs at HEAD 45f59e10a; func_8004A4E0 chassis INCLUDE_ASM stub, zero FAKE constructs (tmp/grind/func_8004A4E0/s10/jtblscan.txt)

## [s11] REDERIVE: sibling-ledger transplant check (math_RotMatrixXYZ/YXZ/ZXY, puts, InitGeom, _patch_gte, _remove_ChgclrPAD, func_800790A4)
- mechanism: inheritance check mandated before any new probe this session
- probe: read each sibling's status/candidate in the auto-attached ledger digest; checked data-model block for shared globals (g_draw_queue, D_80103608, g_draw_queue_cursor)
- result: all 7 siblings are closed/COMPLETED (0 grind sessions each), and the only overlap with func_8004A4E0 is shared GLOBAL DATA (g_draw_queue, D_80103608), not shared code blocks or chassis shape — none has a switch/jtbl/cross-function-dispatch body to transplant. No applicable transplant surface.
- verdict: KILLED
- kill_scope: instance
- measured_on: sibling ledger digest + include/bb2.h decls at HEAD 63ba4892b; func_8004A4E0 chassis INCLUDE_ASM stub, zero FAKE constructs

## [s11] REDERIVE: decomp.me corpus mining (tools/decomp_me_scrape.py) as a new rederive source
- mechanism: tool requires curl_cffi (Cloudflare-evading TLS fingerprint) to hit the decomp.me API; no local corpus is cached under tmp/decomp_me_corpus/
- probe: `python3 tools/decomp_me_scrape.py probe` via both native Windows python3 and WSL .venv python3 (tmp/grind/func_8004A4E0/s11/decompme_probe.txt)
- result: ModuleNotFoundError: curl_cffi not installed in .venv; no cached corpus exists locally. Tool is infeasible in this sandbox session without a package install. Separately: even if reachable, decomp.me scratches are single-function compiled-C targets under the same GCC 2.7.2 per-function compilation model the s8/s10 class kill already covers (toplev.c:2647, stmt.c:4704) — they structurally cannot contain a jump-table entry whose target lives inside a DIFFERENT function's compiled body sharing live register state, so a reachable corpus would not have produced a different rederive outcome here.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/decomp_me_scrape.py at HEAD 63ba4892b, this sandbox's .venv; func_8004A4E0 chassis INCLUDE_ASM stub, zero FAKE constructs

## [s11] KILL RE-AUDIT: chassis + canonical verdict unchanged since s10
- mechanism: driver-mandated re-audit after a flat floor
- probe: canonical func_8004A4E0 (tmp/grind/func_8004A4E0/s11 inline above); sandbox func_8004A4E0 --disable all (tmp/grind/func_8004A4E0/s11/sandbox.txt)
- result: canonical ASM-WHOLE (inline_asm_canonical.txt whole-function grant); sandbox score 163, no_c_body true, build_insns 0 — identical to s10. Prior instance/class kills stand.
- verdict: CONFIRMED

## [s11] A sibling ledger (math_RotMatrixXYZ/YXZ/ZXY, puts, InitGeom, _patch_gte, _remove_ChgclrPAD, func_800790A4) carries a transplantable C form or block for func_8004A4E0's chassis.
- mechanism: sibling-inheritance check mandated before any new probe this session
- probe: read the sibling ledger digest (all 7 closed, 0 grind sessions) and the data-model block for shared globals (g_draw_queue, D_80103608, g_draw_queue_cursor)
- result: All 7 siblings are unrelated completed utility/library functions; the only overlap with func_8004A4E0 is shared GLOBAL DATA references, not shared code shape. None has a switch/jtbl/cross-function-dispatch body. No transplant performed.
- verdict: KILLED
- kill_scope: instance
- measured_on: sibling ledger digest + include/bb2.h decls at HEAD 63ba4892b; func_8004A4E0 chassis INCLUDE_ASM stub, zero FAKE constructs

## [s11] tools/decomp_me_scrape.py can mine the decomp.me GCC-2.7.2/PsyQ corpus this session for a rederive candidate for func_8004A4E0.
- mechanism: decomp.me scratch corpus under matching compilers as a known-good-codegen source
- probe: python3 tools/decomp_me_scrape.py probe, run both via native Windows python3 and via WSL .venv python3 (tmp/grind/func_8004A4E0/s11/decompme_probe.txt)
- result: curl_cffi dependency is not installed in .venv and no local corpus cache exists under tmp/decomp_me_corpus/; the probe is infeasible in this sandbox session. Separately, even a reachable corpus consists of single-function compiled-C scratches under the same GCC 2.7.2 per-function compilation model the s8/s10 class kill already covers (toplev.c:2647, stmt.c:4704), so it structurally could not contain func_8004A4E0's shape (a jump-table entry whose target lives inside a DIFFERENT function's compiled body, sharing live register state across the function boundary).
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/decomp_me_scrape.py at HEAD 63ba4892b, this sandbox's .venv; func_8004A4E0 chassis INCLUDE_ASM stub, zero FAKE constructs

## [s11] The ASM-WHOLE canonical-grant verdict and INCLUDE_ASM chassis for func_8004A4E0 are unchanged since s10, with zero FAKE constructs present.
- mechanism: driver-mandated kill re-audit after 3+ flat sessions with instance kills
- probe: canonical func_8004A4E0; sandbox func_8004A4E0 --disable all (tmp/grind/func_8004A4E0/s11/sandbox.txt)
- result: canonical verdict ASM-WHOLE (inline_asm_canonical.txt whole-function grant), sandbox score 163, no_c_body true, build_insns 0 -- identical to s10. All prior instance and class kills (s3, s6, s7, s8, s9, s10) stand unweakened.
- verdict: CONFIRMED
