---
name: switch-vs-ifchain-branch-sense
paths: [".claude/rules/switch-vs-ifchain-branch-sense.md"]
description: "One case of a multi-way dispatch has inverted branch sense (bne vs target beq + swapped j targets): the body was hand-decompiled as an if-goto chain — rewrite it as a real `switch`."
metadata:
  type: recipe
---

# Inverted branch sense on a dispatch → rewrite the if-goto chain as a real `switch`

## Symptom

`diagnose` says CONTROL-FLOW with a tiny distance; the only real diff is one case's polarity: target
`beq $v1,$v0,<set_block>` (value `li` in the delay slot, fall-through `j <default>`), ours
`bne $v1,$v0,<default>`. The body is an if-goto chain mimicking GCC's switch decision tree (explicit
`if (x < 5)` range split, per-case `if (x == K) { ...; goto set_val; } goto default;`).

## Cause

GCC compiles each if-block as test / skip-if-false / block / jump, so the case nearest `default` gets the
inverted sense and isn't cross-jumped into the shared merge label. Real `switch` codegen emits a balanced tree
with positive `beq` tests and cross-jumps the sibling case bodies — the target's shape.

## Fix

```c
switch (D_800A38DC) {
case 4:  var_v0 = 0xC; break;
case 1:
    if (D_800A3748 == 0) { func_8001DA2C(); D_800A3768 = 2; mottest_disp(); return; }
    var_v0 = 0xC; break;
case 6:  var_v0 = 0xC; break;
default: func_8001DA2C(); var_v0 = 2; break;
}
D_800A3834 = var_v0;   /* was `set_val:` */
```

The merge label becomes the post-switch statement; each `goto set_val` becomes `break`. func_8001EFA0: 1 → 0.

Not this rule: a register-rename diff ([[register-alloc-pure-c]]); a real jump table (`lw/addu/jr`);
function-pointer calls that over/under-merge ([[cross-jump-call-merge]]).

## Related

[[shared-end-label]] (the inverse) · [[cross-jump-call-merge]] · [[switch-break-shared-return-sched-hoist]]
