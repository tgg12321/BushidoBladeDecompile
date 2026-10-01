---
name: explicit-rejection-set-defeats-range-fold
description: "u16 var range-excluded by `(u32)((s32)a - K) >= 2` lands in the wrong reg / loses target's andi: write `a != K && a != K+1`; combine recomposes the range check, liveness keeps it in its load reg."
paths: [".claude/rules/explicit-rejection-set-defeats-range-fold.md"]
metadata:
  type: reference
---

# Replace a 2-element subtract-range exclusion with explicit `!= K && != K+1`

## Symptom

A u16 loaded value tested in an `&&` chain with one `a != K0` and a subtract-range exclusion:

```c
u16 a1 = *(u16 *)(entry + 0x6A);
if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
    (u32)((s32)a1 - 0x17) >= 2 && *(s16 *)(entry + 0x96) == 0) { ... }
```

Small distance at the compare block: `lhu` lands in `$v1` (target `$a1`), target's
`andi $v1,$a1,0xFFFF` is missing (combine folded it), the range `addiu` uses the wrong register.

## The lever

```c
if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
    a1 != 0x17 && a1 != 0x18 && *(s16 *)(entry + 0x96) == 0) { ... }
```

combine recomposes `a != K && a != K+1` back into the same `addiu; sltiu 2; bnez` bytes, but `a1` is
now referenced at more compare points; the extended liveness keeps it in its load register and the
first compare materializes the target's `andi`. The explicit rejection set is the natural spelling
of the predicate (semantically identical), not a coercion.

Verify build_insns == target_insns after the substitution; if combine does not recompose (count
grows), the lever does not apply.

## Does NOT apply when

- the excluded values are non-adjacent (no range recomposition; extra `xori/beq`);
- the set has 3+ elements (combine may not recompose all);
- the chain's first compare is not on the same variable;
- the variable is not a halfword load (the andi mechanism is lhu-specific).

Example: func_8001EEB4 (code6cac.c) — 3 → 0 after [[hoist-call-arg-local-flips-jal-delay]]. Narrow
type casts ((s16), (unsigned short), (char)) that try to force the same effect are rejected coercions.

Related: [[register-alloc-pure-c]] · [[u16-global-lhu-lbu-low-byte]]
