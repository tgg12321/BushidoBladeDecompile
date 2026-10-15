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
