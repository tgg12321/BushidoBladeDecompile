# BB2 Decomp board (GitHub Projects v2)

`tools/board_sync.py` projects the engine's worklist onto a GitHub Project named
**BB2 Decomp** (`https://github.com/users/tgg12321/projects/<number>`).

- The board is a **one-way mirror** of `engine/queue.json` (+ `memory/wip/`).
  It never writes back to engine state and never deletes items.
- Columns map 1:1 to engine status: `active`→Backlog, `authorize`→Needs-Decision,
  `parked`→Blocked, completed→archived Done.

## Usage (Windows-side; needs `gh` authed with the `project` scope)

    python tools/board_sync.py --dry-run     # preview the change set
    python tools/board_sync.py               # sync the active cards
    python tools/board_sync.py --seed-done   # one-time: backfill completed funcs as archived Done

`--dry-run` skips all item mutations (no cards are added/updated/archived), but it
**does** create the "BB2 Decomp" project and its custom fields if they don't exist
yet, because that setup is idempotent. So the very first `--dry-run` materializes
an empty project + fields, then reports the planned (un-applied) actions.

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

## Phases 2-5

This is Phase 1 (visibility). Claim/lease (git branches), worktree isolation,
PR reintegration, and the multi-agent entry protocol land in later phases — see
`docs/superpowers/specs/2026-06-13-decomp-kanban-board-design.md`.
