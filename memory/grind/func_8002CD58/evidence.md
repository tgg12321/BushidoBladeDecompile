# func_8002CD58 evidence

## 2026-09-24 canonical gate

`engine canonical func_8002CD58` returns `ASM-PARTIAL`: 54 of 352
instructions are canonical GTE/cop2 operations (`ctc2`, `lwc2`, `mtc2`,
`swc2`, and GTE commands) across 24 regions. Per `docs/DECOMP_WORKFLOW.md`,
this function must not be ground as ordinary pure C. No candidate was written
and main remains `INCLUDE_ASM`.
## 2026-09-24 correction (operator review)

The rotation above was improper and has been reversed (`queue unpark`).
`ASM-PARTIAL` is NOT the `ASM-REGION`/`ASM-STRUCTURAL` "do not grind" route:
the gate is region-granular (engine/canonical.py docstring) — the cop2 spans
take canonical GTE macros (inline_c.h / gtemac.h islands with the disclosed
cop2 addressing preamble, `.claude/rules/cop2-addressing-preamble-cluster.md`)
and EVERYTHING ELSE is ordinary pure C. Precedent: func_8001F2E4 (ASM-PARTIAL,
347 insns) reached COMPLETED-INLINE-ASM-CANONICAL in one manual session
(c7a9e4c6a). No attempt has been made on this function yet.
