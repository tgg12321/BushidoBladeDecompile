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
- **`self_vet.md`** (added later): mandatory self-vet record for any
  `candidate-ready` claim (see `grindlib.py`).
- **`migration_pin.json`** (2026-08-19 asm-until-matched migration): the
  honest floor measured at migration, read by `engine/queue.py` for queue
  ordering when the body is INCLUDE_ASM. See
  `.claude/rules/asm-until-matched.md`.
- **`retired-chassis-2026-08/`** (same migration): banked pre-migration body
  (`body.c`) + retired rule stack (`rules.txt`).

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

> **Amended 2026-08-19** (R1/R2/R3, `docs/grind/modality-effectiveness-2026-08-19.md`,
> owner ruling in `.claude/rules/asm-until-matched.md`): synthesis moved to
> rung 5 (session 6), the ladder never cycles a second time (one flat cycle ⇒
> escalation), and permuter is hard-capped at 2 sessions per function. The
> live order is `grindlib.py:LADDER`, which is authoritative:
> `structural ×2 → permuter ×2 → synthesis → forensics ×2 → rederive ×2`.

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

---

## Addendum — the review-pipeline hardening (owner-approved 2026-08-07 per review audit)

An owner-approved audit of the Judge / layer-2 pipeline (raw findings in
`tmp/review_audit/`, principally `fails.txt` + `entries.json`) measured an 8.3%
FAIL rate on submissions and, more importantly, WHY those FAILs happened:

- **46% of FAILs were cheat-by-spelling where the worker prompt never contained
  the standards.** The session was judged against a policy it had never been
  shown.
- **10 of 18 sampled FAILs cited a rule whose own text excluded the construct** —
  the session had a rule name but not the rule's scope sentence.
- **23% were authority artifacts**: the work was sound and the Judge simply had
  no verdict for "this grant is above my authority", so it FAILed and the driver
  re-ground a finished function.
- **59% of FAILs sat in respelling loops** — the constraint existed but bound
  nothing, so the next session re-proposed the same construct differently.

Six changes. Five were owner-approved 2026-08-07 from the audit findings; the sixth was added the same day after the drill incident described below:

1. **Standards front-loaded into the session role.** `roles/grind-session.md`
   now carries, VERBATIM, the 6-test cheat checklist, the frozen SOTN-accepted
   list with its non-extension clause, the ~25-family forbidden catalog, and the
   `/* FAKE: <what>, mechanism: <named pass>, lever-exhaustion: <where> */`
   template with its three prerequisites - plus the governing line: *first reach
   of an unsanctioned family is a cheat regardless of spelling; if no family
   covers your construct, the answer is a ruling request, not a submission.*
2. **Self-vet artifact + layer-1 gate.** A `candidate-ready` outcome now requires
   `memory/grind/<func>/self_vet.md`: the 6 tests answered in writing per
   construct, every claimed family carrying its rule's SCOPE sentence quoted
   verbatim plus a precedent as file:line or commit hash, and an
   annotation-conformance line. The driver rejects a candidate without it as an
   INVALID SESSION (discard + respawn, same as a scope violation). Before any
   Judge cycle, the driver spawns the `cheat-reviewer` agent on the diff +
   self-vet; a layer-1 FAIL short-circuits back to the worker. Layer-1 FAILS OPEN
   on an unreachable reviewer - the default-FAIL Judge remains the authoritative
   gate, and layer-1 must never become a second way to lose a proven candidate to
   an API hiccup.
3. **Binding no-respelling.** A construct-class FAIL now (a) force-advances the
   modality ladder via a persistent `ladder_skip`, so the next session attacks
   differently instead of respelling; (b) records the construct in
   `state.json.banned_constructs`, and the driver rejects a later candidate whose
   self-vet re-declares it; (c) if the FAIL's stated ground is ANNOTATION FORMAT
   ONLY, routes the next session to a one-shot `annotation-fix` brief - fix the
   comment, resubmit, no new constructs permitted.
4. **ESCALATE verdict.** `judge.md`'s schema is now `PASS|FAIL|ESCALATE`, where
   ESCALATE means *the work is sound and complete but the grant is above my
   authority* (rule extension, new family, owner-policy question). The driver
   files an OWNER-ESCALATION and parks the function exactly like an owner-gated
   park, instead of triggering a re-grind.
   > **SUPERSEDED 2026-08-18/19**: `.claude/rules/judge-sole-gate.md` retired
   > owner-pending states (ESCALATE resolves under standing policy — terminal
   > park or pipeline grant, never a wait), and
   > `.claude/rules/integration-handoff-self-serve.md` makes
   > `escalate_kind=integration-handoff` driver-executed with the function
   > STAYING ACTIVE (`grind.ps1` add-scope-allow path).
   Default-FAIL is unchanged: uncertainty
   about whether a construct is a CHEAT is still a FAIL. The Judge also now
   states `fail_ground` (CONSTRUCT / EVIDENCE / ANNOTATION-FORMAT) and
   `banned_construct`, which is what makes change 3 mechanical.
