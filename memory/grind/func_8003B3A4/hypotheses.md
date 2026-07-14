# Hypothesis ledger — func_8003B3A4

- [s2 2026-07-13] H: judge-prescribed direct conditional write matches. Probe: sandbox. Result: distance 6. **KILLED** (rejected/direct-conditional-write-at-macro.c).
- [s2 2026-07-13] H: ternary single store matches. Probe: sandbox. Result: distance 6. **KILLED** (rejected/ternary-single-store.c).
- [s2 2026-07-13] H: value-diamond-into-local then direct store matches. Probe: sandbox. Result: distance 8 (worse). **KILLED** (rejected/diamond-local-then-store.c).
- [s2 2026-07-13] H: constant-offset store (&D_8010277C)[1] escapes the symbolic fold. Probe: sandbox. Result: distance 6 (folds back). **KILLED** (rejected/offset-neighbor-const-fold.c).
- [s2 2026-07-13] H: the pointer-variable alias is the ONLY C form producing the pre-branch $v1 address + register-indirect sb. Probe: the four kills above + s1 asm analysis. **CONFIRMED** — annotated /* FAKE */ alias re-measures 0; disposition awaits ruling (pointer-alias-fake-exception vs binding regression diagnosis).
