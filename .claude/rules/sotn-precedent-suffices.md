---
name: sotn-precedent-suffices
paths: [".claude/rules/sotn-precedent-suffices.md"]
description: "Owner ruling Q50 (2026-09-30): a construct that verifiably exists in matched PS1-build SOTN code is admissible on that citation (Q55 precedence over older refusals; Q51 reuse; Q52 self-marked fakes; Q53: annotation blocking, other paperwork hygiene since Q91). Supporting evidence since Q91, not a requirement."
metadata:
  type: rules
  tier: hygiene
---

# SOTN precedent suffices (owner ruling 2026-09-30, Q50; Q51-Q53, Q55)

> **Tier (owner ruling Q91):** hygiene, except that conditions (1)-(4) below and the
> `/* SOTN: */` tag are blocking whenever a citation admits an item 2-3 refusal (Q55,
> [[completion-bar]] item 3).

Owner: *"SOTN precedent is good enough for any constructs if they verifiably exist in the
SOTN repo"*. "Verifiably" (author's reading):
1. **A PS1-build file**: a file:line in a `config/splat.us.*` / `splat.hd.*` member (not
   PSP/Saturn, not inside a non-PS1 version guard), naming the commit read (local clone
   `C:/Users/Trenton/Desktop/sotn-decomp` @db41b28), C source only. A header construct counts
   with a splat-member use site that itself meets (2) and (4).
2. **Same thing when read**: open the file and read the construct in context; a shape-index
   hit is not a citation.
3. **Layer-2 verifies** (1), (2), (4) against the SOTN source. Manual path only; a Judge PASS
   is not enough.
4. **Matched code (Q55)**: not `INCLUDE_ASM`/`INCLUDE_RODATA`, not `NON_MATCHING`/disabled
   under SOTN's PS1 defines, not in a function SOTN still carries as asm.

Psyz and other decompilations are not evidence. Every SOTN-admitted construct carries
`/* SOTN: <file>:<line> @<commit> */` (no symbol names in the tag).

- **Q55 precedence:** a citation meeting (1)-(4) admits a construct an older rule refuses
  (inline-asm default ban, volatile two-prong, fabricated dead call, "may not be re-proposed"),
  with Q53's prerequisites.
- **Q51 reuse:** a variable reused exactly as a cited SOTN variable is reused (same roles,
  written and read at corresponding statements) is admitted without a Ruling 5-11 package.
- **Q52 self-marked fakes:** a construct SOTN marks as hack/debt (any `FAKE`/`fake`/`hack`/
  `TODO`/`FIXME` comment, hack-named label/identifier/macro, `HACKS`-only code) counts, and
  ours carries `/* FAKE: ... */`.
- **Q53 paperwork (tiered by Q91):** match-motivated ⇒ `/* FAKE */` annotation and layer-2
  review are blocking ([[completion-bar]] items 3, 6); exhaustion and byte-neutrality write-ups
  are hygiene. Simplest known form (item 5) still applies.

A Q50 admission is not a frozen-list extension. Records: docs/grind/decisions.md 2026-09-30
OWNER RULING — SOTN precedent suffices, and the Q51-Q53, Q55 entries.

Related: [[no-new-park-categories]] · [[ordinary-c-judge-decidable]]
