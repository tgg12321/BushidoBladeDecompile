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

## H-C (param types) — KILLED (s2)
m2c infers (s16 arg0, s16 arg1, u32 *arg2). Tried: score 10->43, build 107->112.
s16 params inject sign-extends on every a0/a1 use; target has ZERO (params flow
straight into s2/s5). Original params are register-width s32. Dead axis.

## A+B COUPLING (frontier item 3) — KILLED (s2)
Hypothesis: correcting types / fixing one gap shifts RA for both. FALSIFIED:
applied the arm-split (Gap A materialized, build 108) and disassembled the tail
— byte-identical to HEAD's tail (base=s1, move v0,s2). Gap A's presence does not
touch Gap B. The gaps are INDEPENDENT; each needs its own solution.

## [s2] Gap A is a cse deletion, Gap B is a local_alloc live-range split — both nailed at RTL, clean structural levers measured dead
- Gap A: else recompute = RTL insn 74 `(set reg75 (plus reg72 3))`; cse deletes
  it (reg75 already holds reg72+3, reg72=a0 preserved across else). Pre-if init
  (sched anchor) is what makes cse fold => catch-22. No value-neutral spelling
  survives (cse folds all a0+3 spellings pre-availability-check).
- Gap B: fork truncates a0->HI for the `s1[11]=a0+3` store then cse reuses that
  (HI)a0 for the 3 a0 stores (base stays s1). Target adds 3 to full s2, stores
  s2 directly, copies base to v0. The v0 base copy is a local_alloc live-range
  split of the 2-pred join block; cse copy-propagates any C-level `p=s1`.
- verdict: KILLED (structural modality's clean levers exhausted; NOT proven
  impossible — see frontier for the remaining non-structural avenues).

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

## [s2] Narrowing the signature to m2c's inferred types (s16 a0, s16 a1, u32 *a2) shifts RA to close the gaps.
- mechanism: narrower param types change value-tracking and RA pressure.
- probe: Changed signature + cast a2 uses; sandbox --disable all.
- result: score 10->43, build_insns 107->112. s16 params inject sign-extends (sll/sra) on every a0/a1 use; target has ZERO sign-extends (a0/a1 flow straight into s2/s5). Original params are register-width s32.
- verdict: KILLED

## [s2] Gap A and Gap B are coupled: materializing the else recompute (Gap A) shifts RA and also fixes the tail base copy (Gap B).
- mechanism: frontier item 3 premise: one structural change alters allocation for both.
- probe: Applied arm-split (Gap A materialized, build 108) and disassembled the tail.
- result: Tail is BYTE-IDENTICAL to HEAD's tail (base=s1, move v0,s2, sh v0/s5 stores). Gap A presence does not touch Gap B. The two gaps are fully independent.
- verdict: KILLED

## [s2] Gap B tail base copy (addu v0,s1,zero) can be materialized by keeping s1 the record pseudo while a separate C-level tail base pointer survives copy-prop.
- mechanism: local-alloc gives a short-lived tail-base pseudo a caller-save; a kept-live spelling forces the copy (param-reuse-base-copy-cse-canon).
- probe: s16 *p=s1 for the 5 HI stores + s1 referenced in the trailing word store (mixed, both live); also SI-temp s32 t=a0+3;s1[11]=t.
- result: Mixed p/s1: score 10, build 107 (cse copy-propagates p=s1 regardless; copy vanishes, == WIP single-set result). SI-temp: build 106 (drops the (HI)a0 truncation) but score stays 10. Target's base copy is a local_alloc live-range split of the 2-pred join block, not a C-level pointer copy.
- verdict: KILLED

## [s2] Gap A: our fork drops the else recompute `addiu s3,s2,3` (folded), leaving build 1 insn short (107 vs 108).
- mechanism: cse deletes RTL insn 74 (set reg75 (plus reg72 3)) because reg75 already holds reg72+3 (reg72=a0 preserved across the else calls, reg75 untouched); the pre-if init that anti-dep-anchors the redefinition for sched is exactly what feeds cse the available expression.
- probe: cc1 -da dumps: insn 74 present in dump.i.rtl, DELETED in dump.i.cse (insn 71 links straight to code_label 76). Confirmed no value-neutral spelling escapes (cse constant-folds a0-(-3), (a0+1)+2, etc. to (plus a0 3) before the availability check).
- result: Root cause confirmed. Structural spellings that keep the pre-if init all fold; dropping it (arm-split) materializes the insn but sched1 hoists it 3 slots early (score 11). Catch-22 between cse-fold and sched-anchor stands.
- verdict: CONFIRMED
