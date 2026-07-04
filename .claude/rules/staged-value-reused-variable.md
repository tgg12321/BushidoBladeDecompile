---
name: staged-value-reused-variable
description: SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code
paths: [".claude/rules/staged-value-reused-variable.md"]
---

# Staged value through a reused variable — SANCTIONED (owner ruling 2026-07-03)

## The plain-language version (read this first)

Sometimes the only way to make our compiler produce the exact same
machine code as the original game is to write one line of C as two:

```c
/* the natural way (compiles to the wrong instruction ORDER): */
arg5 = tbl_125c[idx_1494[1]];

/* the sanctioned way (same behavior, right instruction order): */
v0 = idx_1494[1]; /* FAKE: ... */
arg5 = tbl_125c[v0];
```

`v0` is a variable the function **already has for another job** (here it
normally holds a screen-timing result). At this exact point its old value
is finished with — nothing reads it again before it's overwritten — so
borrowing it is completely safe. The value we put in it is **real and
used on the next line**. No dead code, no do-nothing statements, no
assembly, no register pins.

Why it changes anything at all: the 1998-era compiler keeps a little
tally per variable — "how many times does this variable get assigned in
this function?" A variable assigned more than once is treated as
"ordinary" by the instruction scheduler, while a fresh one-time variable
gets a special "load me as late as possible" priority. Borrowing an
existing (multiply-assigned) variable turns that priority off, and the
compiler then lines the instructions up the way the original programmers'
compiler did. That's the whole trick: two honest lines instead of one,
chosen so the compiler's bookkeeping matches 1998.

## Why this is allowed (the evidence)

The original developers wrote code exactly like this. The SOTN
decompilation (our community gold standard) ships this precise shape in
its matched, byte-verified code:

- `src/st/{cen,lib,no3,st0,top,mar}/cutscene.c` — six files:
  `// fake reuse of i?` over `i = *scriptCur++; ... table[i]` — a value
  loaded into the pre-existing loop counter `i`, then used as an index.
- `src/dra/menu.c` (3 sites): `j = menu->unk1D; // FAKE?` — a live field
  value staged through the existing `j`, then used as an index.
- `src/st/no0/clock_room.c`: `primIndex = g_Player.status; // FAKE`.

This is a **live-code** cousin of the already-sanctioned 2026-07-01
carve-outs ([[dead-store-fake-exception]], [[named-local-fake-exception]]);
it is strictly milder than both — those allow genuinely dead writes,
this one has none.

## The exact bounds (ALL must hold — this is a narrow exception)

1. **The value is real and used.** The staged assignment's value must be
   read by nearby code (typically the next statement). If the value is
   never read, this rule does NOT apply — that's the dead-store rule's
   territory with its own prerequisites.
2. **The variable already exists for a real job.** You may only borrow a
   variable the function genuinely uses elsewhere (a loop counter, a
   status flag, a poll result). Inventing a new variable just to have
   something to borrow is NOT this rule (a fresh named intermediate is
   fine C on its own merits and needs no exception — but then it also
   won't have the "assigned more than once" property this trick needs).
3. **The borrow is provably safe.** The variable's previous value must be
   dead at the staging point (it gets overwritten before any later read),
   and the staged value must not be needed after the variable's next real
   assignment. State the liveness argument in the annotation or notes.
4. **Annotate it:** `/* FAKE: <what is staged and why>, mechanism:
   <named compiler pass>, lever-exhaustion: <where documented> */`.
   A bare `/* FAKE */` fails review.
5. **Last resort, with receipts.** Only after the natural spellings were
   tried and measured (documented in the function's WIP notes or the
   commit message). This is a matching tool, not a style.
6. **Everything else still applies.** Layer-1 + layer-2 adversarial
   review, the oracle SHA1 gate, and the rest of the cheat catalog are
   unchanged. This rule does NOT open the door to: dead stores without
   their own rule's prerequisites, unused variables or arrays, register
   pins, inline asm, volatile tricks, or build-time byte editing.

## How big is this exception?

Small. It adds exactly one ordinary assignment statement of a real,
immediately-consumed value, using a variable that already exists. A
reader unfamiliar with the project sees two plain lines of C. Among the
project's sanctioned exceptions it is the only one with zero dead code.

## Origin

marionation_Exec (src/system.c) session 2026-07-03: the staged-index form
is the only measured spelling (out of 30+ documented alternatives) that
reproduces the original instruction order of the debug_printf block
(masked distance 6 → 4). First submissions FAILed layer-1 review for
citing the wrong rule and lacking annotations; the owner sanctioned the
family after the SOTN census above. Mechanism reference: GCC 2.7.2
sched.c `adjust_priority` → `birthing_insn_p` (the "assigned once?"
check is literally `reg_n_sets[regno] == 1`).

## Related
- [[dead-store-fake-exception]] / [[named-local-fake-exception]] — the
  2026-07-01 siblings (both allow MORE than this rule)
- [[defeat-licm-hoist-var-reuse]] — variable reuse inside loops (a
  different, loop-scoped mechanism; do not cite it for straight-line code)
- [[no-new-park-categories]] — the vetting checklist that still applies
