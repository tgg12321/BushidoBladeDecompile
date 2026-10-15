# Hypothesis ledger — func_8004A76C

## s1 (recon)

### H1 — a GCC file-scope global-register-variable (`register T x asm("$16")`) could
reproduce the unsaved-`$s0` S7 shape from ordinary C, making this a decompilable (not
whole-body-asm) function.

- mechanism: `register` storage class + `asm("$N")` binds a C variable to a fixed GPR for
  the whole TU, which would make a "callee-saved register used without save" shape
  reproducible from C instead of requiring hand asm.
- probe: this exact question was raised and closed by `docs/audits/INLINE-ASM-AUDIT-2026-10-01.md`
  item A6 (read in full this session). Re-confirmed this session: (1) the sole caller
  `func_8004A4E0` is independently whole-body hand asm (ABI evidence in
  `memory/grind/func_8004A4E0/evidence.md` s1/s18, hands `$s0` live across the `jal` with no
  GCC-emittable mechanism); (2) the loop body is hand-software-pipelined (`rtpt` for
  iteration N+1 issued before iteration N's stores retire, entry `j` jumps into the unrolled
  loop's second half to prime the pipeline) — not a shape any GCC 2.7.2 scheduler emits from
  a straight-line C loop; (3) a global register variable is itself a register pin, banned by
  `.claude/rules/completion-bar.md` item 2 (the wall) regardless of whether it reproduces
  bytes.
- result: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A76C.s (39 insns), no FAKE carrier (no C body exists to
  carry one)
- predicate_cite: docs/audits/INLINE-ASM-AUDIT-2026-10-01.md:93 (A6 row, "CLOSED, no
  change" — caller is whole-body asm, loop is hand-pipelined, global-register-var is itself
  a banned pin)

## Frontier (next session / next modality)

1. hypothesis: the ASM-WHOLE canonical grant is correct and the 2026-10-08 reopen's question
   ("decide afresh: hand-written asm vs compiled C") is already answered for this function.
   mechanism: `scan_hand_coded --single` tier=STRONG 1/8 (S7) directly on func_8004A76C —
   unlike sibling func_8004A4E0 (LOW 0/8, Judge-FAILed on exactly the STRONG-tier bar at
   docs/grind/decisions.md:1320), this function clears the scanner-STRONG requirement on its
   own evidence, plus the closed A6 audit naming it specifically.
   next_probe: in `escalation` modality (not before — mandated-modality rule forbids
   disposing here), file the owner-gated/decisions.md entry citing A6 + the STRONG scan
   tier as the gate evidence, and the re-activation triggers (an engine signal change, or
   sibling func_8004A4E0/func_8004A808/func_8004A940 movement).
2. hypothesis: func_8004A808 and func_8004A940 (same TU, same ABI cluster, also reopened)
   will scan the same way — worth checking their scan tier before any of the three is
   escalated, since a consistent STRONG result across the cluster strengthens the record.
   mechanism: shared callee-saved-register ABI cluster per func_8004A4E0's audit.
   next_probe: `scan_hand_coded.py --single func_8004A808` / `func_8004A940` (quick, no build).

## [s1] A GCC file-scope global-register-variable (register T x asm("$16")) could reproduce the unsaved-$s0 S7 shape from ordinary C, making this function decompilable rather than whole-body hand asm.
- mechanism: register storage class + asm("$N") binds a C variable to a fixed GPR for the whole TU, which in principle could make a 'callee-saved register used without save' shape emittable from C.
- probe: Re-read docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6 (which raised this exact question) plus func_8004A4E0's own canonical audit (caller ABI evidence) and the raw asm/funcs/func_8004A76C.s body (software-pipelining shape).
- result: A6 already closed this with three independent reasons: (1) the sole caller func_8004A4E0 is itself whole-body hand asm handing $s0 live across the jal in a way GCC cannot emit; (2) the loop is hand software-pipelined (rtpt for iteration N+1 issued before iteration N's stores retire, entry j jumps into the unrolled loop's second half) — not a GCC-schedulable shape; (3) a global register variable is itself a register pin, independently banned by the wall regardless of byte reproduction.
- verdict: KILLED
- kill_scope: class
- measured_on: asm/funcs/func_8004A76C.s (39 insns); no C body exists so no FAKE carrier is present
- predicate_cite: docs/audits/INLINE-ASM-AUDIT-2026-10-01.md:93

## s2 (structural)

### H2 — structural-modality levers (block-local var splits, declaration order, type
narrowing, statement re-association) require an existing C body with a sandbox-measured
non-zero distance to operate on; this function has neither.

