---
name: grinder-park-queue-dirt-deadlock
description: "RESOLVED 2026-07-19 — the grinder owner-gated park path omitted engine/queue.json from its commit, so the park stayed as working-tree dirt, the next session's scope check reverted it (engine/ not in AllowedDirtyPattern), and the function bounced back to the queue top forever (~40 sessions burned on motion_SetMotion 2026-07-18). Fix: stage engine/queue.json in the park commit."
metadata:
  type: project
---

# Grinder owner-gated park deadlock (RESOLVED 2026-07-19)

## Symptom
`motion_SetMotion` accumulated 44+ grind sessions with the honest floor pinned
at 10 the entire time, and ~25 identical `grind: motion_SetMotion parked
owner-gated pending ruling` commits — the function never left the queue top
despite the owner-gated park mechanism firing repeatedly.

## Root cause
The `owner-gated` branch in `tools/grinder/grind.ps1` ran `queue park <func>`
(which writes the parked status into `engine/queue.json`) but then staged only
`memory/grind docs/grind metrics/events.jsonl` for the commit — **not
`engine/queue.json`**. So:

1. `queue park` marks the function parked in the working tree.
2. The commit omits `engine/queue.json` → the park stays as uncommitted dirt.
3. The next session's scope check (`grind.ps1` step 5) sees `engine/queue.json`
   dirty; `engine/` is **not** in `$AllowedDirtyPattern`
   (`memory/grind/|docs/grind/|tmp/|metrics/events.jsonl|src/|include/`), so it
   is a SCOPE VIOLATION → `git checkout -- .` reverts the park.
4. The function is `active` again, still the queue top → grind → owner-gated →
   park → reverted → repeat, indefinitely.

The `Match:`/`queue done` path never hit this because it always staged
`engine/queue.json` explicitly (grind.ps1 ~line 243).

## Fix
Stage `engine/queue.json` in the owner-gated park commit, mirroring the done
path. Committed 2026-07-19. Compounding factor that hid it: the fable usage
limit ([429 backoff](#)) meant the wasted sessions were also interleaved with
spawn-failure retries, so the log looked like "usage limit" rather than "park
loop" until the 50-commit ledger history was read.

## Lesson (generalizes)
Any grinder path that mutates `engine/queue.json` via an engine command MUST
commit that file in the same step. `engine/` is deliberately outside the
allowed-dirty surface (it is not session scratch), so leftover queue.json dirt
is always reverted by the next scope check — silently. When adding a new
disposition path to the driver, stage every tracked file the engine command
touched, not just the ledger dirs. See [[grinder-pipeline]] and
[[grinder-clobbers-uncommitted-edits]].
