> **SUPERSEDED 2026-07-06.** The fleet is retired — the Grinder (`tools/grinder/`, operate via the `decomp-grind` skill) is the autonomous pipeline. This runbook is historical reference only; do not launch the fleet.

# Fleet Runbook — operator one-pager

The single on-ramp for operating the **Autonomous Decomp Fleet**. Quickstart →
lifecycle → what-to-check → troubleshooting, consolidated from `tools/fleet/README.md`
(reference) and `docs/fleet/HANDOFF.md` (live state). When in doubt about the *current*
campaign state, the HANDOFF is authoritative; this is the *procedure*.

> **Agents:** invoke the **`decomp-fleet` skill** to operate the fleet — it encodes this
> runbook as an executable flow (orient → status → drill → launch → report).

---

## What it is, in one paragraph

A zero-human-intervention multi-agent system (`tools/fleet/`) that works the BB2 decomp
backlog continuously. Six role-agents run in isolated git worktrees; **only an adversarial
review chain (Auditor → Verifier) may promote a function to COMPLETED on `main`.** Prime
directive above throughput: **no cheat ever reaches `main`.** There is **no token/time/cost
framing anywhere** — by design (cost pressure makes agents quit early).

## The roster → card states

| Lane | Role | Card state | Merges to main? |
|---|---|---|---|
| `fleet-bw1`, `fleet-bw2` | Backlog Worker ×2 | Backlog → In-Review / Blocked / Needs-Decision | No |
| `fleet-blk` | Blocked Worker (no-quit) | Blocked (10 tries → rotate to bottom) | No |
| `fleet-adj` | Adjudicator | Needs-Decision (frozen SOTN families only; novel + canonical-asm park) | No |
| `fleet-aud` / `fleet-aud2` | Auditor + Verifier | In-Review → Done | **YES — sole authority** |
| `fleet-ovr` | Overseer | circuit-breaker (oracle / toolchain / contamination) | No |

The **supervisor** (`fleet.ps1`) launches the lanes, drives Auditor/Verifier/Overseer
inline, and is the **only** process that mutates `main`.

```
 BACKLOG ─claim─> IN_PROGRESS ─┬─ match ──> IN_REVIEW ─[AUDITOR→VERIFIER]─┬─ PASS ─> DONE (merged)
   ▲                           │                                          └─ FAIL ─> BACKLOG/BLOCKED (+feedback)
   │                           ├─ stuck ──> BLOCKED ─[no-quit]──> IN_REVIEW / NEEDS_DECISION
   └─ go-back ─────────────────┴─ borderline ──> NEEDS_DECISION ─[ADJUDICATOR]─ legit ─> IN_REVIEW
                          auditor idle ──> re-audit COMPLETED log ──> regression self-heals + logged
```

---

## Quickstart

```powershell
# 0. (agents) just invoke the decomp-fleet skill — it runs steps 1-4 for you.

# 1. STATUS — what's it done, what needs you (safe, read-only):
pwsh tools/fleet/status.ps1

# 2. DRILL — validate the live pipeline + safety invariants (watch the GO/NO-GO):
pwsh tools/fleet/drill.ps1

# 3. LAUNCH detached (resumes the campaign; survives this shell/session):
pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked

# 4. STOP gracefully (supervisor winds down its lanes; -Force to hard-kill now):
pwsh tools/fleet/stop.ps1
```

The supervisor **refuses to start unless `main` builds byte-identical to the oracle.**

### The recommended launch config
`launch.ps1` defaults to `-Workers 2` with all lanes. The HANDOFF currently recommends
`-Workers 1 -NoBlocked` because the easy functions are already done — the active queue is
the **hard tail**. Always re-read `docs/fleet/HANDOFF.md` for the current recommendation
rather than hardcoding.

### Model tiers (quality/risk, never cost)
```powershell
pwsh tools/fleet/launch.ps1 -CheapModel sonnet -StrongModel opus -ReauditModel sonnet
```
- **Cheap** (default `sonnet`): active backlog decomp, overseer.
- **Strong** (default `opus`): blocked no-quit grind, forward merge-gate (auditor +
  verifier), adjudicator. The auditor is tiered *by mode* — re-audit = ReauditModel,
  forward in-review gate = strong.
- **Reaudit** (default = inherit `CheapModel`): re-audit patrol model. Split out
  from `CheapModel` 2026-06-22 so the active backlog tier can be bumped (e.g. to
  opus, after sonnet active produced 2 gate-failed candidates / 0 completions)
  without dragging the high-volume re-audit patrol with it (sonnet re-audit had
  caught 42 historical cheats with precise reasoning by then). Recommended:
  `-CheapModel opus -ReauditModel sonnet` for the current hard tail.
