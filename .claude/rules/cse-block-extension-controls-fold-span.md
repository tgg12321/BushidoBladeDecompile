---
name: cse-block-extension-controls-fold-span
description: "CSE merges values across a join the target keeps separate (or vice versa): cse1's block extends past singly-used labels (cse.c:8102-8184). Free escape: a real if/else whose arm ends in jump+BARRIER."
paths: [".claude/rules/cse-block-extension-controls-fold-span.md"]
metadata:
  type: reference
---

# CSE folds across join labels — `cse_end_of_basic_block`'s extension test

## Symptom (two shapes, one mechanism)

- **Too FEW instructions:** the target rematerializes an address / reloads a value in each arm; your
  build computes it once and forwards it (`.cse` dump shows the per-arm pseudos merged, per-arm
  reloads `DELETED`).
- **Too MANY instructions:** a value the target folds stays separate because your branch shape ended
  the cse block early.

The knob is **where cse1's basic block ends**. A join label does NOT end it.

## Mechanism (`tools/gcc-2.7.2/cse.c:8102-8184`)

`cse_end_of_basic_block` extends the block past a conditional branch whenever
`LABEL_NUSES (JUMP_LABEL (p)) == 1`. Under a target-shaped layout both branches are followed
(`Processing block from 2 to 94`). The only escapes:

- **(a)** `LABEL_NUSES != 1` (e.g. an `&&` short-circuit boundary);
- **(b)** the backward scan (8109-8114) hits a `CODE_LABEL` with `LABEL_NUSES != 0` just before the
  join (skips NOTEs and NUSES-0 labels; breaks on LOOP_END/SETJMP notes);
- **(c)** `no_labels_between_p` (8168-8172) finds a `CODE_LABEL` between the branch and the arm end.

## The free escape — a jump+BARRIER-terminated arm

(a)/(b)/(c) usually cost an extra jump/branch or an unreferenced label (the forbidden dead-goto label
pad). The ordinary-C exception: spell the conditional as a real **`if/else`** instead of
"initialise to a default, then conditionally overwrite". An `else` arm ends in an unconditional jump
+ BARRIER, and the fold stops at the join.

Example (func_8003B9D0 region B, a 7-insn shortfall closed): `x = -1; if (c) x = v;` →
`if (c) x = v; else x = -1;`.

## Measured dead (do not re-probe)

- `&&` boundaries (escape a): net-negative at every count (+2/+13/+15 insns).
- `thread_jumps` cannot supply a second `LABEL_REF` when join labels precede different code.
- Carrier copies (same-region SImode copy) are reverted by cse1's `canon_reg` before combine
  ([[param-reuse-base-copy-cse-canon]]).

## Check which pass owns the fold

`cc1 ... -da` → `<file>.i.cse` / `.i.combine`. `Processing block from N to M` tells you whether the
block extended past your join; whether the insn is `DELETED` in `.cse` (cse's fold) or only in
`.combine` (e.g. `expand_compound_operation -> nonzero_bits`) decides the lever — a cse-shaped lever
cannot move a combine fold.

Related: [[exit-path-return-set-cse-join]] · [[split-read-defeats-hoist]] ·
[[defeat-combine-symbol-fold]] · [[duplicated-statement-into-arms]]
