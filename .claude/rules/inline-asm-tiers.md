---
name: inline-asm-tiers
paths: ["src/*.c"]
description: "Four-tier inline-asm classification: tier 2 (GTE/BIOS/HW) is authentic and fine; tier 3 (register pins, INLINE_MOVE_ALIASING, scheduling barriers) is the SOTN-bar gap. Surfaced by dc.sh classify-inline-asm."
metadata:
  type: rules
---

# Inline-asm tier system

When BB2's inline-asm policy is honest, every `__asm__` block in
`src/*.c` falls into one of four tiers:

| Tier | What it is | SOTN bar |
|---|---|---|
| **1** | Full canonical-asm function body — original was hand-written asm, or the function emits via file-scope `__asm__("glabel ...")`. Listed in `inline_asm_canonical.txt`. | ✅ accepted |
| **2** | Inline `__asm__` in a C function body using opcodes that ONLY EXIST in asm form: GTE coprocessor ops (`ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2`/`.word 0x4XXXXXXX`), BIOS vector jumps (`j 0xA0`/`B0`/`C0`), cache/DMA register pokes (`0x1F8003xx`). Authentic — original devs wrote these. | ✅ accepted |
| **3** | Inline `__asm__` or `register T x asm("$N")` pin used to steer GCC's allocator or scheduler. General-purpose opcodes (`move`, `addu`, `nop`, `lui`, `negu`, etc.) that have C equivalents but we wrote them in asm to force matching. NOT in original source — they're workarounds for our `mips-gcc-2.7.2` fork diverging from the original `cc1psx`. | ❌ **the BB2-specific gap** |
| **4** | Pure C, no inline asm. Matches byte-for-byte without hints. | ✅ accepted — gold standard |

# Why this distinction matters

For a long time the BB2 project lumped tier 2 and tier 3 together as
"inline asm" and treated the whole category as suspect. But:

- The original PSX devs *did* write inline asm — for GTE ops, BIOS
  trampolines, hardware register pokes. That's tier 2.
- They almost never wrote inline asm for general-purpose register
  allocation or scheduling control. That's tier 3, and it's BB2's gap
  from the SOTN community standard.

`INCLUDE_ASM`-ing a tier-3 function as if it were unsolved would hide
real work. Pretending tier-3 functions are at SOTN standard would lie.
The honest position is: tier 2 is fine; tier 3 is a measurable debt
metric we track and reduce over time.

# Concrete examples in BB2

Tier 2 (authentic):
```c
__asm__ volatile ("ctc2 %0, $0" :: "r"(t5));        // GTE control reg load
__asm__ volatile (".word 0x4A486012" :: ...);       // mvmva GTE op
__asm__ volatile ("sw %0, 0x1F800360" :: "r"(v0));  // scratchpad poke
```

Tier 3 (toolchain workaround):
```c
register s32 cached asm("$16");                            // allocation hint
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));    // INLINE_MOVE_ALIASING
__asm__ volatile("" ::: "memory");                          // scheduling barrier
```

# How tiers are surfaced

`bash tools/dc.sh classify-inline-asm` walks `src/*.c`, classifies every
`__asm__` block and every `register T x asm("$N")` pin, and emits:

```
=== inline-asm tier classification ===
  tier-2 (authentic GTE/BIOS/HW): N instances across X pure-tier-2 funcs
  tier-3 (toolchain workaround):  M instances (A __asm__ blocks + B register-asm pins) across Y pure-tier-3 funcs
  mixed (both tiers in one func): Z funcs

  GAP TO SOTN BAR:
    (Y + Z) functions use tier-3 inline asm
    M total tier-3 instances to retire
```

The summary one-liner also runs in `dc.sh start` so each session sees
the current gap.

# Route to SOTN bar

**Measured 2026-05-18 (commit `32b2da9`):** on a 16-function sample of
the tier-3 corpus, **0 functions are COMPILER_FIXABLE.** Even Sony's
actual `cc1psx` fails identically to decompals on the pure-C
reconstructions. The compiler-patch route does not work; see
[[compiler-patch-low-roi]] for the data + methodology and
`docs/diagnostics/cc1psx_tier3_diagnostic_16funcs.csv` for the raw
results.

The only path with measured non-zero ROI for tier-3 retirement:

**Per-function pure-C re-attempt.** For each tier-3 function, log
attempts in `.bb2_attempts/<func>.jsonl` (gated by
[[attempts-log-gate]]). Each successful retirement demotes a function
from tier 3 to tier 4. The gate enforces breadth (≥4 distinct
technique categories, ≥6 attempts, ≥30 minutes) before allowing escape-
valve commits, so functions actually exhaust pure-C territory before
being declared "needs hints."

Secondary path: **reclassify tier-3 functions whose inline asm is
load-bearing.** The diagnostic surfaced 5 functions where stripping the
inline asm broke the C body — strong evidence the asm wasn't a workaround
hint, it was authentic computation. Those belong in tier 2 (authentic
inline asm in a C function) or tier 1 (canonical asm function body),
not tier 3. An audit pass could shrink the tier-3 count without changing
any source code.

# Related

- [[inline-move-aliasing]] — the most common tier-3 pattern
- [[register-asm-pins]] — pin reliability + when they're tier 3
- [[attempts-log-gate]] — gate enforcement for new tier-3 commits
- [[compiler-patch-low-roi]] — 0/16 measured ROI for the compiler-
  patch route (so it's not on the table)
- [[cc1psx-calibration-only]] — prior project decision to not switch
  the build to cc1psx
- [[community-standard]] — what SOTN/Vagrant/etc accept
