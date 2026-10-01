---
name: fake-varargs-explicit-homing
description: "printf-style wrapper whose target homes arg regs as body-scheduled stores (interleaved/delay slot) was NOT `...`: write 4 named args + explicit stores through the va pointer. True `...` emits bulk pre-subu homes."
paths: [".claude/rules/fake-varargs-explicit-homing.md"]
metadata:
  type: reference
---

# Fake-varargs wrappers: explicit arg homing, not `...`

## Symptom

A small printf/debug-print wrapper whose diff clusters on the argument-register home stores. With a
`...` signature your build emits a **bulk pre-subu block** (`sw $5,4($sp); sw $6,8($sp);
sw $7,12($sp)` before `addiu $sp,-frame`); the target has them **post-subu, scheduled into the
body** — interleaved with call setup, maybe in the `jal` delay slot, the first named arg homed from
a copy register. This is NOT a compiler divergence; it proves the original was not variadic.

## Why

- True-variadic functions in the target binary (e.g. DispSleepMenuTex) show the same bulk pre-subu
  homing our cc1 emits for `...`.
- Pretend-args homing is prologue text and cannot schedule; RTL-scheduled homes are named-param
  homing (an addressable named param) or explicit C stores.
- GCC 2.7.2 homes ONLY the addressable named param; extra homed registers mean the original stored
  them explicitly.

## The matching shape (the pre-stdarg idiom)

```c
void debug_printf(s32 fmt, s32 a, s32 b, s32 c) {
    s32 *ap = &fmt;          /* &fmt -> compiler homes fmt */
    ap[1] = a;               /* explicit homes: RTL stores, schedulable */
    ap[2] = b;
    ap[3] = c;
    func_80079244(1, fmt, ap + 1);   /* callee walks the homed block */
}
```

Every store is live (the callee reads the block), so this is program logic. FORBIDDEN sibling:
`(fmt,a,b,c)` with b/c **unused**, declared only to suppress the varargs prologue (dead-param
family).

## How to apply

1. Confirm the target's homes are body-scheduled; if the target shows the bulk pre-subu block, keep
   `...`.
2. Write the named-arg + explicit-store form; sweep spellings (`ap+1` vs `&a`, K&R) and prefer the
   most natural that matches.

Related: [[narrow-stack-param-subword-offset]] · [[no-compiler-divergence]]
