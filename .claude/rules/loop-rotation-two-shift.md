---
name: loop-rotation-two-shift
paths: [".claude/rules/loop-rotation-two-shift.md"]
description: "Bit-search loop where target has the shift twice (peeled before the loop + recomputed in the bottom delay slot): write a natural for-loop and let GCC rotate it; use an opaque `one` var to keep sllv+and."
metadata:
  type: recipe
---

# Two `sllv` from loop rotation

## Symptom

A bit-search / shift loop whose target has the shift **twice** — once before the loop (initial mask) and once
in the loop's bottom delay slot, with the back-edge targeting the `and`, not the first `sllv`:

```mips
sllv  v0,a2,v1        # mask = one << i   (initial, i=0)
and   v0,a0,v0        # loop top
bnez  v0, found
addiu v1,v1,1
slti  v0,v1,24
bnez  v0, <loop top>
sllv  v0,a2,v1        # delay slot: recompute for next iter
```

This is GCC **loop rotation**. You cannot hand-write it: an explicit initial `mask = one << i` before the loop
is constant-folded (one instruction short), and an explicit-goto loop keeps a single `sllv`. Never inject the
initial `sllv` as inline asm (cheat, [[inline-asm-policy]]).

## The fix — a natural `for` loop

```c
bit_found = -1;
i = 0;
one = 1;                       /* opaque var, NOT literal 1 */
for (; i < 0x18; i++) {
    mask = one << i;
    if (arg0 & mask) { bit_found = i; break; }
}
```

GCC rotates it and the peeled shift is not folded (rotation runs after the fold point).

## Companion levers

1. **Opaque `one`.** `arg0 & (1 << i)` is rewritten to `(arg0 >> i) & 1` (`srav; andi`). `s32 one = 1;` keeps
   the `sllv`+`and` mask form. (Frozen-list "opaque arithmetic variables" entry; NOT valid as a dummy array
   subscript or pointer offset — owner ruling Q22, [[named-local-fake-exception]].)
2. **Statement order / variable reuse for the post-loop block.** Branch sense and register choice follow source
   structure: restoring the original explicit-goto block order and reusing the same `mask` variable for the
   later load closed func_8008ACD0's last 6 (a separate temp landed in the wrong register).

## Related

[[register-alloc-pure-c]] · [[switch-vs-ifchain-branch-sense]] · [[store-before-jal]]
