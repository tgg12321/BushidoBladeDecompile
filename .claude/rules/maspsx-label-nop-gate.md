---
name: maspsx-label-nop-gate
paths: ["src/*.c", "maspsx_label_nop_funcs.txt", "tools/maspsx/**"]
description: "The maspsx .L-label load-delay blind spot (load-consumer variant, incl. store-value consumers and if/else merge labels) is a pure-C RETIREMENT path, not a park: add the function to maspsx_label_nop_funcs.txt and remove the compensator (source __asm__(nop) OR a regfix insert-nop rule via retire). Per-function-scoped so it doesn't cascade. Don't broaden the gate globally."
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

**The consumer need not be a load.** The gate fires on `line_loads_from_reg(after_label,
r_dest)`, and for a **store** that helper returns True when `r_dest` is the store's
*value* operand (`__init__.py:95`, "a bit of a lie") — so `lw $rN; .L:; sw $rN,disp($base)`
(the loaded value is stored straight back, e.g. through a global) is covered too. It is
also not loop-specific: a plain if/else **merge label** between the load and its consumer
hits the same blind spot.

**The compensator may be a regfix rule, not source asm.** Older instances of this exact
shortfall were patched with a `func: insert "nop" @ <idx>` regfix rule instead of a source
`__asm__("nop")`. Same blind spot, same fix — but you finish with `retire` (drops the rule)
rather than deleting a source line. The tell is identical: `canonical` → C, pure-C distance
1, the lone diff a missing nop across a `.L` merge label.

## This is now a PURE-C path (not a park)

As of 2026-05-26, maspsx has a **per-function-scoped** gate for exactly this
load-consumer case. To retire the function to Tier-4:

1. Add the function name to **`maspsx_label_nop_funcs.txt`** (one per line).
2. Remove the compensator: delete the source `__asm__("nop");` **or**, if it was a
   regfix `insert "nop"` rule, just `retire <func>` (the allowlist makes maspsx emit
   the nop, so the rule is now redundant).
3. SHA1 must == oracle (`retire` / `verify-oracle --rebuild` gate it), then `queue done`.

maspsx then emits the load-delay nop for that function (it tracks `self.current_func`
from `.ent`), so the C needs no inline asm. Confirmed: `spu_DmaTransfer` (main.c,
commit 25f8f56); `cdrom_DmaToRam` (system.c, 2026-05-26) — `lw $v1,%lo(g_cd_index_reg)`
→ `.L80081DA4:` → `lbu $v0,0($v1)` (the CD `DATA_REQ` poll loop). distance 1 (the lone
stripped nop), 0 rules; allowlist + delete the `__asm__("nop")` → SHA1 == oracle. Note
the function's *other* loop (`.L80081DF0`) has its base produced by `addu`, not a load,
so the gate correctly leaves it untouched.

`gnd_get_fog` (text1b.c, 2026-05-26) — **store-value consumer + regfix compensator**:
`lw $v0,%gp_rel(D_800A3420)` → `.L80060E1C:` (if/else merge, not a loop) → `nop` →
`sw $v0,%gp_rel(D_800A37D4)`. The compensator was a `gnd_get_fog: insert "nop" @ 6`
regfix rule (no source asm). Added to the allowlist + `retire` (drops the rule) → SHA1
== oracle, 100% pure C. The other path (`lw $v0,D_800A3424; j .L80060E1C; nop`) has the
load 2 insns before the merge, so no hazard and the gate doesn't touch it.

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
