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
