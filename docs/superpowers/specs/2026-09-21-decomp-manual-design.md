# `decomp-manual` — the hand-driven lane, with the grinder's context

**Date:** 2026-09-21
**Status:** Approved by owner (design review 2026-09-21)
**Replaces:** the `decomp-orchestrate` skill (deleted by this work).
**Complements:** the Grinder (`tools/grinder/`, `decomp-grind` skill), which stays
the default autonomous pipeline.

## Problem

The Grinder is the right default and has earned it: 279 of 400 all-time `Match:`
commits (70%), peaking at 14–16 completions/day. But the measured shape of its
spend says it is the wrong instrument for what is left.

Evidence (measured 2026-09-21 from `git log --grep='^Match:'` and
`tmp/grind/grind.log`):

- **Sessions per grinder completion:** median 2, mean 6.1, max 126. 66% of its
  completions close in ≤2 sessions.
- **The tail eats the budget:** 19 functions taking ≥20 sessions consumed 899 of
  1,711 total sessions — **53% of all session spend for 7% of the completions.**
- **Structural overhead:** 220 of 1,482 logged sessions (15%) were discarded
  whole — invalid outcome schema or scope violation. The Round-4 audit measured
  ~0.4B tokens in 8 days buying nothing.
- **The remaining queue is all tail.** Of 94 active items, **93 are at distance
  ≥250** (median 390, max 2,989). The easy mass that produced the median-2
  statistic is gone.
- **Cold restarts re-derive.** The Round-3 audit found 3,485 body-locates, 2,087
  raw ledger reads and 1,078 raw asm reads across sessions — work already on
  disk, re-fetched because every grind session starts cold.

The modality that demonstrably closes the tail is a sustained hand-driven
session: `func_800747D8` sat 10 grinder sessions and landed on the manual path;
`sys_VSync` closed manually after 7 cold-start worker sessions thrashed. The
existing manual skill (`decomp-orchestrate`) does not serve this — it is 283
lines whose largest section documents the `headless_loop.ps1` batch path, last
actually run **2026-06-13**, superseded by the Grinder on 2026-07-06.

## Owner decisions captured in this design

Settled in the 2026-09-21 design review:

1. **Exclusive, never concurrent.** A manual session stops the Grinder, works,
   and relaunches it. One writer on `main`, ever. Rejected: concurrent lanes on
   different functions (would need worktrees, which the project deliberately
   retired and which re-arm `main_reintegration_lock`).
2. **Replace, don't accumulate.** `decomp-orchestrate` is deleted rather than
   kept alongside. Two skills both describing "work a function by hand" is a
   discovery hazard. The `headless_*` tools stay on disk, undocumented.
3. **Maximally free within the non-negotiables.** A manual session may choose its
   own approach (no modality ladder), touch files outside the function, skip the
   rigid outcome schema, and keep full context across the whole function.
4. **Shared ledger.** Manual writes `memory/grind/<func>/` — the Grinder's own
   ledger, not `memory/wip/`.
5. **`end` relaunches by default.** Guards against the pipeline sitting idle
   after a manual session ends.

## What does NOT relax

The four freedoms above are scoped by the project's standing invariants, which
this lane inherits unchanged:

- Exactly two completion states (`COMPLETED-C`, `COMPLETED-INLINE-ASM-CANONICAL`);
  no gradations, no "almost done" ([[completion-standard]]).
