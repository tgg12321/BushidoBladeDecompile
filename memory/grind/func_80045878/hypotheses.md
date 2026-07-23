# Hypothesis ledger — func_80045878

Floor: 10 (sandbox --disable all; build_insns 107 vs target 108). Verdict C.
Two coupled residual gaps (see evidence.md). m2c target shape:
tmp/grind/func_80045878/s1/m2c_target.c.

## H-A (Gap A — else recompute) — CONFIRMED mechanism, residual = scheduling
Statement: target recomputes `addiu s3,s2,3` (=a0+3) at the END of the else arm
(0x800458F8); our build folds it into the pre-if `s3=a0+3` (cse.c) => 107.
Probe s1: arm-split (set s3 in BOTH the taken arm and the else arm, drop the
pre-if init). RESULT: build_insns 107->108 (recompute MATERIALIZES) but score
10->11 — sched1 hoists the fresh single-def recompute 3 slots early (build 2c6c,
before `li v0,-1; sh; sh`) because s3 feeds the next call arg. Target keeps it
last. => Gap A insn is reachable; residual is ANCHORING it last.
Next probe: a form that keeps the recompute AND anchors it after the two stores
without a barrier — e.g. keep s3 live-in (redefinition anti-deps hold position)
while structurally defeating the cse fold; or reorder so the else def is not the
launch-priority winner. Read sched.c launch-priority + cse.c fold conditions.

## H-B (Gap B — tail base copy) — biggest chunk (~9 diffs)
Statement: target's final store block copies s1 into a caller-save base
(`addu v0,s1,zero`), stores through v0, uses s2 (=a0) DIRECTLY for the a0
stores, holds scratch (a0+3, 0x8000) in v1. Our build stores through s1 with an
extra `move v0,s2`. Single-set `s16 *p = s1` alias copy-props away (WIP-ruled).
Mechanism: local-alloc assigns a short-lived tail-base pseudo to caller-save v0;
need s1 kept as the long-lived record pseudo while a separate tail base
materializes and survives copy-prop.
Next probe: keep s1 referenced/live past the tail base copy, or multi-set the
base per param-reuse-base-copy-cse-canon; inspect .greg dump to see s1's pseudo
disposition. NOT yet attempted beyond the ruled-out single-set alias.

## H-C (param types) — untested, low priority
m2c infers (s16 arg0, s16 arg1, u32 *arg2); source declares all s32. Could shift
RA/scheduling for both gaps. Cheap to try; test after H-A/H-B if they stall.

## [s1] Target recomputes `addiu s3,s2,3` (=a0+3) at the end of the else arm; our build folds it into the pre-if s3 init (cse.c), leaving build 1 insn short (107 vs 108).
- mechanism: cse.c: pre-if `s3=a0+3` dominates the else; a0 and s3 (callee-save) are unchanged across the else calls, so the else `s3=a0-(-3)` is a redundant available expression and is folded.
- probe: Arm-split: drop the pre-if init, set s3=a0+3 in the taken arm and s3=a0-(-3) in the else arm; sandbox --disable all.
- result: build_insns 107->108 (recompute materializes) but score 10->11: sched1 hoists the fresh single-def recompute 3 slots early (build 2c6c, before li v0,-1; sh; sh) because s3 feeds the next call arg; target keeps it last (0x800458F8).
- verdict: CONFIRMED

## [s1] Target's final store block copies s1 into a caller-save base (addu v0,s1,zero), stores through v0, uses s2(=a0) directly, and holds scratch (a0+3, 0x8000) in v1; our build stores directly through s1 with an extra move v0,s2 (~9 of the 10 diffs).
- mechanism: local-alloc assigns a short-lived tail-base pseudo to caller-save v0; a single-set `s16 *p=s1` alias is copy-propagated away (s1 dead after) so no base copy materializes.
- probe: Not re-run this session (WIP already ruled out single-set alias at 107/floor 10); mapped only.
- result: Confirmed as the dominant residual chunk via objdump alignment of the tail store block against target.
- verdict: CONFIRMED
