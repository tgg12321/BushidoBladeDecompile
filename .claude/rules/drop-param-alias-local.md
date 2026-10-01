---
name: drop-param-alias-local
description: "A param->local alias (T *t0 = a0;) keeps $a0 busy so a short local misses it: drop the alias and use the param directly; the long chain promotes the param itself and frees $a0."
paths: [".claude/rules/drop-param-alias-local.md"]
metadata:
  type: reference
---

# Removing an explicit param→local alias frees the param register for reuse

## Symptom

```c
void func(u32 *a0, ...) {
    u32 *t0 = a0;        /* explicit alias */
    s32 size = 5;        /* target puts this in $a0; yours lands in $t1 */
    ...
    t0[N] = ...;
}
```

Small distance; body identical except one short local in a higher register, and the prologue order
swapped (`li size,5` first in yours vs `move t0,a0` first in target). Historically "held" by a pin
`register s32 size asm("a0")` — forbidden ([[register-asm-pins]]).

## Cause

The alias makes a separate pseudo; `size = 5` is scheduled first while param `$a0` is still live, so
it cannot take `$a0`.

## Fix — use the param directly

```c
void func(u32 *a0, ...) {
    s32 size = 5;
    ...
    a0[N] = ...;
}
```

`a0` now has a long live range; RA promotes it to `$t0` itself and schedules that move first (higher
INSN_PRIORITY from the long chain), `$a0` dies at insn 0 and `size` takes it. Example:
initLoadImage (gpu.c), 4 → 0, pin-free. This is the inverse of [[register-alloc-pure-c]] Lever A:
remove a local to lengthen a chain.

## Does NOT apply when

- the param is reassigned elsewhere (dropping the alias changes semantics);
- the other local is long-lived (priorities comparable);
- there is no second register conflict (the alias may be load-bearing for another reason).

Related: [[register-alloc-pure-c]] · [[register-asm-pins]] · [[pointer-alias-fake-exception]]
