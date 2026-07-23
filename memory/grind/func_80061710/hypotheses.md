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
- **status:** CONFIRMED on 710 (grind s2). V7 mask-atomic-first = sandbox 6;
  middle-load block-local split = sandbox 7 (both pin-free, 0 rules), exactly the
  sibling predictions. Best sanctioned floor for 710 = 6 (improved from the
  9 the WIP/recon recorded). V7 residual diffed: the 6 is PURELY the schedule
  difference (mask leading-block vs target's interleave); every interleaved
  (target-schedule) form sits at the v0<->v1 wall (9) because local-alloc gives
  the longer-lived interleaved mask the lower reg. STRUCTURAL AXIS MEASURED DEAD:
  no structural form reaches committable 0.
- **next_probe:** none grindable in structural modality — the only 0-basin is the
  constant-staging copy-preference (H1), owner-gated under 658's census. An
  ESCALATION-modality session files owner-gated citing 658's escalation.

### H3 — No clean permuter basin (deny-copy-source proven dead on 658 AND NOW ON 710)
- **mechanism:** 658 s4b removed the reused `val` source (30,762 iters) → NO
  score-0. The score-0 basin is uniquely the constant-staging copy; find_free_reg
  has no lever to move mask off $v0 except a copy-suggestion, and the constant has
  no natural copy source.
- **status:** CONFIRMED NATIVELY ON 710 (grind s4, permuter modality). Campaign 1
  (pin-free floor-9 chassis, ~4574 iters) re-found only the val-staging basin —
  4× score-0, all the H1 constant-staging coercion (rejected/permuter-constant-
  staging-cheat-s4.c). Campaign 2 (deny-copy-source: `val` block-scoped so it is
  not a live copy source, ~19k iters/~23 min) plateaued at best 50 with ZERO
  score-0, exactly replicating 658 s4b. The score-0 basin is UNIQUELY the
  constant-staging copy-preference; no committable permuter form exists on 710.
- **next_probe:** none — permuter axis measured dead natively; do NOT re-run.
  Disposition is the owner SOTN census under 658's escalation (names 710).

## Rejected forms bank
- middle-load-block-local-split-floor7.c (s2)
- v7-mask-atomic-first-floor6.c (s2, best sanctioned)
- mask-position-permutation-sweep-floor9.c (s3 — forms C/A/E, all 9)
(Historical WIP-ruled-out forms listed in evidence.md.)

## [s3] Some UNTRIED mask-position permutation reaches a committable floor below 6 (or 0) without the coercion.
- mechanism: If an interleave position other than V7's leading-block let mask land in $v1 while loads stay $v0, a sanctioned interleaved form could beat 6. Tested the three positions not in the s2 bank: mask split across load2 [C], mask atomic after load1 [A], compute-early/store-late [E].
- probe: Each form applied pin-free; sandbox --disable all (tmp/grind/func_80061710/s3/structural_permutation_measurements.txt).
- result: ALL THREE = sandbox 9. Only mask-dies-before-load1 (V7) reaches sub-9 (=6). Every position at/after load1 leaves mask live-across a load -> local-alloc gives it $v0 -> loads to $v1 -> full v0<->v1 rename = 9.
- verdict: KILLED. Mask-position permutation space exhaustively mapped; structural axis dead re-confirmed independently on 710.

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

## [s2] V7 mask-atomic-first is the best SANCTIONED (non-coercion, 0-rule) structural floor for 710 at ~6.
- mechanism: Hoisting the mask store before the 3 loads makes the mask pseudo die before load 1, so mask+all loads reuse $v0 (register conflict freed). But this emits mask as a leading block instead of target's interleave, leaving a pure schedule residual.
- probe: Applied V7 (pin-free, mask=0x10FF10; D_800A3464=mask; before the 3 arg0[] loads); sandbox --disable all.
- result: sandbox 6 (46/46, 0 rules) — exactly the sibling-611A4 s3 prediction. Middle-load block-local split = sandbox 7 (matching sibling-658 s2). V7 residual objdump vs asm/text1b.s confirmed the 6 is purely schedule (mask leading-block vs interleaved).
- verdict: CONFIRMED

## [s2] Some pure structural form reaches committable sandbox 0 on 710 without the constant-staging coercion.
- mechanism: Would require target's interleaved mask schedule AND mask allocated to $v1; but an interleaved mask is live across the loads, and GCC 2.7.2 local-alloc gives the longer-lived pseudo the lower-numbered reg (mask->$v0), forcing the loads to $v1 = the v0<->v1 rename wall at 9.
- probe: V7 (6) and middle-load split (7) measured; all interleaved (target-schedule) positions proven at 9 on siblings; the only 0-basin (constant-staging copy-preference val=0x10FF10; mask=val;) is the policy-blocked coercion. No natural copy source exists (658 s4b deny-copy-source, 30,762 iters, no score-0).
- result: No sanctioned structural form scored below 6 or reached 0. Interleave-vs-register tension is architectural.
- verdict: KILLED

## [s3] Some untried mask-position permutation reaches a committable floor below 6 (or 0) without the constant-staging coercion.
- mechanism: If an interleave position other than V7's leading-block let the mask pseudo land in $v1 while the 3-load web stays $v0, a sanctioned interleaved form could beat 6. Tested the three positions absent from the s2 rejected bank / WIP list: mask split across load2 [C], mask atomic after load1 [A], compute-early/store-late [E].
- probe: Each form applied pin-free to src/text1b.c; sandbox --disable all. See tmp/grind/func_80061710/s3/structural_permutation_measurements.txt.
- result: All three = sandbox 9 (46/46, 0 rules). Only mask-dies-before-load1 (V7) reaches sub-9 (=6). Every position at/after load1 leaves the interleaved mask live-across a load -> local-alloc.c:472 gives the longer-lived pseudo the lower reg ($v0) -> loads pushed to $v1 -> full v0<->v1 rename = 9.
- verdict: KILLED

## [s4] A committable (non-cheat) score-0 permuter basin exists for func_80061710 (native measurement, not inherited from siblings).
- mechanism: If find_free_reg could be steered to put the reused 3-load web in $v0 and the 0x10FF10 mask in $v1 without the constant-staging copy-preference, some pure-C structural mutation would reach score 0. Directed permuter over the full mutation space would find it if it exists.
- probe: Campaign 1 pinfree-floor9 chassis (base_score 50, ~4574 iters): base pin-free reproduces the exact 9-line v0<->v1 tail residual (46==46 insns). tmp/perm_710_s4.
- result: 4x score-0 finds (output-0-1..0-4, first at ~635 iters), EVERY one the constant-staging coercion (`mask=(val=0x10FF10);` / `val=0x10FF10;...mask=val;`) - identical to the func_80061658 Judge-FAILed construct. No non-cheat score-0.
- verdict: KILLED

## [s4] The constant-staging copy-preference is the UNIQUE score-0 basin on 710; denying the reused-live-local copy source eliminates all score-0.
- mechanism: The score-0 layout requires local-alloc.c:472 copy-preference to move the mask off $v0, which needs an existing live pseudo as copy source (the switch's `val`). A fresh temp introduced right before the tail has no prior live range -> same allocation as a direct assignment. Block-scoping `val` inside each switch case removes it as a live copy source.
- probe: Campaign 2 denycopy-blockscope-val chassis (base_score 125, ~19k iters / ~23 min, harvest --stop after >5 min no novel find): `val` block-scoped so not live across the tail. tmp/perm_710_s4b.
- result: Best score 50 (pure tail v0<->v1 residual, switch-region diff fixed); scores 125/95/75/50; ZERO score-0. Natively replicates sibling 658 s4b (30,762 iters, no score-0). With the copy source denied, no score-0 form exists.
- verdict: CONFIRMED
