---
name: defer-store-past-later-compute-into-jal-delay
paths: [".claude/rules/defer-store-past-later-compute-into-jal-delay.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "A regfix `reorder` over a pre-call store cluster retires by HOISTING the stored value into a local and moving the `GLOBAL = val;` store statement AFTER a later-computed value — GCC then schedules the sw into the following jal's delay slot, matching target. Retired the AllocRobRmd reorder @88-102."
metadata:
  type: reference
---

# Defer a global store past a later compute → GCC drops it into the jal delay slot

## Symptom

A function carries a regfix `reorder N..M @ a-b` over a cluster around a call,
and the masked `sandbox --disable all` distance is a handful (e.g. 12). An
index-aligned objdump diff shows the ONLY real difference is the **position of a
single `sw` (store to a global/struct field)**: your build emits it EARLY (right
after the value is computed) while the target defers it — typically into the
**delay slot of a following `jal`** (`jal foo; sw $vN,disp($base)`), or just after
a later-computed value.

The source computes-then-stores immediately, then computes more things, then calls:

```c
s32 *texA = base + (rmd[1] >> 2 << 2);
GLOBAL_FIELD = base + (rmd[3] >> 2 << 2);   /* store scheduled EARLY by GCC */
s32 *texB    = base + (rmd[4] >> 2 << 2);
foo(texA, ...);   /* target stores GLOBAL_FIELD in THIS jal's delay slot */
foo(texB, ...);
```

## Fix — hoist the value to a local, move the store statement later

Split the assignment into (1) compute the value into a fresh local and (2) the
store, and place the **store** AFTER the later computation (texB here):

```c
s32 *texA = base + (rmd[1] >> 2 << 2);
s32  fld  = base + (rmd[3] >> 2 << 2);   /* value computed here */
s32 *texB = base + (rmd[4] >> 2 << 2);
GLOBAL_FIELD = fld;                       /* store deferred past texB */
foo(texA, ...);                           /* GCC now schedules the sw into the delay slot */
foo(texB, ...);
```

GCC's scheduler keeps the store's value live in a register (no extra spill — the
value already had to live in a reg) and floats the `sw` down to the `jal`'s delay
slot, exactly where the target has it. This is ordinary source restructuring (a
human computing all the offsets up front, then storing, then calling, would write
this) — NOT a coercion device. Net anti-cheat: the regfix `reorder` is retired.

## Confirmed case — AllocRobRmd (text1a.c, 2026-06-14)

`reorder 90,89,88,97,94,95,98,91,92,93,99,100,102,101,96 @ 88-102` over the
texA/`a0[10]=...`/texB cluster before two `func_80044010` calls. The source wrote
`a0[10] = (s32)rmd + (rmd[3]>>2<<2);` BEFORE computing texB, so GCC emitted
`sw $v0,40($s2)` early; target defers it into the second call's delay slot.
Hoisting to `s32 fld10 = ...;` and moving `a0[10] = fld10;` after the texB
computation dropped `sandbox --disable all` 12 → 1 (the residual 1 was an
unrelated maspsx `.L`-label load-delay nop, retired via [[maspsx-label-nop-gate]]).
`retire` then dropped all 4 of the function's rules; SHA1 == oracle, COMPLETED-C.

## When it applies / doesn't

- Applies when the diff is a single misplaced **store** around a call and the
  function carries a `reorder` rule over that region. The lever is moving the
  store statement DOWN in source past an independent later compute.
- If the misplaced insn is a load or the value needs a callee-save register held
  across the call, see [[store-before-jal]] (the sibling "save arg into a
  callee-save so the store fits the delay slot" recipe).
- Don't manufacture a useless later compute just to defer the store — that's a
  coercion. The later compute must already exist (texB here).

## Related
- [[store-before-jal]] — sibling delay-slot store lever (callee-save variant).
- [[maspsx-label-nop-gate]] — the other half of the AllocRobRmd retirement.
- [[walking-pointer-serializes-parallel-loads]] — another "source statement
  structure drives scheduling" lever.
