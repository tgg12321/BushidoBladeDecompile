---
name: dead-branch-scheduling
paths: ["regfix.txt", "src/*.c"]
description: "When target's asm has an \"unreachable\" instruction between an unconditional `b L`'s delay slot and the next label `.L_pos`, that's a GCC branch-likely scheduling artifact — emit it via regfix `insert_after` because no natural-C source produces it."
metadata:
  type: recipe
---

## Symptom

Target asm has an instruction in dead position:

```
b .L_done
 negu $t0, $t0     (delay slot — runs before the branch)
sra  $t3, $t0, 15  (DEAD — b is unconditional, never falls through)
.L_pos:
andi $t0, $t0, 0x7FFF
.L_done:
```

The `sra` between the b's delay slot and `.L_pos` is unreachable: control transfers from the delay slot directly to `.L_done` (skipping over `.L_pos`). The bgez branch jumps to `.L_pos` so it never falls through to the dead `sra` either.

But GCC emits it anyway as part of its branch-likely instruction-scheduling pass: the negative branch's "wrap-up" code is templated to include the same operations as the positive branch's prelude, even though one of them ends up dead after the b.

## Recipe

**No natural-C source emits this.** The dead instruction is a GCC scheduler artifact. The fix is a regfix `insert_after` rule that places the dead instruction AFTER the b's delay slot and BEFORE the `.L_pos` label.

For each component (e.g., t0/t3, t1/t4, t2/t5 in `func_8007EA0C`):

```
func: insert_after "sra\t$<t3-style-reg>,$<t0-style-reg>,15" @ <maspsx_idx_of_b_delay_slot>
```

Where `<maspsx_idx_of_b_delay_slot>` is the maspsx-output index of the `negu` (or whatever) in the b's delay slot. The insert places the dead instruction at idx+1, before the next label.

## Reference matches

- `func_8007EA0C` (commit `76230dd`, 2026-05-12): 3 `insert_after` rules — one per t0/t3, t1/t4, t2/t5 sign-split block. Each adds `sra $tN, $tM, 15` between the b's delay slot (`negu $tM, $tM`) and the `.L_pos` label.

## When this applies

GCC's branch-likely scheduler emits the dead instruction when:
- The branch is unconditional (`b L` = `beq $0, $0, L`)
- The negative branch ends with the b
- The positive branch (after `.L_pos`) starts with an instruction whose result was already computed in the negative branch's prelude (e.g., both branches start with `sra $tN, ...`)

In these cases the scheduler emits the prelude-equivalent instruction at the end of the negative branch, even though it's dead.

If you see target's asm with an instruction in dead position (unreachable), don't try to fix it via C — go straight to `insert_after`.

## How to spot it

Look at target.s for two patterns:

1. An `b .L<X>` followed by exactly one delay-slot instruction, followed by an instruction that does NOT have a label, followed by `.L_pos:`. The middle instruction is the dead one.

2. The dead instruction is structurally similar to what you'd compute in the positive branch's first instruction — typically a `sra` with the same operands as a sra earlier in the negative branch.

If both hold, write the `insert_after` rule.
