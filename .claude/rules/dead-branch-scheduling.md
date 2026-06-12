---
name: dead-branch-scheduling
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "ARCHIVED FORBIDDEN — `insert_after \"sra ...\"` regfix rule to manufacture a dead instruction the branch-likely scheduler emits is bytes-from-rule, not bytes-from-C-compilation. Same family as [[lost-codegen-insert-cheat]]. Score-inert under the cheat-invisible sandbox."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — dead-branch-scheduling regfix is forbidden

This file used to document a recipe for manufacturing the dead
instruction GCC's branch-likely scheduler emits between a `b`'s delay
slot and the next label, via:

```
func: insert_after "sra\t$<reg>,$<reg>,15" @ <maspsx_idx_of_b_delay_slot>
```

acknowledged in the recipe itself as "no natural-C source emits this"
— which is precisely the rule that makes it cheat-regfix. Bytes that
come from the rule text rather than from C compilation violate the
project's pure-C bar. See [[lost-codegen-insert-cheat]] for the
explicit policy on `insert`/`insert_after`/`insert_before` regfix
rules: they only count as compliant when the instruction is a
mechanical consequence of compiling a different but equivalent C
form, never when no C form produces the instruction.

## What this means for the affected functions

Functions that used this recipe — `func_8007EA0C` (per the original
reference), and any sibling with the post-mvmva sign-split dead-sra
pattern — carry **cheat-regfix debt** under the expanded catalog. The
honest status:

- The `sra $tN, $tM, 15` lives in dead position because the original
  branch-likely scheduling pass laid it there. That's a GCC scheduler
  decision driven by C structure.
- The C structure that triggers branch-likely + the dead-position
  emit involves the **bgez/negu/sll/b/negu** sign-split pattern (see
  the now-archived `strength-reduce-defeat.md`). Both of these were
  the legs of a single matching technique that's no longer valid.
- The pure-C path requires finding C source that produces both the
  sign-split scheduling AND the dead-sra in one compile. If no such
  C exists, the function is canonical-asm by definition (the
  scheduling decision can't be reached from any C input).
- [[hand-coded-asm-recognition]] is the gate for that judgment.

## Historical content

The original recipe text is preserved in git history:
`git show HEAD~1:.claude/rules/dead-branch-scheduling.md`

## Related
- [[lost-codegen-insert-cheat]] — the broader category this falls into
- [[inline-asm-policy]] — the expanded cheat catalog
- [[canonical-asm-retirement]] — the legitimate end state when no C
  form reaches a target's codegen
- [[hand-coded-asm-recognition]] — the gate that confirms hand-coded
  vs compiled asm
