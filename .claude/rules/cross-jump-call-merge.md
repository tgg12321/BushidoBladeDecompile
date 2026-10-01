---
name: cross-jump-call-merge
paths: [".claude/rules/cross-jump-call-merge.md"]
description: "Target has more jalr call sites than your build: jump2 cross-jump merged calls with equal CALL_INSN_FUNCTION_USAGE. Give each fn-ptr its REAL arg COUNT (count splits calls; arg modes do not)."
metadata:
  type: reference
---

# The cross-jump call-merge wall — arg COUNT is the lever

## Symptom

A dispatch/handler function (status-byte interpreter, multi-case switch calling function pointers)
that plateaus at a high diff although the body is semantically right. The tell: **target.s has
more `jalr`/`jal` call sites than your build** — distinct handler calls collapsed into one.

```
mipsel-linux-gnu-objdump -d <yourbuild>.o | grep -wc jalr   # vs grep -c jalr asm/funcs/<func>.s
```

If build < target, register/scheduling work will not close it.

## Why

`jump_optimize(insns, 1, 1, 0)` (toplev.c:3142, cross_jump=1, no disable flag in 2.7.2) →
`find_cross_jump` (jump.c:2371) merges blocks jumping to the same label that share a suffix ≥ 2
insns. Two CALL_INSNs match only if their `CALL_INSN_FUNCTION_USAGE` (the `(use (reg argN))` list)
is `rtx_equal` (jump.c:2426). Argument MODES are promoted to a uniform register mode and do NOT
differentiate calls; argument COUNT changes the used-register set and DOES. Calls that end blocks
differently (jump vs fall-through to epilogue) also stay split.

## The fix

1. Run the jalr count. If build < target, stop chasing register diffs.
2. For each handler call, count the argument registers (`$a0`-`$a3`) actually set/live into that
   call in the TARGET — that is the real arg count. m2c routinely over-infers a trailing arg; trust
   the asm, never blind diff-minimising.
3. Declare each fn-ptr with its real count and call it with exactly that many args.
4. Verify by jalr count; the residual is then ordinary ordering/register work.

Example: saTan0Main (main.c) — handlers take 4 (0x90), 2 (0xE0) and 3 (B0/C0/F0/FF, correctly
merged) args; declaring those counts gave target's 3 jalr where every uniform-4-arg form gave 1.
The permuter cannot find this (it mutates bodies, not declarations).

Does NOT defeat the merge: distinct arg types, any `-O`/scheduling flag
([[compiler-flags-canonical]]), register pins, the permuter from a 1-jalr seed.

Related: [[cross-jump-store-tail-merge]] (store-suffix analogue) · [[shared-end-label]] ·
[[store-before-jal]]
