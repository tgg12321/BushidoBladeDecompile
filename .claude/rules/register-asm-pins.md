---
name: register-asm-pins
paths: [".claude/rules/register-asm-pins.md"]
description: "FORBIDDEN in committed code: `register T x asm(\"$N\")` pins are DIAGNOSTIC-ONLY hints. Use one to learn the target register, then strip it and find the C structure that makes GCC choose it."
metadata:
  type: reference
  status: forbidden
---

# Register-asm pins — diagnostic only

A `register T x asm("$N")` pin left in committed source is cheat-asm ([[inline-asm-policy]]). Use a pin only
to *learn* which register the target wants; the finished match must reach it through C structure, pin removed.

**Pins are hints, not bindings.** GCC 2.7.2's allocator overrides a pin when its own preference is strong
(e.g. `register s32 neg_threshold asm("t1") = -threshold;` landed in `$a0` because `$a0` was long dead). A pin
holds reliably only when it agrees with what the RA would pick, or when the variable is also an `__asm__`
`"=r"`/`"r"` operand. Two pins on one register with overlapping liveness → unpredictable.

How to apply:
1. Build and check `mipsel-linux-gnu-objdump -d build/src/<file>.o` to see whether the pin took.
2. An ignored pin means the C structure is wrong. Never add more pins, an `__asm__` move, or any other
   coercion ([[inline-asm-policy]]).
3. Find the structure that makes GCC choose the register naturally — declaration order, intermediates,
   liveness ([[register-alloc-pure-c]]).
4. Strip every pin before committing. The sandbox strips pins before scoring, so they cannot help anyway.
