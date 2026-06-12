---
name: or-tree-shape-shift
paths: [".claude/rules/or-tree-shape-shift.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "Reordering operands or reparenthesizing an associative+commutative OR/AND expression solely to shift cc1's RTL OR-tree shape is a cheat-by-spelling. The same family as DImode chain and combine-foldable chain-extender."
metadata:
  type: rule
---

# Parenthesization-axis mutations in associative+commutative expressions are cheats

## Status

**FORBIDDEN as of 2026-06-06.** Codified after the directed-permuter
campaigns on func_8007CBB0 (round-14, commit `d4872471`) and
func_8007C97C (round-14, commit `e621af5a`) — run on the same day —
both independently surfaced sub-baseline forms that achieved
significant masked-distance gains (CBB0 41→17, C97C 27→26) via the
same mechanism: reordering operands in a multi-way `|` (or `&`)
expression to change cc1's RTL OR-tree shape. **Both forms were
caught by the cheat-reviewer and recorded in `rejected_forms/`.**

The construct's only effect is to flip cc1's `expand_binop_LSHIFT_OR` /
combine pass tree topology, which propagates to different INSN_PRIORITY
sort orders at sched.c. The runtime value is identical
(associative+commutative semantics); the bytes differ because the
scheduler picks a different emission order from the differently-shaped
RTL tree.

## What this looks like

### Confirmed instance 1 — func_8007CBB0 round-14 (display.c GP0 packet builder)

```c
/* CANDIDATE BASELINE (score 41, PASS-vetted): */
(&D_800F1858)[5] = (*D_8009BF48 & 0x7FF) | (((arg1 >> 31) << 10) | 0xE1000000);

/* THE CHEAT (score 17, REJECTED): */
(&D_800F1858)[5] = ((arg1 >> 31) << 10) | (*D_8009BF48 & 0x7FF) | 0xE1000000;
```

The three operands `cmd << 10`, `BF48 & 0x7FF`, `0xE1000000` are
permuted across the three positions. Empirically:
- `cmd | BF48 | 0xE1` → score 23
- `BF48 | cmd | 0xE1` → score 36
- `cmd | 0xE1 | BF48` → score 17 (the cheat that scored best)

The "right" position for the dither operand was determined by
ENUMERATION, not by program logic.

### Confirmed instance 2 — func_8007C97C round-14 (display.c GP0 packet builder)

```c
/* CANDIDATE BASELINE (score 27, PASS-vetted): */
return ((u32)b1 << 0xF) | ((u32)r << 0xA) | 0xE2000000u | ((u32)b2 << 5) | (u32)g;

/* THE CHEAT (score 26, REJECTED): */
return ((u32)b1 << 0xF) | ((u32)b2 << 5) | ((u32)r << 0xA) | (u32)g | 0xE2000000u;
```

Same family: the four channel operands and the GP0 command byte are
permuted; baseline ordering follows the bit-position descending order
(`b1@15 r@10 b2@5 g@0`) which is the natural human-readable form, the
cheat ordering scrambles them to a non-semantic sequence.

## The 6-test failure mode

| Test | Result |
|---|---|
| 1. Semantic purpose | **FAIL** — OR is associative + commutative; any operand order produces identical runtime value. No semantic content in the shuffle. |
| 2. Human-programmer | **FAIL** — natural orderings are by bit-position (descending or ascending) or by source-offset (struct field order). Neither cheat ordering matches. |
| 3. GCC-internals justification | **FAIL** — when asked WHY the cheat ordering works, the only honest answer is "it shifts cc1's RTL OR-tree shape so the scheduler picks a different emission order." That's the cheat signal. |
| 4. Permuter-derived | **FAIL** — both instances were FOUND by directed permuter. The permuter's `PERM_GENERAL{alt1, alt2, alt3}` over operand permutations specifically enumerates this space. |
| 5. Family check | **FAIL** — same intent as the forbidden DImode chain ([[global-label-drift-sibling-cheat]]) and the combine-foldable chain-extender ([[register-alloc-pure-c]] §6 forbidden marker): shift RTL pseudo / tree structure to change scheduling without changing emitted semantics. |
| 6. Naming-announces-intent | N/A — the operands keep their original names; the cheat is in the order, not the spelling. |

## Why the catalog needs THIS family explicitly

The prior catalog covered:
- Chain-extender (route a value through an extra arithmetic step to bump
  `reg_n_refs` — `register-alloc-pure-c.md` §6)
- Dead-store / dead-conditional-store (write a value that DCE removes
  but pre-DCE analysis sees — Lever D)
- DImode chain (round-trip through `unsigned long long` to alter pseudo
  numbering — `global-label-drift-sibling-cheat.md`)
- Named-intermediate declaration order (LUID biasing via decl order —
  ALLOWED per `narrow-byte-args-packed-call.md` because it has semantic
  purpose when the names map to GP0 fields)

The OR-tree-shape-shift is a DISTINCT mechanism: it doesn't add arithmetic,
doesn't add dead code, doesn't round-trip through a wider type, and
doesn't add a declaration. It just shuffles operands WITHIN an existing
expression. The cheat-reviewer should recognize it specifically.

## How to recognize the shape

Hand-vet checklist for any worker about to commit a multi-operand OR
or AND expression:

1. **Is the expression associative + commutative?** OR (`|`) and AND
   (`&`) and XOR (`^`) are. Subtraction and shift are not. ADD (`+`) is
   formally associative but in practice GCC reorders less often for ADD.
2. **Are the operands in a non-natural order?** Natural orderings:
   - bit-position descending (`b1@15 | r@10 | b2@5 | g@0`)
   - bit-position ascending (reverse)
   - source-offset of contributing field (struct-order or array-order)
   - alphabetical / numerical by name (rare but acceptable)
3. **Could you justify the chosen order without referencing cc1?** If
   the only honest answer is "the permuter found this ordering scores
   best" or "this matches target's RTL tree shape" — it's a cheat.

If unsure, run the cheat-reviewer agent on the form. The reviewer
specifically tests for this family now.

## What IS allowed

A worker may freely choose ANY of the natural orderings (1) listed
above. A worker may NOT enumerate orderings looking for the
score-minimum.

When the permuter returns a sub-baseline whose only difference from the
baseline is operand permutation in an associative+commutative
expression, the worker MUST recognize it as this cheat family and
reject without surfacing.

## Confirmed cases

- `func_8007CBB0` round-14 (commit `d4872471`, 2026-06-06) — score 17
  form REJECTED, preserved at
  `memory/wip/func_8007CBB0/rejected/round14-or-operand-reorder.c`.
- `func_8007C97C` round-14 (commit `e621af5a`, 2026-06-06) — score 26
  form REJECTED, preserved at
  `memory/wip/func_8007C97C/rejected/round14_or_reorder_decl_reorder.c`.

Both functions remain INCOMPLETE at their honest pure-C floors
(CBB0=41, C97C=27).

## Related

- [[no-new-park-categories]] — "cheats by any spelling" parent policy
  that this rule extends.
- [[register-alloc-pure-c]] §6 — forbidden chain-extender family
  (different mechanism, same intent: shift RTL state to alter
  allocation/scheduling without semantic content).
- [[global-label-drift-sibling-cheat]] § DImode chain — sibling
  forbidden technique with same "shift pseudo structure to bend
  scheduling" intent.
- [[review-discipline-before-commit]] — the cheat-reviewer architecture
  that now catches this family.
- [[narrow-byte-args-packed-call]] hi/lo split — the LEGITIMATE
  cousin: named-intermediate declarations whose order maps to GP0
  field positions have semantic content (hi vs lo), and are sanctioned.
  This rule does NOT forbid that — it forbids the order shuffle
  within a single OR expression where the operands have no
  declaration to anchor to.
