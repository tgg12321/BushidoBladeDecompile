# Hypothesis ledger — func_80038658

## [s1] 2026-08-20 (recon) — function matched; ledger closed

- H1 "The old chassis' shared-accumulator + register pins is the right
  structure and only the pins need a legit respelling" — **KILLED**: de-pinned
  accumulator forms measure 9–27; the accumulator pseudo structurally
  conflicts with $v0 in .greg. No spelling of a single shared accumulator was
  found that allocates to $v0.
- H2 "Target is the fully natural per-arm switch; the $v0 pattern is
  first-free allocation of independent temps and the shared stores are
  cross-jump/label artifacts" — **CONFIRMED** (sandbox 0): per-arm direct
  stores + `if (ret == 1)` sense + arm-internal `fail = N; goto fail_store;`
  + end-of-function `fail_store:` label reproduce all 55 bytes.
- Frontier: none — sandbox distance 0 this session, candidate applied in
  `src/code6cac_c_mid.c`, awaiting driver byte re-verify + layer-1 + Judge.
