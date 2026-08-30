---
name: asm-until-matched
paths: ["src/*.c", "tools/grinder/**", "engine/queue.py"]
description: "Owner ruling 2026-08-19 — NO cheat is committed on main in any form; a not-yet-decompiled function is committed as INCLUDE_ASM(\"asm/funcs\", <func>); C lands exactly once, at COMPLETED-C; candidates live in memory/grind/<func>/ only. Plus the modality-ladder retune (R1/R2/R3/R5)."
metadata:
  type: rule
---

# ASM-until-matched (owner ruling 2026-08-19)

> Owner: "I want agents to just continue to work on functions until they are
> fully decompiled with no cheats at all." — adopting the community (SOTN)
> convention for representing unfinished work.

## The rule

1. **Main carries no cheats, in any spelling, for any INCOMPLETE function.**
   No register pins, no cheat-asm, no coercion
   constructs. The committed representation of "not yet decompiled" is
   exactly:

       INCLUDE_ASM("asm/funcs", <func>);

   — the original bytes, honestly labeled, oracle-green by construction.
2. **C lands on main exactly once per function: at COMPLETED-C**, through the
   unchanged gates (sandbox 0 → layer-1 → Judge default-FAIL →
   verify-oracle → queue done). There is no intermediate committed state.
3. **In-progress candidates and retired chassis live in the
   function's ledger** (`memory/grind/<func>/`): `candidate.c` is the working
   frontier; `retired-chassis-2026-08/` banks the pre-migration body + rules;
   `migration_pin.json` records the honest floor at migration for queue
   ordering (engine/queue.py reads it when the body is INCLUDE_ASM).
4. **The rule system is GONE.** The historical regfix/asmfix build-time rule
   machinery reached zero rules on 2026-08-25 and was removed entirely
   (files, pipeline stages, guard) on 2026-08-30. There is no mechanism to
   patch assembly at build time, and none may be reintroduced.
5. This supersedes the 2026-08-19 chassis-refresh proposal (never
   implemented) — the owner refused committing any cheat-class artifact,
   including honest whole-body splices paired with draft C.

## Why (evidence)

- The cheat-invisible sandbox already made cheats unable to affect scoring;
  what they still did was **mislead**: rule-calibrated committed bodies sent
  sessions down dead paths (CD_datasync burned s1–s6 on rule-era constructs
  the reference later indicted) and PINNED stale chassis on main (its banked
  floor-7 candidate was uncommittable against 15 calibrated rules; the queue
  lied 18 vs 7).
- The 2026-08-19 modality report: 45% of all grinder sessions went to eight
  functions whose floors moved 9 times in 344 sessions — grinding against
  stale chassis and a cycling ladder.

## Ladder retune (adopted same ruling, from docs/grind/modality-effectiveness-2026-08-19.md)

- **R1** — one full flat ladder cycle ⇒ escalation; never cycle the ladder a
  second time (evidence: 1 drop in 285 cycle-2+ transitions). No-deferral is
  preserved: escalation IS the modality change; the target never rotates.
- **R2** — the single synthesis pass runs at s6 (before forensics/rederive),
  not s10. Revert trigger: ~15 early-synthesis sessions with zero drops.
- **R3** — hard cap: at most 2 permuter sessions per function, ever
  (3rd+ measured 0/64).
- **R5** — the driver records the closing session's modality in the journal
  and the Match commit.
- R4 (repeat-FAIL construct matching) is deliberately NOT adopted yet —
  needs a design that can't deadlock legitimate work; spec separately.

## Related

[[completion-standard]] · [[community-standard]] · `no-new-regfix-rules` (retired rule, deleted 2026-08-30) ·
[[judge-sole-gate]] · [[integration-handoff-self-serve]] ·
[[no-deferral-work-to-completion]] — unchanged by this ruling: the queue is
still the only worklist and the top item is still worked to completion.
