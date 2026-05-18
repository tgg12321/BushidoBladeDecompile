---
name: strength-reduce-defeat
paths: ["src/*.c"]
description: "When `if (t<0) -((-t)<<N); else t<<N` collapses to a single `sll` because GCC's optimizer sees both branches as mathematically equivalent. The fix is to wrap the negate operations in single-instruction `negu` inline asm so the branches stay opaque to GCC's branch-folding pass. Plus the source-order trick that lands the negate in the b's delay slot naturally."
metadata:
  type: reference
---

## Symptom

Target has the explicit sign-preserving multiply pattern:

```
bgez $t3, .L_pos
 nop                  (delay)
negu $t3, $t3        (abs)
sll  $t3, $t3, 3
b    .L_done
 negu $t3, $t3       (delay slot — back to negative)
.L_pos:
sll  $t3, $t3, 3
.L_done:
```

The natural-C decompilation is:

```c
if (t3 < 0) {
    t3 = -((-t3) << 3);
} else {
    t3 = t3 << 3;
}
```

But GCC emits a single `sll t3, t3, 3` for both branches, because it sees the negative branch's `-((-t)<<N) ≡ t<<N` mathematically (signed shift IS sign-preserving for arithmetic, modulo overflow at bit 31). The `if-else` is folded away.

## Recipe

Wrap the two negate operations in single-instruction `negu` inline asm. The volatile asm is opaque to GCC's optimizer, so it can't see that the negative branch is mathematically equivalent.

```c
if (t3 < 0) {
    __asm__ volatile ("negu %0, %1" : "=r"(t3) : "r"(t3));
    t3 = t3 << 3;
    __asm__ volatile ("negu %0, %1" : "=r"(t3) : "r"(t3));
} else {
    t3 = t3 << 3;
}
```

(Don't use `+r` constraint — GCC 2.7.2 doesn't support it. Use `=r/r` with the same variable for input and output, then assign back.)

This emits the canonical pattern after one more pass to land the second `negu` in the `b`'s delay slot — see "Source order trick" below.

## Source order trick: land the trailing negate in the delay slot

Target's pattern has the second negu IN the b's delay slot. To get this with natural-C source, place the second negate operation LAST in source order so `as` (with `.set reorder`) auto-fills the delay slot from the immediately preceding instruction.

When the negate is part of a multi-step sequence (negate → operate → negate), put the FINAL negate as the absolute last operation before the implicit branch-to-join. Example from the pre-mvmva sign-split in `func_8007EA0C`:

```c
/* WRONG order — negu_t3 emits before andi_t0, missing the delay-slot fill: */
if (t0 < 0) {
    s32 a = -t0;
    t3 = -(a >> 15);     /* compiles: sra+negu(t3) */
    t0 = -(a & 0x7FFF);  /* compiles: andi+negu(t0) */
}
/* RIGHT order — andi emits before negate, so the trailing negu_t0
 * lands in the b's delay slot: */
if (t0 < 0) {
    s32 a = -t0;
    t3 = a >> 15;        /* sra */
    a = a & 0x7FFF;      /* andi */
    t3 = -t3;            /* negu(t3) */
    t0 = -a;             /* negu(t0) — lands in delay slot */
}
```

The principle: **when the implicit branch-to-join needs a useful instruction in its delay slot, place that instruction LAST in source order**.

## When you still need regfix to finish

Even with the C tricks above, you'll typically need:

1. **subst `j .L<N>` → `b .L<N>`** — GCC may emit J-format jumps where target uses BEQ-format `b`. They differ in opcode (J=0x02 vs BEQ=0x04). Subst rule: `subst "j\\s+\\.L<N>" "b\t.L<N>" @ <maspsx_idx>`.

2. **`fill_delay @ j_idx <- source_idx`** — to move a useful instruction into the j's delay slot when GCC emitted `[insn, j, nop]` instead of `[j, insn-in-delay]`.

3. **`insert_after "sra $tN, $tN, 15" @ <delay_slot_idx>`** — to add the dead-sra GCC's branch-likely scheduler emits between the b's delay slot and the next label. See `recipes/dead-branch-scheduling.md`.

## Reference matches

- `func_8007EA0C` (commit `76230dd`, 2026-05-12): 13 regfix rules total — 6 j→b substs + 4 fill_delay + 3 insert_after for the dead sra. Pre-mvmva sign-split AND post-mvmva sll-by-8 both use the negu-asm pattern.

## When this applies

Any function where GCC strength-reduces a sign-preserving signed shift, multiply by power of 2, or division. Symptom: target has `bgez+negu+<op>+b+negu` pattern, mine has just `<op>`. The optimizer is folding because both branches produce mathematically equivalent results.
