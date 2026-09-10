# Hypothesis ledger — func_8002EBDC

## s1 (recon, 2026-09-10)

### H1 — sibling-template transplant closes the whole function — CONFIRMED (score 0)
- Statement: func_8002EBDC is the structural twin of the merged cluster sibling
  func_8002E838 (src/code6cac_b.c:1366, inline_asm_canonical.txt:373); writing
  its body with the same five SDK-macro islands per GTE pass and ordinary C for
  the head (ratan2 / squared magnitude / LUT lookup), the identity-matrix
  blocks, the RotMatrix* calls, the three signed /256 scalings and the negated
  second pass reaches sandbox 0 with no coercion.
- Mechanism: the target's non-island code is textbook GCC 2.7.2 output for
  those statements (cse constant folding of the scratchpad base inside the first
  EBB, `$s2` constant hold for 0x1000, the bgez/addiu 0xFF/sra 8 signed-divide
  idiom, sched interleaving of the next lw/mult into the divide gap).
- Probe: body_v1 (tmp/grind/func_8002EBDC/s1/body_v1.c) spliced in place of the
  INCLUDE_ASM, `sandbox func_8002EBDC --disable all`.
- Result: **score 0, 182/182, rules_dropped 0** on the first measurement;
  comment-only body_v2 (macro-name + inline_c.h:101-110 citations per the
  2026-09-02 Ruling A) re-measured 0. `verify-oracle --rebuild` refused
  (dirty-build-inputs guard, by design) — bytes proof is the driver's step.
- Verdict: CONFIRMED.

### H0 — DATA MODEL declaration fix (D_8008D118 header decl absent) — not a codegen signal
- The DATA MODEL flag is "decl NONE in include/*.h"; the TU-local
  `extern u8 D_8008D118;` (src/code6cac_b.c:278) reproduces both LUT sites
  byte-exact (score 0 measured with it). No declaration change needed; see the
  OBJECT MODEL entry in evidence.md.

## Frontier
- (none) — function at sandbox 0; next step is the Judge / owner-cluster grant
  door (honest bucket COMPLETED-INLINE-ASM-CANONICAL, allowlist line written by
  the driver/operator, as for func_8002E838 / func_8002EA24 / func_80031890).