5. **Review metrics.** Every review boundary emits a `review` event into
   `metrics/events.jsonl` - `{func, layer: layer1|judge|layer2, verdict, cause}`
   - via `tools/grinder/record_review.py`, which reuses `engine.metrics`'
   silent/best-effort/never-raises append contract. The audit had to mine ledgers
   and commit prose precisely because no event ever recorded a verdict.

6. **Clean-tree preflight on the drill.** `tools/grinder/drill.ps1` now refuses
   to run when `git status --porcelain` is non-empty (metrics/events.jsonl
   excepted — it is append-only telemetry that engine commands touch
   constantly), printing the dirty paths and exiting 2. There is deliberately no
   `-Force` override. The guard belongs to the DRILL WRAPPER ONLY: grind.ps1's
   own `-Once` path keeps the discard behaviour, which is correct for a real
   session.

### Why fix 6 exists — the 2026-08-07 drill incident

The drills are not read-only. Drill A and Drill B each spawn `grind.ps1 -Once`,
and Drill B deliberately provokes the driver's invalid-session discard path:
`git checkout -- .` plus `git clean -fdq -e memory -e docs -e src -e include`,
applied to the ENTIRE worktree rather than to the drill's own changes.

During this implementation session the drill was run as a verification step
while another agent had a large naming wave in progress. The first iteration
classified that agent's work as a SCOPE VIOLATION and discarded it: **342 files
carrying unstaged modifications** (`asm/funcs/*.s` plus `asm/data/7D920.data.s`)
were reset to their staged content, and an untracked file under `tools/` was
deleted. Two properties limited the damage and are worth remembering:
`git checkout -- .` restores from the INDEX, so **staged work survived** (all 714
staged entries came through intact), and HEAD never moved, so nothing was lost
from history. The affected paths were recoverable from the driver's own
`tmp/grind/grind.log` SCOPE VIOLATION line, which lists every path with its
porcelain status code.

The lesson is not "be careful with the drill" — it is that a destructive tool
must refuse unsafe input rather than rely on the operator remembering. Hence the
preflight. Operator note: both drills (plain and `-WithJudge`) remain REQUIRED
before relaunching the Grinder on these changes, and must be run on a clean tree.

Surfaces changed: `tools/grinder/roles/grind-session.md`,
`tools/grinder/roles/judge.md`, `tools/grinder/grindlib.py`,
`tools/grinder/grind.ps1`, `tools/grinder/drill.ps1`,
`tools/grinder/record_review.py` (new),
`tools/grinder/tests/test_grindlib.py`.

## Addendum — review-loop breaker (2026-09-04)

**Incident.** func_80062020 (s14–s19) produced ONE pure-C body that byte-matched
the oracle six times. In one evening it drew five layer-1 FAILs and three Judge
PASS rulings; func_80072CD4 began the same alternation an hour later. Cause: the
layer-1 gate ran on every submission, nothing in the driver bound it to a
per-function Judge PASS ruling (the brief named only *family grants* as
supersession), and its FAIL summaries were stored in `judge_constraints`, so it
cited its own earlier verdicts as precedent ("already FAILed four times today").
Every FAIL also banned the construct and force-advanced the modality, so sessions
were spent re-asking the Judge the same question.

**Mechanism (all in `grindlib.py` + `grind.ps1`, covered by
`TestReviewLoopBreaker`).** Every review verdict is keyed by a BODY HASH of the
candidate (`body_hash`: C comments stripped, whitespace collapsed, so a
comments-only re-file is the same body). `state.json` gains `review_ledger`
(`{layer, verdict, hash, when, summary}`), `judge_clearances`, and
`reviewer_history`. At `candidate-ready`, before layer-1, the driver asks
`review-disposition`:

| disposition | meaning | driver action |
|---|---|---|
| `judge-cleared` | the Judge's LAST word on this body is a PASS ruling | skip layer-1 → bytes → FINAL CALL |
| `layer1-repeat` | no Judge word; layer-1 already FAILed this body | skip layer-1 → bytes → FINAL CALL (the Judge decides once) |
| `judge-failed` | the Judge's LAST word is a FINAL CALL FAIL | reject with NO review spent; constraint + modality advance |
| `fresh` | never reviewed | normal layer-1 gate |

A Judge PASS on a *ruling request* records a clearance of `candidate.c`'s body at
ruling time. Layer-1 FAIL findings go to `reviewer_history` (the brief renders
them as "what the reviewer looks for", not constraints; legacy
`LAYER-1 CHEAT-REVIEWER FAIL` lines in `judge_constraints` are split out at
render). Both review briefs receive a `REVIEW RECORD` block (dated clearances +
every prior verdict on this body) and the precedence rule: the Judge outranks
layer-1; a later dated Judge PASS ruling supersedes older bans/FAILs; a
reviewer's own earlier FAILs are never a ground.

Surfaces changed: `tools/grinder/grindlib.py`, `tools/grinder/grind.ps1`,
`tools/grinder/roles/judge.md`, `tools/grinder/roles/grind-session.md`,
`.claude/agents/cheat-reviewer.md`, `tools/grinder/tests/test_grindlib.py`.
