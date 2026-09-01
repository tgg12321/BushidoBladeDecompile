---
name: ordinary-c-judge-decidable
paths: [".claude/rules/*.md", "tools/grinder/**", "engine/queue.py", "docs/grind/*.md", "src/*.c"]
description: "Owner ruling 2026-08-31: agent escalations to the owner are retired. Ordinary-C candidates are Judge-decidable against the frozen construct-class list (non-membership = FAIL, never a packet); dead-store deadness is store-level; named-intermediate relaxed to multi-read (SOTN new_var_temp); exhaustion dispositions become the silent `foreclosed` queue state. The anti-cheat wall (no non-C mechanisms, by any spelling) is unchanged."
metadata:
  type: rule
---

# Ordinary C is Judge-decidable — escalations to the owner are retired (owner ruling 2026-08-31)

Owner (Trenton), verbatim, 2026-08-31:

> "My primary concern is agents slipping through new 'cheats' by another
> name, like what historically happened with regfix and asmfix. But if what
> the agent is trying to do is SOTN standard, and not a cheat, i dont want
> them escalating it to me either."

Approved as three rulings ("Okay go ahead", same conversation, after the
operator's proposal was laid out in full).

## The category distinction this ruling rests on

**The regfix/asmfix cheat class was out-of-band mechanisms** — register
pins, asm patches, build-time output rewriting: things living OUTSIDE the
committed C that forced bytes the C didn't produce. That class is dead by
construction (cheat-invisible sandbox, detectors, default-FAIL Judge on
anything asm-shaped, no third byte source) and NOTHING in this ruling
touches that wall.

**Ordinary compilable C is a different category.** A `sandbox --disable
all` score of 0 proves the candidate is a genuine preimage of the original
bytes under the original compiler. Choosing among semantically-truthful C
spellings by observing codegen is the METHOD of matching decompilation,
not a cheat signal. The SOTN norm is exactly this: the bytes decide; the
committed C is a preimage; naturalness is aspirational, not gating.

## Ruling 1 — ordinary-C candidates are Judge-decidable, never escalated

A candidate that is 100% ordinary compilable C (zero asm, pins, pragmas,
gate-list or build changes — mechanically enforced, unchanged) is decided
by the pipeline under a mechanical checklist:

1. **Zero non-C mechanisms** (sandbox + detectors, unchanged).
2. **Construct-class membership**: every no-semantic-purpose construct in
   the candidate sits inside the frozen SOTN-accepted list
   ([[no-new-park-categories]] § SOTN-accepted, as amended by this ruling)
   with that entry's own prerequisites met. The list stays OWNER-ONLY to
   extend — but **non-membership is now a clean FAIL(CONSTRUCT) plus a
   borderline-ledger entry, never an ESCALATE**. The Judge's
   `family-extension` and `policy-question` escalate kinds are retired.
3. **The rename test replaces motive-testing.** Judge and reviewers rule
   on the C TEXT: does each construct have a truthful semantic reading, do
   neutral names survive, is the mandated annotation present? A construct
   with a real semantic reading (e.g. typing a 16-byte GTE scratchpad slot
   as `VECTOR` when the SDK macros consume it as one) is NEVER a cheat
   merely because the agent chose it after observing the scheduler —
   "scheduling-motivated respelling" is not a FAIL ground when the
   spelling is semantically truthful. Constructs with NO semantic reading
   remain governed by their family entries (annotation, exhaustion,
   review), unchanged.
4. **Simplest-known-form**: when multiple byte-exact forms are known, the
   one with the fewest no-semantic-purpose constructs lands.

**Class amendment ratified by this ruling (the SOTN `new_var_temp`
class):** the named-intermediate entry's 2026-08-17 prong (1) is relaxed
from "once-written, once-read" to **"once-written"** — a fresh local
holding a real, consumed value may be read any number of times. Evidence:
SOTN-master PSX `new_var_temp` declarations
(`docs/reference/sotn-construct-index.md:649` — `src/dra/cd.c:520-522`,
`src/dra/42398.c:254-259`, `src/dra/menu.c:1956`, `src/dra/5087C.c:213`,
`src/st/rnz0/e_fire_demon.c:494`, `libsnd/vmanager.c` ×6).

**Evidence caveat, carried verbatim from the record and PRESENTED TO THE
OWNER before approval** (decisions.md:16536-16539): "the index carries
declaration lines only. Whether those temporaries are written once or
several times is NOT verifiable from this repo, so this is precedent for
the existence of fresh RA-purposed locals in SOTN PSX master, not a
demonstration that their shape matches ours line-for-line." The operator's
proposal to the owner restated it in terms: "the SOTN new_var_temp
citations prove fresh RA-purposed locals exist in SOTN master, but not
that their shape matches — so this is a judgment call that's genuinely
yours, once, as a class ruling." The owner ruled WITH that caveat in hand;
this relaxation is an owner judgment call on flagged-as-partial evidence,
not a claim the evidence is line-for-line conclusive.

**Multi-WRITE carriers remain banned** (the y1 FAIL, decisions.md:1833,
and the 2026-08-30 21:30 `c` FAIL both stand). All other prongs (real
value, byte-neutral, fresh not borrowed, destination not
live-pre-initialized, standard prerequisites) are unchanged.

## Ruling 2 — dead-store deadness is STORE-level

In [[dead-store-fake-exception]], the scope sentence governs and the
bullet is amended to match: a store whose STORED VALUE is never read is in
scope **even if the destination variable is later re-assigned and read**
(the defensive-init pattern: `x = a; ... x = b;` with every read after
the second store). That is what GCC's DCE actually deletes and what the
rule's own scope sentence always said. All other prerequisites of that
rule (lever exhaustion, named mechanism, `/* FAKE */` annotation, dual
review) are unchanged.

**Informed-approval record.** This resolution was not presented to the
owner as a neutral textual footnote: the operator's proposal stated in
terms that the scope sentence and bullet "disagree on exactly this case
(a store that's dead but to a variable that is later read)", that the
contradiction was surfaced by the func_80045878 packet, and that under
Rulings 1+2 that function's candidate could close — and the owner
approved knowing that consequence. **The clarification does not by itself
legalize any previously-FAILed instance**: every candidate, including
func_80045878's, is adjudicated fresh by layer-1 + the default-FAIL Judge
against the amended text with all prerequisites verified.

## Ruling 3 — exhaustion dispositions are silent: the `foreclosed` state

The `escalated` queue status and session-filed OWNER-ESCALATION decision
packets are **retired**. A function whose frontier is empty with both
endgame-lock gates failing (the 2026-07-27 standing-ruling shape) takes
the **`foreclosed`** status:

- The driver records the disposition to `docs/grind/decisions.md` +
  `docs/grind/journal.md` (proof-of-foreclosure, evidence pointers) and
  marks the queue item `foreclosed`. **Nothing is surfaced to the owner;
  no packet is filed; no question is asked.**
- `queue next` skips foreclosed items. The function stays
  `INCLUDE_ASM` on main — this path can only ever refuse C, never accept
  it, so it carries zero cheat risk.
- **Re-activation triggers**: a new owner class grant covering the
  function's residual, a toolchain-fidelity finding, or an explicit owner
  `queue unpark`. Foreclosure is a disposition with triggers, never a
  pending decision.

Mechanical questions the pipeline can execute itself (integration
handoffs, scope grants, stale-ban clearance, canonical-asm grants with
qualifying evidence) keep their existing driver-executed paths
([[integration-handoff-self-serve]], [[judge-sole-gate]] rule 3) — those
were never owner-waits and remain the ONLY two Judge ESCALATE kinds.

## Migration (2026-08-31, executed with this ruling)

The 18 `escalated` items resolve as follows:

- **Returned to active**: `func_80045878`, `func_800324D0` (mechanical
  integration question — driver path), `func_80062020` (the owner answers
  its packet's question YES: ruling 6a supersedes the stale
  `banned_constructs` entries; the bans are cleared and the ordered
  adjudication proceeds on the merits). **Nothing about any function's
  outcome is pre-decided by this document**: each returned item's
  candidate is adjudicated fresh — layer-1 + default-FAIL Judge, on the
  merits, against the amended rule text — exactly like any other
  candidate. In particular, whether func_80045878's s13b form satisfies
  the amended prongs and every dead-store prerequisite is the Judge's
  call, not this ruling's.
- **Foreclosed**: the remaining 15 (exhaustion dispositions under the
  2026-07-27 standing ruling; each item's reason points at its latest
  decisions.md entry).

## What this ruling does NOT change

- The completion bar, the oracle, the cheat catalog for non-C mechanisms,
  the frozen family list's owner-only extension, no-deferral, the
  default-FAIL posture of layer-1 and the Judge, and two-layer
  adversarial acceptance for manual work.
- [[escalation-not-parked]] (2026-08-24) is superseded ONLY in disposition
  shape: its two-state model becomes active/foreclosed, its
  decision-packet mechanism is retired. Its AUTO-REJECT class survives
  intact — a would-be packet whose YES lowers a standard is now simply a
  FAIL.

## Related

[[no-new-park-categories]] · [[dead-store-fake-exception]] ·
[[judge-sole-gate]] · [[integration-handoff-self-serve]] ·
[[escalation-not-parked]] · [[completion-standard]] ·
[[no-deferral-work-to-completion]] · [[difficult-is-not-impossible]]
