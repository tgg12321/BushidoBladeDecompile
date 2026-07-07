# The Grinder — single-lane walk-away decomp pipeline

**Date:** 2026-07-06
**Status:** Approved by owner (design review 2026-07-06)
**Replaces:** the Autonomous Decomp Fleet (`tools/fleet/`) as the primary autonomous workflow.

## Problem

The fleet treads water. Evidence from `docs/fleet/log.jsonl` (all-time):

- 148 worker lane-cycles → 107 `blocked` (72%), **0 approved / 0 merged**. Every
  `Match:` on main came from hand-driven or solo-loop sessions, never the fleet.
- 24 merge-gate failures — candidates that passed BOTH adversarial review layers,
  then failed the oracle on main (worktree drift): full pipeline cost, zero yield.
- Hard functions attempted 9–12× each in fresh contexts (file_LoadSectors 12×,
  func_80045294 10×, all blocked). Ten shallow passes never accumulate into one
  deep session; each pays full re-orientation.
- 933 main-contamination-cleaned events — pure guard-rail churn.
- The worker prompt already carries maximal anti-quit exhortation. Agents take
  the `blocked` exit 72% of the time anyway. **Prose tenacity does not work; the
  exit must be structurally absent.**

The modality that demonstrably works on the hard tail is the owner's own
deep-dive pattern (marionation_Exec sessions 6a→6O): one function, cumulative
context via a checkpoint ledger, hypothesis discipline, instrumentation
(ALLOCDBG/GREG), permuter + m2c, rejected-forms banking. The Grinder
industrializes that.

## Owner decisions captured in this design

1. **Pre-authorized Judge** — a strongest-model agent carries the owner's static
   policy and rules autonomously (SOTN bar, hard evidence for canonical-asm,
   exhaustion before any FAKE); the owner reads plain-English recaps after the
   fact instead of ruling in real time.
2. **Single lane, end to end** — one focused agent pipeline; no parallel lanes,
   no fleet coordination. The Judge is spawned in for final calls.
3. **Grind until done, period** — the purest no-deferral reading: the pipeline
   stays on the queue-top function however long it takes. Trust comes from the
   ledger: every session must show measured floor descent or newly-eliminated
   hypothesis space. Zero completions for weeks is acceptable if the evidence is
   real.

## Architecture

Four components:

### 1. The Driver (`tools/grinder/grind.ps1`)

A deterministic script — not an agent; it cannot get tired or rationalize.
Infinite loop:

```
read queue top
→ assemble session brief from the ledger (+ mandated modality)
→ spawn ONE headless grind session
→ validate the session's structured output
→ update ledger / revert stray edits / verify oracle
→ repeat (or: candidate proven → spawn Judge → merge or constrain)
```

The driver is stateless between iterations; all state lives on disk (ledger +
queue + git). It is the only continuously-running process. Sessions edit
main's working tree but never commit; the driver is the only thing that
commits to main (and it verifies the oracle around every session).

### 2. The Grind Session

One fresh Opus-tier agent per invocation. Receives: the function, the full
ledger, and a mandated modality. Works one context window deep. Ends by
returning a structured ledger update. Its output schema has **no `blocked`
outcome** — a session either produces a candidate or produces ledger progress
(a hypothesis confirmed/killed with measurements). A session that returns
neither is invalid: discarded, src edits reverted, respawned.

### 3. The Judge

Strongest-model agent, spawned by the driver only at decision points:

- (a) a bytes-proven candidate needs the final cheat-policy call;
- (b) a session requests a ruling mid-grind (FAKE construct, canonical-asm
  evidence, sanction question).

Default-FAIL. The owner's static policy is its system prompt (see Policy).
Every ruling appends a plain-English recap entry to `docs/grind/decisions.md`.

### 4. The Ledger (`memory/grind/<func>/`)

The persistent brain — what makes session N+1 deeper than session N. Schema:

- **`state.json`** (machine state the driver reads): `session_count`,
  `current_modality`, `floor_history` (one line per session: session, floor,
  modality, headline), `frontier` (≤3 live hypotheses, each with a named GCC
  mechanism and a concrete next probe), `judge_constraints` (rulings that bind
  all future sessions).
