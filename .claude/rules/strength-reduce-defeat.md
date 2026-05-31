---
name: strength-reduce-defeat
paths: ["src/*.c"]
description: "ARCHIVED FORBIDDEN — `__asm__ volatile (\"negu %0, %1\" : \"=r\"(t3) : \"r\"(t3))` to defeat GCC's mathematical simplification of `-((-t)<<N) ≡ t<<N` is general-purpose-opcode inline asm. The inline-asm-policy table explicitly lists `negu` as a forbidden cheat opcode. Score-inert under the cheat-invisible sandbox."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — strength-reduce-defeat is forbidden

This file used to teach a recipe for reproducing target's
sign-preserving `bgez+negu+sll+b+negu` strength-reduction-defeat
pattern via:

```c
if (t3 < 0) {
    __asm__ volatile ("negu %0, %1" : "=r"(t3) : "r"(t3));
    t3 = t3 << 3;
    __asm__ volatile ("negu %0, %1" : "=r"(t3) : "r"(t3));
} else {
    t3 = t3 << 3;
}
```

The `negu` opcode IS explicitly named in the [[inline-asm-policy]]
forbidden category:

> **cheat** — Inline `__asm__` or `register T x asm("$N")` pin used to
> steer GCC's allocator or scheduler. General-purpose opcodes
> (`move`, `addu`, `nop`, `lui`, `negu`, etc.) that have C equivalents
> but we wrote them in asm to force matching.

Wrapping a unary negate in `__asm__` purely to make it opaque to GCC's
combine/simplify passes is steering codegen via inline asm — the
canonical cheat shape.

## What this means for the affected functions

Functions that used this recipe — `func_8007EA0C` per the original
reference, and any display.c sibling with the pre-mvmva sign-split
pattern — carry **cheat-asm debt**. The cluster overlap with
[[gte-3x3]] (also archived) is not a coincidence: both recipes were
the same project's mid-2026-05 push to match the display.c GTE wrapper
family via inline-asm techniques that the 2026-05-31 catalog
expansion forbids in retrospect.

The honest paths forward for these functions:

- **Pure C structure.** GCC folds `-((-t) << N)` to `t << N` because
  it sees the equivalence. The lever is finding C in which the two
  expressions do **not** look equivalent to GCC's combine pass — e.g.
  by routing one of them through a function call boundary, a
  loop-induction variable, or a value-dependency edge combine can't
  see across. Has not been searched exhaustively for this cluster.
- **Canonical-asm authorization.** If the target shows strong
  hand-coded signatures (per [[hand-coded-asm-recognition]] /
  [[packed-multiply-cluster]]'s S8 signal / disassembler "handwritten
  instruction" annotations), retire as `COMPLETED-INLINE-ASM-CANONICAL`
  via `inline_asm_canonical.txt`. The display.c GTE wrapper cluster
  is the canonical example of this routing.

## Historical content

The original recipe text is preserved in git history:
`git show HEAD~1:.claude/rules/strength-reduce-defeat.md`

## Related
- [[inline-asm-policy]] — current expanded cheat catalog
- [[hand-coded-asm-recognition]] — the gate to classify a cluster
  function as canonical-asm vs pure-C-reachable
- [[canonical-asm-retirement]] — the legitimate end state
- [[packed-multiply-cluster]] — sibling display.c cluster, scan
  -hand-coded S8 routes it to canonical-asm
- [[difficult-is-not-impossible]] — anti-quit discipline for the
  pure-C path
