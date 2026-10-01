---
name: fork-divergence-inline-asm
paths: [".claude/rules/fork-divergence-inline-asm.md"]
description: "Owner ruling 2026-07-13 (Ruling-2): a region-scoped asm island is admissible only where our cc1 SIGSEGVs on the faithful C that cc1psx compiled to the target bytes. Four-gate evidence; never whole-function."
metadata:
  type: rule
---

# Fork-divergence inline asm (Ruling-2, 2026-07-13)

A small `__asm__` island inside an otherwise pure-C function is admissible ONLY when our GCC 2.7.2 fork provably
cannot emit the target's bytes because the C source the original PsyQ `cc1psx` compiled them from **crashes our
cc1**. The function stays pure C outside the island. No function currently holds this class (the original
`_spu_FiDMA` question moved to docs/grind/borderline.md 2026-08-24).

## The four-gate evidence (all mandatory, reviewer re-runs each)

1. **Reproducible crash.** A minimal C file that SIGSEGVs `tools/gcc-2.7.2/cc1` at production flags, committed
   with the evidence (not under `tmp/`).
2. **cc1psx counter-exhibit.** The same file compiles cleanly under `tools/cc1psx_wrapper.sh`; `.s` preserved.
3. **cc1psx output matches the target** byte-for-byte within the affected region.
4. **Probe grid.** >= 5 behaviour-equivalent C reformulations, each with its outcome (crash / different
   structure / match), showing every non-crashing form provably cannot reach the target shape.

Missing any gate ⇒ ordinary search item.

## Disposition

- The island covers ONLY the instructions the probe grid proved unreachable — not surrounding calls or control
  flow a compiling form already emits.
- No hardcoded-`$N` templates ([[inline-asm-policy]]); operands use `%N` placeholders.
- Completion class COMPLETED-INLINE-ASM-CANONICAL, listed in `inline_asm_canonical.txt` under a
  "fork-divergence sub-class" header citing the crash class and the four-gate evidence paths; layer-2 review
  mandatory.

## Not covered

Fork divergences without a crash (RA, scheduling, reload, prologue order) are ordinary C search work
([[register-alloc-pure-c]], [[no-compiler-divergence]]). Not a per-idiom template; not whole-function
canonical asm; not a license to patch cc1 (the toolchain stays frozen).

Related: [[canonical-asm-authorization-recipe]]
