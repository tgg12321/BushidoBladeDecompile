---
name: rotation-not-foreclosure
paths: ["engine/queue.py", "tools/grinder/**", ".claude/rules/*.md", "docs/grind/*.md"]
description: "Owner ruling 2026-09-08: the foreclosed state is RETIRED. A flat-floor function is ROTATED to the back of the active worklist, never to a terminal bucket, and returns automatically (queue drain, toolchain change, sibling movement). Before any rotation the driver runs the cc1psx self-disproof and every ladder instrument must have run once in the flat window (no modality repeats at the same floor without a new instrument). Everything gets decompiled; nothing loops without progress."
metadata:
  type: rules
---

# Owner ruling 2026-09-08 — rotation replaces foreclosure

Owner (Trenton), verbatim, 2026-09-08:

> "I don't want the endgame lock to fire earlier, kind of the opposite. I
> want little to no foreclosures at all. It's just delaying the inevitable,
> everything has to be decompiled eventually. In my ideal world, we just work
> an item until it is complete. Though I don't want an agent just looping
> infinitely and eating tokens overnight, making no progress because it feels
> it is deadlocked by policies somehow."

Approved as a package ("Alright go ahead with all your recommendations on
this") after the operator laid out the evidence below.

## The evidence the ruling rests on

1. **Foreclosure was never terminal in practice.** Of 36 functions the
   journal ever foreclosed, 26 later reached COMPLETED-C (median 5 sessions
   after the first foreclosure). What unblocked them was almost never another
   session of the same ladder: it was a systemic change (the `-msoft-float`
   adoption closed four in one evening), an owner unpark with new guidance,
   or a sibling's solution. Foreclosure had been functioning as
   *rotate-to-back-until-the-world-changes* under a terminal-sounding name,
   with the return step manual.
2. **"Deadlocked by policy" was a misdiagnosis at least once.**
   func_8002D780 was foreclosed at floor 2/202 with a record saying only a
   policy grant or a toolchain change could reopen it. The original PsyQ
   compiler (cc1psx), which no session had run in 14 sessions, produces the
   same residual order from the same source: the gap is a SPELLING not yet
   found, and a pure-C preimage exists by construction (the original was C).
   A policy exit gives a hard search an incentive to declare exhaustion.
3. **Flat sessions repeated instruments.** At floor 2 the ladder re-ran
   synthesis, forensics and rederive twice each with no new tool; the
   diagnosis sharpened but the floor did not move. Half of one night's
   tokens went to two functions that ended foreclosed.

## Ruling 1 — `rotated` replaces `foreclosed`; the return is automatic

- The queue status `foreclosed` is retired. A function whose ladder is
  exhausted is **ROTATED**: status `rotated`, reason pointer, `rotated_at`
  stamp. It stays in the worklist; it is never "done", "accepted incomplete",
  or parked. Legacy `foreclosed` / `parked` / `escalated` items are read as
  `rotated`.
- **Automatic return** (`queue auto-return`, run by the driver at every
  session boundary; also `queue next` when the active list is empty):
  (a) **queue drain** — when no active item remains, the oldest rotated item
      returns to active;
  (b) **toolchain change** — when the toolchain fingerprint (CC_FLAGS lines,
      cc1 binary, maspsx sources, the maspsx gate lists, prologue_fix) moves,
      every rotated candidate is re-measured; any whose honest floor moved
      returns with the new floor recorded;
  (c) **sibling movement** — a coupled ledger's floor drop or completion
      stamped after `rotated_at` returns the item (the transplant session
      is forced, as for active siblings).
- Every return resets the exhaustion window (`exhaustion_base`), exactly as
  an owner unpark does, so a return buys a full fresh ladder, never one
  session.
- The record is `docs/grind/decisions.md` (`ROTATED` in the heading) plus the
  journal line; no owner question, no packet. An owner `queue unpark` still
  works and is now just an early return.

## Ruling 2 — the cc1psx self-disproof is mandatory before rotation

Before the driver may declare exhaustion (assign `escalation` modality) it
runs the original compiler on the current candidate out of tree
(`engine cc1psx-check <func>`): if cc1psx lands strictly closer to the target
than our cc1, the residual is a compiler-fidelity lead, not a spelling
problem — the driver records it in the ledger, constrains the next session to
the fidelity investigation (`rederive`), and does NOT rotate. The result is
banked in `state.json` (`cc1psx_check`) keyed by the candidate's hash, so it
reruns only when the candidate changes.

## Ruling 3 — no modality repeats at the same floor without a new instrument

Once the honest floor has been flat for two sessions, the driver assigns the
first ladder rung that has NOT run since the floor went flat, in ladder
order. A rung may repeat at the same floor only when every rung has run in
the window. The `enumerate` rung (`tools/spelling_enum.py` — exhaustive
name/inline × declaration-order × operand-order sweep of a marked region,
scored with the engine's metric) joins the ladder so that a residual that
lives in one block is searched systematically before any human-style
re-derivation repeats. Exhaustion therefore means "every instrument ran in
the flat window", not "eight sessions passed".

## What this does NOT change

The anti-cheat wall is untouched: no pins, no asm outside the canonical
grant, no scheduling barriers, the frozen construct list, the default-FAIL
Judge, the two endgame-lock AND-gates as the STANDARD (they now decide the
record's wording, not whether the function leaves the worklist). No new
completion category exists. "Difficult" is still not "impossible"
([[difficult-not-impossible]]); this ruling makes the pipeline act on it.

Supersedes the disposition shape in [[escalation-not-parked]] and the
`foreclosed` mechanics in [[ordinary-c-judge-decidable]] § exhaustion; the
2026-07-27 standing ruling and its 2026-09-02 amendment survive as the
wording rule for the rotation record.
