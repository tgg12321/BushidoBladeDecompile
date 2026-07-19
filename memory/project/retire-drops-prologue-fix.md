---
name: retire-drops-prologue-fix
description: "engine.integrate.retire_function dropped ONLY regfix/asmfix rules, never prologue_fix (prologue_config.json/delay_slot_ra/frame_fix) — so a function matched in pure C but carrying a stale prologue_fix entry was refused by queue done and circuit-broke the grinder (tslPolyF4Init 2026-07-19). Fixed: retire now drops prologue_fix too, SHA1-gated."
metadata:
  type: project
---

# retire must drop prologue_fix, not just regfix/asmfix (fixed 2026-07-19)

## The gap
`engine/integrate.py: CONFIGS = [REGFIX, REGFIX2, ASMFIX]`. `retire_function`
iterated only those, so it dropped regfix/asmfix rules but left a function's
**prologue_fix** entries (`tools/prologue_config.json`,
`tools/delay_slot_ra_funcs.txt`, `tools/frame_fix_funcs.txt`) untouched.
prologue_fix is a tracked cheat (audit 2026-06-15) and `queue done` /
`check_completion_integrity.py` refuse any non-canonical function carrying one.

## The failure it caused
`tslPolyF4Init` (src/system.c) carried a stale prologue_config.json entry from a
prior completion. A grind session found a pure-C form matching WITHOUT it
(honest `sandbox --disable all` = 0 — that mode already strips prologue_fix via
`cheats._write_prologue_overrides`). The candidate path retired regfix/asmfix
(none), verify-oracle passed, the Judge PASSed the C — then `queue done` refused
on the still-present prologue entry, and the driver's "queue done refused a
judge-PASSed candidate" **Circuit-Break** halted the entire pipeline. Nothing
in the grinder could drop the prologue entry (sessions are scope-limited to
src/; retire skipped it).

## The fix
- `engine/cheats.py: drop_prologue_fix_entries(func)` — removes the func's
  entries from all three prologue configs in place; returns count.
- `engine/integrate.py: retire_function` — calls it after the regfix/asmfix
  drops, and backs up `cheats.PROLOGUE_CONFIGS` so the SHA1-mismatch rollback
  restores them. Safe by construction: a function that genuinely needs
  prologue_fix fails the rebuild and rolls back (retire returns ok:False).
- `tools/grinder/grind.ps1` — the `Match:` commit stages every retire-touchable
  config (regfix, regfix_stage2, asmfix + the 3 prologue files); a `queue done`
  refusal banks a constraint and continues instead of circuit-breaking.

## Lesson
`retire`'s contract is "delete every rule keyed by func." Any cheat class the
completion gate (`queue done` / check_completion_integrity) enforces MUST be in
retire's drop set AND in the grinder's Match-commit staging list, or a pure-C
match that clears the cheat is unfinishable and halts the driver. The
maspsx cheat-pathway gate lists ([[maspsx-gate-lists]]) are the next candidates
if a similar refusal recurs. Related: [[grinder-park-queue-dirt-deadlock]]
(same shape — a config the driver mutated but didn't commit/stage),
[[grinder-pipeline]].
