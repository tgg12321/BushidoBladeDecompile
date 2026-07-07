---
name: decomp-fleet
description: >-
  RETIRED 2026-07-06 — superseded by the decomp-grind skill (the Grinder
  pipeline). Use decomp-grind for anything phrased like "start the decomp
  pipeline" / "run the autonomous decomp". This skill remains only as the
  historical operator reference for the retired multi-agent fleet
  (tools/fleet/): backlog workers, blocked worker, Adjudicator,
  Auditor+Verifier gate, Overseer. Do not launch the fleet.
---

# BB2 Autonomous Decomp Fleet — operator

You operate the **Autonomous Decomp Fleet**: a zero-human-intervention multi-agent
system (`tools/fleet/`) that works the Bushido Blade 2 matching-decompilation
backlog continuously. Specialized role-agents run nonstop in isolated git
worktrees; **only an adversarial review chain (Auditor → Verifier) may promote a
function to COMPLETED on `main`.**

This skill is the **on-demand entry point** to that system. The default action of
invoking it is **orient → status → drill → launch → report**. You are the operator,
not a worker — you start the fleet, watch it come up safely, surface what needs the
owner's decision, and can stop it. The fleet itself does the decomp.

> **Prime directive (above throughput):** no cheat, workaround, or reward-hack
> ever reaches `main`. There is **no token/time/cost framing anywhere** in this
> system — by user directive, cost pressure makes agents anxious and quick to quit
> ([[no-budget-caps]]). Never introduce it into a role prompt, a status report, or
> your own reasoning here.

## Bootstrap — load context first

1. **`tools/fleet/README.md`** — the operator guide (roster, safety model, tuning).
2. **`docs/fleet/HANDOFF.md`** — the live state: where the campaign stands, what the
   re-audit patrol has found, resume-vs-fresh, and known gotchas. **Always read this
   before launching** — it tells you the current recommended config.
3. **`docs/fleet/RUNBOOK.md`** — the consolidated quickstart + lifecycle + troubleshooting.
4. Full design + rationale (only if you need internals): `docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md`.

## The roster (6 agent instances → 6 card states)

