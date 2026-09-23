# `func_8005763C` grind ledger

- Queue top after two documented rotations on 2026-09-22.
- Canonical verdict: C, hand-coded tier LOW, 276-instruction INCLUDE_ASM floor.
- Identity: `calc_line_seg_intersect`; fixed-point line-segment intersection with
  axis-aligned fast paths and +/-50 endpoint tolerance.
- v1: semantic cleanup of the m2c reconstruction scored 125/276.
- v2: raw m2c declaration/expression order scored 124/276.
- v3: mutating coordinate parameters in place exposed the original register
  allocation shape and dropped the honest floor to 45/274.
- One-output-pointer alias ablation: a local alias for `arg8` reduced the score to
  27, but the closing form proved the alias unnecessary and removes it.
- `cc1psx-check` on the score-45 and score-27 forms showed fidelity leads
  (38 and 19 respectively), so the function was not rotated. Splitting
  `x1 = arg0 >> 3` into assignment then shift removed that divergence and dropped
  the project compiler to score 10 while cc1psx remained at 19 (SOURCE-SIDE).
- Random permuter pass: 1,313 iterations across four workers; no closing result.
  Its best useful mutation independently identified the split `x1` assignment.
- Final control-flow rederivation places the zero-return block before the success
  scaling block, matching both semantics and the target branch layout. Direct use
  of `arg8` ablated the pointer alias and reduced score 23 to 1. Expressing the
  vertical first-segment rejection as an explicit early zero return closed the
  last delay-slot difference.
- Final candidate: honest C, 276/276 instructions, sandbox `--disable all` score 0,
  no inline asm, volatile, register pinning, duplicate object handles, or dead
  match-only state. Endpoint alias and unreachable-return ablations both retained
  score 0.
- Fresh adversarial review rejected the first score-zero form because two gotos
  entered the interiors of branch bodies. A whole-cluster structured ablation
  replaced those shared-label ladders with four ordinary guarded cases. It also
  scores 0/276 with `--disable all`, proving that the interior-entry constructs
  were unnecessary; the structured form is the final candidate.
- Final gates: integrated `--disable all` sandbox score 0/276; clean oracle
  rebuild SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`; fresh adversarial
  re-review PASS on the revised body; completion-integrity and new-cheat audits
  PASS; `queue done` recorded the function as COMPLETED-C.
