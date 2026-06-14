# Design: GitHub Kanban board + multi-agent coordination for BB2 decomp

- **Date:** 2026-06-13
- **Status:** Approved (design) — Phase 1 ready for implementation planning
- **Author:** Trenton (with Claude)
- **Scope note:** This is an umbrella architecture doc. **Phase 1 (Board sync) is specified in
  full here and is what we build first.** Phases 2–5 are designed at roadmap level and each gets
  its own detailed spec when reached.

## 1. Motivation

Two problems, in priority order:

1. **Visibility / progress (primary).** Today the only view into the ~448 remaining
   cheat-carrying functions is `engine/queue.json` + the SessionStart hook. There is no
   glanceable, big-picture, "how close am I / where is the long tail stuck" surface (incl. from
   a phone).
2. **Multi-agent coordination (the real pain).** Running multiple agents on the repo at once is
   hard. The difficulty is **shared, load-bearing state**, not just "who is doing what":
   - `src/*.c` + `regfix.txt`/`asmfix.txt` — two agents editing the same file textually conflict.
   - `build/` — the sandbox scores against the pristine `build/`; `verify-oracle --rebuild` and
     `queue done`'s full build *write* to it. Concurrent rebuilds corrupt the shared oracle
     reference. (This is the silent killer.)
   - `engine/queue.json` + the "work the top item" rule — N agents reading "the top" grab the
     *same* function.

### Existing substrate (and its scar tissue)

- `tools/setup_worktree.ps1` bootstraps a worktree by **junctioning** the heavy gitignored deps
  (`.venv`, `build/`, `gcc-2.7.2`, `maspsx`, `disc`) from main — cheap, no multi-GB copy.
- **Hazard 1 — shared `build/`:** junctioned, so any worktree's full rebuild mutates main's
  oracle reference. Safe for `sandbox` (builds to `tmp/`); unsafe for concurrent `verify-oracle
  --rebuild` / `queue done`.
- **Hazard 2 — cleanup:** `git worktree remove --force` follows junctions and on 2026-06-03
  wiped main's `.venv`. Must use `tools/safe_remove_worktree.ps1`.
- **Hazard 3 — HEAD staleness:** the Workflow `isolation:'worktree'` framework has created
  worktrees lagging main; a worker once built a hypothesis on stale `regfix.txt` (see
  `.claude/rules/verify-claims-against-main.md`). `setup_worktree.ps1` now hard-resets to main
  HEAD.

The coordination design must respect all three.

## 2. Goals / non-goals

**Goals**
- An always-accurate GitHub Project mirroring engine state (visibility).
- A collision-proof way for several agents (manual sessions primarily, orchestrator fan-out
  occasionally) to work disjoint functions in parallel.
- Zero risk to the engine's authority and the oracle invariant.

**Non-goals**
- Driving *what gets worked next* by dragging cards (rejected: would make the board
  co-authoritative — see §3).
- Running the build/oracle in GitHub CI (toolchain is WSL-local + gitignored; impractical).
- A per-function GitHub Issue for all 1410 functions (PRs serve as per-function threads instead).

## 3. Locked decisions

- **Approach 1 — board is a one-way projection; the git remote is the atomic claim primitive.**
  `engine/queue.json` remains the **sole source of truth**. The board can never write back into
  engine state in Phase 1. Claims (Phase 2) are atomic because creating/pushing a branch fails if
  it already exists — a real compare-and-swap the Projects v2 API cannot provide.
  - *Rejected — Approach 2 (board-authoritative kanban):* dual source of truth vs the engine's
    mechanical gates, non-atomic claim races, reimplementing easiest-first ordering + completion
    gating on a flakier store. This is where drift bugs live.
  - *Rejected — Approach 3 (dashboard only):* solves visibility but not the coordination pain.
- **Board scope = "Remaining board + Done archive."** Working board holds the ~448 remaining
  (incomplete) functions; the ~962 completed are seeded as **archived** Project items (full
  searchable record, off the kanban). Verified safe: Projects v2 caps are 50,000 active items
  (Feb 2025) and archived items don't count against the active limit.
- **Items = Project draft items**, not real Issues. When a function is actively worked (Phase 2+),
  its PR becomes the per-function thread + commit cross-link.
- **Sync = manual, idempotent reconcile** first; later wired into `queue regen` / `queue done`.

## 4. Architecture (5 layers, Approach 1)

```
engine/queue.json ──(one-way render)──▶ tools/board_sync.py ──gh GraphQL──▶ GitHub Project
   (source of truth)                         (reconcile)                    (mirror/dashboard)
        ▲                                                                        │
        │  git branch decomp/<func>  ◀── atomic claim (push fails if exists) ───┘
        │  worktree (isolated edit loop, sandbox→tmp/)
        └── serialized LAND on main: full build + SHA1==oracle gate → queue done → PR merge
```

