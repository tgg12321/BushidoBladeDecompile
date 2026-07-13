---
name: decomp-work
description: >-
  RETIRED 2026-07-06 — the board-claiming batch worker was part of the retired
  fleet era; use the decomp-grind skill (the Grinder) for autonomous work, or
  decomp-orchestrate for driving one function by hand. Historical reference only.
---

# BB2 Decomp worker — work a batch off the board

You are ONE worker. You take **N items of a lane** off the "BB2 Decomp" board and
drive each to **COMPLETED-C** (or block it), working in a **private git worktree**
so you never conflict with other worker agents running this same skill in parallel.

## 1. Parse the request
- **N** — the count (e.g. "work **10** …"). Default 10 if unspecified.
- **lane** — from "backlog**ged**" → `backlog` (default), "**blocked**" → `blocked`
  (revisit parked items). `decision` and `all` are also valid lanes.
- "**until completion** / to completion" — drive each item to COMPLETED-C; if one is
  genuinely unworkable, `block` it (with a reason) and move on. Either way, do N items.

## 2. Isolate yourself FIRST (this is what makes parallel workers safe)
NEVER work on `main` — other workers share it. Create your own worktree with a
**private `build/`** (so your `src/`/`regfix.txt`/`queue.json`/`build/` edits can't
clobber another worker, and theirs can't clobber you):
```
# pick a short unique id (your session id, or a timestamp)
git worktree add ../bb2-work-<id> -b work/<id>
cd ../bb2-work-<id>
pwsh tools/setup_worker_worktree.ps1     # junctions read-only deps; COPIES build/ (private)
```
`board.py` automatically reads the SHARED board state (claims, cards, queue) from the
**main** repo even when you run it here — so claims coordinate across all workers
while your file edits stay private.

## 2b. ⚠ RUN THE ENGINE VIA `tools/wteng.ps1 <id>` — NEVER relative `tools/eng.ps1`
**This is load-bearing for isolation (2026-06-14 contamination incident).** Your
PowerShell tool's cwd is ALWAYS the MAIN repo — the `cd ../bb2-work-<id>` above does
NOT persist across tool calls. So a RELATIVE `& tools/eng.ps1 ...` resolves to MAIN
and silently builds / scores / **mutates main's `src`**, corrupting BOTH your own
results (you read main, not your edits) AND other agents' work (stray edits land on
shared main). A `PreToolUse` guard now BLOCKS relative `tools/eng.ps1` / bare `make`.

**Every engine and build command goes through the worktree-pinned wrapper**, passing
YOUR worktree id (the `<id>` you chose above) so it targets your worktree regardless
of cwd:
```
& tools/wteng.ps1 <id> canonical <func>
& tools/wteng.ps1 <id> sandbox <func> --disable all
& tools/wteng.ps1 <id> retire <func>
& tools/wteng.ps1 <id> verify-oracle            # NOT --rebuild while iterating
& tools/wteng.ps1 <id> queue done <func>
& tools/wteng.ps1 <id> make                     # raw full-build SHA1 gate (the authoritative check)
```
**Edit/Write + git must ALSO stay in your worktree (cwd is main!).** Capture your
worktree's ABSOLUTE path ONCE and use it for every file + git op:
```
$wt = (Resolve-Path ../bb2-work-<id>).Path     # do this once; <id> is yours
```
- **Edit/Write** the file at `$wt\src\...` / `$wt\memory\wip\...` — NEVER the main repo
  path (`...\Bushido Blade 2 Decompile\...`). Bare relative paths and the main path both
  hit MAIN. A guard blocks edits to MAIN's build inputs while worktrees are live, but
  it can't read your intent — always edit your own `$wt\...`.
- **git** every call as `git -C $wt …` (commit/add/checkout/status). A bare `git` from the
  Bash/PowerShell tool runs on MAIN (cwd) and would commit/stage onto main.
- `board.py` IS intentionally main-shared (claims/queue coordinate there) — run it normally.

## 3. The loop — repeat until you have done N items
```
python tools/board.py next --lane <lane> --claim
```
- Returns + CLAIMS the top item NOT already claimed/done/blocked by another worker
  (so two workers never take the same function). If it reports the claim FAILED
  (someone beat you in the race), just run it again for the next item.
