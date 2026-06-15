# Autonomous Decomp Fleet — design

**Date:** 2026-06-15
**Author:** Claude (Opus 4.8), driven by Trenton's brainstorm before stepping away
**Status:** approved (build-on-headless_loop substrate, full pre-flight drill), building

## Goal

A zero-human-intervention multi-agent system that works the Bushido Blade 2 decomp
backlog continuously. Specialized role-agents run nonstop in isolated git worktrees;
**only an adversarial Auditor may promote a function to COMPLETED**, and it merges to
`main`. The prime directive, above throughput: **no cheat, workaround, or reward-hack
ever reaches `main`.** Secondary: relentless tenacity (agents quit too early) and **no
token/dollar framing anywhere** (it makes agents anxious and quick to defer).

## Why this is mostly an *activation*, not a greenfield build

The repo already contains nearly every primitive (it ran a multi-agent orchestrator/worker
model until the 2026-05-25 pivot to single-agent-on-main). We reuse:

| Need | Existing primitive |
|---|---|
| Isolated worker workspace | `tools/setup_worker_worktree.ps1` (private build/, junctioned deps, HEAD parity) |
| cwd-safe engine in a worktree | `tools/wteng.ps1 <id> <cmd>` |
| Only-one-writer-to-main | `tools/reintegrate_lock.ps1` + hook `main_reintegration_lock.py` |
| Workers can't touch main | hook `worktree_contamination_guard.py` |
| Adversarial cheat gate | agent `cheat-reviewer` (6-test, default-FAIL, "cheats by any spelling") — reused as the Auditor + Verifier role prompts |
| No-quit semantics | the `resilience-judge` *principles* are inlined into the worker prompts (default-BLOCK self-policing); the judge agent is NOT mechanically wired in this version — tenacity is enforced by the prompts + the automatic 10-try blocked rotation, not a separate gate |
| Per-function progress + feedback | `memory/wip/<func>/` (candidate.c + meta.json + notes.md + rejected/) |
| Headless driver pattern | `tools/headless_loop.ps1` (oracle baseline + per-iter re-check + never-push) |
| Worklist + ordering | `engine/queue.json` (active/authorize/parked; easiest-first) |

