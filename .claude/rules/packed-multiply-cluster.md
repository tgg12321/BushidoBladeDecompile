---
name: packed-multiply-cluster
paths: ["src/display.c", "src/text1b.c"]
description: "Packed fixed-point multiply (8007Exxx display.c): redundant mask before a discarding shift = S8/STRONG = hand-coded (retire as canonical asm); else use the H-structure pure-C recipe. From func_8007EDBC."
metadata:
  type: recipe
---

# Packed fixed-point multiply functions (8007Exxx display.c cluster)

Leaf functions that load packed s16 values, multiply by Q12/Q16 coefficients,
shift, and repack. Two outcomes depending on whether the original was
hand-coded asm or compiled C. **Run `dc.sh scan-hand-coded --single <func>`
FIRST** — it now classifies this cluster correctly (signal S8, added 2026-05-21).

## Recognition: hand-coded vs C (the S8 signal)

The decisive tell that a packed-multiply function was HAND-CODED (not compiled
from C) is a **redundant low-bits mask before a discarding left-shift**:

```mips
andi $t1, $t0, 0xFFFF    # keep low 16 bits
sll  $t1, $t1, 16        # ...then shift them all into the high half
```

The `sll 16` discards exactly the bits the `andi 0xFFFF` kept (and the mask's
clearing of the high bits is undone by the shift), so the mask is
mathematically redundant AND single-use (the sll overwrites $t1). **GCC's
combine pass ALWAYS elides this** (subreg-of-AND / ashift-of-AND
simplification). Verified 2026-05-21: both our decompals fork and the original
cc1psx (Sony GCC 2.7.2.SN.1) elide it — proven byte-identical, so this is NOT a
fork-vs-cc1psx divergence (do not reach for that framing; see
[[cc1psx-calibration-only]]). A target that KEEPS the mask was hand-written.

This is signal **S8** in `tools/scan_hand_coded.py` — decisive (STRONG) like
S6/S7. If scan-hand-coded reports S8/STRONG, the function is canonical asm:
retire it via `inline_asm_canonical.txt` + a file-scope `__asm__(glabel)` block
(decimal memory offsets — maspsx parses `0x0($a0)` as base-10 and chokes; hex
immediates like `0xFFFF` are fine). Do NOT keep grinding pure C — you can't
win, GCC cannot emit the mask.

**Reference:** `func_8007EDBC` retired as canonical asm in commit 2db22e1
(display.c packed fixed-point multiply, `andi $t1,$t0,0xFFFF; sll $t1,16`).
~38 pure-C forms failed; cc1psx byte-identical to the fork; the coupling logic
is combine.c:1458 (`added_sets_2 = !dead_or_set_p(i3, i2dest)`) — combine keeps
the def + the 2nd-use insn ONLY for a multi-use mask, and drops it for a
single-use one.

## Matching the genuinely-C ones: the H-structure

When scan-hand-coded does NOT report S8 (no redundant mask), the function IS
compiled C and a packed widening multiply CAN be matched in pure C. The
reusable structure that cracks the `lw / multu / mfhi / ordering` pattern:

1. **Precompute `hi = w >> 16` into its own variable.** This forces a full
   `lw` word load (not an `lh`/`lhu` half load) of the packed source.
2. **Use a u64 widening multiply: `u64 p = (u64)(u32)x * (u32)m;`** GCC emits
   `multu` and DCEs the `mfhi` when only the low half is used — matching the
   target's `multu` + discarded-hi pattern.
3. **Reuse one u64 product variable sequentially** (reassign it) rather than
   parallel temporaries — matches the target's register cadence.
4. **Low-multiply-first ordering** in source matches the emit order.

This was developed while grinding func_8007EDBC (before it proved hand-coded);
it remains the right approach for the genuinely-C members of the cluster.

## Related
- [[hand-coded-asm-recognition]] — the S1-S8 signal framework + self-auth gate
- [[inline-asm-injection]] — do NOT re-spell the redundant mask as a hardcoded
  `$N` asm template; that is the same bytes from rule text, and the audit blocks it
- gte-3x3 (sibling rule, same display.c cluster — the GTE mvmva wrappers)
- [[minimize-regfix]] — canonical-asm retirement removes ALL the function's
  regfix rules (func_8007EDBC dropped 70, including a lost_codegen cheat)
