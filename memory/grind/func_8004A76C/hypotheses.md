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
