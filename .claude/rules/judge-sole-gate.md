---
name: judge-sole-gate
description: "OWNER RULING 2026-08-18 — user-escalation/approval is REMOVED from the workflow. The default-FAIL Judge (frozen static policy) is the sole acceptance gate; borderline material is LOGGED to docs/grind/borderline.md for later owner review, never parked pending a decision. The bar itself is unchanged: no cheats, SOTN standard, 100% C or hard evidence for inline asm."
paths: [".claude/rules/*.md", "tools/grinder/**", "engine/queue.py", "engine/cheats.py", "inline_asm_canonical.txt", "docs/grind/*.md"]
metadata:
  type: rules
---

# Owner ruling 2026-08-18 — no user-escalation/approval; Judge is the sole gate

Owner (Trenton), verbatim, 2026-08-18:

> *"I want to remove the concept of a user-escalation/approval. Our workflow
> should be mature enough now that we can prevent anything egregious from
> slipping through by relying on the Judge to reject anything that isnt SOTN
> standard. Anything borderline can be logged somewhere that we can evaluate
> later down the line, but i dont want work to just pile up or be 'parked'
> pending my decisions anymore."*

This generalizes the 2026-07-27 standing auto-ruling
([[endgame-lock-disposition]]) to the FULL escalation surface: the two
residual owner-wait paths (canonical-asm sign-off, family-sanction requests)
and the reviewer's blocking NEEDS_USER verdict are retired. **The standards
are unchanged and permanent** ("No cheats, SOTN standard, 100% C or hard
evidence for inline asm") — what changes is WHO executes them: the pipeline,
never a wait on the owner.

## The five operative rules

1. **No pending-owner states, anywhere.** The park reason "pending owner
   ruling", decisions.md filings titled "awaiting owner ruling — do not
   self-resolve", and the Judge's park-and-wait ESCALATE routing are all
   retired. Every disposition is terminal when made. Terminal
   OWNER-ACCEPTED INCOMPLETE parks (standing ruling 2026-07-27) remain —
   they are dispositions, not pending decisions.

2. **The Judge is the sole acceptance gate for autonomous work.** It stays
   default-FAIL, read-only, and bound to the FROZEN static policy
   (`tools/grinder/roles/judge.md`). Its ESCALATE verdict is redefined:
   "sound work, grant above my authority" no longer exists as a wait —
   see rules 3 and 4 for the two cases it used to cover. The manual path
   keeps the layer-2 `cheat-reviewer` gate ([[review-discipline-before-commit]]).

3. **Canonical-asm authorization is pipeline-executed.** When
   `tools/scan_hand_coded.py` shows STRONG (S1/S2/S6-class) evidence and
   the candidate passes the Judge, the DRIVER (not the Judge — the Judge
   never mutates) writes the `inline_asm_canonical.txt` entry citing this
   ruling + the scanner evidence, and appends a borderline-ledger entry so
   the owner can audit the grant later. This formalizes the owner's
   2026-06-13 directive ([[self-authorize-within-parameters]]) and the
   GTE-wrapper auto-authorize precedent ([[gte-wrapper-misroute-park]]).
   Without STRONG evidence, asm remains refused — nothing here lowers the
   evidence bar.

4. **The frozen SOTN family list remains OWNER-ONLY to extend — but
   extension requests never wait.** A candidate family extension, even
   with exhibited SOTN-master precedent, is NOT granted by any agent or by
   the Judge (a Judge that extends its own policy source is no longer
   default-FAIL). It is logged to the borderline ledger with the exhibited
   evidence, and the function takes the standing disposition under the
   CURRENT list (refusal / terminal OWNER-ACCEPTED INCOMPLETE park per
   [[endgame-lock-disposition]]). The owner reviews the ledger in batches;
   a later ruling can unfreeze specific entries and the function becomes
   re-attemptable.

5. **Reviewer NEEDS_USER is retired as a blocking state.** A
   `cheat-reviewer` NEEDS_USER verdict maps to **FAIL + borderline-ledger
   entry** (the work is not committed; the question is recorded, not
   answered by the agent). The no-self-resolution rule stands in stronger
   form: the agent may not re-adjudicate — the ledger entry IS the
   disposition.

## The borderline ledger — `docs/grind/borderline.md`

Append-only, informational (nothing in it is pending). Entry schema:

```
## YYYY-MM-DD — <function or scope> — <category>
category: canonical-asm-grant | family-candidate | needs-user-downgrade | policy-question
evidence: <scanner output / SOTN citation / reviewer question — pointers, not prose dumps>
disposition taken: <what the pipeline actually did under current policy>
```

The owner evaluates entries later at their own cadence. An entry never
authorizes anything by itself; only a subsequent owner ruling (landed per
[[ruling-record-lands-before-code]]) spends it.

## What this does NOT change

- The completion bar ([[completion-standard]]), the cheat catalog and
  cheats-by-any-spelling posture ([[no-new-park-categories]]), the frozen
  family list contents, the two-layer adversarial acceptance for manual
  work, the oracle, and the no-deferral queue discipline.
- New conversational owner rulings still land per
  [[ruling-record-lands-before-code]] before code spends them.
- Project-architecture decisions that are genuinely substrate-wide (e.g.
  the global rodata reorder behind JTBL-INFRA) are logged as
  `policy-question` entries — they proceed only on a landed owner ruling,
  but no function sits blocked on them (JTBL-INFRA's queue bucket is empty).

## Migration notes

- Pre-existing "awaiting owner ruling" filings in docs/grind/decisions.md
  are re-processed under this ruling as they are next touched: STRONG-
  evidence canonical-asm requests take the rule-3 grant path; family
  candidates take the rule-4 log-and-refuse path.
- The engine queue's `authorize` bucket no longer means "needs user
  sign-off"; it means "needs the pipeline grant path" (currently empty).

## Related

[[endgame-lock-disposition]] · [[review-discipline-before-commit]] ·
[[self-authorize-within-parameters]] · [[no-park-permanently]] ·
[[ruling-record-lands-before-code]] · [[hand-coded-asm-recognition]] ·
[[canonical-asm-authorization-recipe]]