- Then read the briefing (everything you need is on the card):
```
python tools/board.py card <func>
```
- Drive the **engine match loop IN YOUR WORKTREE** (the canonical loop — see
  `CLAUDE.md`), **always via `& tools/wteng.ps1 <id> …`** (per §2b — relative
  `tools/eng.ps1` is BLOCKED and would hit main): `wteng <id> canonical <func>` →
  `wteng <id> sandbox <func> --disable all` (edit `src` toward distance 0) →
  `wteng <id> retire` / `wteng <id> make` → `wteng <id> queue done <func>`. Reach
  **COMPLETED-C**: zero regfix/asmfix rules, zero cheat-asm, full build SHA1 == oracle.
  The authoritative gate is **`wteng <id> make`** (`OK: bb2 matches!`) — do NOT use
  `verify-oracle --rebuild` to check an edit (it reverts your `src` to rebuild the
  reference). If the top item has a WIP checkpoint, resume from
  `memory/wip/<func>/candidate.c` as the card says.
- **Three lookups BEFORE deep iteration** (all read-only, all in your worktree):
  1. **Near-duplicate lead** — grep `tmp/duplicates_leads.txt` for the function. If
     it has an entry, the RHS is a COMPLETED-C analog; read its `src/` body as a
     starting template. (Tool: `python3 tools/find_duplicates.py` regenerates;
     SessionStart hook surfaces a lead for the queue top automatically.)
  2. **Permuter directive hint** — if you recognise the rule pattern (see
     `.claude/rules/codegen-technique-index.md`), `python3 tools/permuter_annotate.py
     --func <f> --hint <rule-slug>` writes a PERM_*-annotated candidate to
     `tmp/permuter_candidates/<f>.c`. `--list-hints` for the catalog. Closing forms
     from permuter are PROPOSALS — vet per [[no-new-park-categories]].
  3. **decomp.me corpus search** — when local levers are exhausted, `python3
     tools/decomp_me_scrape.py search --asm-file asm/funcs/<func>.s` finds scratches
     whose target asm overlaps yours (gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5).
     Coarse pre-filter; inspect the top hits for analogous C shape.
- Then record the outcome on the board:
  - **Matched (COMPLETED-C):** `python tools/board.py done <func>` and commit in your
    worktree.
  - **Genuinely stuck** (needs user canonical-asm authorization, or a documented
    plateau): checkpoint a WIP entry, then `python tools/board.py block <func>
    --reason "<one line>"` and move on.
- Count it (matched OR blocked) toward N. Stop when you reach N.

## 4. Hand off (do NOT merge to main yourself)
When you've done N items, report:
- your branch `work/<id>`, and the funcs **completed** vs **blocked**,
- that **reintegration is a separate step**: your byte-identical matches live on
  `work/<id>` and get merged to `main` afterward (by the user or `decomp-orchestrate`).
  Merging to `main` mid-run is what would cause cross-worker git races — so don't.

**Reintegration order (for whoever merges, e.g. `decomp-orchestrate`):** merge the
`work/<id>` branches → run `engine queue regen` (rebuild `engine/queue.json` from
truth; don't hand-merge it) → only THEN run `board_sync.py` (the mirror). `metrics/
events.jsonl` auto-merges (`merge=union`). `board_sync` now treats Done/archived
cards as terminal (won't revert a worker's completion even against a stale queue),
but regen-then-sync keeps the mirror accurate. (A genuinely RE-OPENED function —
rare engine regression — needs a manual un-archive, since board_sync won't resurrect
a Done card.)

Once your branch is merged, clean up safely:
```
pwsh tools/safe_remove_worktree.ps1 ../bb2-work-<id> -Force   # NEVER `git worktree remove` directly
```
(`safe_remove_worktree.ps1` detaches the dep junctions before removing — using plain
`git worktree remove` would follow them and delete main's `.venv`/toolchain.)

## Why concurrent workers don't conflict
- **Private worktree + private `build/`** → your `src`/`regfix`/`queue`/`build` work
  never touches another worker's (and the engine's `sandbox` only *reads* `build/`).
- **board.py claims** (a shared index in the main repo) → no two workers take the same
  function; `next` skips claimed/done/blocked items.
- **`board_sync` leaves In-Progress/In-Review cards alone** → your claims aren't reverted.
- Board (GitHub) ops are single-card + idempotent; the hot path (`next`/`card`) is local.

Claims are **best-effort** (a rare double-claim only costs a little duplicate effort —
fine, since every function needs decompiling). The **engine oracle gate** (`queue done`,
SHA1 == oracle) is the real completion bar — `board.py done` only reflects it on the board.

## See also
- `decomp-board` — the board CLI reference (commands, efficiency model).
- `CLAUDE.md` / `decomp-orchestrate` — the engine loop, completion standard, and the
  orchestrator that launches workers + handles review + reintegration.
- `docs/history/board.md` — full board-system reference (archived).
