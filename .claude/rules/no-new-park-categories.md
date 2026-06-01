---
name: no-new-park-categories
paths: ["src/*.c", "engine/queue.py", "engine/cheats.py", ".claude/rules/*.md", "CLAUDE.md", "AGENTS.md"]
description: "User policy 2026-06-01: do NOT propose new auto-park / canonical-infrastructure categories that accept cheat-tolerant outcomes. The two existing carve-outs (jtbl-infra, GTE leaf wrappers) are NO-C-FORM exceptions, not generalizable patterns. Register-rotation walls, cross-jump merges, RA plateaus, prologue scheduling — these are ALL pure-C problems that the agent has not yet solved, not new categories to retire as 'infrastructure'."
metadata:
  type: rules
---

# Standing policy: no new cheat-tolerant park categories

> **User policy, codified 2026-06-01:** *"We won't be sanctioning any kind of
> new policy or methodology that is akin to a cheat. ... We can discuss more
> on what to do with register rotation infrastructure but it will never be
> something worse than the SOTN standard."*

When a worker / orchestrator surveys a cluster of parked functions and
notices a recurring wall pattern (e.g. "20+ regfix rules of register
renames", "cross-jump-store-tail-merge floor", "prologue save-order
4-insn reorder"), the temptation is to propose a **new auto-park category**
that retires those functions en masse with their cheats intact, by analogy
to the existing carve-outs:

- **jtbl-infra** ([[jtbl-rodata-split-infrastructure]]) — rodata-split jump
  tables. The GCC-emitted jtbl can't be at the address the function
  references because rodata link order puts the asm/data block before the
  C file's `.rodata`. There is **literally no pure-C form** that resolves
  this without a project-wide rodata reorder.
- **GTE leaf wrappers** ([[gte-wrapper-misroute-park]]) — pure cop2-op
  leaves (avsz3/avsz4 + mtc2/mfc2). The cop2 ops have **no C analog**.
  There is **literally no pure-C form**.

Both carve-outs share the property: **no C input to the same compiler can
produce the target bytes**, by construction (the GCC emitter or linker
literally cannot reach that state from any C source). They are not
"this function is hard to match", they are "no C exists for this".

## What this policy forbids

**Do NOT** propose, document, suggest, or implement any of the following as
project-accepted auto-park / canonical-infrastructure categories:

- **Register-rotation infrastructure.** Functions plateaued behind N-cycle
  register-allocation tiebreaker ties (cpu_side_move_dir_4 /
  marionation_Exec / saEft00Add and the func_8007B***/8007C*** cluster
  members in the same shape) — these are pure-C-reachable, the lever just
  hasn't been found. See [[difficult-is-not-impossible]] and the
  ALLOCDBG-instrumented sessions in [[register-alloc-pure-c]] for the
  proof-of-concept that the levers DO exist for these allocations; the
  remaining gap is more search, not a new category.
- **Cross-jump merge walls.** Functions plateaued behind GCC's `jump2`
  `find_cross_jump` block-suffix merge ([[cross-jump-store-tail-merge]],
  [[cross-jump-call-merge]]). These have known C levers (arg-count for
  CALL suffixes, mixed exit forms for STORE suffixes); when the levers
  don't close a specific function, the answer is more permuter / more
  derivation, not a new category.
- **Prologue save-order infrastructure.** Functions where cc1's
  `save_restore_insns` (mips.c) emits the prologue saves in the order
  GP_REG_LAST → FIRST while target wants the opposite (the
  `func_8007C2A0`/`C4B8` twin wall). This is a register-allocation tie at
  the prologue surface; same category as the above — a C lever exists or
  the function is canonical-asm-authorizable per [[hand-coded-asm-recognition]]
  + user sign-off, but it is NOT a new infrastructure category.
- **Anything labeled "X infrastructure"** as a justification to retire N
  similar functions en masse with their cheats intact.

## What the SOTN standard accepts

[[community-standard]] is the bar: pure C, or canonical-body asm for code
that was *originally* hand-written assembly (not just code GCC won't
emit in our fork). The two carve-outs are no-C-form exceptions; anything
else is a pure-C problem awaiting a C-source solution.

## What to do when you see the temptation

You are surveying the queue / standing escalations and notice 4 functions
with the same 20-rule register-rotation pattern. You are tempted to
suggest "register-rotation-infrastructure" as a new park category. **Stop.**
Per this policy:

1. **The standing answer for register-rotation walls is more search** —
   directed-PERM permuter, instrumented cc1 dumps (BB2_ALLOC_DEBUG,
   BB2_SCHED_DEBUG, BB2_PRIO_DEBUG; see [[register-alloc-pure-c]]),
   cross-reference matched siblings, m2c-reconstructed structure, novel C
   restructurings.
2. **The fallback for a function with strong hand-coded signals is
   canonical-asm authorization** (per [[hand-coded-asm-recognition]]
   /[[canonical-asm-retirement]], requires user judgment).
3. **The fallback for a function without those signals is to keep it
   parked**, in the existing "INCOMPLETE — search continues" state. The
   queue is a worklist of unfinished work; it can hold parked items
   indefinitely without inventing a new "done" category for them.

When surfacing escalations to the user, do NOT phrase options as "accept
X as a new infrastructure category." Phrase them as "(a) permuter budget,
(b) canonical-asm authorization IF the hand-coded signals support it,
(c) keep parked." Categories (a) and (b) and (c) are exhaustive; there
is no (d) "new infrastructure carve-out."

## Related

- [[community-standard]] — the SOTN bar this policy enforces
- [[completion-standard]] — the three function states (INCOMPLETE /
  COMPLETED-C / COMPLETED-INLINE-ASM-CANONICAL); no fourth state exists
- [[no-compiler-divergence]] — companion HARD RULE; the toolchain is
  frozen, so the variable is the C
- [[difficult-is-not-impossible]] — the cardinal rule this policy enforces
  in spirit: stuck = unfinished work, never proven-impossible
- [[jtbl-rodata-split-infrastructure]] — the jtbl carve-out (legitimate,
  no-C-form)
- [[gte-wrapper-misroute-park]] — the GTE leaf carve-out (legitimate,
  no-C-form)
- [[hand-coded-asm-recognition]] — the legitimate escape for
  hand-written-asm constructs
- [[register-alloc-pure-c]] — the pure-C lever playbook with instrumented-
  cc1 evidence that allocation walls ARE C-reachable
