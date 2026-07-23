# Hypothesis ledger — func_80061710

## Live frontier (after grind s1 recon)

### H1 — Cluster resolution is owner-gated on the constant-staging SOTN census (DOMINANT)
- **mechanism:** 710's ONLY pure-C sandbox-0 form is `val = 0x10FF10; mask = val;`
  (constant-staging through a reused live local to steer local-alloc.c:472
  copy-preference / find_free_reg off $v0 for the mask). Straight-line register-
  CHOICE coercion, outside every sanctioned family; Judge-FAILed for sibling 658.
- **status:** CONFIRMED that 710 reaches sandbox 0 via this form (s1). The
  disposition is a POLICY ruling reserved to the owner — func_80061658's
  OWNER-ESCALATION (2026-07-23) puts exactly this census question and names 710
  as a cluster member it resolves. A future ESCALATION-modality session should
  file/point owner-gated for 710 citing the 658 census (or the driver parks the
  cluster together once the owner rules).
- **next_probe:** none grindable — awaits owner SOTN-master-branch census.

### H2 — Best SANCTIONED (committable) floor for 710 is ~6–7, never 0
- **mechanism:** block-local split of the middle load → floor 7 (658 s2);
  mask-atomic-first V7 (mask dies before load 1, both reuse $v0) → floor 6
  (611A4 s3). Neither reaches a committable pure-C 0 — every interleaved mask
  position sits at the v0<->v1 wall (9).
- **status:** UNMEASURED on 710 specifically but PROVEN on both siblings (same
  tail cadence, same residual). If a structural-modality session runs, confirm
  the 6/7 sanctioned floor here; do NOT expect it to reach 0.
- **next_probe (if structural modality assigned):** apply mask-atomic-first (V7)
  and middle-load block-local split; expect 6 and 7 respectively.

### H3 — No clean permuter basin (deny-copy-source proven dead on 658)
- **mechanism:** 658 s4b removed the reused `val` source (30,762 iters) → NO
  score-0. The score-0 basin is uniquely the constant-staging copy; find_free_reg
  has no lever to move mask off $v0 except a copy-suggestion, and the constant has
  no natural copy source.
- **status:** KILLED at the family level. A permuter-modality session on 710 will
  re-find only the val-staging basin (the H1 coercion) — no committable output.
- **next_probe:** none worth spending; would duplicate 658 s4/s4b.

## Rejected forms bank
(none NEW this session — the one form tested reached 0. Historical WIP-ruled-out
forms are listed in evidence.md.)

## [s1] func_80061710 is a reachability wall like sibling func_800611A4 (no pure-C-0 form exists).
- mechanism: The t=$2/mask=$3 cluster's v0<->v1 residual (local-alloc.c:472 death-count routing) has no committable pure-C form on 611A4 across 8 sessions.
- probe: Pin-free constant-staging form val=0x10FF10; mask=val; sandboxed --disable all.
- result: sandbox score 0 (46/46, 0 rules) — a pure-C-0 form DOES exist for 710.
- verdict: KILLED

## [s1] func_80061710 is the func_80061658 subfamily: reaches sandbox 0 only via the constant-staging coercion the Judge already FAILed.
- mechanism: val=C; mask=val; stages the mask constant through the reused switch local to steer local-alloc.c:472 copy-preference off $v0 for the mask — straight-line register-CHOICE coercion, no semantic purpose, outside every sanctioned family.
- probe: Same probe: constant-staging pin-free form sandboxed; compared to 658's Judge-FAILed construct and 658 s4b deny-copy-source result (uniquely this basin).
- result: sandbox 0 via the identical construct; no committable (non-coercion) sub-9 form exists (proven exhausted on both siblings).
- verdict: CONFIRMED
