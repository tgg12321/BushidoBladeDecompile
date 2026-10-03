---
name: packed-multiply-cluster
paths: ["src/display.c", "src/main/368E4.c"]
description: "Packed fixed-point multiply (8007Exxx display.c): a redundant mask before a discarding shift = S8/STRONG = hand-coded (canonical asm); otherwise use the H-structure pure-C recipe."
metadata:
  type: recipe
---

# Packed fixed-point multiply functions (8007Exxx display.c cluster)

Leaf functions that load packed s16 values, multiply by Q12/Q16 coefficients, shift, and repack. **Run
`python3 tools/scan_hand_coded.py --single <func>` FIRST.**

## Hand-coded tell: signal S8

```mips
andi $t1, $t0, 0xFFFF    # keep low 16 bits
sll  $t1, $t1, 16        # ...then shift them all into the high half
```

The mask is redundant and single-use; GCC's combine ALWAYS elides it (both our cc1 and cc1psx — not a fork
divergence; combine.c:1458 keeps such a def only when multi-use). A target that KEEPS the mask was hand-written.
S8/STRONG ⇒ canonical asm via `inline_asm_canonical.txt` + a file-scope `__asm__(glabel)` block (decimal
memory offsets — maspsx parses `0x0($a0)` as base-10; hex immediates are fine). Do not grind pure C. Never
re-spell the mask as a hardcoded-`$N` `__asm__` inside C ([[inline-asm-policy]]). Reference: `func_8007EDBC`
(commit 2db22e1).

## Genuinely-C members: the H-structure

When S8 does not fire, the function is compiled C:

1. Precompute `hi = w >> 16` into its own variable (forces a full `lw` of the packed source).
2. Widening multiply `u64 p = (u64)(u32)x * (u32)m;` — `multu` with the unused `mfhi` DCE'd.
3. Reuse one u64 product variable sequentially rather than parallel temporaries.
4. Low-multiply-first in source matches the emit order.

Related: [[canonical-asm-authorization-recipe]]
