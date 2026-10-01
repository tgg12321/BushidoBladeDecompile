---
name: hoist-call-arg-local-flips-jal-delay
description: "Pre-call store not in the jal delay slot, two load-delay nops, late arg-setup lw: hoist the call's global-loaded arg into a local declared FIRST in a block around the last store + call."
paths: [".claude/rules/hoist-call-arg-local-flips-jal-delay.md"]
metadata:
  type: reference
---

# Hoist a call's late-loaded arg into a local declared FIRST in a block

## Symptom

Small distance (4-6) whose only cluster is the last pre-call store and the jal delay slot.

Target:
```mips
lw   $v0, off($v1)
lw   $aN, %gp_rel(GLOBAL)($gp)   # arg setup interleaved
lw   $v0, off($v0)
jal  callee
sw   $v0, off($a1)               # pre-call store in the delay slot
```

Yours: two load-delay `nop`s, the `sw` before the call, the arg `lw` last, and a `nop` delay slot
(reorg picked the post-call `lw $v0,GLOBAL`, which is unusable).

## Cause

The arg-setup pseudo is born late (high LUID at the call expression), so it schedules after the
store; reorg's preferred fill is invalid and the slot stays empty.

## Fix

```c
{
    s32 last_arg = GLOBAL;                                    /* declared first: low LUID */
    *(s32 *)(dst + 8) = *(s32 *)(*(s32 *)(outer + 8) + 8);   /* the store */
    some_call(dst_a, dst, last_arg);
}
```

The arg `lw` now schedules early (filling a load-delay slot) and the pre-call `sw` becomes the
delay-slot fill: three fewer insns. This is the frozen-list "named-intermediate declaration order"
family ([[no-new-park-categories]], [[narrow-byte-args-packed-call]]): `last_arg` is the value
actually passed.

Example: func_80060B70 (text1b.c) — 5 → 0.

## Does NOT apply when

- the last arg is computed locally (already low LUID);
- the pre-call store reads a register the call USES as input (e.g. `$a0`);
- the post-call first insn is itself a valid fill;
- the diff is a different scheduling pattern.

Related: [[store-before-jal]] · [[call-return-if-result-reuse-v0]] ·
[[defer-store-past-later-compute-into-jal-delay]]