- `-CheapModel haiku` is cheaper but a riskier match rate.

---

## Lifecycle

### Drill (pre-flight, do this before any unattended run)
`drill.ps1` runs: coord init + re-audit seed → ONE real backlog-worker cycle → ONE real
Auditor cycle (review + privileged merge on PASS) → post-pipeline oracle check →
deterministic self-test (a non-matching candidate is proven unable to survive the merge
gate; main stays green). Ends with **GO** (safe to launch) or **NO-GO** (investigate).

### Resume vs fresh
- **Resume** (default): `tmp/fleet/state.json` (gitignored) persists, so a relaunch
  continues the re-audit cursor, reshuffle, and queues. Committed findings in
  `docs/fleet/` are durable on `main` regardless.
- **Fresh** (only when explicitly wanted):
  ```powershell
  pwsh tools/fleet/stop.ps1 -Force
  Remove-Item tmp/fleet/state.json
  pwsh tools/fleet/seed_reaudit.ps1
  pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked
  pwsh tools/fleet/reshuffle.ps1
  ```

### Reshuffle the active queue
`pwsh tools/fleet/reshuffle.ps1` re-tiers the active backlog toward the most-tractable
items first (verdict C + distance ≤15 + low rules; asm-suspect pushed back).

### Maximum durability across a Windows logoff
A detached `launch.ps1` survives a closed shell and an ended agent session, **but not a
full logoff**. For that, register the supervisor as a Scheduled Task (see
`tools/fleet/README.md` for the exact `schtasks` line).

---

## What to check when you're back

| Source | Tells you |
|---|---|
| `pwsh tools/fleet/status.ps1` | completions, regressions, halts, lane health, recent commits |
| recent `Match:` commits on `main` | each = one autonomously-completed function |
| `docs/fleet/pending_owner.md` | canonical-asm / novel-technique items awaiting **your** authorize-or-reject |
| `docs/fleet/regressions.md` | committed functions flagged carrying a cheat — each needs a clean re-do decision |
| `docs/fleet/adjudications.md` | every Adjudicator ruling (your audit trail) |
| `docs/fleet/incidents.md` | Overseer events (oracle breaks, recoveries, halts) |
| `docs/fleet/log.jsonl` | full structured event stream |
| `tmp/fleet/logs/` | per-lane + supervisor stdout (gitignored) |

**Re-audit findings are NOT auto-re-opened** — they're surfaced for owner review (a
false positive would churn workers on established completions). You decide which to re-do.

---

## Troubleshooting / gotchas

- **Won't launch / NO-GO:** the oracle isn't green on `main`. Check `git status` and run
  the oracle rebuild; resolve before launching. The supervisor will not start on a dirty
  or non-matching tree.
- **"Already running":** `launch.ps1` refuses a double-launch if `tmp/fleet/supervisor.pid`
  points at a live process. Use `status.ps1`, or `stop.ps1` first.
- **Don't edit tracked fleet code while it runs** — `Assert-MainClean` / the merge
  pre-check reverts uncommitted edits as "contamination." Stop → edit → commit → relaunch.
- **Committing to `main` while worktrees are live** needs the reintegration lock
  (`& tools/reintegrate_lock.ps1 acquire` … `release`); the PreToolUse hook blocks
  un-locked main mutations. The supervisor handles this for its own merges.
- **Re-audit cursor wraps** — after a full pass it re-audits clean functions again
  (wasteful on long runs). One-pass fix is deferred; noted in HANDOFF.
- **Worktrees** live in `..\bb2-worktrees\bb2-work-<id>`. Remove only via
  `tools/safe_remove_worktree.ps1` — never `git worktree remove --force` (junction-follow
  hazard into main's `.venv`/`build`/`disc`).
- **RAM bloat / orphaned agents:** `pwsh tools/reap_orphans.ps1` (dry-run default;
  `-Execute` to reap). The fleet itself is crash-safe via a kill-on-close Job Object.
- **gh 401:** board updates retry; if the token fully lapses, `gh auth refresh`. The audit
  and `regressions.md` keep working without it.

---

## See also
- `tools/fleet/README.md` — full reference (file map, safety model, tuning).
- `docs/fleet/HANDOFF.md` — current campaign state + recommended config (authoritative).
- `docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md` — design + rationale.
- `decomp-fleet` skill — the executable form of this runbook.
