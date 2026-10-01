---
name: compare-operand-order-register
description: "A local/global register pair swapped across a compare block: write `local > GLOBAL` instead of `GLOBAL < local` — operand order sets RTL emission order and flips which register the local gets."
paths: [".claude/rules/compare-operand-order-register.md"]
metadata:
  type: reference
---

# Reversing a comparison's operand order can flip which register a value gets

## Symptom

A check block of comparisons like

```c
w = rect[2];
if (D_8009BE78 < w) goto bad;     /* GLOBAL < local */
```

where your build puts `w` in `$a0` and the global in `$a1`, but the target has `w` in `$a1` and the
global in `$v1` — a consistent `$X <-> $Y` register swap over the block.

## The lever — write `local OP global`

Flip the operands (and the operator) so the local is the left-hand operand:

```c
if (w > D_8009BE78) goto bad;     /* same semantics */
```

cc1 materializes the comparison operands in a different order for `<` vs `>`
(`mips_emit_compare` paths), so pseudo-creation order changes and the allocator's ascending choice
follows. It is an RTL-emission-order lever, not a scheduling one, so it survives `-O2`.

Example: func_8007B3A8 (display.c) — flipping four `GLOBAL < local` compares took the honest
distance 15 → 5.

## Does NOT apply when

- target's `slt` operand order shows the original was the other way (check target.s);
- the local is a loop counter/accumulator (may change loop form — [[loop-rotation-two-shift]]);
- the compare is against `0` (both forms emit `bltz`).

Related: [[register-alloc-pure-c]] · [[store-before-jal]]
