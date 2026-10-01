---
name: staged-value-reused-variable
paths: [".claude/rules/staged-value-reused-variable.md"]
description: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
metadata:
  type: rule
---

# Staged value through a reused variable — SANCTIONED (owner ruling 2026-07-03)

```c
/* natural (wrong instruction ORDER): */
arg5 = tbl_125c[idx_1494[1]];
/* sanctioned (same behavior, target order): */
v0 = idx_1494[1]; /* FAKE: ... */
arg5 = tbl_125c[v0];
```

`v0` is a variable the function **already has for another job**; its old value is dead at this point, and
the staged value is **real and used on the next line**. Mechanism: sched.c `adjust_priority` →
`birthing_insn_p` (`reg_n_sets[regno] == 1`) gives a once-set pseudo max "load late" priority; borrowing a
multiply-set variable turns that boost off. SOTN precedent: `src/st/*/cutscene.c` (`// fake reuse of i?`),
`src/dra/menu.c` (`j = menu->unk1D; // FAKE?`), `src/st/no0/clock_room.c`.

## The exact bounds (ALL must hold — this is a narrow exception)

1. **The value is real and used.** The staged assignment's value must be read by nearby code (typically the
   next statement). If the value is never read, this rule does NOT apply — that's the dead-store rule's
   territory with its own prerequisites.
2. **The variable already exists for a real job.** You may only borrow a variable the function genuinely
   uses elsewhere (a loop counter, a status flag, a poll result). Inventing a new variable just to have
   something to borrow is NOT this rule.
3. **The borrow is provably safe.** The variable's previous value must be dead at the staging point (it gets
   overwritten before any later read), and the staged value must not be needed after the variable's next
   real assignment. State the liveness argument in the annotation or notes.
4. **Annotate it:** `/* FAKE: <what is staged and why>, mechanism: <named compiler pass>, lever-exhaustion:
   <where documented> */`. A bare `/* FAKE */` fails review.
5. **Last resort, with receipts.** Only after the natural spellings were tried and measured (documented in
   the ledger or the commit message).
6. **Everything else still applies.** Layer-1 + layer-2 adversarial review, the oracle SHA1 gate, and the
   rest of the cheat catalog are unchanged. This rule does NOT open the door to: dead stores without their
   own rule's prerequisites, unused variables or arrays, register pins, inline asm, volatile tricks, or
   build-time byte editing.

A variable governed by a reused-variable ruling (ordinary-c-judge-decidable Rulings 5-12, Q51) may not also
claim this entry.

## Related

[[dead-store-fake-exception]] / [[named-local-fake-exception]] (2026-07-01 siblings) ·
[[defeat-licm-hoist-var-reuse]] (loop-scoped, do not cite for straight-line code) ·
[[no-new-park-categories]]