**Invariant preserved by construction:** any single *correct* function replacement is a
byte-identical swap, so a worktree built from `main + <one fixed func>` still yields full
SHA1 == oracle. Independent function matches therefore compose; the only conflicts are textual
(two agents touching the same `src` file), resolved at land time by rebase.

## 5. Phase 1 — Board sync (full spec; build this first)

### 5.1 GitHub Project

- One user Project named **"BB2 Decomp"** under `tgg12321` (remote
  `tgg12321/BushidoBladeDecompile`). `gh` is already authenticated with the `project` scope.
- Created/owned via `gh api graphql` (Projects v2). The script ensures the project + its fields
  exist (create-if-missing), so first run bootstraps everything.

### 5.2 Status field (single-select), created up-front with all forward-compatible values

`Backlog · Needs-Decision · Blocked · In-Progress · In-Review · Done`

Phase 1 populates only **Backlog**, **Needs-Decision**, **Blocked**, and archives **Done**.
`In-Progress` / `In-Review` are reserved for Phase 2 but the field is created with them now so the
schema is stable.

**Mapping from engine state → Status — column is the engine's own `status` field, 1:1, no
interpretation** (the engine taxonomy is `_STATUS_RANK = {active, authorize, parked}`, with
`authorize` set for `verdict ∈ _AUTHORIZE = {ASM-WHOLE, ASM-STRUCTURAL, JTBL-INFRA}`):

| Engine `status` | Board Status | Notes |
|---|---|---|
| `active` | Backlog | the pure-C grind worklist, all verdicts (C / ASM-PARTIAL / ASM-SUSPECT), distance-ordered |
| `authorize` | Needs-Decision | needs your canonical-asm sign-off (verdict ASM-WHOLE / ASM-STRUCTURAL / JTBL-INFRA) |
| `parked` | Blocked | preserved across `queue regen` |
| completed (not in queue) | Done → archived | |

