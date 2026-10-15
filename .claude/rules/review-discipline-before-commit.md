---
name: review-discipline-before-commit
paths: [".claude/rules/*.md", "CLAUDE.md", "AGENTS.md", "engine/queue.py", "engine/cheats.py"]
description: "Every COMPLETED-C / cheat-cleanup / canonical-asm-authorization commit (and every rule doc that sanctions a technique) MUST pass a fresh default-FAIL cheat-reviewer (layer-2), recorded with `layer2 record`, BEFORE it lands. Mechanical gates are necessary, not sufficient. Tools/engine/naming commits take one code reviewer instead; every review stops at two rounds (Q120)."
metadata:
  type: rules
  tier: blocking
---

# Independent adversarial review BEFORE commit

Owner (2026-06-02): *"Before an item can be considered complete it needs to pass an
independent audit from a review agent designed to be critical and adversarial."* Detectors
catch only the syntax they were built for; sandbox 0 + SHA1 == oracle cannot see a cheat that
uses ordinary C. The semantic layer is a separate agent.

## The reviewer: `cheat-reviewer` (`.claude/agents/cheat-reviewer.md`)

Default-FAIL, independent (not the worker, not the orchestrator), read-only, returns JSON
`decision` (PASS | FAIL | NEEDS_USER), `function`, `summary`, `evidence`, `next_action`, and the
`body_hash` it reviewed. Its rubric is [[completion-bar]] (owner ruling Q91): every FAIL cites
one of its items; hygiene gaps are reported as `hygiene_debt`, never as a FAIL.

## When

MUST, before: `Match:` (COMPLETED-C), `cheat-cleanup:`, `auth:` (canonical-asm row) commits; any
addition to `.claude/rules/` that sanctions a technique; any reclassification of a function's
completion state; rule edits that relax a FORBIDDEN classification. MAY, any time (spot
checks, retro-audits, vetting permuter output).

**Tools, engine, registry and naming commits (owner ruling Q120, 2026-10-15):** no
cheat-reviewer. One fresh code reviewer; fix what it finds, then one re-check of the exact final
diff before commit. (Naming waves keep their `naming-reviewer` passes, [[naming-bar]].)

**Round cap (Q120), every reviewer kind:** at most two rounds. After round 2 a finding blocks
only if it can occur in the real tree (a shape present in `src/` / `include/` or that the
current workflow produces); hypothetical or invalid-C cases go in the commit body as hygiene
debt.

Brief it with: the function; the current and proposed bodies (or paths/refs); the technique
and how it was derived; every rule/precedent cited; the mechanical detector output.

## Layer 2 is what accepts (owner directive 2026-06-10)

An in-session review by the worker is layer 1 and provisional. Acceptance needs a FRESH
`cheat-reviewer` spawned by the orchestrator with an adversarial brief (worker's verdict not
credited; rule-doc changes in the commit audited too). Split verdicts are FAIL; wait for every
reviewer you started.

**Recorded and enforced (owner ruling Q39, 2026-09-29).** Record every layer-2 verdict:
`python3 -m engine.cli layer2 record <func> --reviewer <id> --scope <match|cheat-cleanup|auth>
--verdict-file <json>` (or `--verdict <V> --expect-hash <h>`; `<h>` = the reviewer's
`body_hash`, from `layer2 hash <func>`). A PASS is refused if `src/` no longer hashes to it.
Appends to `memory/grind/<func>/layer2.jsonl`; commit it with the landing. `queue done` refuses
unless the LATEST record is a PASS on the CURRENT definition: re-review after any change to
the function's definition (comment/layout edits inside it keep the hash). Renaming a callee or
global changes the key of every function referencing it and voids their PENDING PASSes;
completed functions are judged by `engine/departures.py` against the body that left the queue.
No override flag. The Grinder records the Judge's final call (`grinder-final-call` scope).

## Verdicts

- **PASS** — commit may proceed.
- **FAIL** — do not commit; follow `next_action` (another lever, or log a genuine policy
  question to the borderline ledger). Never bypass, never override.
- **NEEDS_USER** — FAIL + a `needs-user-downgrade` entry in `docs/grind/borderline.md`
  ([[judge-sole-gate]] rule 5). Re-invoke only with genuinely NEW evidence; never
  re-adjudicate the recorded question against a precedent of your own choosing.

## No self-sanctioning rule docs

A `.claude/rules/` addition that sanctions a technique used by the same commit is reviewed
independently (reviewer told the doc author is the technique's author). A rule for a genuinely
new technique family NEVER ships in the match commit that uses it: describe the finding in the
commit message or ledger and register the rule separately after its own layer-2 and a landed
owner ruling.

The orchestrator never performs the review itself and never overrides it. Mechanical detectors
(`engine/volatile_cheats.py`, `engine/inlineasm.py`) and the permuter vetting checklist still
run first; the reviewer is the semantic layer on top. Periodic retro-audits of landed
COMPLETED-C functions are encouraged.

Related: [[completion-bar]] · [[no-new-park-categories]] · [[judge-sole-gate]] · [[inline-asm-policy]]
