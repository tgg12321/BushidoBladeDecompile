---
name: judge-sole-gate
paths: ["tools/grinder/**", "engine/queue.py", "engine/cheats.py", "inline_asm_canonical.txt", "docs/grind/*.md"]
description: "Owner ruling 2026-08-18: no owner-wait states; the default-FAIL Judge is the sole autonomous acceptance gate; canonical-asm grants are driver-executed on STRONG scanner evidence; borderline questions are logged, never waited on. Includes the endgame-lock AND-gates."
metadata:
  type: rules
---

# No owner-wait states; the Judge is the sole gate (owner ruling 2026-08-18)

Owner: *"I want to remove the concept of a user-escalation/approval ... Anything borderline
can be logged somewhere that we can evaluate later down the line, but i dont want work to
just pile up or be 'parked' pending my decisions anymore."* The standards are unchanged and
permanent ("No cheats, SOTN standard, 100% C or hard evidence for inline asm"); the pipeline
executes them.

## Rules

1. **No pending-owner states.** No "awaiting owner ruling" filings, park-and-wait ESCALATE
   routing, or decision packets. Every disposition is terminal when made; an item that does
   not close stays in the worklist ([[rotation-not-foreclosure]]). A would-be question whose
   YES would lower a standard (breach [[completion-bar]] items 2-4, waive the canonical-asm
   evidence bar) is a plain FAIL, never filed.
2. **The Judge is the sole acceptance gate for autonomous work**: default-FAIL, read-only,
   bound to the static policy (`tools/grinder/roles/judge.md`, rubric [[completion-bar]]). Its only ESCALATE
   kinds are the driver-executed mechanical paths: integration handoff / scope grant
   ([[integration-handoff-self-serve]]) and rule 3. The manual path keeps the layer-2
   `cheat-reviewer` ([[review-discipline-before-commit]]).
3. **Canonical-asm authorization is pipeline-executed.** With STRONG
   `tools/scan_hand_coded.py` evidence (S1/S2/S6 class) and a Judge PASS, the DRIVER (never
   the Judge) writes the `inline_asm_canonical.txt` entry citing this ruling + the scanner
   evidence and appends a borderline entry for later audit. Without STRONG evidence asm stays
   refused, except the GTE macro classes in [[inline-asm-policy]].
4. **No extension requests exist (owner ruling Q91).** A construct outside the pre-cleared
   shapes in [[no-new-park-categories]] is decided directly on [[completion-bar]] item 3:
   labelled, honest, not on item 3's refused list, existing types used and the simplest
   known form ⇒ admissible (items 3-5); lying, a refused construct, a second extern / alias symbol / raw-offset cast for declared bytes (item 4),
   a heavier form, or behind the wall ⇒ FAIL. A SOTN citation
   ([[sotn-precedent-suffices]]) is supporting evidence, not a requirement.
5. **Reviewer NEEDS_USER = FAIL + a `needs-user-downgrade` borderline entry.** The agent may
   re-invoke with genuinely NEW evidence but may never re-adjudicate the recorded question.

## Endgame-lock gates (owner policy 2026-07-20, standing 2026-07-27)

For a function a few instructions short in honest C with its sanctioned levers exhausted
(usually an RA/scheduling tiebreak), the only non-C exits are two AND-gates, default refuse:

- **Gate 1 — canonical asm** only with STRONG `scan_hand_coded --single <fn>` signals. A LOW
  tier is dispositive: a compiler-scheduling/RA artifact is ordinary GCC output, never a
  hand-coded signature. Passing ⇒ rule 3's grant path.
- **Gate 2 — a labelled FAKE spelling** under [[completion-bar]] item 3 (Q91): honest,
  annotated with its measured reason, simplest known form. Coercions that breach items 2-4
  (pins, volatile outside its catalog, invented object models) stay refused.

Both fail ⇒ the function stays `INCLUDE_ASM` and in the worklist ([[asm-until-matched]]);
nothing is asked of the owner. Owner: "My standards will never change."

## The borderline ledger — `docs/grind/borderline.md`

Informational; nothing in it is pending. Entries are appended; outdated ones may be deleted by
an owner-directed cleanup (cite a deleted entry via the pin commit named in the ledger header).

```
## YYYY-MM-DD — <function or scope> — <category>
category: canonical-asm-grant | family-candidate | needs-user-downgrade | policy-question
evidence: <pointers, not prose dumps>
disposition taken: <what the pipeline did under current policy>
```

An entry authorizes nothing. Only a later owner ruling does, landed as its own `rules:` commit
before any code spends it.

Related: [[review-discipline-before-commit]] · [[integration-handoff-self-serve]] ·
[[rotation-not-foreclosure]] · [[canonical-asm-authorization-recipe]] · [[inline-asm-policy]]
