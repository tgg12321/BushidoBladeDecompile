---
name: scratchpad-gte
paths: ["src/text1b.c", "src/display.c"]
description: "ARCHIVED FORBIDDEN — the scratchpad-GTE recipe (register pins, move %0,%1 base-pointer aliasing, volatile s32 * coercion casts, 4 compound regfix delete+reorder+subst rules) is cheat-asm + cheat regfix per the 2026-05-31 expanded catalog. No path forward as a committed match."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — scratchpad-GTE recipe is forbidden

This file used to document a recipe for matching functions that compute
into PS1 scratchpad (`0x1F800xxx`) then run a GTE op on the result
(`cpu_check_tubazeri` and family). The recipe combined:

1. `register T x asm("$N")` pins on `$a3`/`$t4`/`$t5`/`$t6`/`$t7` to force
   target's working-register layout.
2. `__asm__ volatile("move %0, %1" : "=r"(mp) : "r"(q))` aliasing — the
   `INLINE_MOVE_ALIASING` pattern — to force a base-pointer reload.
3. `volatile s32 *va0 = (volatile s32 *)a0;` casts to defeat CSE on the
   diff-source loads ("volatile casts defeat CSE on the source loads —
   target re-reads a0[N] every diff").
4. A 4-rule compound regfix cluster (`delete @ nop_idx`, `reorder` to
   move `lui+ori` inline with the last diff, two `subst` rules to swap
   `lw $a0`/`lw $a1` order before the jal).

**Every element is in the cheat catalog** ([[inline-asm-policy]]):
- Register-asm pins → cheat-asm
- `move %0, %1` aliasing → `INLINE_MOVE_ALIASING` cheat-asm
- `volatile T *` coercion cast on a non-volatile global → 2026-05-31
  volatile-coercion catalog entry
- Multi-rule regfix `delete`/`reorder`/`subst` cluster around a jal —
  the index-anchored rules are exactly the brittle rule shape that
  [[lost-codegen-insert-cheat]] and [[global-label-drift-sibling-cheat]]
  warn against

All are **score-inert under the cheat-invisible sandbox** — stripping
them does not change the honest pure-C distance, so writing them gains
nothing.

## What this means for the scratchpad+GTE cluster

The 2026-05-12 `cpu_check_tubazeri` "match" via this recipe is now a
**parked function** (cheat-asm + cheat regfix carrier). The honest
status of this family:

- The scratchpad write idiom `__asm__ volatile("sw %0, 0x1F800360" :: "r"(v0))`
  is canonical inline asm (PS1 scratchpad poke — see
  [[inline-asm-policy]] canonical category). That part is fine.
- The base-pointer aliasing + `volatile s32 *` CSE-defeat + scheduling
  regfix around the GTE block are NOT fine. The fix is either:
  (a) restructure the C so cc1's natural scheduling and CSE liveness
      reach target without the coercion (the [[register-alloc-pure-c]]
      lever family + [[difficult-is-not-impossible]] discipline applies),
      OR
  (b) authorize the function as canonical-body asm if the disassembler /
      hand-coded-asm signals indicate the original was hand-written
      ([[hand-coded-asm-recognition]] / [[canonical-asm-retirement]]).

## Historical content

The original recipe text is preserved in git history:
`git show HEAD~1:.claude/rules/scratchpad-gte.md`

## Related
- [[inline-asm-policy]] — current expanded cheat catalog
- [[register-alloc-pure-c]] — pure-C levers for register-allocation gaps
- [[canonical-asm-retirement]] — the legitimate canonical-asm path for
  genuinely hand-coded constructs
- [[lost-codegen-insert-cheat]] — the cheat family of single-rule
  manufactured-instruction `insert_after`s; the scratchpad recipe's
  `delete`/`reorder`/`subst` cluster is the multi-rule analogue
