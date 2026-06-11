---
name: review-discipline-before-commit
paths: ["src/*.c", ".claude/rules/*.md", "CLAUDE.md", "AGENTS.md", "engine/queue.py", "engine/cheats.py", "regfix.txt", "asmfix.txt"]
description: "Standing policy 2026-06-02: every COMPLETED-C / cheat-cleanup / canonical-asm-authorization commit MUST pass an independent adversarial review by the cheat-reviewer agent BEFORE it lands. The orchestrator does not perform this review — it is a separately-defined agent (.claude/agents/cheat-reviewer.md) designed to be critical and adversarial by default. The mechanical gates (sandbox==0, SHA1==oracle, retire succeeded) are necessary but NOT sufficient."
metadata:
  type: rules
---

# Independent adversarial review BEFORE commit

> **Standing policy (user, 2026-06-02):** *"Before an item can be considered
> complete it needs to pass an independent audit from a review agent
> designed to be critical and adversarial. I don't even want the
> orchestrator handling that necessarily — I want a separately defined
> agent we can kickoff to evaluate any given work."*

## What this addresses

The 2026-06-02 techniques audit found that 3 catalog rules were themselves
cheats — including 2 rules the orchestrator added that same day, after
the worker reported COMPLETED-C and the mechanical gates (sandbox==0,
SHA1==oracle, retire succeeded, queue done accepted) all passed. The
post-mortem traced the failure to:

1. **Detectors only catch syntactic patterns they were designed to catch.**
   They are a backstop, NOT the standard. Today's two forbidden techniques
   used entirely normal C syntax (`goto end;`, return-value accumulator,
   local variable declarations); they passed every detector mechanically.
2. **The "vet before surfacing" discipline was scoped to permuter output**,
   not to worker-derived techniques. Workers vetted auto-search proposals
   but did not apply the same checklist to their own derivations.
3. **The orchestrator celebrated and committed without a semantic review
   step.** No agent in the workflow asked "does this C body have semantic
   purpose, or is it codegen-driven?"

The fix codified by this rule: a separately-defined adversarial reviewer
that the orchestrator (or any agent) invokes for every COMPLETED-C class
commit. The reviewer is independent: a different agent, with a different
system prompt, with explicit instructions to default to FAIL.

## The reviewer is `cheat-reviewer` (`.claude/agents/cheat-reviewer.md`)

System prompt is in the agent definition. Key properties:

