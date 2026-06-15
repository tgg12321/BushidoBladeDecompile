---
name: cache-subtrahend-load-order
paths: [".claude/rules/cache-subtrahend-load-order.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "A `$6<->$7` reg-swap + reordered first-two-loads cluster on a block of `field = x - ((hi - x) << 1)` reflections closes when you cache the SECOND operand (the `hi` subtrahend) of each into a named local declared in target's load order; minuends stay inline."
metadata:
  type: reference
---

## Symptom

A block computes several point-reflections of the form

```c
*(s32 *)(p + LO) = x - ((*(s32 *)(p + HI) - x) << 1);   /* x = *(p+LO) */
```

(reflect `x` across the midpoint `*(p+HI)`), repeated for 2-3 coordinate
pairs. The cheat-free build sits a few insns off target with a
`regfix $6 <-> $7 @ a-b` register swap PLUS two `subst` rules that reorder
the first two loads of the block. The diff shows:

- target loads the **`HI` (second) operand FIRST**, then the `LO` operand —
  e.g. `lw v1,24(a0)` (HI) before `lw a2,8(a0)` (LO);
- your build loads `LO` first, into the *higher* of the a2/a3 pair;
- consequently every value derived from `LO`/the-next-coordinate has a2 and
  a3 swapped vs target through the rest of the block.

It is one coupled register-allocation decision (which of `$a2`/`$a3` holds
the first reflected value), driven entirely by **load order**.

## Why naive read-reordering fails

Shuffling the *minuend* reads (`a`, `b`, `new_var`) does nothing — they're
already loaded in a reasonable order. Adding ONE high-operand temp
(`hi0 = *(p+0x18)`) and leaving the others inline makes it WORSE (the lone
hoisted load perturbs allocation without fixing the pairing). The fix is to
hoist ALL the `HI` subtrahends together so the whole block's load schedule
flips to target's `HI, LO, HI, LO, …` interleave.

## Fix — cache every subtrahend into a named local, in target's load order

```c
p = (u8 *)BASE;
hi0 = *(s32 *)(p + 0x18);          /* second operand of coord-0 reflect */
a   = *(s32 *)(p + 0x8);           /* minuend, inline order */
hi1 = *(s32 *)(p + 0x1C);
b   = *(s32 *)(p + 0xC);
*(s32 *)(p + 0x5C) = (s32)FUNC;    /* whatever non-reflect store sits here */
hi2 = *(s32 *)(p + 0x20);
*(s32 *)(p + 0x8)  = a - ((hi0 - a) << 1);
*(s32 *)(p + 0xC)  = b - ((hi1 - b) << 1);
c = *(s32 *)(p + 0x10);            /* last minuend loads late, as in target */
*(s32 *)(p + 0x10) = c - ((hi2 - c) << 1);
```

The named `hiN` locals interleave with the minuend reads to reproduce
target's load schedule; the a2/a3 pairing then falls out pure-C, retiring
the `$6 <-> $7` swap and both load-reorder `subst` rules together.

## Why this is not "cheats by any spelling"

`hi0/hi1/hi2` are the **second endpoint coordinates** of the reflection — a
human writing "reflect each coordinate across the far point" would naturally
read the far point's components into named locals. They carry real values
that are USED (each is a `subu` operand), unlike a dead-store or a literal
param rename. The declaration order mirrors the data's natural pairing, not a
reverse-engineered LUID. Sandbox `--disable all` → 0; full SHA1 == oracle.

## Confirmed case — func_80053304 (text1b.c, 2026-06-14)

Queue/board top, verdict C, distance 9, 3 rules (2 load-reorder `subst` +
`$6 <-> $7 @ 48-60`). Caching the three `0x18/0x1C/0x20` subtrahends into
`hi0/hi1/hi2` → sandbox 0; `retire` dropped all 3 rules, SHA1 == oracle;
COMPLETED-C.

## Related

- [[hoist-call-arg-local-flips-jal-delay]] — sibling "hoist a late-loaded
  value into a local declared first in the block to fix scheduling".
- [[register-alloc-pure-c]] — parent RA playbook; lower-reg-in-target means
  YOUR build is the anomaly (here: load order is the blocker).
- [[split-read-defeats-hoist]] — sibling structural-load lever.