- mechanism: every structural technique transforms source text to change codegen under a
  fixed semantic; with no C body (`src/main/3AB48.c:30` is a bare `INCLUDE_ASM` stub) there
  is no source to transform and no `--diff` hunk classification (source-level / operand-only
  / not-scored) to target. This is not "structural work tried and ineffective" — it is
  structural work with no object to apply it to, same as s1 already found for recon.
- probe: re-ran `canonical func_8004A76C` (still ASM-WHOLE, same reason) and `sandbox
  func_8004A76C --disable all` (still `{"score":37,"no_c_body":true,"build_insns":0}`,
  `--diff` still "no diff: function is still whole-body INCLUDE_ASM") to confirm nothing
  changed since s1 before concluding the modality is inapplicable here.
- result: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c (INCLUDE_ASM stub, unchanged), no FAKE carrier (no C
  body exists)

### H3 — the func_8004A4E0 custom-ABI cluster (func_8004A76C/func_8004A808/func_8004A940)
will scan STRONG uniformly, strengthening the cluster-wide escalation record ahead of any one
member's escalation session (s1 frontier item 2).

- mechanism: shared callee-saved-register ABI violation across the cluster, per
  func_8004A4E0's own canonical audit — each callee leaks unsaved `$sN` state back across its
  `jal` boundary to the caller.
- probe: `scan_hand_coded.py --single func_8004A808` and `--single func_8004A940` (no build
  needed).
- result: CONFIRMED. func_8004A808: tier=STRONG 3/8 (S3+S4+S7, unsaved `$s0`). func_8004A940:
  tier=STRONG 3/8 (S3+S4+S7, unsaved `$s0`-`$s7`, all eight). All three callees now carry an
  independently-measured STRONG verdict; only the caller func_8004A4E0 scans LOW, which is the
  asymmetry already on record from its own Judge FAIL.

## Frontier (next session / next modality) — unchanged from s1, now with cluster scan done

1. hypothesis: the ASM-WHOLE canonical grant is correct and the reopen's question is already
   answered for this function specifically.
   mechanism: scan_hand_coded STRONG 1/8 (S7) measured directly on func_8004A76C, plus the
   closed A6 audit, plus (new this session) both sibling callees independently confirming
   STRONG on the same S7 signal.
   next_probe: in `escalation` modality (not before), file the docs/grind/decisions.md
   disposition entry citing A6 + the cluster-wide STRONG scan results, then return
   owner-gated.
2. (s1 frontier item 2 is now closed — see H3 above. No remaining frontier item from it.)

## [s2] Structural-modality levers (block-local var splits, declaration order, type narrowing, statement re-association) require an existing C body with a sandbox-measured non-zero distance to operate on; this function has neither.
- mechanism: src/main/3AB48.c:30 is a bare INCLUDE_ASM stub (canonical gate = ASM-WHOLE); no C source exists to transform and no --diff hunk classification (source-level/operand-only/not-scored) exists to target.
- probe: Re-ran `canonical func_8004A76C` and `sandbox func_8004A76C --disable all [--diff]` to confirm nothing changed since s1 before concluding the modality is inapplicable.
- result: canonical still ASM-WHOLE ('whole-function canonical-asm grant in inline_asm_canonical.txt'); sandbox still {"score":37,"no_c_body":true,"build_insns":0}; --diff still reports no diff exists.
- verdict: KILLED
- kill_scope: instance
- measured_on: current src/main/3AB48.c (INCLUDE_ASM stub, unchanged this session), no FAKE carrier (no C body exists)

## [s2] func_8004A808 and func_8004A940 (same jal-cluster, same custom callee-saved-register ABI reopened alongside func_8004A76C) will scan STRONG on scan_hand_coded the same way, strengthening a cluster-wide record ahead of any one member's escalation.
- mechanism: Shared callee-saved-register ABI violation across the func_8004A4E0/func_8004A76C/func_8004A808/func_8004A940 cluster: each callee leaves $sN registers unsaved and relies on the caller (itself whole-body hand asm) to treat them as live across the jal, a shape no GCC-emitted callee can produce.
- probe: scan_hand_coded.py --single func_8004A808 and --single func_8004A940 (no build needed).
- result: func_8004A808: tier=STRONG score=3/8 (S3 no-spills, S4 front-loads, S7 unsaved $s0 -- 'GCC-impossible, decisive'). func_8004A940: tier=STRONG score=3/8 (same S3/S4/S7 triplet, S7 firing on all eight callee-saves $s0-$s7, the maximal shape). Both independently confirm the S7 signal already measured on func_8004A76C itself; only the cluster's caller func_8004A4E0 scans LOW (0/8), the asymmetry already on record from its own s18/s18b Judge FAIL.
- verdict: CONFIRMED
