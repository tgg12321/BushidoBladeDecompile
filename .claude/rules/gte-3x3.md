---
name: gte-3x3
paths: ["src/display.c", "src/text1b.c"]
description: "ARCHIVED FORBIDDEN — the gte-3x3 recipe (register pins + single-instruction lui mask barrier + __asm__ memory barriers + move %0,%1 return pin) is cheat-asm per the 2026-05-31 expanded catalog. Score-inert under the cheat-invisible sandbox; the audit flags it; no path forward as a committed match. Display.c GTE 3x3-mvmva wrappers are canonical hand-coded asm (see inline_asm_canonical.txt)."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — gte-3x3 recipe is forbidden

This file used to document a recipe for matching `calc_fc_frame_8007EC5C` /
`func_8007E4DC` / `func_8007EB4C` and siblings in `src/display.c` via three
coordinated tricks:

1. Pin the `0xFFFF0000` mask to `$at` via a single-instruction `lui` `__asm__`
   barrier.
2. Pin the return value to `$v0` with a trailing `move %0, %1` `__asm__`.
3. Insert `__asm__ volatile("" ::: "memory")` scheduling barriers between
   paired pack-store blocks.

**Every element is in the cheat catalog** ([[inline-asm-policy]]):
- `register T x asm("$N")` pins → cheat-asm
- Single-instruction `__asm__ volatile("lui %0, 0xFFFF" : "=r"(mask))` to force
  per-cycle re-materialization → general-purpose-opcode inline asm used to
  steer scheduling
- `__asm__ volatile("" ::: "memory")` → scheduling-barrier cheat-asm
- `__asm__ volatile("move %0, %1" : "=r"(v0) : "r"(vec))` return-value pin →
  the canonical `INLINE_MOVE_ALIASING` cheat

All four are **score-inert under the cheat-invisible sandbox** — stripping
them does not change the honest pure-C distance, so writing them gains
nothing. The engine's `func_cheat_asm_count` + `mark_done` gate refuses
completion for any function carrying them.

## What this means for the display.c GTE wrappers

The 2026-05-12 "first-attempt match" of `calc_fc_frame_8007EC5C` etc. via
this recipe only worked because the recipe encoded **exactly what hand-coded
asm looks like** — the recipe's bytes happened to equal target bytes
because the original was hand-written asm. With the recipe forbidden, the
honest pure-C distance for these functions is the distance to a non-existent
pure-C form.

The display.c GTE wrapper cluster — `calc_fc_frame_8007EC5C` and its
3x3-mvmva siblings — are **canonical hand-coded asm**:
- Target asm annotates every cop2 op `/* handwritten instruction */`.
- Hardcoded source registers in the final `swc2 $11, 0x10($a1)` are the
  SOTN signature for hand-coded assembly.
- The mvmva→mfc2→mtc2→nop→mvmva pipeline interleaving (cycle N+1's setup
  during cycle N's GTE latency) is canonical hand-scheduling.
- The per-cycle `lui $at` re-materialization (cycles 1 + 3 but not 2) is a
  hand-coded scheduling choice.
- `addu $v0, $a1, $zero` placed at the very end before `jr` is the
  canonical hand-coded "return pinned to end" pattern.

`calc_fc_frame_8007EC5C` was authorized **ASM-WHOLE / canonical-body** on
2026-05-31; the cluster siblings (`func_8007E4DC`, `func_8007EB4C`) should
follow the same path when they reach the queue top. See
`inline_asm_canonical.txt` and [[canonical-asm-retirement]].

## Historical content

The original recipe text is preserved in git history:
`git show HEAD~1:.claude/rules/gte-3x3.md`

## Related
- [[inline-asm-policy]] — current expanded cheat catalog (the authoritative
  policy reference, replaces this recipe as guidance)
- [[canonical-asm-retirement]] — how a hand-coded GTE wrapper legitimately
  ends as canonical inline asm in `inline_asm_canonical.txt`
- [[gte-wrapper-misroute-park]] — the pure GTE leaf wrapper auto-authorize
  category (contrast: those are no-C-form by construction; this cluster is
  hand-coded asm by signature)
- [[packed-multiply-cluster]] — sibling display.c cluster, scan-hand-coded
  S8 signal routes it to canonical-asm too
