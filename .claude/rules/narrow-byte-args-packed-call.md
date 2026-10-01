---
name: narrow-byte-args-packed-call
paths: [".claude/rules/narrow-byte-args-packed-call.md"]
description: "Call wrapper packing 3 byte-masked s32 args (`arg & 0xFF`) into one slot: declare those params `u8` and drop the masks; for a const-ORed pack, split into named `hi`/`lo` intermediates, `hi` first."
metadata:
  type: reference
---

# Narrow `u8` parameters for byte-packed-arg call wrappers

## Symptom

A GPU/SPU command wrapper that takes 4+ `s32` args, 3 of which are only used as `arg & 0xFF` and packed into
one 32-bit slot for a downstream (often function-pointer) call:
`((a3 & 0xFF) << 16) | ((a2 & 0xFF) << 8) | (a1 & 0xFF)`. The honest build shows the args in natural
ascending callee-saves while the target has them reversed (`arg0→$s3 … arg3→$s0`), the packing accumulator
lands in `$a3` instead of `$s0`, and prologue scheduling diverges.

## The fix — declare the byte-packed params `u8`, drop the masks

```c
void func(s32 arg0, u8 arg1, u8 arg2, u8 arg3) {
    func_helper(&str, arg0);
    fn(p[3], arg0, 8, ((u32)arg3 << 16) | ((u32)arg2 << 8) | (u32)arg1);
}
```

With `u8` params, `PROMOTE_ARGS` treats the args as already narrowed at entry: no `andi`, the `(u32)` widens
are no-ops, and RA/scheduling fall out as the target (func_8007B4D0: 7 → 0 first try). The original C almost
certainly declared them `u8`; m2c's `s32 + (x & 0xFF)` is semantically right but byte-noisy. Callers and
externs keep working (ABI promotion); updating externs is optional.

Does NOT apply if any packed arg has a non-byte use elsewhere (`arg3 + 1`, `arg3 == 5`) — then the mask is
load-bearing; fall back to [[register-alloc-pure-c]].

## Const-OR variant — named `hi`/`lo` intermediates (the named-intermediate sub-trick)

When a constant is ORed into the pack (e.g. GPU set bit `| 0x80000000`), arg2's chain is one hop longer, so
its INSN_PRIORITY beats arg3's and sched picks it first regardless of LUID. Split into two named
intermediates with `hi` FIRST so arg3's shift gets the lower LUID while arg2's chain keeps its length:

```c
hi = (u32)arg3 << 16;                 /* expanded first -> low LUID */
lo = ((u32)arg2 << 8) | 0x80000000;
fn(p[3], arg0, 8, (hi | lo) | (u32)arg1);
```

(func_8007B564: 15 → 0. One inline expression: distance 6; one local: 4.) This is the frozen-list
"named-intermediate declaration order" entry ([[no-new-park-categories]] § SOTN-accepted): each named
intermediate is once-written, holds a real consumed value, is byte-neutral, and the entry's prerequisites
(dump-proven named mechanism, documented lever exhaustion, `/* FAKE: ... */` annotation, layer-1 + layer-2
review) apply in full.

## Related

[[register-alloc-pure-c]] (Lever B, narrow integer type) · [[narrow-stack-param-subword-offset]] ·
[[u16-global-lhu-lbu-low-byte]] · [[inline-asm-policy]] (pins are cheats)
