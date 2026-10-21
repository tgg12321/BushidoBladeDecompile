---
name: lanes
description: >-
  Run BB2's parallel lanes from one place: keep N background lane sessions (naming and
  foundation) going through tools/lanes/orchestrate.py, change N while they run, show status, or
  drain. Use when asked to start / run / spin up lanes, run N lanes at once, run naming and
  foundation in parallel, change how many lanes are active, check on the lanes, or stop them.
  Argument: a number (start with that many), `max N`, `status`, `plan`, or `stop`.
---

# BB2 lanes: one orchestrator

`tools/lanes/orchestrate.py` keeps N background Claude sessions running, one per lane. Each runs
`/naming-session <lane>` or `/foundation-session <lane>` in its own worktree. When a session
closes its lane, the orchestrator stops it and starts the next lane with work, until the backlogs
run dry. This session only drives the orchestrator: it never opens a lane, edits files or lands
anything itself.

## Arguments

| Invocation | Do |
|---|---|
| `/lanes 3` (any number; `/lanes` alone = 3) | Start (below). Add `naming` or `foundation` to pass `--only <kind>`. |
| `/lanes max 5` | `python tools/lanes/orchestrate.py max 5` (0 = drain). Takes effect within a minute. |
| `/lanes status` | `python tools/lanes/orchestrate.py status`, plus `python tools/lanes/lanes.py status`. |
| `/lanes plan` | `python tools/lanes/orchestrate.py plan` (what would start next; starts nothing). |
| `/lanes stop` | `python tools/lanes/orchestrate.py max 0`: running sessions finish their units and close, none start, then the orchestrator exits. Never `claude stop` a session mid-land. |

## Start

1. `python tools/lanes/orchestrate.py status`. If it says the orchestrator is already running,
   don't start a second one: run `max <N>` instead and report.
2. `python tools/lanes/orchestrate.py plan` and tell the user which lanes will start first.
3. Run `python tools/lanes/orchestrate.py <N> [--only naming|foundation]` with
   `run_in_background: true`. It runs for hours; you are notified when it exits.
4. Report in a few lines: N, the first lanes it started (from its output, with each
   `claude attach <id>`), and how to change things (`/lanes max N`, `/lanes status`,
   `/lanes stop`). Then end the turn; don't poll.

When the background run exits, read the tail of its output and report: lanes finished, any
`lost` (session ended with its lane open: point at `claude logs <id>` and
`python tools/lanes/lanes.py status`), `stalled`, or `could not start` lines, and why it stopped
(no work left, drained, or start failures).

## Notes

- Sessions run with `--permission-mode bypassPermissions` (nobody can answer prompts in a
  background session); pass `--permission-mode <mode>` or `--model <model>` through if the user
  asks.
- Ctrl-C / killing the orchestrator leaves the sessions running; starting it again adopts them.
- A session idle 90 min with its lane open is reported, never killed: usually it is waiting on a
  `lanes.py land`. Look with `claude attach <id>`.
- A lane whose session changed nothing in its backlog is not restarted in that run.
