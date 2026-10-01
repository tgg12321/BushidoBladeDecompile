---
name: or-tree-shape-shift
paths: [".claude/rules/or-tree-shape-shift.md"]
description: "FORBIDDEN: enumerating operand orders/groupings of an associative+commutative expression to shift cc1's OR-tree shape. Narrow carve-out (2026-08-20): ONE mechanism-derived, FAKE-annotated order."
metadata:
  type: rule
---

# Operand-order / parenthesization shuffles in associative+commutative expressions

## FORBIDDEN (2026-06-06)

Reordering operands or reparenthesizing an associative+commutative expression (`|`, `&`, `^`, `+`) solely to
change cc1's RTL tree shape — and thereby sched.c's INSN_PRIORITY emission order — is a cheat-by-spelling.
The runtime value is identical; only the tree topology changes. Example (func_8007CBB0, REJECTED):

```c
/* natural (PASS): */ x = (*D_8009BF48 & 0x7FF) | (((arg1 >> 31) << 10) | 0xE1000000);
/* cheat:          */ x = ((arg1 >> 31) << 10) | (*D_8009BF48 & 0x7FF) | 0xE1000000;  /* found by enumeration */
```

**Natural orderings are always free:** bit-position descending or ascending, source/struct field order, or
name order. **Enumerating orderings for the score-minimum is forbidden** as a derivation procedure. A permuter
sub-baseline whose only change is an operand permutation in such an expression must be recognized as this
family and rejected without surfacing.

Recognition: (1) the expression is associative+commutative; (2) the order is non-natural; (3) the only
justification is "the permuter scored it best" / "matches target's RTL tree".

## Narrow carve-out (owner ruling 2026-08-20)

A SINGLE committed operand order or parenthesization in an associative+commutative expression (`|`, `&`, `^`,
`+`), chosen to match target, is sanctioned when ALL hold:

1. **Honest orderings measured dead first** — documented lever exhaustion in the ledger (structural axes,
   natural orderings, other sanctioned levers).
2. **Named, dump-proven GCC-pass mechanism** for why the order matters (e.g. sched1 INSN_PRIORITY chain-length
   ties). "The permuter scored it best" is not a mechanism; a permuter find may corroborate a mechanism-derived
   order, never substitute for one.
3. **Mandatory annotation at the site**, e.g. `/* FAKE: operand order chosen to match target; natural orders
   measured dead (ledger) */`.
4. **Identical runtime semantics** (associative+commutative only) and `build_insns == target_insns`.
5. **Layer-1 + layer-2 adversarial review.**

SOTN precedent: `src/st/no3/e_warg.c:433-445` (annotated trial-found reordering), `src/dra/7E4BC.c:1561-1570`.
The 2026-06-06 func_8007CBB0 / func_8007C97C enumeration-derived rejections stand.
Record: `docs/grind/operand-order-survey-2026-08-20.md`.

## Related

[[no-new-park-categories]] · [[register-alloc-pure-c]] (forbidden chain-extender, same intent) ·
[[narrow-byte-args-packed-call]] (the legitimate named-intermediate `hi`/`lo` cousin)
