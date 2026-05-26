# Archived: the named-recipe library (the "fingerprinted techniques" system)

Archived 2026-05-26 alongside the rest of the `dc.sh` workflow.

## What this was

These 16 JSON files were the project's **named-recipe library** — the
"fingerprinted techniques" system. Each recipe captured a recurring matching
pattern as:

- a `fingerprint` block — `structural` signals plus a `blocker_class`, used to
  recognize when the recipe applied to a new function;
- a `rule_skeleton` — the concrete transform (C restructure ops and/or
  regfix-rule shapes);
- `notes` + provenance (`from_memory` / `from_commit` / `proof`).

The intended loop was: finish a match → `capture_recipe.py` classified the
commit's diff and either confirmed an existing recipe or drafted a new JSON →
a future similar function found it via `recipes.py` (`dc.sh recipes <func>`)
and applied it via `dc.sh apply-recipe <recipe> <func>`.

The two tools that produced and consumed these files are archived next to this
directory:

- `../capture_recipe.py` — the registrar (classify a match → confirm/draft)
- `../recipes.py` — the matcher (list / suggest / apply)

## Why it was archived

The `dc.sh`-driven, skill-based loop was replaced by the **greenfield engine**
(`engine/`, `python3 -m engine.cli`). The engine does not read or write
recipes — it routes C-vs-asm, measures the cheat-free distance, and gates on a
byte+link SHA1 match. With `dc.sh` retired, `capture_recipe.py` (the only
producer) and `recipes.py` (the only consumer) went with it, leaving this
library orphaned: data with no producer and no consumer. The files were kept in
`tools/recipes/` for a while and became a source of confusion — they *looked*
live. Archiving them removes that ambiguity.

Most recipes here also encode a `regfix`-rule `rule_skeleton`. Under the
engine, regfix is a cheat-invisible substrate: the sandbox strips those rules
before scoring, so a recipe that "applies regfix rules" cannot move the honest
pure-C distance. The mechanical-replay value these recipes had during the
`dc.sh` cleanup phase does not carry over.

## Where the live equivalent is

The **technique knowledge** these recipes captured was not lost — the durable
part migrated to two live homes:

1. **`.claude/rules/*.md`** — path-scoped technique docs (e.g.
   `shared-end-label.md`, `gte-3x3.md`, `scratchpad-gte.md`,
   `packed-multiply-cluster.md`, `dead-branch-scheduling.md`). They auto-load
   when an agent reads a matching source file (per `paths:` frontmatter), and
   the metrics layer fingerprints each as a technique `slug`. This is the live
   registry; new findings are registered here at match-finish (see the "Register
   findings" step in `CLAUDE.md`).
2. **`docs/MATCHING.md`** — the symptom-indexed reference playbook still
   documents these patterns (LICM unhoist, call-loop family, early-exit alias
   breaker, varargs prologue, nested-bool memcard, delay-slot fill, label-shift,
   hoist-removal) as reference knowledge.

If you are looking for "how do I handle pattern X," read those — not these
JSONs.
