---
name: rotation-not-foreclosure
paths: ["engine/queue.py", "tools/grinder/**", "docs/grind/*.md"]
description: "Owner rulings 2026-09-08/09-26: foreclosure is RETIRED; a truly stuck function is ROTATED to the back of the worklist and returns automatically. Rotate ONLY after multiple flat sessions with the instruments run — never after one session, one layer-2 FAIL, or when close."
metadata:
  type: rules
---

# Rotation replaces foreclosure (owner rulings 2026-09-08, 2026-09-26)

Owner: *"everything has to be decompiled eventually. In my ideal world, we just work an item
until it is complete. Though I don't want an agent just looping infinitely ... making no
progress."* No function is ever parked, foreclosed, escalated, or "accepted incomplete".
Every incomplete function is ACTIVE or ROTATED.

## Ruling 1 — `rotated`, with automatic return

- An exhausted function gets status `rotated` (reason pointer, `rotated_at`). It stays in the
  worklist. Legacy `foreclosed` / `parked` / `escalated` items read as `rotated`.
- `queue auto-return` (driver, every session boundary; also `queue next` on an empty active
  list) returns items on: (a) **queue drain** (oldest rotated first); (b) **toolchain change**
  (fingerprint of CC_FLAGS, cc1, maspsx sources, gate lists, prologue_fix moves ⇒ rotated
  candidates re-measured; moved floors return); (c) **sibling movement** (a coupled ledger's
  floor drop or completion after `rotated_at`).
- Every return resets the exhaustion window (`exhaustion_base`), so it buys a full ladder.
- Record: `docs/grind/decisions.md` (`ROTATED` in the heading) + journal line. `queue unpark`
  is an early return.

## Ruling 2 — cc1psx self-disproof before exhaustion

Before declaring exhaustion the driver runs `engine cc1psx-check <func>` on the current
candidate. If cc1psx lands strictly closer than our cc1, it is a fidelity lead: record it,
force a `rederive` session, do NOT rotate. Banked in `state.json` (`cc1psx_check`) by
candidate hash.

## Ruling 3 — no rung repeats at a flat floor without a new instrument

After two flat sessions, the driver assigns the first ladder rung not yet run since the floor
went flat (including `enumerate`, `tools/spelling_enum.py`). A rung repeats only after every
rung has run in the window. Exhaustion means "every instrument ran in the flat window".

## Ruling 4 — rotate only when truly stuck across multiple sessions

Owner: *"I dont want things rotated if they are close, i only want items rotated if agents are
truly stuck and we feel we are burning multiple sessions wasting time on it."*

- Rotation needs several sessions with a flat honest floor AND the instruments run (Ruling 3).
  This binds hand-run `queue rotate` (manual lane, any agent) as fully as the driver.
- Never grounds on their own: one session without a match; one layer-2/Judge FAIL (it bans a
  construct, not the function; the objection is the next frontier); an open borderline
  question.
- **Close means stay**: a small remaining diff, or an all-operand-only diff (0 source-level
  hunks), stays at the top however the last session ended.
- Instead of rotating: bank the ledger (floor, ruled-out forms, frontier) and leave it active.
- A rotation commit states the session count and flat-floor history it rests on.

The anti-cheat wall, the frozen construct list and the default-FAIL Judge are unchanged; no
completion category is added.

Related: [[judge-sole-gate]] · [[no-compiler-divergence]] · [[asm-until-matched]]
