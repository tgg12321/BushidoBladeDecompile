---
name: foundation-session
description: >-
  Run one bounded foundation session on BB2 in its own lane worktree: pins, bss, tables, data,
  shift, hygiene, fake, assets or trace (docs/foundation/backlog.tsv). Opens the lane, does at
  most three units, lands each through tools/lanes/lanes.py (gated, fast-forward only), and stops.
  Use when asked to run a foundation lane, work the moddable-foundation backlog, remove linker
  pins, link BSS, type data, add the shift test, or continue any of those lanes. Argument: the lane.
---

# BB2 foundation: one bounded lane session

Invocation: `/foundation-session <lane>` (for example `/foundation-session pins`). The lane is a
`lane` value in `docs/foundation/backlog.tsv`. With no argument, run
`python tools/lanes/lanes.py status`, pick the first lane whose rows are not all landed and that no
live session owns, and say which one you took.

## The lane model (read once)

- You own **one** worktree: `../bb2-worktrees/ln-<lane>`, branch `lanes/<lane>`. Every edit, build
  and commit happens there. Main is only a landing pad. `tools/hooks/lane_guard.py` blocks edits to
  main, edits to other lanes, and unpinned `git` mutations.
- Subagents start with their cwd in **main**. Brief every subagent with the absolute worktree path,
  and have it use absolute paths, `git -C "<wt>"`, and `& tools/wteng.ps1 "<wt>" <cmd>`.
- Claims keep lanes apart. `path:<glob>` reserves files. A shared file (headers, linker scripts,
  backlogs, census) can only be claimed as a freeze (`path:named_syms.txt!`), and that blocks every
  other lane's landing on it, so hold a freeze only while a unit needs it. `topic:<text>` reserves
  work (for example `topic:data:8008D120-8008E000`).
- `lanes.py land` replays your commits onto the current main in a private landing worktree, runs the gates
  (oracle SHA1 rebuild, tus-check, integrity, engine test), regenerates the census, fast-forwards
  main and pushes. The gates run without the main lock; the lock covers only the final check and
  fast-forward. A land takes many minutes (full rebuild + engine test), so run it with
  `run_in_background: true` and wait for the completion notice, never under a short tool timeout.
  If a land was interrupted anyway, `lanes.py recover <lane>` (the next `land` also does it).
  You may keep working in the lane meanwhile; commits made during it land next time (if the
  branch could not be moved because of uncommitted edits, just land again: already-landed
  commits are skipped). Exit codes: 3 bounce (conflict: `lanes.py drop <lane> <sha>` removes
  only the bounced commit and keeps later ones; redo that unit on the new main. Use
  `fresh --discard` only to throw away everything unlanded), 4 gate failed, 5 main
  moved but the push failed (retry with `python tools/lanes/lanes.py push`; tell the user if that
  fails too).
  Never hand-merge into main.
- If a commit is exactly the output of a deterministic script, add a `Lane-Replay: <shell command>`
  trailer (run at the worktree root in WSL with the venv active). On a conflict, `land` re-runs the
  command instead of bouncing. Don't use the trailer on hand edits.

## Shape of a session

1. `python tools/lanes/lanes.py open <lane> --kind foundation`. It prints the worktree path.
   `python tools/lanes/lanes.py fresh <lane>` to start from current main (it refuses if unlanded
   commits exist: land those first).
2. Read the lane's rows in `docs/foundation/backlog.tsv` (columns `id lane stage claims note`;
   stages `idea` -> `active` -> `landed`, or `dropped` with the reason in `note`). Take the first
   `active` row, else the first `idea` row.
3. Do at most **3 units**. A unit is one row, or one TU / one range of a per-TU row such as
   `fake-reduce`, `hygiene-w` or `data-split`. Each unit:
   - `lanes.py claim <lane> <the row's claims> [topic:row:<id>]`.
   - Do the work in the worktree. Prove it: SHA1 == oracle via
     `& tools/wteng.ps1 "<wt>" verify-oracle --rebuild` (commit first; it refuses dirty build
     inputs), plus whatever check the row's note asks for (for example map addresses
     unchanged for `pins-*`).
   - Review before landing:
     - Changes to function bodies (`fake-reduce`, `hygiene-w` edits inside functions) are
       completion-class: a fresh `cheat-reviewer` PASS and a layer-2 record on each body
       (`.claude/rules/completion-bar.md`).
     - Everything else gets one fresh adversarial `general-purpose` reviewer (default FAIL). It
       checks that the change does what the row says, keeps bytes identical, and hides no
       address assumption.
   - Write the message to main's scratch dir and commit with an absolute path:
     `git -C "<wt>" commit -F "<main>\tmp\<msg>.txt"`. A relative `tmp/...` resolves inside
     `<wt>` under `-C` (conventions:
     `docs/COMMIT_CONVENTIONS.md`: `build:`, `data:`, `tools:`, `cheat-cleanup:`, or
     `<tu>.c: <action>`). In the same or a following commit, update the row's
     `stage`/`note` in `docs/foundation/backlog.tsv` (backlog rows merge row by row on land).
   - `python tools/lanes/lanes.py land <lane>`, then `lanes.py unclaim <lane>` for the unit's
     claims. That is the stopping point.
4. **Stop** after 3 units, or earlier if context passes ~300k or a unit fails twice. Before calling
   the session ended, every lane commit must be on main (`lanes.py status` shows `unlanded=0`;
   land or drop what is left), then run `python tools/lanes/lanes.py close <lane> --remove`. It
   removes the lane and landing worktrees through `tools/safe_remove_worktree.ps1` and deletes the
   branch. Report one line per unit, say plainly that the session is complete, and stop. Don't
   hand off and don't wait on anything.

## Lane notes

- **pins**: do `pins-audit` before any drop. The script for `pins-drop` must be deterministic and
  re-runnable (Lane-Replay). Hold the freeze for as short a time as possible: claim at the start of
  the unit, land, unclaim. Naming lanes wait on it.
- **shift**: the shift target must never change the matching build. Once it exists, ask the owner
  before adding it to `lanes.json` gates.
- **data**: never recreate `asm/data/*.rodata*.s`, never `make setup`. Splitting a `.s` file means
  updating `bb2.ld` object order: run `tus-check`.
- **fake**: one TU per unit, claimed as `path:src/main/<tu>.c`. Naming lanes may still rename
  symbols in it; that merges line by line on land.
- **assets / trace**: tools and docs only, no build input. `land` still runs the full gates when
  anything outside `docs/` or `memory/` changes (a `tools/` script can be a build input).

## Hard limits

Never `make setup`, never recursive force deletes (worktrees go through
`tools/safe_remove_worktree.ps1`, which `lanes.py close --remove` calls), no attribution lines,
commit with `-F`, and Python writes use `newline='\n'`. Put scripts in `tmp/*.py` (shell quoting
is guarded). New `.md` files need `[new-doc]` and a reason in the commit body.
