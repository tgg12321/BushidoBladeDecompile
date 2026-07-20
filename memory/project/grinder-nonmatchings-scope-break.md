---
name: grinder-nonmatchings-scope-break
description: "decomp-permuter import.py makes nonmatchings/ relative to cwd; a grind permuter session running it from repo root created a root-level nonmatchings/ the root .gitignore missed, so the driver scope check discarded every session → circuit-break (func_800611A4 2026-07-19). Fixed: gitignore /nonmatchings/ + driver sweeps it each session boundary."
metadata:
  type: project
---

# Stray root nonmatchings/ circuit-broke the grinder (fixed 2026-07-19)

## Mechanism
`tools/decomp-permuter/import.py:224` runs `os.makedirs("nonmatchings/")`
relative to the current directory. Run from `tools/decomp-permuter/` it is
harmless (that dir gitignores its own `nonmatchings/`). A grind **permuter**
session invoked it from the repo ROOT, creating a root-level `nonmatchings/`.
The root `.gitignore` did not cover it → `git status --porcelain` showed
`?? nonmatchings/` → the driver's scope check (`AllowedDirtyPattern` =
`memory/grind/|docs/grind/|tmp/|metrics/events.jsonl|src/|include/`) treated it
as out-of-surface dirt and discarded the session. Three permuter respawns each
recreated it → 3 consecutive invalid sessions → circuit-break.

## Fix
- `.gitignore`: `/nonmatchings/` — never committed; now invisible to the scope
  check no matter where the permuter runs.
- `tools/grinder/grind.ps1`: `Reap-PermuterOrphans` (every session boundary)
  removes any stray root `nonmatchings/` so it can't accumulate.

## Lesson
Any tool a grind session shells out to that writes **relative to cwd** can drop
an artifact outside the allowed scope surface and silently discard the session.
The durable defenses are (1) gitignore the artifact so the scope check never
sees it, and (2) sweep it at the session boundary. Same family as
[[grinder-park-queue-dirt-deadlock]] and [[retire-drops-prologue-fix]] — a file
the driver didn't expect in the tree. Related: [[grinder-pipeline]],
[[permuter-fresh-seed-discipline]] (permuter artifacts belong under
tmp/grind/<func>/sN/).
