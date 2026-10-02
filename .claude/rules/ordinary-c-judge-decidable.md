---
name: ordinary-c-judge-decidable
paths: ["tools/grinder/**", "engine/queue.py", "docs/grind/*.md"]
description: "Owner rulings 2026-08-31+: ordinary C is judged on the C text (Ruling 1(3)); no-semantic-purpose constructs are labelled FAKEs under completion-bar item 3 (Q91); dead-store deadness is store-level; reused-local Rulings 5-12 (dossiers hygiene); clearly-fine unprecedented C may PASS (13B)."
metadata:
  type: rule
  tier: blocking
---

> **Owner ruling Q91 (2026-10-02):** the completion bar is [[completion-bar]]. Where this file
> and that one differ, that one wins: Ruling 1(2) below is replaced by its item 3, and the
> Rulings 5-12 dossiers are hygiene tier.

# Ordinary C is Judge-decidable (owner ruling 2026-08-31 and later rulings)

Owner: *"My primary concern is agents slipping through new 'cheats' by another name, like what
historically happened with regfix and asmfix. But if what the agent is trying to do is SOTN
standard, and not a cheat, i dont want them escalating it to me either."* Out-of-band mechanisms
(pins, asm patches, build-time rewriting) stay dead by construction. Choosing among truthful C
spellings by observing codegen is the METHOD of matching decompilation, not a cheat signal.
Each ruling's question and verbatim answer: docs/grind/owner-rulings-2026-09-26.md and the
docs/grind/decisions.md OWNER RULING entry named in its heading.

## Ruling 1 — the checklist (no escalation)

1. **Zero non-C mechanisms** (sandbox + detectors).
2. **Labelled hacks (Q91).** Every no-semantic-purpose construct carries a
   `/* FAKE: <measured reason> */` annotation and asserts nothing false ([[completion-bar]]
   item 3). The pre-cleared shapes in [[no-new-park-categories]] are examples; a construct
   outside them is judged on item 3 directly, never failed for non-membership.
3. **The rename test**: judge the C TEXT. Does each construct have a truthful semantic
   reading, do neutral names survive, is the mandated annotation present? A semantically
   truthful spelling is never a cheat merely because it was chosen after observing the
   scheduler. Constructs with no semantic reading are labelled FAKEs ([[completion-bar]] item 3).
4. **Simplest-known-form**: of several byte-exact forms, the one with the fewest
   no-semantic-purpose constructs lands.

**Named intermediate relaxed to "once-written"** (SOTN `new_var_temp` class): a fresh local
holding a real, consumed value may be read any number of times. Evidence caveat presented to
the owner: SOTN's index shows such locals exist, not that their shape matches line-for-line.
**Multi-WRITE locals (Q91):** one that meets a Ruling 5-12 or Q51 shape is ordinary C; one
that does not is a reused local with no semantic reading, admissible as a labelled FAKE under
[[completion-bar]] item 3 (`/* FAKE: reused for <measured reason> */`). The Rulings 5-12
dossiers below are hygiene tier.

## Ruling 2 — dead-store deadness is STORE-level

In [[dead-store-fake-exception]], a store whose STORED VALUE is never read is in scope even if
the variable is later re-assigned and read (`x = a; ... x = b;`, all reads after the second).
All other prerequisites of that rule are unchanged. No previously FAILed instance is
legalized; each is judged fresh.

(Ruling 3, the silent `foreclosed` state, is superseded by [[rotation-not-foreclosure]].)

## Ruling 4 — compound-assignment splits are ordinary C (2026-09-02)

`ratio *= 0x103B; ratio >>= 12;` for `ratio = (ratio * 0x103B) >> 12;`, or `v = a; v += b;`,
is truthful C under Ruling 1(3) when no statement is dead, no annotation is needed and no pad is
introduced. A carrier whose extra write is dead is a dead store: a labelled FAKE under
[[completion-bar]] item 3 when it is the simplest known form.

