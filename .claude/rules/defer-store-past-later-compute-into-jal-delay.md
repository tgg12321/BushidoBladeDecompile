---
name: defer-store-past-later-compute-into-jal-delay
paths: [".claude/rules/defer-store-past-later-compute-into-jal-delay.md"]
description: "A single sw emitted early that target puts in a later jal's delay slot: hoist the value into a local and move the store statement AFTER an existing later compute."
metadata:
  type: reference
---

# Defer a global store past a later compute → GCC drops it into the jal delay slot

## Symptom

An index-aligned diff where the ONLY real difference is the position of one `sw`: your build emits
it right after the value is computed; the target defers it, typically into a following `jal`'s
delay slot (`jal foo; sw $vN,disp($base)`). Source computes-then-stores immediately, then computes
more, then calls:

```c
s32 *texA = base + (rmd[1] >> 2 << 2);
GLOBAL_FIELD = base + (rmd[3] >> 2 << 2);   /* scheduled EARLY */
s32 *texB    = base + (rmd[4] >> 2 << 2);
foo(texA, ...);
foo(texB, ...);
```

## Fix — compute into a local, store later

```c
s32 *texA = base + (rmd[1] >> 2 << 2);
s32  fld  = base + (rmd[3] >> 2 << 2);
s32 *texB = base + (rmd[4] >> 2 << 2);
GLOBAL_FIELD = fld;                       /* store deferred past texB */
foo(texA, ...);                           /* sw lands in the delay slot */
foo(texB, ...);
```

Ordinary restructuring (compute offsets, store, call). Example: AllocRobRmd (now func_80040594, main/309CC.c) — honest
distance 12 → 1.

## Applies when / not

- Applies: a single misplaced **store** around a call; move the store statement down past an
  independent later compute.
- If the value must be held in a callee-save across the call, see [[store-before-jal]].
- Don't manufacture a useless later compute to defer the store — the later compute must already
  exist.

Related: [[store-before-jal]] · [[walking-pointer-serializes-parallel-loads]]
