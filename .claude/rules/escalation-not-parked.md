---
name: escalation-not-parked
paths: ["engine/queue.py", "tools/grinder/**", ".claude/rules/*.md", "docs/grind/*.md"]
description: "Owner ruling 2026-08-24: the parked state is RETIRED. Two dispositions only — ACTIVE (grinding) or ESCALATED (a concrete decision packet owed an owner ruling; batched, non-blocking, never indefinite). Every escalation returns to active once ruled."
metadata:
  type: rules
---

# Owner ruling 2026-08-24 — escalation replaces parking

> **SUPERSEDED AGAIN (owner ruling 2026-09-08, [[rotation-not-foreclosure]]):**
> the `foreclosed` state described below is retired in turn. Exhausted
> functions are ROTATED to the back of the active worklist and return
> automatically (queue drain / toolchain change / sibling movement).

> **SUPERSEDED IN DISPOSITION SHAPE (owner ruling 2026-08-31,
> [[ordinary-c-judge-decidable]]):** the ESCALATED state and the
> decision-packet mechanism are retired. Exhaustion dispositions take the
> silent `foreclosed` queue status (recorded to decisions.md/journal,
> skipped by `queue next`, re-activated on new evidence or owner unpark —
> never surfaced as a question). Ordinary-C construct questions are
> Judge-decidable against the frozen family list (non-membership = FAIL +
> borderline log, never a packet). This file's AUTO-REJECT class and its
> "everything must be decompiled eventually" posture survive unchanged.

Owner (Trenton), verbatim:

> "I don't want anything parked anymore. Everything has to be decompiled
> eventually. There should be no reason to park an item indefinitely. It
> should just be either escalated for a decision on our part, or kicked back
> for further grinding if it isn't up to our standards."

## The two-state model

Every INCOMPLETE function is in exactly ONE of:

- **ACTIVE** — in the distance-ordered grind lane. The grinder works the top;
  no deferral, no cherry-picking (unchanged).
- **ESCALATED** — carries a **decision packet**: a concrete, decidable
  question only the owner can answer (a grant, a family question, a
  toolchain-fidelity question, a routing question). The grinder skips
  escalated items. An escalation is NEVER indefinite: once the owner rules,
  the item RETURNS TO ACTIVE — either with the grant applied, or with a
  refusal plus (implicitly) "keep grinding under standing policy."

There is no third state. "Terminal park", "OWNER-ACCEPTED INCOMPLETE",
"park permanently" do not exist as dispositions.

## Decision-packet requirements

An item may be escalated ONLY with a packet containing:
1. **The question**, phrased so a yes/no (or option-select) answer is
   executable without re-litigation.
2. **The evidence**, as pointers (ledger, decisions.md lines, measurements)
   — not prose dumps.
3. **The consequence of each answer** (what closes, what resumes, at what
   floor).

"This function is hard" is not a packet.

## The AUTO-REJECT class (owner ruling 2026-08-24, second ruling)

Owner, verbatim: "We won't be sanctioning any kind of permanent rules...
That may as well be an auto-reject if it's considered. We are going to keep
things to the highest standard we can. If there is no SOTN precedent and it
feels like backsliding on our own standards, then it should not be
considered."

A packet whose YES answer would LOWER a standard is not a decision — it is
pre-decided NO and MUST NOT be filed as an escalation. The class includes:
- sanctioning any rule/cheat as PERMANENT (there is no permanent-carrier
  state; every rule retires at COMPLETED-C);
- granting a coercion family with no in-hand SOTN-master precedent;
- overriding the canonical-asm evidence bar (a LOW scan tier is an answer,
  not an obstacle to be waived);
- any "accept the debt" disposition in new wording.
Functions whose only known closer is auto-reject-class stay ACTIVE and keep
grinding under standing policy (difficult-is-not-impossible) — the honest
escalations that remain are fidelity/routing/provenance questions and
genuinely gate-PASSING evidence. Exhaustion without a decidable
question means the item stays ACTIVE and the modality changes
(difficult-is-not-impossible, no-deferral — both unchanged).

## Relationship to judge-sole-gate (2026-08-18)

This ruling AMENDS judge-sole-gate rule 1: a pending-owner state exists
again, but only in the batched, non-blocking form above — the pipeline keeps
grinding the rest of the queue; the owner rules on packets at their own
cadence; nothing else waits. The Judge remains the sole acceptance gate for
completions; the borderline ledger remains the audit trail (an escalation's
resolution is recorded there and in decisions.md). What changes is that a
both-gates-fail endgame lock now produces an ESCALATED item with a packet
instead of a terminal park.

## Supersessions

- endgame-lock-disposition option (b) "terminal OWNER-ACCEPTED INCOMPLETE
  park" → escalate with packet (the two AND-gates are unchanged as the
  STANDARD; only the disposition shape changes).
- [[no-park-permanently]] — strengthened: not only is no park permanent,
  the park state itself is gone.
- Queue statuses: `parked` is migrated to `escalated` (2026-08-24); the
  legacy park_reason strings are preserved as history on each item.

## Related

[[judge-sole-gate]] · [[endgame-lock-disposition]] ·
[[no-deferral-work-to-completion]] · [[difficult-is-not-impossible]] ·
[[asm-until-matched]]
