# BB2 Decomp — Documentation Index

This directory holds the contributor-facing documentation for the *Bushido Blade 2* matching decompilation. If you just landed on the project, start with the top-level [`../README.md`](../README.md) (the front door) and [`../BUILD.md`](../BUILD.md) (toolchain setup).

> **Workflow note (refreshed 2026-08-24):** the decomp workflow is the
> **engine** (`engine/`, via `& tools/wteng.ps1 main <cmd>`) driven by the
> **Grinder** (`tools/grinder/` — the default autonomous pipeline since
> 2026-07-06); the worklist is `engine/queue.json` (`queue next` / `queue
> status`). See [`../CLAUDE.md`](../CLAUDE.md). Since 2026-08-19
> (asm-until-matched) a not-yet-decompiled function is committed as
> `INCLUDE_ASM("asm/funcs", <func>);`. The **reference** docs here
> (ARCHITECTURE, MATCHING, GLOSSARY, `formats/`) stay accurate. **Historical**
> framing you may still meet in older docs: the `dc.sh` loop, `WORK_QUEUE.md`,
> active-markers, worktrees, the multi-agent fleet, the recipe library — all
> retired (see HISTORY.md). Reusable techniques live as path-scoped rules in
> [`../.claude/rules/`](../.claude/rules).

## In this directory

| File | When to read it |
|---|---|
| [`ARCHITECTURE.md`](ARCHITECTURE.md) | You want to understand the build pipeline, the PS1 memory map, the splat split, what regfix/asmfix actually do, or why the project has so many post-passes. |
| [`TOOLS.md`](TOOLS.md) | You need to find a standalone Python tool for a specific task. Catalog of ~100 Python tools grouped by workflow phase (plus the retired `dc.sh` subcommand catalog, kept for archaeology). |
| [`MATCHING.md`](MATCHING.md) | You're stuck on matching a function. Symptom-indexed playbook: penalty-profile routing, C-side techniques, regfix syntax and recipes, named recipes (LICM unhoist, call-loop, early-exit alias, GTE 3x3, etc.), common gotchas, dead ends, decision trees. |
| [`GLOSSARY.md`](GLOSSARY.md) | You encounter an unfamiliar term (PsyQ, MIPS, decomp jargon, or BB2-specific Japanese romanizations from the Marionation engine). |
| [`STATUS.md`](STATUS.md) | You want function counts and health indicators (a hand-refreshed, dated snapshot). For the live worklist run `& tools/wteng.ps1 main queue status`. |
| [`grind/`](grind/) | The Grinder's owner-audit surfaces: `decisions.md` (every Judge ruling, append-only, driver-written), `journal.md` (one line per session), `borderline.md` (logged policy questions/grants — nothing in it is pending). |
| [`superpowers/specs/`](superpowers/specs/) | Design specs — including the Grinder pipeline design (2026-07-06, amended 2026-08-19). |
| [`HISTORY.md`](HISTORY.md) | You want the timeline: when each major piece of infrastructure landed, when milestones were hit, when rules changed. Compiled from `git log` and the Codex handoff documents. |

## Elsewhere in the repo

| Top-level file | What's there |
|---|---|
| [`../README.md`](../README.md) | Project front door: elevator pitch, status, tech specs, repo layout, credits, legal. |
| [`../BUILD.md`](../BUILD.md) | End-to-end setup: WSL/Linux toolchain, disc extraction, first build, common errors. |
| [`../CONTRIBUTING.md`](../CONTRIBUTING.md) | How to contribute: work-queue model, active-marker hook, per-function lifecycle, escalation ladder, bridge retirement, PR conventions. |
| [`../CLAUDE.md`](../CLAUDE.md) | Operating instructions for the Claude Code agent that does most of the decomp work. Useful context for human contributors who want to understand the day-to-day workflow. |
| `../WORK_QUEUE.md` *(retired 2026-05)* | Former live work list. Replaced by `engine/queue.json` — the ordered per-function worklist that the `queue next` / `queue status` CLI drives. |
| [`handoffs/2026-05-12-subsystem-map.md`](handoffs/2026-05-12-subsystem-map.md) | What lives in each `src/*.c` file and at what VRAM address range. |
| [`handoffs/2026-05-12-audit.md`](handoffs/2026-05-12-audit.md) | Audit of finished C surface; bridge-signature cleanup; naming-suspect list. |
| [`naming/2026-05-12-triage.md`](naming/2026-05-12-triage.md) | Per-symbol triage of Kengo-derived names — keep, demote, or rename. |
| [`naming/2026-05-12-rename-queue.md`](naming/2026-05-12-rename-queue.md) | Reviewed action queue for Kengo-derived renames. |
| [`../CLAIMS.md`](../CLAIMS.md) | Cross-session coordination: active claims, stabilizer fragility notes. |
| [`handoffs/2026-04-27-codex.md`](handoffs/2026-04-27-codex.md) | Historical handoff: the zero-stub sweep completion (all 1,410 functions resolved). |
| CODEX_HANDOFF_2026-04-17.md (no longer present) | Historical handoff: introduction of asmfix and the codex_lab discipline. Never tracked in git; deleted during 2026-05-18 repo cleanup. |

## Reading order for a new contributor

1. [`../README.md`](../README.md) — the elevator pitch + repo layout.
2. [`../BUILD.md`](../BUILD.md) — get to `OK: bb2 matches!`.
3. [`../CONTRIBUTING.md`](../CONTRIBUTING.md) — work-queue model + per-function lifecycle.
4. [`ARCHITECTURE.md`](ARCHITECTURE.md) — what the build is actually doing.
5. [`TOOLS.md`](TOOLS.md) — skim to know what exists; reference later.
6. [`MATCHING.md`](MATCHING.md) — read in depth when you start your first match.
7. [`GLOSSARY.md`](GLOSSARY.md) — keep open in a tab.

## Reading order for a returning contributor

1. [`STATUS.md`](STATUS.md) — current numbers, queue, what's on `main` right now.
2. [`HISTORY.md`](HISTORY.md) — what's changed structurally since you were last here.
3. `git log --oneline --since="2 weeks ago"` — what's been matched recently.
4. `& tools/wteng.ps1 main queue next` — the current top of the worklist.
