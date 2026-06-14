---
name: decomp-work
description: >-
  Run ONE Bushido Blade 2 decomp WORKER agent that takes a batch of items off the
  board and drives each to completion — safe to run on several agents at once
  without conflict. Invoke with a count + lane, e.g. "work 10 backlogged items",
  "work 20 blocked items until completion", "grind 15 backlog items". The worker
  isolates itself in a private git worktree, claims items off the board so other
  workers skip them, runs the engine match loop on each, and moves the card. Use
  whenever asked to work/grind/clear a batch of board items, or to launch a worker.
  (For DRIVING multiple workers + review + reintegration, that's decomp-orchestrate.)
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
  `CLAUDE.md`): `canonical <func>` → `sandbox <func> --disable all` (edit `src`
  toward distance 0) → `retire` / `verify-oracle` → `queue done <func>`. Reach
  **COMPLETED-C**: zero regfix/asmfix rules, zero cheat-asm, full build SHA1 == oracle.
  (Engine commands run under WSL per `AGENTS.md`. If the top item has a WIP checkpoint,
  resume from `memory/wip/<func>/candidate.c` as the card says.)
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
- `docs/board.md` — full board-system reference.
