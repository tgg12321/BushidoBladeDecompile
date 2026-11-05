---
name: lanes
description: >-
  Run BB2's parallel lanes from one place: keep N background lane sessions (naming and
  foundation) going through tools/lanes/orchestrate.py, change N while they run, show status, or
  drain. Use when asked to start / run / spin up lanes, run N lanes at once, run naming and
  foundation in parallel, change how many lanes are active, check on the lanes, or stop them.
  Argument: a number (start with that many), `max N`, `status`, `plan`, `stop`, `pause <lane>`,
  `resume <lane>`, `ack <lane|all>`, or `codex on|off`.
---

# BB2 lanes: one orchestrator

`tools/lanes/orchestrate.py` keeps N background Claude sessions running, one per lane. Each runs
`/naming-session <lane> --row <id>` or `/foundation-session <lane> --row <id>` in its own
worktree: the orchestrator picks a row the session can start now (not claimed, its claims and
dependencies free) and never gives one row twice, so it starts no session while every workable
row's claims are held. When a session closes its lane, the orchestrator stops it and starts the
next lane with work, until the backlogs run dry. This session only drives the orchestrator: it never opens a lane, edits files or lands
anything itself.

## Arguments

| Invocation | Do |
|---|---|
| `/lanes 3` (any number; `/lanes` alone = 3) | Start (below). Add `naming` or `foundation` to pass `--only <kind>`. |
| `/lanes max 5` | `python tools/lanes/orchestrate.py max 5` (0 = drain). Takes effect within a minute. |
| `/lanes status` | `python tools/lanes/orchestrate.py status` and `tmp/lanes-state/digest.md`, plus `python tools/lanes/lanes.py status`. |
| `/lanes plan` | `python tools/lanes/orchestrate.py plan` (what would start next; starts nothing). |
| `/lanes pause <lane> [--when-away]` / `resume <lane>` | `orchestrate.py pause` / `resume`: a paused lane is never started (`naming` = all naming lanes); `--when-away` starts it only after 20 min without owner input. Persists across runs (`tmp/lanes-state/paused.json`). Does not stop a running session. |
| `/lanes ack <lane>` (or `all`) | `orchestrate.py ack ...`: clear attention items after reading them. |
| `/lanes codex on` / `off` | `orchestrate.py codex on` / `off` (or `run --codex`): one extra `lane-codex` session outside N, following `tmp/codex/fake-lane-brief.md`. Restarted only while Codex is under 95% (5 h) / 98% (week) and not BLOCKED, and unless its last two sessions landed nothing (landed = a main commit with a `Codex run:` trailer); `codex on` re-arms. |
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
- A foundation lane whose session changed nothing in its backlog is not restarted in that run;
  naming lanes wait until the naming backlog changes (a row's id, stage or note, or which rows
  are workable).
  Rows at stage `blocked` (waiting on an owner ruling) count as done, so their lanes don't start.
- Attention is raised only for owner questions (an ended session whose last message asks one
  or cannot be read),
  failed lands (a session that ended with unlanded commits, a failed self-heal land) and stalls
  (idle 90 min with its lane open, low disk, a codex lane that stops restarting). The last
  message is saved to `tmp/lanes-state/attention/<lane>-<id>.md`, logged as `ATTENTION <lane>:
  ...`, and listed by `status` until `ack`. Relay those to the user.
- Everything else is in the one-screen digest `tmp/lanes-state/digest.md` (sessions, gate
  queue, attention, self-heal actions, leftovers to look at, recent ends): read it for
  `/lanes status`.
- Self-heal (the orchestrator, not the sessions): retries leftover lane folders / landed
  branches (`lanes.py tidy`), closes registrations whose owner session is gone (claims freed,
  branch and worktree kept; not while the lane has an unacked attention item, so `ack` after
  answering), and moves a `blocked` naming row to `idea` once an `EVIDENCE` line
  for it lands or its `AFTER:` wave lands (`tools/lanes/heal.py`, its own lane `heal`: the name is reserved, never
  open a lane called `heal`). Sessions need not report those.
