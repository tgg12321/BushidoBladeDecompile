---
name: inline-move-aliasing
paths: [".claude/rules/inline-move-aliasing.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "ARCHIVED FORBIDDEN — `__asm__ volatile(\"move %0, %1\" : \"=r\"(dst) : \"r\"(src))` paired with `register T x asm(\"$N\")` pins is cheat-asm per [[inline-asm-policy]]. Score-inert under the cheat-invisible sandbox; the engine refuses completion for affected functions. Use [[register-alloc-pure-c]] to find the C structure that gives target's RA naturally."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — INLINE_MOVE_ALIASING is forbidden

This file used to teach a recipe for forcing GCC to emit a redundant
`addu $rD, $rS, $zero` register-to-register copy via:

```c
register T dst asm("$RD");
register T src asm("$RS");
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));
```

It was already marked "RETIRED AS AN END STATE" in 2026-05-21, but the
body still functioned as a recipe. Per the 2026-05-31 standing rule that
forbidden techniques are archived rather than annotated, the rule has
been retired in full.

## Why it's cheat-asm

The `move %0, %1` template, even with `%N` placeholders and `=r`/`r`
constraints (so it's *not* the literal-register injection variant from
[[inline-asm-injection]]), still uses a **general-purpose opcode that
has a C equivalent** (`dst = src;`) wrapped in `__asm__` purely to
force GCC's register allocator to materialize a redundant copy. Per
[[inline-asm-policy]]'s category table:

> **cheat** — Inline `__asm__` or `register T x asm("$N")` pin used to
> steer GCC's allocator or scheduler. General-purpose opcodes
> (`move`, `addu`, `nop`, `lui`, `negu`, etc.) that have C equivalents
> but we wrote them in asm to force matching.

The `move` opcode is the textbook example.

## What the pattern in target asm means

You will still see `addu $rD, $rS, $zero` (the assembler form of
`move $rD, $rS`) in target asm — it's a real codegen output of GCC's
allocator when a value flows through two pseudos. Seeing it is
**diagnostic information about target's register flow**, not a recipe
to ship. The job is to find the C structure (typically: a pointer
variable that gets passed to an `__asm__` `"r"` constraint, so GCC
materializes it as a register and the move falls out, OR a local that
extends a value's live range past a redefinition) that makes GCC
choose those registers and emit that copy from compilation, not from a
literal asm template.

[[register-alloc-pure-c]] documents the pure-C levers (block-local
variable split, narrow integer type, loop-local precompute) that
restructure liveness/conflict graphs to reach target's RA.

## Historical content

The original recipe and the detailed pattern explanation are preserved
in git history:
`git show HEAD~1:.claude/rules/inline-move-aliasing.md`

## Related
- [[inline-asm-policy]] — current expanded cheat catalog (authoritative)
- [[inline-asm-injection]] — the sibling cheat: hardcoded `$N` in the
  asm template (no `%N` placeholders) — same intent, different spelling
- [[register-asm-pins]] — the companion pin cheat; pins are
  DIAGNOSTIC-ONLY and must be removed before commit
- [[register-alloc-pure-c]] — the pure-C lever playbook for reaching
  target's register allocation without pins or `__asm__` moves
- [[sandbox-zero-retire-fails]] — the diagnostic that catches a function
  carrying this pattern (sandbox 0 because the strip removes it,
  retire fails because removing it from src is the missing step)
