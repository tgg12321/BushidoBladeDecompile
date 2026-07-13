# BB2 Decomp board (GitHub Projects v2)

`tools/board_sync.py` projects the engine's worklist onto a GitHub Project named
**BB2 Decomp** (`https://github.com/users/tgg12321/projects/<number>`).

- The board is a **one-way mirror** of `engine/queue.json` (+ `memory/wip/`).
  It never writes back to engine state and never deletes items.
- Columns map 1:1 to engine status: `active`→Backlog, `authorize`→Needs-Decision,
  `parked`→Blocked, completed→archived Done.
- On the current queue the live run produces Backlog + Blocked cards only;
  Needs-Decision (and the Phase-2 In-Progress/In-Review) columns will be empty for
  now — that's expected, not a bug.

## Usage (Windows-side; needs `gh` authed with the `project` scope)

    python tools/board_sync.py --dry-run     # preview the change set
    python tools/board_sync.py               # sync the active cards
    python tools/board_sync.py --seed-done   # one-time: backfill completed funcs as archived Done

`--dry-run` is fully read-only — it looks up the project without creating it, makes
no item mutations, and if the project doesn't exist yet it reports what it would
create. Nothing is written to GitHub on a dry run.

`--seed-done` reads `build/bb2.elf` + `build/bb2.map` to enumerate completed
functions via the ELF symbol table; a fresh build must be present before running it.

Re-running with no engine changes makes zero mutations (idempotent). Run it after
`queue regen` / `queue done` to refresh the board.

## Re-run recovery

A failed or interrupted sync is safe to re-run. `apply` creates a draft card, then
sets its fields, then archives if needed, as separate API calls; if a run is
interrupted mid-card, the next sync detects the card by its title and completes the
missing fields — no duplicates, no data loss. So if you see a partially-populated
card during a long run (especially the one-time `--seed-done` backfill of ~1024
archived cards), just re-run `python tools/board_sync.py` rather than deleting
anything by hand.

## Tests

    python tools/test_board_sync.py          # exit 0 = pass (no network; gh is stubbed)

## Rich card bodies (the per-function agent briefing)

Every card body is a self-contained one-stop-shop for an agent working that
function: status + next step, identified cheats/rules (with explanations), the
WIP checkpoint (floor/gap/resume pointer), techniques, decomp history (commits),
the current C source, and the target disassembly. Two tools build/push them:

- **`tools/board_cards.py`** — assembles each function's body from repo data
  (objdump of `build/bb2.elf` for the target asm; the engine brace-matcher for
  the current C; `regfix.txt`/`asmfix.txt` for cheats; one `git log --all` pass
  for history; `memory/wip/<func>/` for the checkpoint) and writes
  `tmp/cards/<func>.md` for all ~1472 board functions. Local, fast, no network.
  - `python tools/board_cards.py` — assemble all. `python tools/board_cards.py <func>` — print one.
  - Bodies are budgeted to ≤65000 chars (GitHub's hard limit is 65536); the two
    ~2900-instruction giants are asm-truncated with a note.
- **`tools/board_enrich.py`** — pushes the `tmp/cards/<func>.md` bodies onto the
  board: updates active cards' bodies, and creates the ~1024 completed functions
  as archived Done cards **with their body in the create call**. Rate-limit-aware
  and resumable.
  - `python tools/board_enrich.py` — full run. `--dry-run` previews (read-only).
    `--limit N` caps content-writes (chunked runs). `--only-active` / `--only-done`.

### Regenerating cards
When functions change (matched, parked, new WIP, new commits), refresh:
`python tools/board_cards.py` then `python tools/board_enrich.py`. The enrich run
only rewrites cards whose body actually changed (idempotent).

### Why it's a multi-hour, resumable grind (rate limits)
GitHub's secondary limit caps **content-creating writes at ~500/hour** (a draft
body update / create each counts). The full population is ~1469 content-writes, so
it spans several hours. `board_enrich` paces ~8s/write, and when the GraphQL budget
runs low it **sleeps until the rate-limit reset** and continues — one unattended
run grinds through multiple windows.

### The Done-card ledger (load-bearing gotcha)
GitHub's `items(first:N)` does **not** reliably return *archived* items, so once a
Done card is archived it's invisible to the board read. `board_enrich` therefore
records every created Done card in a **local ledger**
`tmp/board_enrich_done_ledger.json` (gitignored) and uses *that* — not the API —
to know which Done cards exist. This is what makes resume safe: without it, every
restart would re-create all ~1024 Done cards as duplicates. The ledger is written
*before* finalize (Status=Done + archive), so a crash mid-create still records the
card and the next run finalizes it via the stored id instead of duplicating.
**Keep `tmp/` between resume runs** on the same machine (the ledger lives there).

`board_enrich` never deletes board items (only add / update body / archive).

## Phases 2-5

This is Phase 1 (visibility) + the rich-card enrichment above. Claim/lease (git
branches), worktree isolation, PR reintegration, and the multi-agent entry
protocol land in later phases — see
`docs/superpowers/specs/2026-06-13-decomp-kanban-board-design.md`.
