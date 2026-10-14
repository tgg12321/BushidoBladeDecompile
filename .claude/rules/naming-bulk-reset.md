---
name: naming-bulk-reset
paths: [".claude/rules/naming-bulk-reset.md"]
description: "Owner ruling Q119 (2026-10-14): unreviewed claim-making local and parameter names in game code are reset to neutral in bulk waves, certified by the mechanical gates and a sampling R6, without per-row evidence. Generic, type-confirmed, GPU and geometry names stay. Members, types, globals, functions and files keep normal waves."
metadata:
  type: rule
  tier: hygiene
---

# Bulk reset of local and parameter claims (owner ruling Q119, 2026-10-14)

The owner chose to wipe the unverified claim-making locals instead of investigating each one
(*"yes go with the wipe plan"*). A neutral name makes no claim ([[naming-bar]] N2), so a reset
to neutral cannot add a false one; it may drop a true name, which a later normal wave can restore
with evidence. This file changes only how such resets are reviewed; the rest of [[naming-bar]]
holds.

## Scope

Locals and parameters of function definitions in `src/main/*.c` (not `src/main/psxsdk/**`),
renamed by `f:l=n` pairs. A name is in scope unless it is:
- **neutral:** `argN`, `varN` / `var_N`, `tempN`, `spXX`, register-style `sN` / `tN`, single
  letters, `unk` / `fXX` / `field_XX` forms;
- **generic:** built only from domain-free words (`count`, `idx`, `ptr`, `buf`, `len`, `size`,
  `tmp`, `ret`, `val`, `x`/`y`/`z`/`w`, `pos`, `vec`, `mat`, `src`/`dst`, `flags`, `mask`,
  `offset`, `base`, `cur`/`next`/`prev`, `end`, `str`, `id`; a word not plainly of this kind
  counts as in scope, so a disputed row errs toward neutral);
- **type-confirmed:** its word names its declared type (`rect` declared `RECT`), or it is a PsyQ
  GPU term for a value of that GPU type (`ot`, `prim`, `clut`, `tpage`, `tile`, `env`);
- **geometry:** `sin`, `cos`, `ang` / `angle`, `rot`, `dist`, `scale`, and the coordinate
  abbreviations `cx`/`cy`, `sx`/`sy`, `du`/`dv`, `row`/`col`;
- **reviewed:** the CONFIRMed `proposed_name` of a `naming-reviewer`-passed row in an earlier
  wave's `local_manifest.csv`, on the same binding (Q119 rows do not count).

Parameter names in prototypes (headers, forward declarations) are not renamed by `f:l=n`; a
claim there is left for a normal wave.

Out-of-scope names stay; a normal wave resets one only on a contradicting use. Members, types,
globals, functions and file names are never bulk-reset.

## The wave

1. **Rows:** one row per in-scope (function, name) in `local_manifest.csv`, evidence
   `RESET-BULK (Q119)` plus the old name and `file:line`, verifier_note `Q119 scope test`. The old name is a hint for later
   naming, never evidence ([[naming-bar]] N3). Destinations: a parameter becomes `arg<index>`
   (0-based position), a local `var_<n>` (`n` by declaration order in the function). A
   destination refused for freshness takes the next free `var_<n>`; any other keycheck refusal
   (K&R target, shadowing, ambiguous labels) drops the row.
2. **No pre-apply review.** The rows are CONFIRMed by the scope test above.
3. **Apply and gates:** [[naming-bar]] steps 4-5 unchanged (comments naming an old local change
   by the pair word in the wave; the full keycheck, the oracle SHA1, stage-first).
4. **R6 (post-apply, fresh `naming-reviewer`):** the keycheck ends OK with exactly the
   manifest's pairs; every moved layer-2 key is explained; it samples at least 20 rows or 10%
   (the larger) and FAILs the wave if any sampled row is out of scope or its destination breaks
   step 1; a comment edit that still asserts the wiped claim FAILs it. It does not ask whether a
   use supports the old name.
5. **Commit:** `naming: <wave> -- bulk local reset (Q119)`, body as [[naming-bar]] step 7, with
   the pre-apply verdict recorded as `none (Q119 scope test)`.

A name reset here gets a non-neutral name again only through a normal wave row (N1 and a
pre-apply review), usually when the function is next worked on.