- **Adversarial by default.** Assumes the work has a cheat until proven
  otherwise. Walks a 6-test checklist (semantic purpose, human-programmer
  test, GCC-internals justification test, "necessary only because
  permuter said so" test, family check, naming-announces-intent test).
  When torn between PASS and FAIL, chooses FAIL.
- **Independent.** A separate agent, not the worker, not the orchestrator.
- **Read-only tools.** Read / Grep / Glob / Bash / PowerShell. Cannot
  modify source; cannot commit. The reviewer surfaces a verdict, never
  bypasses the gate.
- **Structured output.** Returns JSON: `decision` (PASS | FAIL |
  NEEDS_USER), `function`, `summary`, `evidence` array, `next_action`.

## When to invoke

The reviewer MUST be invoked before any of:

- `Match:` commit (COMPLETED-C pure-C match)
- `cheat-cleanup:` commit (rule retirement + COMPLETED-C)
- `auth:` commit (canonical-asm authorization via `inline_asm_canonical.txt`)
- Any commit that drops regfix / asmfix rules for a function
- Any addition to `.claude/rules/` documenting a new "technique"
- Any reclassification of a parked function

The reviewer SHOULD be invoked (judgment call) before:

- `park:` commits with NEW evidence — to vet that the parking decision
  isn't covering for an undocumented cheat
- Engine changes to detector logic — to vet that the change doesn't
  weaken policy enforcement
- Rule edits that relax existing FORBIDDEN classifications

The reviewer MAY be invoked anytime — for spot-checks, for retroactive
audits of historical commits, for vetting permuter output, for vetting a
worker's draft before they commit. It is a tool, not a barrier; the
adversarial design means "more reviews" is "more safety."

## Invocation pattern

The orchestrator (or any agent) invokes the reviewer via the `Agent` tool
with `subagent_type: "cheat-reviewer"`. The brief should include:

1. The function name being reviewed
2. The current HEAD's body (for diff context) — or pointer to the path
3. The PROPOSED new body — or pointer to the path / git ref
4. The worker's technique description (what they derived + how)
5. Any rule references the worker cited as justification
6. The output of mechanical detectors (so reviewer can see what they
   missed, not just what they caught)

The reviewer reads the source, applies the 6-test checklist, optionally
invokes mechanical detectors as backstop, and outputs the JSON verdict.

## What each verdict means

- **PASS** — the reviewer affirmatively walked the checklist and ruled
  out cheat patterns for every construct in the proposed change. The
  worker / orchestrator may commit.
- **FAIL** — the reviewer identified one or more constructs that fail
  the checklist (with evidence). The work MUST NOT be committed. The
  worker reverts their src changes, addresses the specific evidence
  (find a non-cheat lever, re-park with the audit-derived reason, or
  surface a policy question to the user). The reviewer's `next_action`
  field is the path forward.
- **NEEDS_USER** — the reviewer found a borderline construct that
  requires user policy judgment. The orchestrator surfaces the
  reviewer's specific question to the user; commit is deferred.

## Two hard process rules (added 2026-06-10, from the fable retro-audit)

1. **No self-resolved NEEDS_USER.** If the reviewer returns NEEDS_USER, the
   question goes to the user -- period. The worker/orchestrator may add
   evidence and re-invoke the reviewer with NEW facts, but may not
   re-adjudicate the same question against a counter-precedent of their own
   choosing and proceed. (Violation case: `func_80052754`, commit `14d99d6e`
   -- worker revised NEEDS_USER -> PASS against a factually-inapt precedent;
   the retro-audit caught it; the user approved the authorization after the
   fact, but the pathway was wrong.)

2. **No self-sanctioning rule docs.** A `.claude/rules/` addition that
   sanctions a technique used by the SAME commit must be reviewed
   independently (cheat-reviewer on the rule doc itself, with the reviewer
   told the doc author is the technique's author) -- and for genuinely new
   technique families, held for user sign-off with SOTN evidence, NOT
   committed alongside the match. (Violation case: `saFidLoad`, commit
   `478e489d` -- the worker authored narrow-carrier-shared-sext-tail.md in
   the match commit; the retro-audit FAILed the technique; the user ordered
   the revert. The exact failure shape the 2026-06-02 techniques audit
   documented.)

## The orchestrator's role

The orchestrator does NOT perform the review. Per user directive
2026-06-02: *"I don't even want the orchestrator handling that necessarily.
I want a separately defined agent we can kickoff to evaluate any given
work."* The orchestrator's role is:

1. Invoke the reviewer at the right moment (per the "when to invoke" list).
2. Surface the verdict to the user (especially NEEDS_USER cases).
3. On FAIL: never bypass. Either follow `next_action` or surface the
   evidence to the user for direction.
4. On PASS: proceed with commit as normal.

The orchestrator does not get to override the reviewer. The reviewer's
adversarial framing exists precisely so that the orchestrator's
incentive to celebrate progress is checked by an independent process.

## What this does NOT replace

- **Mechanical detectors** (`engine/volatile_cheats.py`,
  `engine/inlineasm.py`, etc.) still run as the first-line backstop.
  The reviewer is the SEMANTIC layer on top.
- **The cheat-by-any-spelling vetting checklist for permuter finds**
  (in [[no-new-park-categories]]) still applies. The reviewer handles
  the worker-derived-technique case the original checklist scoped out.
- **Worker discipline** (the persistence contract, the playbook, the
  catalog rules) still applies. The reviewer is end-of-pipeline; the
  worker doing good work upstream is still the primary mechanism.

## Periodic re-audits

The reviewer can be invoked retroactively to audit COMPLETED-C functions
already committed. This is recommended periodically (e.g. monthly) to
catch:

- New cheat families the catalog has been extended to cover (the
  policy/lens evolves; some historical commits may be cheats under the
  new lens)
- Detector additions that surface previously-undetected patterns

The thorough cheat audit (2026-06-02) and techniques audit (2026-06-02)
are precedents — both ran the reviewer's spiritual checklist over
existing commits and found 25 + 3 affected items respectively. A scheduled
re-audit is the next step.

## Related

- `.claude/agents/cheat-reviewer.md` — the reviewer's full system prompt
- [[no-new-park-categories]] — the policy the reviewer enforces
- [[completion-standard]] — the COMPLETED-C bar
- [[inline-asm-policy]] — the expanded cheat catalog
- `memory/project/techniques-audit-2026-06-02.md` — the audit that
  surfaced the need for this review step
- `memory/project/thorough-cheat-audit-2026-06-02.md` — the audit that
  surfaced the broader scope