| Lane | Role | Card state it owns | Touches `main`? |
|---|---|---|---|
| `fleet-bw1`, `fleet-bw2` | **Backlog Worker** ×2 | Backlog → In-Review / Blocked / Needs-Decision | No |
| `fleet-blk` | **Blocked Worker** | Blocked (no-quit; 10 tries → rotate to bottom) | No |
| `fleet-adj` | **Adjudicator** | Needs-Decision (sanctions only frozen SOTN families; novel + canonical-asm park for owner) | No |
| `fleet-aud` (+ `fleet-aud2`) | **Auditor + Verifier** | In-Review → Done (the promotion gate; layer-2 + layer-3 over the worker's layer-1) | **YES — sole authority** |
| `fleet-ovr` | **Overseer** | circuit-breaker (oracle break / toolchain death / contamination) | No |

Workers + Adjudicator run as background processes in their own worktrees. The
Auditor, Verifier, and Overseer are driven **inline by the supervisor**, which is
the **only** process that ever mutates `main`.

## Why it is safe to leave running

1. **Only the supervisor merges to `main`** — workers live in throwaway worktrees;
   the contamination + reintegration-lock hooks block any agent from mutating main.
2. **Every promotion is oracle-gated** — `queue done` (0 rules + 0 cheat-asm + SHA1
   == oracle) re-runs on `main`; a candidate that doesn't reproduce the oracle is
   rolled back and its SHA blacklisted.
3. **Three independent review layers** — worker `cheat-reviewer` → Auditor → Verifier.
4. **Canonical-asm + novel techniques never auto-merge** — they park in
   `docs/fleet/pending_owner.md` for the owner.
5. **The Overseer** restores a green oracle on any break, or halts and leaves a report.

---

## DEFAULT FLOW — "kick up the fleet"

When the user asks to start / launch / resume / kick up the fleet (no other verb),
run this sequence. All commands go through the **PowerShell tool** (`pwsh …`).

### 1. Orient — is it already running, is main clean?
- Check `tmp/fleet/supervisor.pid`. If a live PID exists, the fleet is **already
  running** — skip to **STATUS** and report; do NOT relaunch (`launch.ps1` refuses
  a double-launch anyway).
- Confirm `main` is clean (`git status`) and the oracle is green. The supervisor
  refuses to start unless `main` builds byte-identical to the oracle, but check first
  so you can explain a NO-GO. If the tree is dirty with tracked-file edits, surface
  that — **do not** launch over uncommitted fleet-code edits (see Gotchas).

### 2. Status — what has it done, what needs the owner?
```powershell
pwsh tools/fleet/status.ps1
```
Then read and summarize for the user:
- **`docs/fleet/pending_owner.md`** — canonical-asm / novel-technique items that did
  NOT auto-merge and need a sign-off decision.
- **`docs/fleet/regressions.md`** — committed functions the re-audit patrol flagged as
  carrying a cheat (each needs a clean redo; the byte-correct cheat stays on main only
  until the clean replacement lands).
- Recent `Match:` commits on `main` (each = one autonomously-completed function).

### 3. Decide resume vs fresh
- **Resume** (default): campaign state in `tmp/fleet/state.json` persists, so a relaunch
  continues where it left off (re-audit cursor, reshuffle, findings). This is almost
  always what you want.
- **Fresh** (only if explicitly asked, or state is corrupt): see RESET below.

### 4. Drill — validate the live pipeline (watch the verdict)
```powershell
pwsh tools/fleet/drill.ps1
```
This runs ONE real backlog-worker cycle + ONE real Auditor cycle + a deterministic
self-test that proves a bad candidate cannot survive the merge gate. It can take
several minutes (live agents). Read the final **GO / NO-GO** verdict:
- **GO** → proceed to launch.
- **NO-GO** → do NOT launch. Report the failing invariant (oracle not green, or
  self-test failed) and investigate / escalate to the user.
- The user may say "skip the drill" if they drilled recently and the tree is unchanged;
  honor that, but note in your report that the drill was skipped.

### 5. Launch detached
Use the config the HANDOFF currently recommends (read it — do not hardcode). As of the
last handoff that was:
```powershell
pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked
```
`launch.ps1` defaults to `-Workers 2` with all lanes; the HANDOFF's `-Workers 1
-NoBlocked` reflects the hard-tail reality (the easy functions are done). The process
is detached and survives this shell / your session (but not a full Windows logoff — for
that, the README documents a Scheduled Task).

### 6. Report back
Tell the user, concretely:
- Supervisor PID + that it's up (the launcher tails the first log lines).
- The config you launched with and why (resume vs fresh, worker count, model tiers).
- What the status surfaced: pending-owner decisions, regressions to re-do, last completions.
- How to monitor (`pwsh tools/fleet/status.ps1`) and stop (`pwsh tools/fleet/stop.ps1`).

---

## OTHER ACTIONS (verb-driven)

If the user names a specific action, do that instead of the full default flow:

| User says | Do |
|---|---|
| "status" / "how's the fleet" / "what did it find" | Run `status.ps1`, summarize `pending_owner.md` + `regressions.md` + recent `Match:` commits. No launch. |
| "stop" / "shut it down" | `pwsh tools/fleet/stop.ps1` (add `-Force` to hard-kill immediately). Confirm it wound down. |
| "review the findings" | Walk `pending_owner.md` and `regressions.md` WITH the user, item by item — each pending item needs authorize-or-reject; each regression needs a re-do decision. These are owner calls; present, don't decide. |
| "reshuffle" / "retier the queue" | `pwsh tools/fleet/reshuffle.ps1` (surfaces most-tractable active items first). |
| "fresh" / "restart the campaign" | RESET (below). |
| "is it running?" | Check `tmp/fleet/supervisor.pid` + a live process; report. |

### RESET (fresh campaign — only when explicitly asked)
```powershell
pwsh tools/fleet/stop.ps1 -Force
Remove-Item tmp/fleet/state.json
pwsh tools/fleet/seed_reaudit.ps1
pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked
pwsh tools/fleet/reshuffle.ps1
```
This discards all operational state (attempt counts, re-audit cursor, queues) and
re-seeds the historical re-audit worklist. Findings already committed in `docs/fleet/`
are NOT lost (they're durable on `main`).

### Tuning (only if asked)
- **Model tiers:** `launch.ps1 -CheapModel sonnet -StrongModel opus`. Cheap = active
  decomp + re-audit patrol + overseer; strong = blocked no-quit grind + forward
  merge-gate (auditor + verifier) + adjudicator. `-CheapModel haiku` is cheaper but
  riskier match rate. (Express tiers as quality/risk trade-offs, never as cost savings.)
- `-Workers N`, `-NoBlocked`, `-NoAdjudicator`, `-NoReaudit`.
- `coord.ps1` constants: `$BLOCKED_ROTATE_AT` (10), `$REJECT_ADJUDICATE_AT` (3),
  `$LANE_STALE_MINUTES` (120). `fleet.ps1 -OracleBackstopMinutes` (30), `-LaneTimeoutMinutes` (120).

---

## Gotchas (read before you touch anything)

- **Never edit tracked fleet code while the fleet is running.** `Assert-MainClean` /
  the merge pre-check will `git checkout` / `clean` your uncommitted changes away as
  "contamination." To edit fleet code: stop the fleet, edit, commit, relaunch.
- **Committing to `main` while worktrees are live needs the reintegration lock**
  (`& tools/reintegrate_lock.ps1 acquire` … `release`); the PreToolUse hook blocks
  un-locked main mutations. The supervisor handles this for its own merges — you only
  need it if YOU commit to main while the fleet is up.
- **Re-audit findings are NOT auto-re-opened** — surfaced for owner review (a
  false-positive would churn workers on established completions). The owner decides
  which to re-do.
- **The re-audit cursor currently wraps** (re-audits clean functions again after a full
  pass — wasteful on multi-day runs; a one-pass fix is noted in the HANDOFF as deferred).
- **Worktrees** live in one container `..\bb2-worktrees\bb2-work-<id>`. Remove only via
  `tools/safe_remove_worktree.ps1` — never `git worktree remove --force` (junction-follow
  hazard into main's `.venv`/`build`/`disc`).
- **Orphaned agent processes / RAM bloat:** the fleet is crash-safe via a kill-on-close
  Job Object, but third-party harness leaks are cleaned by `pwsh tools/reap_orphans.ps1`
  (dry-run default; `-Execute` to reap) — see [[orphan-process-reaper]].
- **gh token:** board updates retry a transient 401; if it fully lapses, `gh auth refresh`.
  The audit + `regressions.md` keep working without it.

## Relationship to the other skills
- **decomp-fleet** (this) — the autonomous multi-agent system; the default way to work
  the backlog at scale and unattended.
- **decomp-orchestrate** — drive ONE function (or a headless single-agent loop) on `main`
  by hand; the underlying per-function engine loop each fleet worker runs.
- **decomp-board / decomp-work** — manual board-queue interface + single batch worker.