The fleet is a **multi-lane generalization of `headless_loop.ps1`** plus a coordinator that
adds the lane states the queue doesn't have (In-Review, Needs-Decision, actively-worked
Blocked) **without mutating `engine/queue.py`** (it's pinned by `engine test`).

## Roster — 5 roles, 6 instances

| Role | Inst | Reuses | Promotes to Done? |
|---|---|---|---|
| **Backlog Worker** | ×2 | worker prompt + worktree | No — submits to In-Review / Blocked / Needs-Decision |
| **Blocked Worker** | ×1 | worker prompt + inlined no-quit doctrine | No — no-quit grind; after 10 genuine attempts an item rotates to the **bottom** of Blocked (never abandoned, just reordered so easier blocked work goes first) |
| **Adjudicator** (decomp expert) | ×1 | new role; inherits cheat policy | No — rules on borderline items. Sanctions ONLY families already on the frozen SOTN list; **novel families and canonical-asm do NOT auto-merge — they park for Trenton (`pending_owner`)**. Every ruling logged. |
| **Auditor** | ×1 | `cheat-reviewer` | **YES — sole promotion authority.** A worker candidate has already passed a layer-1 `cheat-reviewer`; the Auditor is the independent layer-2, and on its PASS a fresh **Verifier** (layer-3) re-derives before the supervisor merges. FAIL ⇒ reject (anti-ratchet: cheat-framed feedback, routed to the Adjudicator every 3rd reject). Idle ⇒ re-audit the COMPLETED log; a regression self-heals (clean redo) + is logged to `regressions.md`. |
| **Overseer** (sysadmin) | ×1 | new role | No — circuit-breaker: on oracle break / toolchain death / contamination / deadlock, quarantine the lane, restore a green oracle, log, resume. |

## State machine (lane states; additive over `engine/queue.json`)

```
 BACKLOG ──claim──> IN_PROGRESS ──┬── match found ──> IN_REVIEW ──[AUDITOR]──┬─ PASS ─> DONE (merged to main)
   ▲   ▲                          │                                          └─ FAIL ─> BACKLOG/BLOCKED (top, +feedback)
   │   │                          ├── stuck ────────> BLOCKED ──[BLOCKED WORKER, no-quit]──> IN_REVIEW / NEEDS_DECISION
   │   │                          │                       └─ 10 genuine tries ─> bottom of BLOCKED
   │   └─ adjudicator: go-back ───┴── borderline ──> NEEDS_DECISION ──[ADJUDICATOR]── legit/canonical ─> IN_REVIEW (ruling attached)
   └──────────────────────────── auditor re-audit regression ────────────────────────────────────────
```

- **Source of incomplete-work + ordering:** `engine/queue.json` (unchanged).
- **Lane state, attempt counts, routing, feedback pointers:** `tmp/fleet/state.json` (operational,
  gitignored, persists on disk so the supervisor resumes after a crash). Mutated only through the
  coordinator CLI with an exclusive lockfile, so concurrent lanes never corrupt it.
- **Feedback persistence:** `memory/wip/<func>/` (reused). Auditor/Adjudicator rejections append a
  dated, technical reason + the rejected form; the next worker MUST read it before re-attempting.
- **Durable human-reviewable record (for Trenton's return):** every completion is a commit on `main`;
  every Adjudicator ruling and Auditor reject is appended to `docs/fleet/log.jsonl` (committed
  periodically) and canonical-asm authorizations are flagged in `docs/fleet/pending_owner.md`.

## Components to build

1. **`tools/fleet/coord.ps1`** — coordinator CLI + `tmp/fleet/state.json` (PowerShell +
   named mutex; reliable Windows cross-process locking). Commands: `init`, `claim -Role -Lane`,
   `submit -Lane -Outcome [...]`, `release`, `sweep` (stranded-claim recovery), `status`,
   `heartbeat`, `circuit`, `reaudit-seed`, `dump`. Reads `engine/queue.json` for backlog
   ordering; owns lane routing, attempt/reject counters (the 10-try blocked rotation +
   every-3rd-reject adjudicator escalation), file-level single-writer exclusion, the
   gate-failed-sha set, the regression ledger, and the In-Review / Needs-Decision /
   Pending-Owner queues. Re-audit selection is inline in the auditor claim branch.
2. **`tools/fleet/roles/*.md`** — thorough role prompts (mindset/roleplay framing, tenacity,
   prime-directive, NO token/$ framing): `backlog-worker.md`, `blocked-worker.md`, `auditor.md`,
   `adjudicator.md`, `overseer.md`.
3. **`tools/fleet/lane.ps1`** — generic lane runner: claim → spawn `claude -p <role-prompt>
   --permission-mode bypassPermissions --session-id <sid>` in the lane's worktree → record outcome
   → repeat. Inherits headless_loop's oracle guardrails.
4. **`tools/fleet/fleet.ps1`** — supervisor: provision worktrees, launch lanes as background jobs,
   monitor + restart dead lanes, run the global circuit-breaker (oracle break ⇒ pause all ⇒ Overseer),
   never push.
5. **Pre-flight drill** (`tools/fleet/drill.ps1`): dry-run → one real item per lane → simulated
   oracle break to prove Overseer recovery → only then go fully unattended.

## The three load-bearing invariants (main stays safe even if a lane is buggy)

1. **Only the Auditor lane merges to main**, holding the reintegration lock. Workers commit only
   inside their own worktree branch; they never run `queue done`, never merge, never push.
2. **The Auditor oracle-gates every merge** (full SHA1 == oracle before *and* after). A merge that
   doesn't reproduce the oracle is rejected and the Overseer is fired.
3. **The Overseer reverts on break.** Worktree work is isolated, so the worst case is discarding a
   worktree; main is restored to the last green commit.

## Anti-cheat is the prime directive (how it's enforced in this system)

- Workers physically **cannot** mark Done — they only produce candidates. **Three independent
  review layers** stand between a candidate and `main`: (1) the worker's mandatory pre-commit
  `cheat-reviewer`; (2) the independent **Auditor** (default-FAIL, full 6-test, worker verdict not
  credited); (3) on the Auditor's PASS, a fresh **Verifier** that re-derives from scratch. The
  supervisor merges only when all three pass. This satisfies — and exceeds — the mandatory
  two-layer-adversarial-acceptance directive (2026-06-10) and defeats the "infinite re-attempts vs
  one gate" ratchet.
- **Canonical-asm and novel SOTN families do NOT auto-merge.** They park to `pending_owner`
  (logged to `docs/fleet/pending_owner.md`) for Trenton's sign-off. This is the one deliberate
  override of full autonomy: an LLM self-authorizing canonical-asm is the single highest
  cheat-on-main risk, and it's irreversible-ish, so it waits for a human. (Flip-able later.)
- **Rejections are framed as "this was a cheat — find a different structure," not "tweak to
  pass,"** and every 3rd rejection routes the function to the Adjudicator for a fresh path —
  so feedback can't ratchet the worker toward an Auditor-acceptable cheat-spelling.
- The privileged **merge can't be bypassed**: `queue done` (0 rules + 0 cheat-asm + SHA1==oracle)
  is re-run on `main` for every promotion; a candidate that doesn't reproduce the oracle is rolled
  back and its sha is marked gate-failed (never re-passed).
- Idle Auditor cycles **re-audit the already-committed COMPLETED log** (910 functions) under the
  current lens; a regression is logged to `docs/fleet/regressions.md` and the function is
  re-opened for a clean pure-C redo that *replaces* the cheat (the byte-correct cheat stays on
  `main` only until the clean version lands — reverting it directly would break the oracle).
- The engine sandbox already makes cheats *score-inert*; combined with three adversarial layers,
  cheating cannot help and is caught. No token/$ pressure is introduced that would tempt shortcuts.

## Out of scope (YAGNI for first launch)

- GitHub Projects board sync (needs API auth that may be unreliable unattended; `state.json` + git
  history is the source of truth; board mirroring can be added later).
- More than 2 backlog workers (start at the requested 2; the supervisor can scale lane count later).

## Build progress

- [x] Spec written
- [x] coord.ps1 + state model + named-mutex atomic lock + smoke tests green
- [x] role prompts (6: backlog/blocked worker, auditor, verifier, adjudicator, overseer)
- [x] lane.ps1 (producers) + _fleet_common.ps1 (shared helpers)
- [x] fleet.ps1 supervisor + inline Auditor/Verifier + Overseer circuit-breaker
- [x] list_completed.py + seed_reaudit.ps1 (re-audit patrol seed: 910 functions)
- [x] adversarial self-review (5 reviewers) → 7 blockers + 14 majors found
- [x] all 8 blockers + safety-critical majors fixed (two-layer→three-layer review,
      conflict-free merge, canonical/novel → pending-owner, no-verdict requeue,
      stranded-claim sweep, gate-failed blacklist, regression self-heal + ledger,
      oracle-retry, main-clean assertion, anti-ratchet reject framing)
- [x] fix-verification re-review
- [ ] pre-flight drill green
- [ ] unattended launch + status report
