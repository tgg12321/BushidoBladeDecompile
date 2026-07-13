# BB2 Decomp — Documentation Index

This directory holds the contributor-facing documentation for the *Bushido Blade 2* matching decompilation. If you just landed on the project, start with the top-level [`../README.md`](../README.md) (the front door) and [`../BUILD.md`](../BUILD.md) (toolchain setup).

> **Workflow note (2026-05):** the decomp workflow is now the **engine** (`engine/`,
> `python3 -m engine.cli`), run on `main` by a single focused agent — see
> [`../CLAUDE.md`](../CLAUDE.md). The **reference** docs here (ARCHITECTURE, MATCHING,
> GLOSSARY, `formats/`) stay accurate: the build pipeline, regfix/asmfix mechanics, and
> MIPS techniques are unchanged. The **process** framing in some docs — the `dc.sh`-driven
> loop, `WORK_QUEUE.md`, the `.bb2_active_func` active-marker, worktrees, sub-agents — is
> **historical**. `WORK_QUEUE.md` was retired; the current worklist is
> `python3 -m engine.cli scan-redundant --all`, and the per-function loop lives in CLAUDE.md.
> The **named-recipe library** (`tools/recipes/*.json` + `capture_recipe.py`/`recipes.py`)
> was archived 2026-05-26 to `archive/dcsh_workflow_2026-05-26/recipes/` — the engine doesn't
> consume recipes. Reusable techniques now live as path-scoped docs in
> [`../.claude/rules/`](../.claude/rules) (auto-load on matching source reads; fingerprinted by
> the metrics layer); agents register new findings there at match-finish (the "Register
> findings" loop step in CLAUDE.md). The technique *knowledge* still reads as reference in
> MATCHING.md.

## In this directory

| File | When to read it |
|---|---|
| [`ARCHITECTURE.md`](ARCHITECTURE.md) | You want to understand the build pipeline, the PS1 memory map, the splat split, what regfix/asmfix actually do, or why the project has so many post-passes. |
| [`TOOLS.md`](TOOLS.md) | You need to find a standalone Python tool for a specific task. Catalog of ~100 Python tools grouped by workflow phase (plus the retired `dc.sh` subcommand catalog, kept for archaeology). |
| [`MATCHING.md`](MATCHING.md) | You're stuck on matching a function. Symptom-indexed playbook: penalty-profile routing, C-side techniques, regfix syntax and recipes, named recipes (LICM unhoist, call-loop, early-exit alias, GTE 3x3, etc.), common gotchas, dead ends, decision trees. |
| [`GLOSSARY.md`](GLOSSARY.md) | You encounter an unfamiliar term (PsyQ, MIPS, decomp jargon, or BB2-specific Japanese romanizations from the Marionation engine). |
| [`STATUS.md`](STATUS.md) | You want function counts and health indicators (a hand-refreshed, dated snapshot). For the live worklist run `python3 -m engine.cli scan-redundant --all`. |
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