- **`evidence.md`** (append-only facts bank): instrumentation results, m2c
  digests, sibling comparisons, permuter findings. Facts never expire; sessions
  cite instead of re-running.
- **`hypotheses.md`** (append-only): statement → mechanism → probe → measured
  result → verdict (CONFIRMED/KILLED). A killed hypothesis with numbers is
  progress: searched space.
- **`candidate.c` + `rejected/`**: best form so far; every rejected form named
  for the violated rule. Sessions may not re-submit anything in `rejected/`.

Caps are structural, not truncation: frontier ≤3; floor_history unlimited;
evidence/hypotheses append-only (nothing compacted away). This deliberately
diverges from the WIP compaction contract — the grind ledger is the pipeline's
working memory, not a human-readable summary.

## The loop, end to end

```
queue top → [session 1: recon + baseline] → ledger
          → [sessions 2..k: grind mandated modality] → ledger each time
          → candidate reaches sandbox-0
          → driver verifies bytes ITSELF: retire → full build → SHA1 == oracle
          → only then spawn Judge for the semantic call
          → PASS: queue done, commit "Match:", delete ledger, next function
          → FAIL: judge's reason enters judge_constraints; grind continues
```

Nothing merges without bytes; the Judge never reviews a non-match. Work happens
directly on main (solo-loop style), eliminating worktree drift, reintegration-
lock contention, and contamination churn. The driver holds the tree clean
between sessions: src edits either survive as part of a verified candidate or
are reverted into the `rejected/` bank at session end.

Exactly one grind session is alive at any time. `grind stop` (or a sentinel
file) stops the loop cleanly at the next session boundary.

## The modality ladder (driver-enforced)

The driver assigns each session's modality from ledger state; the agent does
not choose:

1. **Recon** (session 1): baseline floor, canonical route, sibling/duplicate
   scan, m2c reference read. Output = initial frontier.
2. **Structural levers** (2–3): block-local splits, decl order, type
   narrowing, the codegen-technique-index catalog.
3. **Directed permuter** (4–5): `permuter_annotate` on the diverging region;
   findings are proposals vetted against the cheat catalog.
4. **Compiler forensics** (6–7): instrumented cc1 RTL/ALLOCDBG/GREG dumps;
   name the exact pass and decision producing the divergence.
5. **Re-derivation** (8–9): fresh m2c re-decompile, decomp.me corpus,
   Kengo/sibling transplant — a structurally different C shape.
6. **Synthesis** (10): re-read the whole ledger, write the merged attack,
   reset the frontier. Ladder repeats from 2.

A modality is "spent" only when its ledger section shows real output (dumps
captured, forms swept with scores) — checked mechanically by the driver, so a
session cannot skip a rung with prose.

## The Judge's policy (the owner's static rulings, encoded)

- **SOTN standard or nothing.** A technique is sanctioned only if it is on the
  frozen SOTN list with prerequisites met. First-reach or un-annotated use of
  an exception = FAIL.
- **Hard evidence for asm.** Canonical-asm requires STRONG hand-coded signals
  (existing S1/S2/S6 criteria). The canonical gate's verdict is input, never
  proof.
- **Exhaustion before FAKE.** Any `/* FAKE */` construct requires: the full
  modality ladder spent (verified against `hypotheses.md`, not the agent's
  claim), a named GCC-pass mechanism, and the annotation — all three.
- **Default-FAIL, evidence-cited.** Every verdict quotes the ledger lines it
  relied on.

Recap surface: `docs/grind/decisions.md` — date, function, question, verdict,
plain-English justification. This is the owner's post-hoc audit trail.

## Anti-cheat / anti-quit (structural, not exhortational)

- Give-up: **unrepresentable** (no blocked outcome; invalid sessions discarded
  and respawned).
