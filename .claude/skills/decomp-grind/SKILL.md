---
name: decomp-grind
description: >-
  Operate the Bushido Blade 2 (SLUS-00663) Grinder — the DEFAULT autonomous
  decomp pipeline (tools/grinder/): a deterministic single-lane driver that
  grinds the queue top to COMPLETED-C however many sessions it takes, with a
  persistent per-function ledger and a pre-authorized default-FAIL Judge.
  Use whenever asked to start / kick off / launch / resume the decomp pipeline
  ("start our decomp pipeline"), check on the grind, review what the Judge
  ruled, or stop it. Replaces the retired decomp-fleet skill.
---

# BB2 Grinder — operator

You operate **the Grinder** (`tools/grinder/`): one deterministic driver, one
function at a time, grind-until-done. You are the operator, not a worker — you
verify preconditions, launch it detached, and report; the pipeline does the
decomp. Design: `docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`.
Operational memory: the auto-memory `project/grinder-pipeline.md`
([[grinder-pipeline]] — harness memory, not a repo path).

> **Prime directives:** no cheat reaches `main` (bytes proven BEFORE a
> default-FAIL Judge rules on the C); no token/time/cost framing anywhere
> ([[no-budget-caps]]); grind-until-done — never rotate targets.

## DEFAULT FLOW — "start our decomp pipeline"

All commands via the PowerShell tool, from the repo root.

### 1. Orient — refuse to double-launch, verify the world is grinder-shaped
```powershell
# already running?
if (Test-Path tmp/grind/grind.lock) { $p = Get-Content tmp/grind/grind.lock;
  if (Get-Process -Id $p -EA SilentlyContinue) { "RUNNING pid $p" } else { "stale lock (dead $p) — reclaimable" } }
git status --short          # tree must be clean (untracked files false-fail the drills!)
git worktree list | Select-String 'bb2-work'   # MUST be empty — see Invariants
& tools/reintegrate_lock.ps1 status            # MUST be FREE
```
If already running → skip to STATUS and report; do NOT relaunch (the driver's
pidfile also refuses). If the tree is dirty with tracked edits → surface to the
owner, don't launch over them. Untracked stray files → commit or move to tmp/
first (they trip the driver's scope check and the drills).

### 2. Drill — GO/NO-GO (skip only if the owner says so)
```powershell
pwsh tools/grinder/drill.ps1              # A: give-up discard, B: scope reject (~5 min)
pwsh tools/grinder/drill.ps1 -WithJudge   # + C: live judge FAILs a planted cheat (first launch / after driver edits)
```
NO-GO → do not launch; investigate, report the failing drill.

### 3. Launch detached
```powershell
# NB: the embedded `"…`" quotes are REQUIRED — Start-Process does not quote
# ArgumentList items itself, and the repo path contains spaces.
Start-Process pwsh -WindowStyle Hidden -ArgumentList '-NoProfile','-File',"`"$PWD\tools\grinder\grind.ps1`""
Start-Sleep -Seconds 5
pwsh tools/grinder/status.ps1             # confirm: driver RUNNING (pid), target, modality
# status must say RUNNING and tmp/grind/grind.log must show a NEW "grinder starting (pid …)"
# line — a silent no-lock/no-log result means the spawn itself failed.
```
The driver survives this session ending (not a full logoff). It grinds the
queue top continuously; completions land as `Match: <func> — COMPLETED-C
(grinder, N sessions)` commits.

### 4. Report back
- Driver pid + current target (func, origin, session #, next modality).
- Where the owner audits later: `docs/grind/decisions.md` (every Judge ruling,
  plain English) + `docs/grind/journal.md` (one line per session) +
  `pwsh tools/grinder/status.ps1`.
- How to stop: `pwsh tools/grinder/grind.ps1 -Stop` (clean, at the next
  session boundary).

## OTHER VERBS

| Owner says | Do |
|---|---|
| "status" / "how's the grind" | `pwsh tools/grinder/status.ps1`; summarize floor trajectory, killed-hypothesis count, completions, latest Judge rulings. Diagnostics a session may be told to run (all read-only; see each file's docstring): tools/fake_ablate.py, tools/loop_movables.py, tools/nrefs_census.py, tools/label_census.py. |
| "stop" | `pwsh tools/grinder/grind.ps1 -Stop`; confirm the pidfile clears at the next boundary. |
| "what did the judge rule" | Read + summarize `docs/grind/decisions.md` (newest first). |
| "it crashed / circuit-break" | Read `docs/grind/INCIDENT.md` + `tmp/grind/grind.log` tail. Fix the stated cause (oracle break → engine loop recovery; usage-limit exhaustion → just relaunch later). Relaunching resumes exactly — all state is on disk. |
| "resume" | Same as start; the ledger means no work is ever lost between runs. |
| "why is it stuck" / "did a sibling solve this" | `python tools/grinder/grindlib.py siblings . <func>` — the SIBLING LEDGERS block the next brief will carry (coupled ledgers, their current floors, whether this ledger has read them since they moved). A sibling floor DROP stamps `sibling_progress` on every coupled ledger and forces ONE `rederive` session there (2026-09-04 post-mortem: CD_datasync sat 41 sessions while foreclosed CD_sync held the fix). Rotated siblings are shown AND return automatically on that notice (`queue auto-return`, owner ruling 2026-09-08 [[rotation-not-foreclosure]] — the foreclosed state is retired; `queue unpark` is now just an early return). |
| "a family was granted" / "re-scan after grant" | Do this the same day the grant lands. `pwsh tools/grinder/grind.ps1 -Stop` first and confirm the pidfile clears (this writes ledgers + `engine/queue.json`; queue dirt discards a live session — `--apply` refuses while the lock pid is alive). Dry-run `python tools/grinder/grant_rescan.py --term "<keywords>"` → review the `WOULD SUPERSEDE` lines → `--apply --only <func> ... --family "<name>" --ref "<file:line>"` → commit `memory/grind engine/queue.json` → relaunch. |

## Invariants (violating these breaks the pipeline)

1. **No `bb2-work-*` worker worktrees may exist while the grinder runs.** Their
   presence arms `main_reintegration_lock`, which blocks grind sessions from
   editing src on main (a session once hijacked the lock over this). The fleet
   is retired; never recreate its worktrees. Remove strays ONLY via
   `tools/safe_remove_worktree.ps1 <path> -Force`.
2. **Nobody holds the reintegration lock during grinding** — same reason.
3. **One driver, ever** — the pidfile enforces it; never delete
   `tmp/grind/grind.lock` while its pid is alive.
4. **Don't edit `tools/grinder/*` while the driver runs** — the running loop
   already loaded its script; stop → edit → drill → relaunch.
5. **The queue is the only worklist** — no cherry-picking, no rotation. The
   owner can reorder the queue; the grinder just takes the top.
6. **INCOMPLETE = `INCLUDE_ASM` on main** (owner ruling 2026-08-19,
   [[asm-until-matched]]): sessions never commit intermediate C; candidates,
   banked chassis (`retired-chassis-2026-08/`), and the queue-ordering floor
   (`migration_pin.json`) live in `memory/grind/<func>/`.

## Failure modes the driver already handles (don't intervene)
- Session crash / usage-limit / timeout → invalid session, discarded,
  respawned (3 consecutive → circuit-break + INCIDENT.md, clean stop).
- Judge API hiccup with a proven candidate → backoff ×5, candidate waits.
- Oracle red after a session → circuit-break, never limps.
- Driver killed / reboot → relaunch; stale pidfile auto-reclaims; ledger +
  queue + git ARE the state.
