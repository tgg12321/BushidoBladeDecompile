# Hypothesis ledger - func_80042C80

## s1 (recon, 2026-09-08)
- H1 CONFIRMED - Declaration fix / data model: `extern s16 Judge[]` (TU-local) is the correct
  object model; no header/address row exists but the distance-0 body proves the s16 element
  type. No declaration change needed.
- H2 CONFIRMED - Load shape: sinA/sinB/sinC/cosA as s16 locals loaded before the `a1[2] = sinB`
  store and first used after it reproduce the target's four lhu+sll+sra loads; cosB/cosC as s32
  reproduce the two lh loads (combine.c:914-917 use_crosses_set_p on MEM src). Measured: cc1
  output lh 2 / lhu 7 / sll16 7 == target counts; frame vars=40 == target.
- H3 KILLED (instance) - `a1[2] = sinB;` placed before the cosB/cosC Judge loads (f1): sandbox 83
  on the vars=40 chassis with no FAKE constructs. The store may-alias the Judge loads, so sched1
  cannot hoist the cosB/cosC loads above it.
- H4 CONFIRMED - Same body with the store after `cosB_cosC = cosB * cosC;` (f3): sandbox 0,
  build_insns 122 == target 122, verify-oracle ok. Body is in src/text1a_c.c and candidate.c.
