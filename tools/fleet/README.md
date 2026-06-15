# Autonomous Decomp Fleet

A zero-human-intervention multi-agent system that works the Bushido Blade 2 decomp
backlog continuously. Specialized role-agents run nonstop in isolated git worktrees;
**only an adversarial review chain may promote a function to COMPLETED on `main`.**

Full design + rationale: `docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md`.

## Prime directive

No cheat, workaround, or reward-hack ever reaches `main`. Throughput is secondary;
there is no token/time/cost pressure anywhere in the system (by design — it makes
agents anxious and quick to quit).

## The roster (6 agent instances)

| Lane | Role | What it does |
|---|---|---|
| `fleet-bw1`, `fleet-bw2` | Backlog Worker ×2 | pull the top backlog function, match it in pure C, hand a candidate to review (or to Blocked / Needs-Decision) |
| `fleet-blk` | Blocked Worker | no-quit grind of the Blocked queue; after 10 genuine tries an item rotates to the bottom (never abandoned) |
| `fleet-adj` | Adjudicator | deep ASM/decomp expert for borderline items; sanctions only frozen SOTN families; **canonical-asm + novel families park for you** |
| `fleet-aud` (+ `fleet-aud2`) | Auditor + Verifier | the promotion gate: layer-2 + layer-3 independent adversarial review on top of the worker's layer-1; merges to `main` only when all pass |
| `fleet-ovr` | Overseer | circuit-breaker: diagnoses + recovers oracle breaks / toolchain failures, or halts safely |

Workers + Adjudicator run as background processes in their own worktrees. The Auditor,
Verifier, and Overseer are driven inline by the supervisor, which is the **only**
process that ever mutates `main`.

## Safety model (why it's safe to leave running)

1. **Only the supervisor merges to `main`** (workers live in throwaway worktrees; the
   contamination + reintegration-lock hooks block any agent from mutating main).
2. **Every promotion is oracle-gated** — `queue done` (0 rules + 0 cheat-asm + SHA1 ==
   oracle) is re-run on `main`; any candidate that doesn't reproduce the oracle is
   rolled back and its sha is blacklisted.
3. **Three independent review layers** (worker `cheat-reviewer` → Auditor → Verifier)
   before any merge.
4. **Canonical-asm + novel techniques never auto-merge** — they wait for you.
5. **The Overseer** restores a green oracle on any break, or halts and leaves a report.

## Operating it

```powershell
# 1. DRILL first (watch it) — validates the live pipeline + safety invariants:
pwsh tools/fleet/drill.ps1

# 2. LAUNCH detached (survives this shell / the launching session):
pwsh tools/fleet/launch.ps1

# 3. MONITOR any time (read-only dashboard):
pwsh tools/fleet/status.ps1

# 4. STOP gracefully (supervisor winds down its lanes):
pwsh tools/fleet/stop.ps1            # add -Force to hard-kill immediately
```

The supervisor refuses to start unless `main` builds byte-identical to the oracle.

## What to check when you're back

- **`pwsh tools/fleet/status.ps1`** — completions, regressions, pending-owner items, lane health.
- **Recent `Match:` commits on `main`** — each is one autonomously-completed function.
- **`docs/fleet/pending_owner.md`** — canonical-asm / novel-technique items awaiting
  your sign-off (these did NOT auto-merge). Review and either authorize or reject each.
- **`docs/fleet/regressions.md`** — committed functions the re-audit patrol flagged as
  carrying a cheat; each is being re-done cleanly (the byte-correct cheat stays on main
  only until the clean replacement lands).
- **`docs/fleet/adjudications.md`** — every Adjudicator ruling, for your audit.
- **`docs/fleet/incidents.md`** — any Overseer events.
- **`docs/fleet/log.jsonl`** — the full structured event stream.
- **`tmp/fleet/logs/`** — per-lane + supervisor stdout (gitignored).

## Files

| File | Purpose |
|---|---|
| `coord.ps1` | coordinator / message-bus + `tmp/fleet/state.json` (named-mutex atomic) |
| `fleet.ps1` | supervisor: launches lanes, drives Auditor/Verifier/Overseer, sole main-writer |
| `lane.ps1` | one producer lane (backlog/blocked/adjudicator) |
| `roles/*.md` | the role system-prompts (mindset + contract per role) |
| `_fleet_common.ps1` | shared helpers (agent spawn, oracle check, worktree bootstrap) |
| `list_completed.py` / `seed_reaudit.ps1` | seed the re-audit patrol with the 910 committed functions |
| `drill.ps1` / `launch.ps1` / `stop.ps1` / `status.ps1` | operate the fleet |

## Tuning

- **Model tiers (cost control):** `launch.ps1 -CheapModel sonnet -StrongModel opus`.
  - `CheapModel` (default `sonnet`) → backlog/active decomp, the re-audit patrol, overseer.
  - `StrongModel` (default `opus`) → blocked no-quit grind, forward merge-gate review +
    verifier, adjudicator.
  - The auditor is tiered *by mode*: re-audit patrol = cheap; forward in-review gate = strong.
  - `-CheapModel haiku` for even cheaper active decomp (riskier match rate).
- `coord.ps1` constants: `$BLOCKED_ROTATE_AT` (10), `$REJECT_ADJUDICATE_AT` (3),
  `$LANE_STALE_MINUTES` (120).
- `fleet.ps1 -OracleBackstopMinutes` (30), `-LaneTimeoutMinutes` (120), `-Workers N`,
  `-NoBlocked`, `-NoAdjudicator`, `-NoReaudit`.

For maximum durability across a full Windows logoff, register the supervisor as a
Scheduled Task (`schtasks /create /tn bb2fleet /tr "pwsh -File <abs>\tools\fleet\fleet.ps1" /sc onlogon /ru <you> /it`)
instead of the detached `launch.ps1` process.
