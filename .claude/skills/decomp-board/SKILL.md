---
name: decomp-board
description: >-
  Work the Bushido Blade 2 (SLUS-00663) decomp via the "BB2 Decomp" GitHub
  Projects board treated as a QUEUE — pull the top function, read its one-stop-shop
  card briefing, claim it, and move it through In-Progress → Done, all with
  single-card operations (NEVER pulling the full card list). Use whenever an agent
  should take the next function off the board, fetch a function's card info, or
  claim / move / complete / block a card. Pairs with the engine loop (which still
  owns the actual match + oracle gate).
---

# BB2 Decomp board — agent queue interface

The **GitHub board is a queue.** Every remaining function needs decompiling, so
there is no prioritization to do: **pull the top item and work it.** The engine's
`engine/queue.json` is the ordered (easiest-first by honest pure-C distance)
source of truth; the board mirrors it. `tools/board.py` is your efficient front
door — all commands are Windows-side (`python tools/board.py …`).

## The one hard rule: NEVER pull the full card list
The board has 448 active + 1024 archived cards. Listing them all wastes tokens and
API budget. So the tool is built so you never have to:
- **`next` / `card` / `status` are 100% LOCAL** (zero API) — they read
  `engine/queue.json` and `tmp/cards/<func>.md`.
- **Moves** (`claim` / `done` / `block` / `release`) are single-card mutations via
  a cached func→id index (`tmp/board_index.json`).
- The ONLY command that reads the whole board is `board.py index --rebuild` — run
  it once, and again only after large board changes.

## The loop
```
python tools/board.py next             # the top function to work (local, instant)
python tools/board.py card <func>      # that function's FULL briefing (local)
python tools/board.py claim <func>     # → In-Progress, so other agents skip it
   … decompile it via the ENGINE loop …
python tools/board.py done <func>      # match verified → card to Done + archive
```
- `python tools/board.py next --claim` — pull the top AND claim it in one step.
- Stuck / needs human canonical-asm authorization → `board.py block <func> --reason "…"`.
- Abandon a claim → `board.py release <func>`.

## What a card briefing gives you (the one-stop shop)
`board.py card <func>` prints everything needed to start, so you rarely need
anything else first:
- status + a one-line brief + the concrete **next step**
- **identified cheats / regfix+asmfix rules**, each explained
- the **WIP checkpoint** (candidate vs head floor, remaining gap, "resume from
  `memory/wip/<func>/candidate.c`")
- **techniques** (relevant `.claude/rules` slugs)
- **decomp history** (the function's commits, newest first)
- collapsible **current C source** and **target disassembly**

## Commands
| Command | What it does | Cost |
|---|---|---|
| `board.py next [--claim] [--json]` | top available function (skips claimed/parked) | local, ~0 |
| `board.py card <func> [--refresh]` | one function's full briefing | local (`--refresh` = cheap single-card regen) |
| `board.py claim <func>` | → In-Progress + record claim | 1 mutation |
| `board.py done <func>` | → Done + archive (board side of completion) | 2 mutations |
| `board.py block <func> --reason "…"` | → Blocked | 1 mutation |
| `board.py release <func>` | drop claim, → Backlog | 1 mutation |
| `board.py status [--json]` | queue counts + current top | local, ~0 |
| `board.py index --rebuild` | refresh the func→card-id cache (only full read) | one full read |

## The board reflects completion; the ENGINE gates it
`board.py done` only moves the card — it does NOT build or verify. A function is
truly **COMPLETED-C** only when the engine's oracle gate passes. The real match is:
`canonical <func>` → `sandbox <func> --disable all` (edit `src` toward 0) →
`retire`/`verify-oracle` → `queue done <func>` (full build SHA1 == oracle). Run
`board.py done <func>` AFTER `queue done` succeeds, to reflect it on the board.
(See `CLAUDE.md` for the engine loop and the completion standard.)

## Multi-agent
`claim` / `next --claim` move a card to In-Progress and record it in
`tmp/board_index.json` so other agents' `next` skips it. This is a **best-effort
coordination hint** (non-atomic, last-writer-wins on a race), not a hard lock —
acceptable because every function needs doing anyway, so an occasional
double-pick only costs a little duplicate effort. `board_sync` will not revert an
In-Progress card, so your claim survives a board re-mirror.

## Maintenance (NOT the hot path)
- Refresh card briefings after decomp state changes (matches, parks, new WIP/commits):
  `python tools/board_cards.py` then `python tools/board_enrich.py` (idempotent —
  only changed cards re-upload).
- Rebuild the id/claim index after large board changes: `board.py index --rebuild`.
- Full board-system reference (sync / cards / enrich / ledger / rate limits):
  `docs/board.md`.
