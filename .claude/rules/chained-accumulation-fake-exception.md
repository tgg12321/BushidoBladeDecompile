---
name: chained-accumulation-fake-exception
paths: [".claude/rules/chained-accumulation-fake-exception.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)

description: "NARROW SANCTIONED FAMILY (owner ruling 2026-08-11, cff7f1f5): a chained same-variable accumulation of RELATED staged values through ONE live local, every intermediate read by the next statement, that stays UNFOLDED because combine's 2->2 split gate (combine.c:1836 reg_referenced_p on I2DEST) refuses the merge — `/* FAKE: ... */`-annotated. Dead stores, invented holders, unrelated reuse and multi-write carriers stay outside."
metadata:
  type: rule
---

# Chained same-variable accumulation — the FAKE-annotated family (owner ruling 2026-08-11)

**Owner ruling 2026-08-11** (Trenton, in person; docs/grind/decisions.md
"OWNER RULING (in person) — main (ings.c): chained same-variable accumulation
GRANTED as a sanctioned family extension", commit cff7f1f5). This document is
the family's rule doc that the grant's condition 2 requires — registered
through the independent-review path (fresh layer-2 cheat-reviewer on this
text, landed as its own commit, NOT shipped inside the match that uses it —
[[review-discipline-before-commit]]).

## What is sanctioned (exact scope — the grant's condition 4)

A run of consecutive ordinary-C assignments to ONE live local variable that
stage a single RELATED computation, where **every intermediate value is read
by the very next statement** and the final value is consumed:

```c
s32 lim = D_800A36F1;   /* real input */
lim = lim - 1;          /* each statement reads the previous value ... */
lim = lim << 8;
lim = lim + 0x80;       /* ... and the result is consumed: */
if (cnt >= lim) break;  /* threshold (n-1)*256+128 */
```

against the single-expression form `lim = ((D_800A36F1 - 1) << 8) + 0x80;`.

Why the bytes differ (the mechanism the annotation must name): every statement
reuses the SAME pseudo, so when `try_combine` tries to merge the `addiu -1`
into the `sll` and distribute (combine.c:8196 shift-plus distribution), the
resulting 2->2 split is REFUSED by the guard at combine.c:1836 — "We can't
overwrite I2DEST if its value is still used by NEWPAT" (`reg_referenced_p
(i2dest, newpat)` is true because the chain root is the same pseudo). The
3-insn path is blocked by the same guard. The chain therefore stays UNFOLDED
(`addiu -1; sll 8; addiu 0x80`), which is exactly what the target bytes carry
(asm/funcs/main.s 0x7B58-0x7B6C). Every fresh-variable spelling of the same
expression folds to `sll; addiu -128` — so the stepwise spelling plausibly
reconstructs the original source shape rather than bolting a trick onto it.

This is the split-init family ([[split-init-accumulation-sanctioned]], owner
2026-06-13; made ordinary C by [[ordinary-c-judge-decidable]] Ruling 4,
2026-09-02) extended on both of its scoped axes: a chain of MORE THAN ONE
step, and a chain that MATERIALIZES in the bytes instead of folding away.
Because it materializes, it is NOT the combine-foldable chain-extender of
[[dead-store-fake-exception]] (which requires ZERO emitted bytes) and must
not be claimed under that clause.

## Prerequisites (ALL mandatory, verified per instance)

1. **Real value, real consumer.** The input is a genuine program value and
   the final result is consumed by real code (a threshold, an index, an
   address). The chain computes something the function semantically needs.
2. **Every intermediate is read by the next statement.** No statement in the
   chain is dead; no value is overwritten unread. (A chain with an unread
   step is a dead store and belongs to [[dead-store-fake-exception]] on its
   own terms, or is simply forbidden.)
3. **One live variable, related values only.** The chain stages ONE
   computation through ONE local. Reusing the variable for an unrelated
   value, or introducing a holder local that exists only to carry the chain,
   is outside this family (invented holders remain the banned
   constant-holder / dead-scalar class).
4. **Byte-materializing evidence.** The target bytes carry the unfolded
   chain, and the session measured that the single-expression form and the
   fresh-variable forms FOLD (cite the sandbox scores or the micro-harness,
   e.g. memory/grind/main/ evidence, tmp/grind/main/s2/foldM4.c/.s). The
   WIP/ledger records that the natural single-expression and fresh-variable
   forms were tried and measured to fold BEFORE this construct was reached
   for — it is a last resort, like every FAKE family.
5. **Named mechanism in the annotation.** A `/* FAKE: ... */` comment at
   the site naming the combine 2->2 split gate (combine.c:1836
   `reg_referenced_p` on I2DEST) and stating that fresh-variable spellings
   fold — wording to that effect; a bare `/* FAKE */` does not qualify.
6. **Dual review.** Layer-1 (or the Grinder's layer-1 reviewer) and the
   default-FAIL Judge / a fresh layer-2 cheat-reviewer both rule on the
   instance; a Judge or reviewer FAIL on the merits is not overridden by this
   document.

## What stays outside (cheats-by-any-spelling, [[no-new-park-categories]])

- Dead stores and dead self-assignments (governed only by
  [[dead-store-fake-exception]] with its own prerequisites).
- Multi-WRITE carriers whose extra write is dead (banned; ordinary-c-judge-
  decidable Ruling 1 caveat).
- Invented holder locals, constant-holder / dead-scalar RA-bias locals.
- Reusing the chain variable for an UNRELATED value to bias allocation.
- Register pins, `__asm__`, volatile coercion, scheduling barriers.
- Claiming this family for a chain that combine folds to zero bytes — that
  is the chain-extender clause, with ITS zero-bytes prerequisite.

## Instances

- `main` (src/ings.c poll loop) — the grant's exhibit; first instance.

## Related

[[split-init-accumulation-sanctioned]] · [[dead-store-fake-exception]] ·
[[ordinary-c-judge-decidable]] · [[no-new-park-categories]] ·
[[review-discipline-before-commit]] · [[codegen-technique-index]]
