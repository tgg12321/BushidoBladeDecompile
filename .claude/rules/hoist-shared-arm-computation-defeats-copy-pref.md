---
name: hoist-shared-arm-computation-defeats-copy-pref
description: "Both if/else arms compute the same `z = x + y` and the residual is its register ($a0 vs $v1): hoist it out of the arms (jump2 re-duplicates it). Includes the hard-reg preference feasibility test."
paths: [".claude/rules/hoist-shared-arm-computation-defeats-copy-pref.md"]
metadata:
  type: reference
---

# Hoist a shared computation out of two branches — break copy-preference propagation

## Symptom

```c
if (cond) {
    x = func(y);             /* y gets an $a0 copy-pref */
    if (x == -1) return -1;
    z = x + y;
} else {
    x = k;
    z = x + y;
}
if (z > BOUND) { ... }
```

Small distance; target computes the sum per-arm (e.g. `addu $v1,$a3,$s0` in a delay slot + a copy at
the join), yours matches structurally but the sum lands in `$a0` instead of `$v1`.

## Mechanism

Each `z = x + y` is its own pseudo with `REG_DEAD y`; `global.c:expand_preferences` merges `y`'s
`$a0` preference into the sum (no conflict, y dies at the add) and `find_reg` honours it (MIPS has
no `REG_ALLOC_ORDER`). Measured dead: split-init (`sum = X; sum += Y;`, refolded), declaration
position, a do-while(0) wrap (blocks the cross-jump merge).

## The fix

```c
if (cond) {
    x = func(y);
    if (x == -1) return -1;
} else {
    x = k;
}
z = x + y;                 /* one expression, one pseudo */
if (z > BOUND) { ... }
```

One pseudo, no inherited preference; jump2 duplicates the simple assignment back into the arms at
codegen, so the bytes keep the per-arm layout. Plain DRY refactoring — no FAKE annotation needed
(the inverse of [[duplicated-statement-into-arms]]). Owner-adjudicated as a sanctioned pure-C lever
(saTan2Main = SsVabOpenHeadWithMode, 5 → 0).

Applies when the residual is register choice on a value fed by an argument chain, the expression is
duplicated in both arms, and the hoist is byte-neutral elsewhere. Does NOT apply when the arms'
expressions differ, have per-arm side effects, combine already unified them (score doesn't move), or
other tells show the original truly duplicated.

## Feasibility test — can this pseudo hold that preference at all?

Check before writing C (the instrumented cc1 is `tools/gcc-2.7.2/cc1`, not `build/cc1`;
`BB2_FINDREG_DEBUG`):

1. **A hard-reg preference is created ONLY by a reg<->hard-reg COPY insn** (`set_preference`,
   global.c:1671, walks only the first operand of an expression source). A pseudo defined by `sll`
   and used by `addu` has no preference; if the wanted register has no ABI anchor (argument, return
   value, copy) in the function, the preference cannot be created.
2. **A pseudo live from function entry conflicts with the argument registers and cannot KEEP a
   preference for one** — `prune_preferences` (global.c:896-907) strips registers the allocno
   conflicts with.

Also: `find_reg` (global.c:952 ff.) pass 0 excludes `regs_someone_prefers[allocno]` (built from
lower-priority conflicting allocnos' preferences — an indirect lever only via a REAL
preference-carrying pseudo), and after the scan overrides `best_reg` with a free preferred register
(global.c:1057-1080). `expand_preferences` (global.c:798-841) runs before pruning across any
`single_set` with a `REG_DEAD` note. Allocno priority: `floor_log2(n_refs) * n_refs / live_length *
10000 * size` — cutting refs can RAISE priority by shrinking live_length; compute before editing.

Related: [[register-alloc-pure-c]] · [[split-read-defeats-hoist]] · [[compare-operand-order-register]] ·
[[local-alloc-death-count-class-wall]]