## Rulings 5-12 — multi-write locals and copies (on-demand files)

- Ruling 5 (+ extension) one role repeated per block; Ruling 6 one record pointer per exclusive
  path → [[reused-local-one-role]]
- Ruling 7 sprintf only; Ruling 8 vmNoiseOn only; Ruling 9 (+ b′) one meaning at several constant
  offsets; Ruling 10 verified original-source reuse → [[reused-local-meaning-source]]
- Ruling 11 reused local proven necessary (Q20/Q28/Q34 value clauses, Q30/Q31/Q58 proof
  standard); Ruling 12 copy of a stack-passed parameter → [[reused-local-necessity]]

(Hygiene tier since Q91: the dossiers are good practice and recorded as debt when skipped.)

## Ruling 13 (B) — clearly-fine ordinary C without precedent (standing, owner Q64 2026-09-30)

"No precedent" is not a FAIL ground for a construct with a truthful semantic reading under
Ruling 1(3), writable from the function's behaviour, doing real consumed work (no dead store,
no no-op copy, nothing the compiler deletes, no pad/dummy). The reviewer still judges it on its
merits ("in doubt, FAIL"), and any entry or ruling that governs the construct decides instead.
**The wall stays (13 (D), standing):** no pins; no hardcoded-`$N` or non-canonical `__asm__`;
no scheduling barriers; no build/Makefile/linker/gate/compiler change that alters bytes; no
build-time assembly rewriting; nothing refused by an owner ruling on these grounds
([[completion-bar]] item 2). The run-scoped orchestrator-decision and rotation clauses of Rulings 13/14 have
expired.

## Owner rulings on narrow spellings (2026-10-01)

Q76, Q82 and Q90 are instances of [[completion-bar]] item 3 (labelled FAKEs); Q77 is
re-opened by Q91 clause D.

- **Q76 — fixed-point rescale, func_80058580 only.** At its two `lh; sra 4` sites (target 0x8005AB34 and 0x8005ACCC),
  `0x200 - ((x * 0x100) >> 12)` (4.12 multiply by 1/16, the function's own `* k >> 12` idiom on
  that field) is admitted for `0x200 - (x >> 4)`, with a comment at each site naming the 4.12
  factor and the measured cse shift fold. Nothing cancels; not a precedent elsewhere.
- **Q77 — Q45 stands for func_8005C8A8** (re-opened by Q91 clause D). Neither the sibling end-pointer form
  (`end_off = arg2 + 0x4F0; size = end_off - arg2;`) nor end-of-chunk minus start through a
  layout struct (`(u8 *)((T *)arg2 + 1) - (u8 *)arg2`) is admitted: both compute a constant by
  cancellation. The function stays open for a new mechanism.
- **Q82 — one s16 compare, func_80058580 only.** `(s16)work2 < score` at the target's
  `sll; sra 16; slt` (0x8005A338), work2 holding the Q75 best-score value, is admitted despite the
  redundant-width-cast refusal, with a comment citing the sign-extend pair, the Q75 value and the measured
  alternatives. No other cast or site.
- **Q90 — one u32 class bit-set, func_80023F08 only.** At its move-class test
  `(mask = ent[2] | (ent[3] << 16)) & (1 << rec->unk_0A)`, `mask` may be a `u32` local although its only
  effect is the int->u32 conversion that keeps fold-const.c:4437 from rewriting the bit test (int form 3
  off). The comment names that rewrite and the measurements (`c8a67edaf:memory/grind/func_80023F08/casts/receipts.txt`,
  `c8a67edaf:memory/grind/func_80023F08/fake/mask_jump.txt`); fresh layer-2. No other unsigned-widening device, local or site.

Related: [[no-new-park-categories]] · [[dead-store-fake-exception]] · [[judge-sole-gate]] ·
[[review-discipline-before-commit]] · [[staged-value-reused-variable]]
