---
name: reload-spill-reg-reveals-asm-clobbers
paths: [".claude/rules/reload-spill-reg-reveals-asm-clobbers.md"]
description: "A reload scratch reg at an unexpected regno next to an authorized asm island (target `mfhi $t8`, ours `$13`) proves the original named $13-$15 in that island's clobbers — widen the existing clobber list, nothing else."
metadata:
  type: reference
---

# The reload spill register fingerprints the original source's asm clobbers

## Symptom

Near an already-authorized canonical inline-asm island (GTE, BIOS trampoline), a compiler-generated scratch
register differs only in number:

```
ours:    mfhi $13        sra $3,$13,5
target:  mfhi $t8        sra $v1,$t8,5     ($t8 == $24)
```

## Mechanism (`tools/gcc-2.7.2/reload1.c` `order_regs_for_reload`)

- Lines 3644-3663: every hard reg in `regs_explicitly_used[]` goes into `bad_spill_regs` (live on MIPS — no
  `SMALL_REGISTER_CLASSES`).
- Lines 3690-3694: remaining spill candidates are ordered ascending among unused call-clobbered regs.

Inside a compiled TU only an `__asm__` operand/clobber list mentions hard registers explicitly. With only `$12`
mentioned, the first free reg is `$13`; with `$12`-`$15` mentioned, the next candidate is `$24` (`$t8`). If
`$13`-`$15` have zero pseudo uses in the target function and are not fixed/eliminable, explicit mention is the
only route — the original TU named them at asm level.

## What to write

Widen the clobber list of the island that is ALREADY authorized, in its own spelling (this matches PsyQ
`inline_o.h`, whose GTE macros clobber `"$12","$13","$14","$15"`):

```c
__asm__ volatile("...": "=m"(sp_tmp) : "r"(v) : "$12","$13","$14","$15");
```

## Bounds

- Not a pure-C technique; applies only where the island is already authorized/canonical.
- Clobbers may only be widened to registers the target's reload temp proves skipped. Adding clobbers to steer
  allocation otherwise is a register pin in clobber spelling — a cheat.
- Never enlarges the island; address arithmetic on a C-visible value stays in C ([[inline-asm-policy]]).

Ruling: docs/grind/decisions.md 2026-07-28 (func_8002BC68, commit `104fc679`).

## Related

[[canonical-asm-authorization-recipe]] · [[inline-asm-policy]] · [[cop2-addressing-preamble-cluster]]
