---
name: split-read-defeats-hoist
paths: [".claude/rules/split-read-defeats-hoist.md"]
description: "Shared post-branch read through a flag-selected base gets its offsets hoisted across a switch (register-rename plateau): duplicate the read into each flag arm, indexing the known symbol directly."
metadata:
  type: reference
---

# Duplicate a read into the branch arms to stop offset hoisting

## Symptom

A function selects a base from a flag, does ONE shared read through it, then dispatches on a switch:

```c
if (flag & 1) base = GLOBAL_A; else base = (u16 *)GLOBAL_B;
t = ((base[arg_hi] & 0xFF) << 16) | base[arg_lo];
switch (mode) { /* cases index through base[...] */ }
```

GCC hoists the `arg_hi`/`arg_lo` address computations across the switch and the grind plateaus on pure
REGISTER-RENAME diffs (0 structural) with "cluster coupling" (fixing A breaks B).

## Fix — the read in each arm, known symbol indexed directly

```c
if (flag & 1) {
    t = ((GLOBAL_A[arg_hi] & 0xFF) << 16) | GLOBAL_A[arg_lo];   /* direct symbol */
} else {
    base = (u16 *)GLOBAL_B;
    t = ((base[arg_hi] & 0xFF) << 16) | base[arg_lo];
}
switch (mode) { /* cases index arg_lo / arg_hi directly */ }
```

Duplicating the read pins the offset computations inside their branch; indexing the symbol directly makes the
case base materialize as `lui %hi(GLOBAL_A)` in the target's register. This is the frozen-list SOTN-accepted
"duplicate-read into branch arms" entry ([[no-new-park-categories]]; SOTN `color_fake = *palette;` rebinds,
`src/dra/42398.c`). Marking the flag globals `volatile` to force re-reads is FORBIDDEN
([[legitimate-volatile-interrupt-touched]]).

## Meta-lesson

A plateau of pure register renames with cluster coupling is a STRUCTURAL signal: the permuter and micro-levers
optimize allocation within a fixed control-flow shape and cannot find a different one. Stop grinding; change how
the computation is split across control flow.

## Related

[[register-asm-pins]] (why pins don't stick) · [[duplicated-statement-into-arms]] ·
[[hoist-shared-arm-computation-defeats-copy-pref]] (the inverse lever)