- The oracle is the only truth: full build+link SHA1 ==
  `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
- **A fresh adversarial layer-2 `cheat-reviewer`, default-FAIL, before every
  completion-class commit.** Non-negotiable on the manual path — the Grinder's
  Judge does not run here, so this IS the gate
  ([[review-discipline-before-commit]]).
- No cheats on `main`; INCOMPLETE stays `INCLUDE_ASM` ([[asm-until-matched]]).
- No deferral: close the popped function or bank honestly and say so
  ([[no-deferral-work-to-completion]]).

## Architecture

Two artifacts plus one deletion.

### `tools/manual_session.ps1` — the handshake

Only the dangerous, mechanical part. Two verbs.

**`begin [-Func <name>] [-DryRun]`**

1. If the Grinder is running: write the stop sentinel, then **poll until
   `tmp/grind/grind.lock` actually clears.** A clean stop lands at the next
   session boundary, which can be ~10 minutes out; the script reports elapsed
   time rather than hanging silently, and fails loudly on timeout instead of
   proceeding. It never force-kills — killing mid-session loses that session's
   ledger write.
2. Refuse to continue if the tree is dirty or the oracle is red.
3. `queue next` → the top active item, unless `-Func` overrides.
4. Print the context bundle in one shot: `engine dossier <func>`,
   `grindlib.py siblings . <func>`, `engine canonical <func>`,
   `engine sandbox <func> --disable all --diff`.
5. Write `tmp/manual/session.json`: `{func, file, grinder_was_running,
   start_head, started}`.

**`end [-NoRelaunch]`**

1. Assert the tree is clean — everything committed or reverted.
2. Bump `memory/grind/<func>/state.json` (`session_count`, floor, a prose note)
   so the Grinder resumes informed, and commit it.
3. Relaunch the Grinder if `begin` stopped it, unless `-NoRelaunch`. Run
   `drill.ps1` first **only** if `tools/grinder/`, `engine/`, or
   `tools/hooks/` changed during the session.

Order matters in `begin`: stop the Grinder **before** reading state, because
`dossier` and `queue next` mutate/measure and a live driver is writing
concurrently.

### `.claude/skills/decomp-manual/SKILL.md` — the operator manual

Target ~130 lines. Sections:

0. **Handshake** — `begin`, and why never to work over a live Grinder.
1. **The non-negotiables** — the "does not relax" list above, compressed.
2. **Read before you lever** — `sandbox --diff` first. The Round-3 audit found
   only 37% of sessions that ran `sandbox` ever looked at an instruction diff;
   257 sessions spent 834 sandbox calls mutating C against a bare integer.
   `_SsSndCrescendo` plateaued 4 sessions at floor 130 and closed in one once
   the diff was read.
3. **The loop** — iterate on `memory/grind/<func>/candidate.c`, score with
   `sandbox --candidate`. **The tree stays clean throughout**, which makes the
   "engine commands silently revert uncommitted edits" hazard
   ([[engine-queue-ops-revert-uncommitted-tree]]) structurally impossible rather
   than something to remember.
4. **The freedoms** — own approach, files outside the function, prose findings,
   sustained context. Each with its "but": the oracle still gates out-of-function
   edits; findings still get written down; long sessions still bank.
5. **Landing it** — splice into `src/<file>.c` → `verify-oracle --rebuild` →
   `reviewer_precheck.py` → fresh `cheat-reviewer` → PASS: commit +
   `queue done` + `check_completion_integrity.py`. FAIL: revert to
   `INCLUDE_ASM`, bank under `memory/grind/<func>/rejected/<slug>.c`, record why.
6. **Banking without a match** — update the ledger honestly, `end`, relaunch.
7. **Footguns** — the dirty-tree revert, `$?` unreliable under `wsl bash -c`
   (use `$LASTEXITCODE`), LF line endings on build files, PowerShell-first
   scripting, `git commit -F`.

### Deletion

`.claude/skills/decomp-orchestrate/` is removed. `CLAUDE.md`'s "Fallback: a
single focused agent, on main" paragraph is repointed at `decomp-manual`.

## Error handling

| Condition | Behaviour |
|---|---|
| Grinder won't release the lock within the timeout | Report elapsed + last log line; exit non-zero. Never force-kill. |
| Tree dirty at `begin` | Refuse; print the dirt. |
| Oracle red at `begin` | Refuse; this is an incident, not a manual-session start. |
| `tmp/manual/session.json` already exists | A prior session was interrupted; `begin` reports it and offers resume rather than clobbering. |
| Tree dirty at `end` | Refuse to relaunch — the Grinder would discard it as foreign dirt. |

## Implementation steps

1. Write `tools/manual_session.ps1` (`begin`/`end`, `-DryRun`, `-Func`,
   `-NoRelaunch`).
2. Write `.claude/skills/decomp-manual/SKILL.md`.
3. Delete `.claude/skills/decomp-orchestrate/`; repoint `CLAUDE.md`.
4. Test: `-DryRun` both verbs against a stopped Grinder; then one real
   `begin`/`end` cycle against a **running** Grinder, confirming it released the
   lock, the dossier printed, and it relaunched. Confirm the interrupted
   function's ledger survived intact.
5. Commit as `skills: decomp-manual …` per `docs/COMMIT_CONVENTIONS.md`.

## Success criteria

- `begin` against a live Grinder yields a clean tree, a released lock, and the
  full context bundle without any hand-run commands.
- A manual session's findings land in `memory/grind/<func>/` and the relaunched
  Grinder resumes from them.
- No path through the skill can commit a completion without a fresh layer-2
  reviewer verdict.
