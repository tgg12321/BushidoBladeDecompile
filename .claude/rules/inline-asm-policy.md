---
name: inline-asm-policy
paths: ["src/*.c"]
description: "Two-category inline-asm policy: CANONICAL (GTE/cop2/BIOS/HW) is authentic and fine; CHEAT (register pins, INLINE_MOVE_ALIASING, scheduling barriers) is forbidden — a function carrying any cheat-asm is INCOMPLETE."
metadata:
  type: rules
---

# Inline-asm policy

> **Policy ([[completion-standard]], 2026-05-21; expanded 2026-05-31):** a
> function is in exactly one of three categories:
>
>   * **INCOMPLETE** — in `engine/queue.json`. Carries a regfix/asmfix rule,
>     a cheat-asm pin/__asm__, OR a non-zero honest pure-C distance.
>   * **COMPLETED-C** — zero rules, zero cheat-asm in source, byte-matches.
>     Not in queue. Not in `inline_asm_canonical.txt`. The SOTN bar.
>   * **COMPLETED-INLINE-ASM-CANONICAL** — zero rules, has canonical inline
>     asm (GTE/cop2/BIOS/HW) or whole-body `__asm__("glabel ...")` as its
>     accepted finished form. Listed in `inline_asm_canonical.txt`.
>
> A function whose only path to byte-match is a register pin, inline-move,
> scheduling barrier, regfix rule, OR any of the codegen-coercion cheats in
> the **expanded cheat catalog** below is **INCOMPLETE** — not "done with
> a hint." There is **no attempts-log escape valve** to commit cheat-asm.
> Cheat-asm is the *work*, not the answer.
>
> **EXPANDED CHEAT CATALOG (2026-05-31).** The engine's `volatile_cheats`
> detector now also flags these patterns as cheat-asm debt — previously
> slipped through detection as "documented techniques":
>
> - **Alias renames** — `extern (volatile)? T name asm("Y")` with name != Y
>   (separate C handle for an existing global, used to defeat CSE / force
>   re-materialization). See [[inline-asm-injection]] § alias renames.
> - **Inline volatile casts on game-state globals** —
>   `*(volatile T *)&D_xxxxxxxx` (forces non-volatile globals to act
>   volatile for scheduling/CSE coercion). Game RAM globals were never
>   volatile in the original source.
> - **Plain `extern volatile T D_xxxxxxxx;`** — same coercion at declaration
>   level (was documented in [[split-read-defeats-hoist]] as a technique
>   until 2026-05-31 — now forbidden by default; see
>   [[legitimate-volatile-interrupt-touched]] for the SOTN-grounded narrow
>   carve-out applying ONLY to globals asynchronously mutated by an
>   identifiable IRQ handler at use sites that demonstrably require
>   CSE-defeat. The default ban is unchanged for every case OUTSIDE the
>   two-pronged criterion in that rule).
> - **Unused fixed-size local arrays** — `s32 buf[N];` declared with no use,
>   to force GCC to reserve frame bytes. See [[dead-vars-local-array]] for
>   the deprecated rationalization.
> - **Dead self-assignments of function parameters** — `arg0 = 0;` where
>   `arg0` is a parameter never referenced afterward, used to break GCC's
>   value-association for register allocation. See [[register-alloc-pure-c]]
>   Lever D for the deprecated rationalization.
> - **Macro-hidden `__asm__`** — `#define X ... __asm__(...) ...` macros
>   (e.g., `PAD_NOPS_*` in `code6cac_*.c`) that expand to inline asm at
>   every use site. The existing detector skipped `#define` lines; the new
>   detector catches them.
>
> Functions affected by any of these are INCOMPLETE after `queue regen` and
> must reach genuine pure-C COMPLETED-C OR canonical-asm authorization.
> The "documented technique" justification is no longer accepted.

When BB2's inline-asm policy is honest, every `__asm__` block in
`src/*.c` falls into one of the categories below:

| Category | What it is | Acceptable? |
|---|---|---|
| **canonical-body** | Full canonical-asm function body — original was hand-written asm, or the function emits via file-scope `__asm__("glabel ...")`. Listed in `inline_asm_canonical.txt`. | ✅ COMPLETED-INLINE-ASM-CANONICAL |
| **canonical** | Inline `__asm__` in a C function body using opcodes that ONLY EXIST in asm form: GTE coprocessor ops (`ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2`/`.word 0x4XXXXXXX`), BIOS vector jumps (`j 0xA0`/`B0`/`C0`), cache/DMA register pokes (`0x1F8003xx`). Authentic — original devs wrote these. | ✅ COMPLETED-INLINE-ASM-CANONICAL |
| **cheat** | Inline `__asm__` or `register T x asm("$N")` pin used to steer GCC's allocator or scheduler. General-purpose opcodes (`move`, `addu`, `nop`, `lui`, `negu`, etc.) that have C equivalents but we wrote them in asm to force matching. NOT in original source — they're workarounds for our `mips-gcc-2.7.2` fork diverging from the original `cc1psx`. | ❌ INCOMPLETE — **the BB2-specific gap** |
| **(no asm)** | Pure C, no inline asm. Matches byte-for-byte without hints. | ✅ COMPLETED-C — gold standard |

