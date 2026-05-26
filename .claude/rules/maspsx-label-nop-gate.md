---
name: maspsx-label-nop-gate
paths: ["src/*.c", "maspsx_label_nop_funcs.txt", "tools/maspsx/**"]
description: "The maspsx .L-label load-delay blind spot (load-consumer variant) is now a pure-C RETIREMENT path, not a park: add the function to maspsx_label_nop_funcs.txt and delete its source __asm__(nop). Per-function-scoped so it doesn't cascade. Don't broaden the gate globally."
metadata:
  type: reference
---

# maspsx `.L`-label load-delay nop — RETIRE via the per-function gate (don't park)

## Symptom

A function carries a single tier-3 `__asm__("nop")` (no rules, sandbox
`--disable all` == 0). The nop sits inside a loop right before a load:

```c
do {
    s32 val;
    __asm__("nop");      /* <- maspsx blind-spot compensator */
    val = *ptr;          /* lw consuming `ptr`, across the .L loop-top label */
    ...
} while (...);
```

The target asm has a real load-delay nop there: `lw $rN; .L<loop-top>:; lw
$rM,0($rN)` — the loaded base reg `$rN` is consumed by another load *across* the
`.L` merge label. maspsx's `is_label()` only matches `$L`-prefix locals (this GCC
fork emits `.L`), so it doesn't see the hazard and drops the nop.

## This is now a PURE-C path (not a park)

As of 2026-05-26, maspsx has a **per-function-scoped** gate for exactly this
load-consumer case. To retire the function to Tier-4:

1. Add the function name to **`maspsx_label_nop_funcs.txt`** (one per line).
2. Delete the source `__asm__("nop");`.
3. `verify-oracle --rebuild` → SHA1 must == oracle, then `queue done`.

maspsx then emits the load-delay nop for that function (it tracks `self.current_func`
from `.ent`), so the C needs no inline asm. Confirmed: `spu_DmaTransfer` (main.c,
commit 25f8f56).

## Why per-function (do NOT broaden it globally)

Emitting this nop **shifts maspsx output indices** for the rest of the function's
file. Other functions carry **index-anchored** regfix/asmfix rules (e.g.
`delete_between … '.L279:'`); a global gate change shifts those indices and breaks
them (`symbol .L279 already defined` — the saTan4FireDisp cascade). The allowlist
keeps the nop scoped to opted-in functions, so non-listed functions are
byte-unchanged. The **jalr-consumer** variant (`lw $rN; .L:; jalr $rN`) is
always-on in maspsx because it doesn't cascade; only the load-consumer case is
gated.

If a listed function's file has a sibling with an index-anchored rule, adding the
function is still safe **as long as the sibling isn't also listed** — verify with
`verify-oracle --rebuild` (the oracle catches any cascade).

## Related
- [[inline-asm-tiers]] — the bare `__asm__("nop")` was tier-3 debt
- [[sandbox-zero-retire-fails]] — sandbox 0 + a lone source nop is the tell
- [[loop-rotation-two-shift]] — a different loop-codegen retirement in the same
  file family (main.c bit-search loops)