Mapping strictly by `status` keeps board == engine with zero board-side reinterpretation. A parked
item that is *also* canonical-asm-worthy stays in Blocked (that's its engine status); because
`verdict` is a card field, a saved board view "Blocked + verdict = ASM-STRUCTURAL" surfaces those
for decision without board_sync inventing its own routing. Promoting such items to `authorize` is
an engine-side `queue regen` concern, deliberately kept out of board_sync.

### 5.3 Card fields (custom fields on the Project)

| Field | Type | Source |
|---|---|---|
| `func` (item title) | text | queue.json `func` |
| `file` | single-select / text | queue.json `file` |
| `verdict` | single-select | queue.json `verdict` |
| `distance` | number | queue.json `distance` (enables easiest-first sort mirroring the queue) |
| `rules` | number | queue.json `rules` |
| `wip` | single-select (yes/no) | presence of `memory/wip/<func>/` |
| `permuter` | single-select (yes/no) | optional flag; near-empty per `permuter-closability` memory — a field, **not** a column |
| `owner` / `pr` | text | reserved for Phase 2 (claim/land); unset in Phase 1 |

Field budget: well under the 50-fields-per-project cap.

### 5.4 Components (`tools/board_sync.py`, three units)

1. **State collector** — reads `engine/queue.json`, scans `memory/wip/*`, reads
   `inline_asm_canonical.txt`; computes the full function inventory to derive the completed set
   (**Done = all functions − queue items**). Produces `desired_state`: `{func: {status, file,
   verdict, distance, rules, wip, archived}}`.
   - *Implementation detail for the plan:* the full 1410-function inventory's exact source
     (splat function list / `symbol_addrs.txt` / asm `glabel` scan / an engine helper) is selected
     in the implementation plan. Requirement: a deterministic, complete enumeration so
     `completed = inventory − queue` is exact.
2. **Board client** — thin `gh api graphql` wrapper behind a small interface (so it's mockable in
   tests): ensure-project, ensure-fields, list-items (paginated), add-draft-item, update-field,
   archive-item. Caches GraphQL node IDs within a run.
3. **Reconciler** — diffs `desired_state` vs the current board snapshot; emits a **minimal** change
   set: add new items, update changed fields, move on status change, archive on completion.
   **Never deletes and never delete-and-recreates** (that would lose card history/comments/positions).
   Matches items to functions by the `func` title/field.

### 5.5 Data flow

engine files → `desired_state` → fetch current board snapshot → diff → apply mutations → archive
completed. A second run with no engine changes produces **zero** mutations (idempotent).

### 5.6 CLI

- `python tools/board_sync.py` — reconcile (apply).
- `--dry-run` — print the diff/change set, mutate nothing.
- `--project <number|title>` — target project (default "BB2 Decomp").
- `--no-archive-seed` — skip the one-time ~962 archived-Done backfill (for fast iterative runs).

(Windows-side, like `tools/metrics/sync.py`. Invoked from PowerShell or the Bash tool.)

### 5.7 Error handling

- Idempotent and safe to re-run; each mutation is independent so partial failure is recoverable by
  re-running.
- **Never deletes** board data — only add / update / archive.
- Clear, actionable failure if `gh` is unauthenticated or offline (mirrors `metrics/sync.py`'s
  "staleness-only, never data-loss" contract — the engine is unaffected either way).
- Respects GraphQL rate limits (batch mutations, backoff on secondary-rate-limit responses).

### 5.8 Testing

- Engine-style fixtures test: feed a synthetic `queue.json` + a fake board snapshot through the
  reconciler and assert (a) the correct minimal diff, (b) idempotency (run-twice → no-ops),
  (c) it never emits a delete. The board client is mocked at its interface.
- Manual smoke: `--dry-run` against the live project, then a real run, then a second run that
  must show zero mutations.

### 5.9 Out of scope for Phase 1

Claims, worktree changes, PRs, landing, the agent entry protocol, and any board→engine write-back.
Phase 1 is read-only-from-engine projection only.

## 6. Phases 2–5 — roadmap (each gets its own spec when reached)

- **P2 — Claim / lease.** Claim a function by `git worktree add ../bb2-wt-<func> -b decomp/<func>`
  then `git push -u origin decomp/<func>`; the push **fails if the branch exists remotely** = the
  atomic lock. `board_sync` reads `decomp/*` branches → cards show `In-Progress` + `owner`.
  Release on land or abandon (delete branch). Adds board→engine read of branch state only (still
  no write-back into queue.json).
- **P3 — Isolation hardening.** The edit loop already uses `sandbox` (builds to `tmp/`, isolated).
  Eliminate the shared-`build/` hazard by making the authoritative full-SHA1 gate a **serialized
  land step** (single writer to `build/`), not a per-worktree rebuild. Scale-up option if the land
  step becomes a bottleneck: a private `build/` per worktree (disk-heavy, fully parallel verify).
  Standardize cleanup on `safe_remove_worktree.ps1`.
- **P4 — Reintegration.** PR per `decomp/<func>` branch → main. Merge gate is the **local** oracle
  SHA1 at land time (CI cannot build). PR body carries the engine report (sandbox 0, rules retired,
  canonical-gate verdict). On merge: `queue done` runs → `board_sync` moves the card to Done →
  archived.
- **P5 — Agent entry protocol.** A new skill + a CLAUDE.md revision (lift "single focused agent on
  main, no worktrees" → "parallel claimed worktrees; main is the serialized land target"). An agent
  (manual session *or* orchestrator worker) follows: `board_sync` → pick top unclaimed Backlog card
  → claim (worktree + branch push) → `setup_worktree.ps1` → run the engine loop → request land →
  on success `queue done` + PR + board reflects → `safe_remove_worktree.ps1`.

## 7. Risks & open items

- **Item-limit:** resolved — 50k active / archived excluded.
- **GraphQL rate limits** during the one-time ~962 archived-Done seed: mitigate with batching +
  `--no-archive-seed` for iterative runs; the seed is a once-only operation.
- **Completed-function enumeration source** (§5.4): pick the deterministic inventory source in the
  implementation plan.
- **Sync drift if engine state changes without a sync run:** acceptable in Phase 1 (manual sync);
  Phase 1+ wires sync into `queue regen` / `queue done` so the board self-heals.
- **Claim race (P2):** mitigated by git push atomicity, not the Projects API — the core reason for
  Approach 1.

## 8. Phase 1 acceptance criteria

1. `python tools/board_sync.py --dry-run` prints a correct change set against a fresh project.
2. A real run creates "BB2 Decomp" with the Status field + custom fields and populates the ~448
   remaining (incomplete) cards across Backlog / Needs-Decision / Blocked per their engine
   `status`, with Backlog sorted by `distance`.
3. The ~962 completed functions exist as archived items (or `--no-archive-seed` skips them).
4. An immediate second run produces **zero** mutations (idempotent).
5. The fixtures test (§5.8) passes and is green in `engine test` or an equivalent harness.
6. The engine, the oracle, and `queue.json` are untouched by the sync (one-way projection).