# Why this distinction matters

For a long time the BB2 project lumped canonical and cheat asm together as
"inline asm" and treated the whole category as suspect. But:

- The original PSX devs *did* write inline asm — for GTE ops, BIOS
  trampolines, hardware register pokes. That's canonical.
- They almost never wrote inline asm for general-purpose register
  allocation or scheduling control. That's cheat-asm, and it's BB2's gap
  from the SOTN community standard.

`INCLUDE_ASM`-ing a cheat-asm function as if it were unsolved would hide
real work. Pretending cheat-asm functions are at SOTN standard would lie.
The honest position is: canonical inline asm is fine; cheat-asm is a
measurable debt metric we track and reduce over time.

# Concrete examples in BB2

Canonical (authentic):
```c
__asm__ volatile ("ctc2 %0, $0" :: "r"(t5));        // GTE control reg load
__asm__ volatile (".word 0x4A486012" :: ...);       // mvmva GTE op
__asm__ volatile ("sw %0, 0x1F800360" :: "r"(v0));  // scratchpad poke
```

Cheat (toolchain workaround — forbidden):
```c
register s32 cached asm("$16");                            // allocation hint
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));    // INLINE_MOVE_ALIASING
__asm__ volatile("" ::: "memory");                          // scheduling barrier
```

# How categories are surfaced

`python3 tools/classify_inline_asm.py` walks `src/*.c`, classifies every
`__asm__` block and every `register T x asm("$N")` pin, and emits:

```
=== inline-asm classification ===
  canonical (authentic GTE/BIOS/HW): N instances across X pure-canonical funcs
  cheat (toolchain workaround):  M instances (A __asm__ blocks + B register-asm pins) across Y pure-cheat funcs
  mixed (both categories in one func): Z funcs

  GAP TO SOTN BAR:
    (Y + Z) functions use cheat inline asm
    M total cheat instances to retire
```

# Route to SOTN bar

**Measured 2026-05-18 (commit `32b2da9`):** on a 16-function sample of
the cheat-asm corpus, **0 functions are COMPILER_FIXABLE.** Even Sony's
actual `cc1psx` fails identically to decompals on the pure-C
reconstructions. The compiler-patch route does not work; see
[[compiler-patch-low-roi]] for the data + methodology and
`docs/diagnostics/cc1psx_cheatasm_diagnostic_16funcs.csv` for the raw
results.

The only path with measured non-zero ROI for cheat-asm retirement:

**Per-function pure-C re-attempt.** For each cheat-asm function, drive it
to COMPLETED-C (pure C). A successful retirement removes the function's
cheat-asm and rules. There is **no "declare it needs hints and commit"
outcome** — a function that won't reach pure C stays INCOMPLETE; you
keep switching technique ([[escalation-ladder]]). The only non-pure-C
finish is canonical-asm authorization (COMPLETED-INLINE-ASM-CANONICAL)
for a construct proven physically un-compilable — never cheat-asm.
(Attempt logging in `.bb2_attempts/` is still useful as a record of what
was tried, but it no longer unlocks a cheat-asm commit — see
[[attempts-log-gate]].)

Secondary path: **reclassify cheat-asm functions whose inline asm is
load-bearing.** The diagnostic surfaced 5 functions where stripping the
inline asm broke the C body — strong evidence the asm wasn't a workaround
hint, it was authentic computation. Those belong in the canonical
category, not cheat. An audit pass could shrink the cheat-asm count
without changing any source code.

# Related

- [[inline-move-aliasing]] — the most common cheat-asm pattern
- [[register-asm-pins]] — pin reliability + when they're cheats
- [[attempts-log-gate]] — gate enforcement for new cheat-asm commits
- [[compiler-patch-low-roi]] — 0/16 measured ROI for the compiler-
  patch route (so it's not on the table)
- [[cc1psx-calibration-only]] — prior project decision to not switch
  the build to cc1psx
- [[community-standard]] — what SOTN/Vagrant/etc accept