- Cheats can't score: the sandbox strips them (existing engine, unchanged).
- Cheats can't merge: the driver diffs every session — any edit outside the
  function's own `src/`/`include/` files and the grind ledger (specifically any
  touch of `regfix.txt`, `asmfix.txt`, `.claude/rules/`, `engine/`, `tools/`,
  `Makefile`, `*.ld`) auto-rejects the session before any review. Rule
  deletions during `retire` are performed by the driver, never by a session.
- Cheats can't pass: bytes-first verification, then default-FAIL Judge.
- Banned forms can't return: `rejected/` bank + `judge_constraints` are in
  every session brief.

## Failure handling

- **Driver crash / reboot:** relaunch resumes from disk state. `grind.lock`
  pidfile prevents double-launch; a stale lock (dead PID) is auto-reclaimed
  with a logged notice.
- **Session crash / hang / invalid output:** per-session wall-clock timeout;
  dead or invalid sessions are logged, src edits reverted, respawned. Three
  consecutive invalid sessions ⇒ circuit-break.
- **Oracle break:** SHA1 checked before and after every session; any break ⇒
  revert to last green, log incident, continue. Main is never left red or
  dirty between sessions.
- **Circuit-break (the only true halt):** toolchain death, repeated invalid
  sessions, unexpected git state. Driver stops, writes `docs/grind/INCIDENT.md`
  with full context, optionally notifies the owner. It never limps along.
- **Judge/API transient failures:** exponential backoff and retry. A
  bytes-proven candidate waits; nothing is discarded for a transient failure.

## Progress visibility

- **`grind status`**: current function, session #, modality, floor trajectory,
  frontier vs killed-hypothesis count, completions ledger (every `Match:` with
  sessions-taken), tail of `decisions.md`.
- **`docs/grind/journal.md`**: daily one-paragraph digest appended by the
  driver — mechanical summary, no agent prose.
- Returning from time away: read journal (what happened), decisions.md (what
  the Judge ruled and why), `grind status` (where it stands).

## Migration

- **Fleet:** retired in place; `tools/fleet/` kept for reference this campaign,
  deleted after the grinder lands its first few completions. Fleet worktrees
  removed via `tools/safe_remove_worktree.ps1`.
- **Queue:** `engine/queue.json` remains the single worklist; the grinder
  consumes it top-down.
- **Existing WIP checkpoints:** converted to seed ledgers on first contact
  (notes.md → opening evidence entries; rejected/ carries over; meta.json
  floors seed floor_history). Nothing already learned is lost.
- **The 99 regressions (owner ruling 2026-07-06 — fold in):** re-opened as
  ordinary INCOMPLETE queue items; everything gets worked eventually. Each
  item's audit diagnosis (from `docs/fleet/regressions.md`) seeds its grind
  ledger's `evidence.md`, so the grinder starts knowing exactly which construct
  was flagged and why. The Judge's completion bar for these items adds one
  clause: the match must land without the flagged construct family. Queue
  ordering is the standard easiest-first rank (a one-time triage pass strips
  each flagged construct and measures the honest floor — inert constructs make
  the item near-free; load-bearing ones rank by their real distance).
  `regressions.md` retires as a work-tracker once its items are queue members;
  the flagged-but-byte-correct code stays on main only until each clean redo
  lands.
- **Engine, sandbox, oracle, guards:** unchanged. The grinder is a new driver
  on the same proven engine.

## Validation

1. **Unit:** driver logic (modality assignment, invalid-session detection,
   ledger validation) gets a test suite alongside `engine test`.
2. **Drill:** supervised dry run on one known-completable function (a WIP item
   with a documented low floor) — recon → grind → candidate → Judge → real
   `Match:` end to end.
3. **Adversarial drill:** a deliberately-cheated candidate fed to the Judge
   must FAIL; a session emitting blocked-style prose with no measurements must
   be discarded by the driver.
4. **First unattended window:** 24h on the queue top, then owner reviews
   journal + decisions once. Honest evidence trail = walk-away achieved.

## Non-goals

- No token/time/cost framing anywhere in the system ([[no-budget-caps]]).
- No parallel lanes, no rotation, no tractability re-ranking (owner ruled:
  grind until done, period).
- No changes to the engine's scoring, canonical gate, or oracle.
