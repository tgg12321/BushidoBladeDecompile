---
name: named-local-fake-exception
paths: [".claude/rules/named-local-fake-exception.md"]
description: "SANCTIONED LAST-RESORT (owner 2026-07-01): constant-holder locals and dead/unused SCALAR locals biasing RA, with /* FAKE */ + lever exhaustion. Arrays, frame coercion, dummy-subscript locals stay forbidden."
metadata:
  type: rule
---

# Constant-holder / dead scalar locals — the FAKE-annotated exception

**Owner ruling 2026-07-01** (evidence: `pre-slim-2026-10-01:memory/project/sotn-family-research-2026-07-01.md`): a local variable whose only
purpose is codegen influence — holding a constant in a register across calls/statements, or existing as a
declaration that biases register allocation — is a sanctioned last-resort matching lever under the
prerequisites below.

## What is sanctioned (exact scope)

- **Constant-holder local:** `s32 k = 1; f(); g_a = k; ...; g_b = k;` — a local initialized to a constant,
  kept live across intervening calls so the constant sits in a callee-save register instead of being
  re-materialized.
- **Dead / unused SCALAR local declaration** whose presence shifts RA (`int new_var;`).
- **m2c-artifact / generic names** (`new_var`, `temp`) are acceptable AS NAMES when the construct itself
  passes; prefer a meaningful name, but the name alone is not a FAIL trigger.

SOTN precedent: `src/dra/84B88.c:817-860` (`s16 three = 3;`), `src/dra/7879C.c:2067`
(`s32 zero = 0; // needed for PSP`), `src/st/cat/e_hellfire_beast.c:819-823` (`fake = 8;`),
`src/dra/cd.c:520-552` (`new_var2 = 6`).

## Strict prerequisites (cheat-reviewer FAILs if any is missing)

1. **Lever-exhaustion is documented** (ledger / commit body): the natural inline-literal / direct-expression
   forms and the [[register-alloc-pure-c]] A/B/C levers were measured-negative first.
2. **The GCC-pass interaction is named** (e.g. "keeps the constant in a callee-save across the jal instead
   of re-materializing `li 1`").
3. **Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` at the declaration (or
   the holding assignment).
4. **Layer-1 + layer-2 cheat-reviewer** per [[review-discipline-before-commit]].

## Q27 extension (owner 2026-09-28) — always-zero narrow frame locals and per-branch constant holders

- **(A) Always-zero narrow frame local.** A SCALAR local of a narrow integer type (`s16`/`u16`/`s8`/`u8`)
  whose every write is the literal 0, read as an ordinary value, whose purpose is combine's fold of its
  sign/zero extension (combine plants a `(use (reg))` of the dead extension temp; reload gives it a frame
  slot no instruction touches).
- **(B) Per-branch constant holder.** A SCALAR local written a constant in each arm of a branch and read only
  inside that arm, whose purpose is loop.c's movable matching (the constant is not hoisted, stays loaded
  in each arm).
- **Conditions (all required):** (1) exact target proof — the target shows the effect and the candidate
  reproduces it exactly (for (A): number/positions of untouched frame slots, zero instruction-count cost;
  for (B): constant loaded in-arm, loop.c movable arithmetic recorded); a lower score alone is not proof;
  (2) ordinary forms and the other mechanisms measured-negative on record; (3) `/* FAKE: ... */` at every
  such declaration naming the mechanism and pointing at the ledger receipts; (4) layer-1 + layer-2.
- **Not extended:** arrays, pads, locals whose mechanism is frame reservation, non-narrow types for (A),
  any always-zero local read as an array subscript or pointer offset (Q22 below). A (B) local is still not
  a Ruling 11 value. Record: docs/grind/decisions.md 2026-09-28 OWNER RULING — always-zero narrow frame
  locals and per-branch constant holders.

## What stays FORBIDDEN

- **Unused ARRAYS / oversized arrays / frame-size coercion** — [[dead-vars-local-array]] unchanged.
- **Address-coerced locals** (`(void)&local;`) — unchanged.
- **Register pins, asm injection, barriers** — unchanged. Other spellings need their own evidence
  ([[no-new-park-categories]]).
- **Dummy constant locals as array subscripts / pointer offsets (owner ruling Q22, 2026-09-27).** A DUMMY
  local is one whose value at EVERY read is fixed at compile time (the same on every feasible path to that
  read; values may differ between reads; reads include its own `i++`/`i += 1`), whatever its name,
  annotation or write spelling (`s32 zero = 0;`, `one - 1`, a stepped cursor whose every read is fixed).
  A read of a dummy local that is an array subscript or pointer offset, alone or inside a larger index or
  offset expression (`a[k]`, `a[i + k]`, `&a[k]`, `*(a + k)`, `p + k`), is REFUSED — whatever the local's
  other reads, and whether or not it is admitted for another mechanism; spell it with the literal. A
  "pointer offset" is an integer added to/subtracted from a pointer- or array-typed operand; integer-typed
  address arithmetic (`s32 base2; ... base2 + tail`) is outside the refusal. No FAKE-local family admits
  such a read, and it cannot be the named mechanism of any family. For the per-file-declaration exception
  ([[no-new-park-categories]]), a single-declaration spelling relying on it is set aside as refused. A local
  with at least one read whose value is chosen at run time (differs between feasible paths/executions,
  e.g. PsyQ SetDrawEnv's packet cursor) is not a dummy local; a local fixed at its subscript reads and later
  given an unrelated run-time value is judged under the reused-variable rulings. Record:
  docs/grind/decisions.md 2026-09-27 OWNER RULING — dummy constant locals as array subscripts.

## Related

[[dead-store-fake-exception]] · [[pointer-alias-fake-exception]] · [[mmio-volatile-type-level]] ·
[[loop-rotation-two-shift]] (the opaque-`one` sanction this generalizes from)
